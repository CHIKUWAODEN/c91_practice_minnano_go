= Makefileの追加

//read{
大本となるファイルを再配置しました。
引き続き、開発を進める上で重要な土台を作るべく、makeをタスクランナーとして使う形にしてみます。
これにより、go lintやgo vet、goimportsなどといったコードの品質を担保する仕組みを、エディタなどの設定によらない形で利用する仕組みができますので、それに基づいた既存のコードの修正などについても触れてみます。
//}


== 本章の目的

本章では、「1.3 Goを始める start a tour of Go - タスクランナーとしてMakefileを使う」
という節の内容を踏まえ、makeをタスクランナーとして利用するための準備を行います。
みんなのGo言語においては、次のように言われています。


//quote{
などのドキュメントを充実させることも大事ですが、「実行可能なドキュメント」のようなかたちでタスクランナーを充実させ ることはプロジェクトを分かりやすくするために重要です。
//}


fodはソースコードが公開されているとはいえ、個人的なごぐごく小さいプロダクトにすぎません。
ビルドも簡単で、Go言語のチュートリアルで紹介されるレベルの操作しか行いません。
わかりやすくする必要もさほどなく、Makefileを追加するほどのレベルではないでしょう。

それでも、もし自分以外のだれかがこれを利用しようとした場合であれば、はなしは変わってきます。
Makefileが用意されている方が有利であることは間違いないと思います。


== テンプレートに則ったMakefile

まずはみんなのGo言語で紹介されているテンプレートに沿った形のMakefileを用意してみます。
また、それに合わせて必要なプログラムコード本体の改修も行っていきます。


//list[makefile-template][Makfileのテンプレート]{
# メタ情報
NAME     := myproj
VERSION  := $(shell git describe --tags --abbrev=0)
REVISION := $(shell git rev-parse --short HEAD)
LDFLAGS  := -X 'main.version=$(VERSION)' \
            -X 'main.revision=$(REVISION)'

# 必要なツール類をセットアップする
## Setup
setup:
    go get github.com/Masterminds/glide
    go get github.com/golang/lint/golint
    go get golang.org/x/tools/cmd/goimports
    go get github.com/Songmu/make2help/cmd/make2help
  
# テストを実行する
## Run tests
test: deps
    go test $$(glide novendor)
  
# glideを使って依存パッケージをインストールする  
## Install dependencies
deps: setup
    glide Install
   
## Update dependencies
update: setup
    glide update


## Lint
lint: setup
    go vet $$(glide novendor)
    for pkg in $$(glide novendor -x); do \
      golint -set_exit_status $$pkg || exit $$?; \
    done


## Format source codes
fmt: setup
    goimports -w $$(glide nv -x)
  

## build binaries ex. make bin/myproj
bin/%: cmd/%/main.go deps
    go build -ldflags "$(LDFLAGS)" -o $@ $<
  
## Show help
help:
    @make2help $(MAKEFILE_LIST)
    
.PHONY: setup deps update test lint help
//}


== バイナリのプログラムのファイル名修正

さて、このテンプレートとなるMakefileにあるビルド用のターゲット@<code>{bin/%}は、バイナリプログラムのファイル名がmain.goであることと決められています。
@<chapref>{1_lib-and-bin}では@<code>{cmd/fod/fod.go}として、バイナリプログラムのコードを配置しましたが、これはMakefileの定めるところに沿ってはいません。
そこで、元々のファイル名を修正することで対応します。


//emlist[]{
$ git mv cmd/fod/fod.go cmd/fod/main.go
//}


== gomとglide

fodはもともとベンダリングツールとしてmattn/gomを利用してきました。@<fn>{mattn-gom}
しかし、@<list>{makefile-template}で示されたMakefileは、ベンダリングツールとしてMasterminds/glideを利用することを前提としています。@<fn>{mastermind-glide}


//footnote[mattn-gom][https://github.com/mattn/gom]
//footnote[mastermind-glide][https://github.com/Masterminds/glide]


現状ではgomを使っていましたが、@<code>{glide novendor}に相当するコマンドがgomにはないため、Makefileの中でglideを使っている箇所をそのままgomに置き換えることができません。
というわけで、プロジェクトをglideに対応してみようと思います。

まず、gomのパッケージ管理ファイルである既存の@<code>{Gomfile}の中身を確認しておきます。
catコマンドで表示してみたところ、@<list>{gomfile}ようなシンプルな内容となっていました。


//list[gomfile][Gomfileの内容]{
gom 'github.com/k0kubun/pp'
gom 'github.com/nsf/termbox-go'
gom 'github.com/mitchellh/panicwrap'
//}


この@<code>{Gomfile}は削除してしまいます。
かわりにglideのパッケージ管理ファイルを生成します。


//emlist[]{
$ rm Gomfile
$ glide init
[INFO] Initialized. You can now edit 'glide.yaml'
//}


@<code>{glide init}によって、glideのパッケージ管理ファイルである@<code>{glide.yaml}というファイルが生成されます。
このファイルの中身は@<list>{glide-yaml-template}ようになりました。


