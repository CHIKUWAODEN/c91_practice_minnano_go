= Makefileの追加

//read{
大本となるファイルを再配置しました。
引き続き、開発を進める上で重要な土台を作るべく、makeをタスクランナーとして使う形にしてみます。
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
@<chapref>{lib-and-bin}では@<code>{cmd/fod/fod.go}として、バイナリプログラムのコードを配置しましたが、これはMakefileの定めるところに沿ってはいません。
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



== lintの結果をもとに警告を修正する

@<hd>{make_practice|lint_and_fmt}では@<code>{make lint}の実行結果により、たくさんの警告が表示されているのを確認しました。
これらの警告を一つずつ確認して、実際にコードを修正してみましょう。


=== ネーミング規則

多くの言語では定数であることを（暗に）表す名前として、アッパースネークケース（Upper Snake Case）@<fn>{upper-snake-case}を使う事は珍しいことではありませんが、Goではこのようなネーミングは推奨されていません。
@<list>{lint-dont-use-upper-snake-case-in-go}のように、これは警告となって現れ、 キャメルケース（Camel Case）@<fn>{camel-case}を使うように推奨されます。


//footnote[upper-snake-case][アルファベットは全て大文字で、単語をアンダースコアでつなぐ方法]
//footnote[camel-case][TheFunctionなどのように単語の頭文字を大文字とする方法、先頭の単語まで大文字とするのを特にアッパーキャメルケース、小文字とすることをローワーキャメルケースと呼びわけることもある]


//list[lint-dont-use-upper-snake-case-in-go][Goでは定数名にキャメルケースを使う]{
constant.go:4:2: don't use ALL_CAPS in Go names; use CamelCase
//}


これを@<list>{diff-fs-file-type}のように修正しました。
当然ですが、シンボル名が変わることになるので、この定数を参照している箇所をソースツリー全体から探して新しい名前を利用するように修正する必要があります。


//list[diff-fs-file-type][定数名の修正]{
const (
-       FS_TYPE_FILE    = "f"
-       FS_TYPE_DIR     = "d"
-       FS_TYPE_SYMLINK = "s"
+       FsTypeFile    = "f"
+       FsTypeDir     = "d"
+       FsTypeSymlink = "s"
)
//}


=== 公開される名前とコメント


Goでは大文字から始まるシンボルはパッケージの利用者に対して公開されるものとして扱われます。
公開されるシンボルについてコメントが無い場合、コメントを書くか非公開のシンボルとするように促されます。（@<list>{lint-exported-symbol-should-have-comment}）


//list[lint-exported-symbol-should-have-comment][公開シンボルにコメントを記述するように促す警告]{
context.go:24:1: exported method AppContext.Multi should have comment or be unexported
entry.go:9:6: exported type Entry should have comment or be unexported
//}


また、公開されるシンボルについてのコメントは、そのシンボル名から始まるようにすることが推奨されています。


//list[lint-exported-symbol-comment-form][公開シンボルのコメントの書式についての警告]{
filter_directory.go:10:1: comment on exported function DirectoryFilterSingleton should be of the form "DirectoryFilterSingleton ..."
context.go:5:1: comment on exported type AppContext should be of the form "AppContext ..." (with optional leading article)
filter_dotfile.go:38:1: comment on exported method DotfileFilter.Toggle should be of the form "Toggle ..."
//}


これらを踏まえて、修正を施してみましょう。
ここでは、警告がでている箇所に対して次のいずれかの方法で対応します。
fodはライブラリとしての性質も備えているため、警告の一つ一つに対してどの方法を適用するかを判断する必要があります。


  * 公開する必要のないものは非公開のシンボルとなるようにする
  * 公開する必要のあるものは、適切なコメントを加えるようにする


たとえば、@<code>{fod.AppContext}というstructがありますが、これは外部のプログラム（/cmd/fodなどがその例）から利用されているため、公開されるシンボルとしなければなりません。
そのため、適切なコメントとなるように修正します。


//list[lint-fix-comment-appcontext][]{
-// extends cli.Context
+// AppContext extends cli.Context
type AppContext struct {
 	*cli.Context
}
//}


lintによる警告を避けるためだけの簡易な修正となっていますが、コメントの意義を考えるのであればもう少し説明的な内容があった方が良いと思います。

そして次の例が、@<code>{DotfileFilter.Toggle()}のメソッド名を修正することで非公開であることを示すものです。
このメソッドはひとまずライブラリの外部に露出させない


=== レシーバの名前

「レシーバの名前にジェネリックな名前は使うな、その特性を反映した名前にしろ」ということのようです。
「名前重要」というのはプログラマに広く認識されたベタープラクティスの一つというのは疑いのないものでしょうから、警告にしたがって修正してみましょう。


//list[lint-receiver-name][レシーバ名に関する警告]{
selector.go:181:1: receiver name should be a reflection of its identity; don't use generic names such as "me", "this", or "self"
//}


たとえば、@<list>{move-cursor-up-before}関数はファイル選択カーソルの位置を変更するための関数です。


//list[move-cursor-up-before][レシーバ名修正前の関数]{
func (self *SelectorCommon) MoveCursorUp() {
	if self.Cursor > 0 {
		self.Cursor--
	}
}
//}


これを@<list>{move-cursor-up-after}のように修正しました。


//list[move-cursor-up-after][レシーバ名修正後の関数]{
func (selector *SelectorCommon) MoveCursorUp() {
	if selector.Cursor > 0 {
		selector.Cursor--
	}
}
//}