//list[glide-yaml-template][init直後のglide.yamlファイル]{
# Glide YAML configuration file
# Set this to your fully qualified package name, e.g.
# github.com/Masterminds/foo. This should be the
# top level package.
package: main

# Declare your project's dependencies.
import:
  # Fetch package similar to 'go get':
  #- package: github.com/Masterminds/cookoo
  # Get and manage a package with Git:
  #- package: github.com/Masterminds/cookoo
  #  # The repository URL
  #  repo: git@github.com:Masterminds/cookoo.git
  #  # A tag, branch, or SHA
  #  ref: 1.1.0
  #  # the VCS type (compare to bzr, hg, svn). You should
  #  # set this if you know it.
  #  vcs: git
//}


ご覧のとおりですが、依存パッケージが何も記述されていない、いわばテンプレートとなっています。
ここに、依存パッケージを手動で追加していくことにします。
現状では次のようになります。


//list[glide-yaml-add-package][パッケージ定義を追加したglide.yaml]{
...
# Declare your project's dependencies.
import:
  ...
  - package: github.com/k0kubun/pp
  - package: github.com/nsf/termbox-go
  - package: github.com/mitchellh/panicwrap
//}


Makefileに定義されているターゲット@<code>{deps}利用します。
このターゲットは内部で、glideのパッケージインストールの実行コマンドである@<code>{glide install}を実行します。
実行した結果は次のようになります。


//emlist[]{
$ make deps
go get github.com/Masterminds/glide
go get github.com/golang/lint/golint
go get golang.org/x/tools/cmd/goimports
go get github.com/Songmu/make2help/cmd/make2help
glide install
[INFO] Fetching updates for github.com/k0kubun/pp.
[INFO] Fetching updates for github.com/nsf/termbox-go.
[INFO] Fetching updates for github.com/mitchellh/panicwrap.
[INFO] Package github.com/k0kubun/pp manages its own dependencies
[INFO] Package github.com/nsf/termbox-go manages its own dependencies
[INFO] Package github.com/mitchellh/panicwrap manages its own dependencies
[INFO] Project relies on 3 dependencies.
//}


@<code>{glide init}コマンドを実行すると、@<code>{vendor}というディレクトリが生成され、そこにパッケージがインストールされます。
このディレクトリの中身を確認してみましょう。
表示は省略していますが@<code>{$GOPATH/src}以下と同じようなディレクトリとファイルのレイアウトが現れるはずです。


//emlist[]{
$ tree vendor -d -L 3
vendor
└── github.com
    ├── k0kubun
    │   └── pp
    ├── mitchellh
    │   └── panicwrap
    └── nsf
        └── termbox-go
//}


== Makefileによって決定されるメタ情報を利用する

Makefile対応に伴う最後の修正点として、バージョン番号をMakefileによって自動的に決定される値を利用するように修正してみましょう。
まず、Makefileの冒頭にある@<list>{makefile-meta}の部分について見てみましょう。


//list[makefile-meta][Makefileによってメタ情報を決める]{
# メタ情報
NAME     := fod
VERSION  := $(shell git describe --tags --abbrev=0)
REVISION := $(shell git rev-parse --short HEAD)
LDFLAGS  := \
	-X 'main.name=$(NAME)' \
	-X 'main.version=$(VERSION)' \
	-X 'main.revision=$(REVISION)'
//}


さて、VERSIONとREVISION変数の値をgitのコマンドの実行結果から決定し、それをLDFLAGSという変数にさらに展開しています。
LDFLAGS変数は、Go言語のコンパイラが利用するオプションとなっており、Makefile中で@<makefile-target-bin>のようにgo buildコマンドに渡されています。
また、お気づきかと思いますが「みんなのGo言語」で掲載されているテンプレートから少し手を加えて、NAMEをmain.nameとして与えるようにしています。


//list[makeifle-target-bin][LDFLAGSをgo buildコマンドに渡す]{
## build binaries ex. make bin/myproj
bin/%: cmd/%/main.go deps
	go build -ldflags "$(LDFLAGS)" -o $@ $<
//}


ビルドターゲットbinは、cmd/fodをビルドするためのものです。
-ldflagsオプションによって、cmd/fodのmainパッケージ内部のグローバル変数versionとrevisionの値を設定することができます。
これを利用して、バージョン番号文字列を生成する関数を書いてみましょう。


//list[version-func][]{
var (
  name     string
  version  string
  revision string
)

func versionStr() string {
  return fmt.Sprintf(
    "%s version %s revision %s",
    name,
    version,
    revision,
  )
}
//}


cmd/fodはCLIアプリケーションのためのフレームワークとしてurfave/cliを利用しています。
urfave/cliではアプリケーションのバージョンを返すための仕組みが用意されているので、@<list>{cli-app-version}のようにこれを利用します。


//list[cli-app-version][urfave/cliでバージョン番号を表示する]{
func main() {
  ...
  app := cli.NewApp()
  app.Name = name
  app.Version = versionStr()
}
//}


=={make_practice} Makefileをいろいろ試してみる

さて、ここまでで想定していたMakefile対応に伴う修正は一通り済んだので、実際に動作をみてみましょう。
まず、各ターゲットのざっくりとした説明です。


: setup
    fodパッケージプロジェクト自体が依存するパッケージをgo getなどによりインストールします。
: test
    vendorディレクトリ以下のもの除くファイルに対するgo testを実行します。
: deps
    glide installを実行し、fodが依存するパッケージをインストールします。
: update
    glide updateを実行し、依存パッケージを更新します。
: lint
    vendorディレクトリ以下のものを除く、パッケージディレクトリ以下のファイルを検査します。
    具体的にはgo lintやgo vetを実行します。
: fmt
    vendorディレクトリ以下のものを除く、パッケージディレクトリ以下のファイルをフォーマットします。
    具体的にはgoimportsによるフォーマットを実施します。
: bin/%
    コマンドバイナリを生成するためのターゲットです。
    実際にはmake bin/cmd/fodなどのように実行します。
: help
    make2helpによる、Makefileの自身のヘルプを表示します。@<fn>{make2help}


//footnote[make2help][https://github.com/Songmu/make2help]


つづいて、いくつかのターゲットを実際に実行してみた様子などをご紹介します。


=== make bin/

bin/% ターゲットは、cmdディレクトリ以下に配置したコマンドラインアプリケーションをビルドするためのコマンドです。
たとえば@<code>{make bin/fod}とすることで@<code>{cmd/fod}以下のファイルをソースコードとしてビルドを行います。


//emlist[]{
$ make bin/fod
...
go build -ldflags "-X 'main.name=fod' -X 'main.version=c91-before' -X 'main.revision=05e6ee6'" -o bin/fod cmd/fod/main.go
//}


ビルドによって出来た実行バイナリはパッケージディレクトリ直下のbinディレクトリに配置されます。
お試しということで、@<code>{--version}オプションを渡して実行してみると、次のように表示されるでしょう。


//emlist[]{
$ ./bin/fod --version
fod version fod version c91-before revision 05e6ee6
//}


=== help ターゲット

==={lint_and_fmt} lintおよびfmtターゲット

lintターゲット、fmtターゲットはプログラムコードを検査したりフォーマットを整えたりするためのものです。
試しに@<code>{make lint}実行してみたところ、このような感じにエラーとなってしまいました。
成功するようになるまでコードを修正してからコミットを行うようにするのですが、後ほど改めて詳しくみてみようと思います。


//emlist[]{
$ make lint
...
go vet $(glide novendor)
for pkg in $(glide novendor -x); do \
                golint -set_exit_status $pkg || exit $?; \
        done
constant.go:6:2: don't use ALL_CAPS in Go names; use CamelCase
constant.go:6:2: exported const FS_TYPE_FILE should have comment (or a comment on this block) or be unexported
constant.go:7:2: don't use ALL_CAPS in Go names; use CamelCase
constant.go:8:2: don't use ALL_CAPS in Go names; use CamelCase
constant.go:12:2: don't use ALL_CAPS in Go names; use CamelCase
constant.go:12:2: exported const VERSION_MAJOUR should have comment (or a comment on this block) or be unexported
constant.go:13:2: don't use ALL_CAPS in Go names; use CamelCase
constant.go:14:2: don't use ALL_CAPS in Go names; use CamelCase
...
//}


IDEやプログラマ向けのエディタを使っているのであれば、プラグインなどによってファイルのセーブ時に@<code>{goimports}や@<code>{go fmt}、@<code>{go vet}を実行するようになっていることが多いと思いますが、それらの設定はあくまで属人的なため、このようにプロジェクトの標準として示しておくのも良さそうです。


=== test

testターゲットは@<code>{glide novendor}の結果にもとづいてGo言語のソースファイルをテストします。
fodはについては、パッケージディレクトリとcmdディレクトリに含まれるファイルが対象となりますが、ちゃんとテストを書いていなかったので、次のようなさみしいことになりました。
これに関しては、後にテストを追加してみようと思います。

//emlist[]{
$ make test
...
go test $(glide novendor)
?       github.com/ykanda/fod/cmd/fod   [no test files]
?       github.com/ykanda/fod   [no test files] 
//}


== この章のまとめ

本章ではプロジェクトの土台をつくり、ワークフローを規格化するといった事を行いました。
また、それを実際に運用することでコードを是正していくという例にも触れました。

Go言語はイデオマティックな言語で、その慣例に従うことが美徳とされます。
go lintやgo vetによる検査にとどまらず、go fmtやgoimportsなどによるコードの書き換えすら行います。
このスタイルの強要ともいうべき文化は、他の言語（とくにセミコロン言語）などから来た人には最初はアレルゲンになるかもしれません。
まあでもやってるうちに気にならなくなりますし、Goに入らばGoに従えということでやるのがおすすめです。