これまでのselfよりは冗長になりましたが、レシーバが具体的にどんなオブジェクトなのかということが具体的に分かりやすくなりました。
また、直してみると分かるのですが、@<code>{self.Hoge}などと書くとオブジェクト自身のHogeなんだな、という風に見えてしまいますが、レシーバ名が具体的になることによって、そのフィールドは本当にそのレシーバに持たせるのが適当なのか？という不自然さに気づくことがあります。
コードの良くないにおいに早期に気づけるようにするという意味で、レシーバの名前を具体的なものとすることは良いプラクティスといえるのではないでしょうか。


//note{
Go言語のソースコードを見ていると、十分に短い関数にシンプルな（時として一文字などもある）変数名を使うという風潮があるように思います。
カルチャーとしては良いのですが、これが時たま好ましくない形であらわれているのを目にします。
たとえば、省略することにこだわるあまり、意図を損ねかねない形になってしまっていることもあります（適当な名前付けで怒られている自分が言うのもおかしな話しですが）。

*SelectorCommon型のレシーバの名前にselとつけるよりは、selectorと書いてしまった方が具体的だと思うのです。
また、一文字単語も名前の重複が発生するような場合は少し気まずいことになってしまいます。
同じスコープの中にselectorとswitcherという要素があった場合に、単純にsと略すと名前がかぶってしまいますね。
//}


=== varキーワードによる変数宣言時の初期値　

//list[lint-var-zero-value][]{
entry.go:31:20: should drop = "" from declaration of var abs; it is the zero value
//}


=== 不要な型名


//list[lint-should-omit-type][]{
filter_directory.go:8:21: should omit type *DirectoryFilter from declaration of var directoryFilter; it will be inferred from the right-hand side
//}


=== return文とelse節のイディオム

//list[lint-][else節に関する警告]{
selector.go:313:9: if block ends with a return statement, so drop this else and outdent its block (move short variable declaration to its own line if necessary)
termbox.go:37:9: if block ends with a return statement, so drop this else and outdent its block
//}


=== rangeを使ったループの冗長な記述

Golangでコレクション的な要素（array,slice,mapなど）に対してfor文で横断的に処理を刷る場合、rangeを用います。
rangeはキーまたはインデックスと値の二つの値を返し、それを受け取るときには@<code>{インデックス, 値 := range}などのように書きます（map型などの場合はインデックスはキーになります）。
しかし、書き方によっては警告が発せられます。


//list[lint-range-omit-2nd-value][forループで無視される値についての警告]{
selector.go:364:9: should omit 2nd value from range; this loop is equivalent to `for i := range ...`
//}


このような場面で、値には用がないがインデックスを使いたい場合、値の部分は省略するのが良いそうです。
全体的に、余計なものは書かないようにしようという方向で訂正される感じでしょうか。


//list[lint-range-omit-2nd-value-diff][値を使わない場合にはわざわざアンダースコアを書かない]{
 func (selector *SelectorCommon) SetItem(path string, index int) {
-       for i, _ := range selector.Entries {
+       for i := range selector.Entries {
                selector.Entries[i].Marked = false
        }
        entries := selector.GetEntries()
//}


Go言語をたしなんでいる人には言うまでもありませんが、インデックスは使わずに値のみを使いたい場合は@<code>{_, 値 := range}という形でインデックスを無視でき、このときはアンダースコアを省けません。


=== インクリメント演算子を使う

インクリメント操作を行う場合には、複合代入演算子@<code>{+=}を用いるのではなく、インクリメント演算子を持ちいることが推奨されています。（@<list>{lint-should-replace-increment-op}）


//list[lint-should-replace-increment-op][複合代入演算子を使うことによる警告]{
termbox.go:29:30: should replace i += 1 with i++
//}


言われたままに、次のように修正します。
まあ、こちらの方がシンプルでよりGoが是とするものに近いといったことなのでしょうか？


//list[lint-should-replace-increment-op-diff][インクリメント文を使うよう修正する]{
 func drawString(x int, y int, text string, fgColor termbox.Attribute, bgColor termbox.Attribute) {
        runes := []rune(text)
-       for i := 0; i < len(runes); i += 1 {
+       for i := 0; i < len(runes); i++ {
                termbox.SetCell(x+i, y, runes[i], fgColor, bgColor)
        }
 }
//}


この違いの是非はともかくとして、この差分には意味合いの違いがあります。
それは@<code>{+=}が演算子である一方、@<code>{++}は文（ステートメント）であるということです。
@<code>{++}は「型を持たない定数1によってオペランドを加算する」という意味で解釈されます。
しかし、実際に評価されると結果としては変わらないようです。@<fn>{why-use-inc-stmt}


//footnote[why-use-inc-stmt][なんでこれが推奨されるか良く分からなくてオフィシャルを調べたけど結局良くわからなかったので誰か教えてください（時間がなくてひよってきてる）]


== この章のまとめ

本章ではプロジェクトの土台をつくり、ワークフローを規格化するといった事を行いました。
また、それを実際に運用することでコードを是正していくという例にも触れました。

Go言語はイデオマティックな言語で、その慣例に従うことが美徳とされます。
go lintやgo vetによる検査にとどまらず、go fmtやgoimportsなどによるコードの書き換えすら行います。
このスタイルの強要ともいうべき文化は、他の言語（とくにセミコロン言語）などから来た人には最初はアレルゲンになるかもしれません。
まあでもやってるうちに気にならなくなりますし、Goに入らばGoに従えということでやるのがおすすめです。

