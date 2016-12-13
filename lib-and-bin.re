= ライブラリとコマンドの分離

//read{
まずはじめにライブラリとコマンドを分割してみようと思います。
いわゆる「ファイルを開く」ダイアログというのはSDKやライブラリ、フレームワークによって提供され、使いまわせる部品として扱われているのが普通です。
しかし、fodは現状バイナリという形でのみ提供されていて、ライブラリとして他のプログラムから利用することはできません。
まずはコマンドラインアプリケーションとしてのfodと、シンプルな部品として提供されるfodとに分割してみましょう。
//}

== 本章の目的

本章では「みんなのGo言語　4.2 デザイン インターフェイスとリポジトリ構成」を参考に、
バイナリをメインの成果物とし、ライブラリをサブの成果物とするGoのプロダクトとなるように形を替えます。

== 改修前のレイアウト

はじめに、修正前のプロジェクト構成を見てみましょう。
次に示すのは、改修を行う前のリポジトリルートでtreeコマンドを実行した結果（を見やすいように整形したものです）。
ごらんのように、全てのファイルがパッケージのソースディレクトリの直下に置かれる形になっており、すべてmainパッケージに属する形となっています。


//emlist[改修前のレイアウト]{
./
├── Gomfile
├── README.md
├── constant.go
├── context.go
├── debug.go
├── define.go
├── entry.go
├── filter.go
├── filter_directory.go
├── filter_dotfile.go
├── filter_file.go
├── filter_filename.go
├── fod.go
├── selector.go
├── selector_directory.go
├── selector_file.go
└── termbox.go
//}

この中でfod.goがコマンドラインバイナリとしてのfodのエントリポイントが含まれているファイルです。
シンプルなツールゆえに、アプリケーションとしてのプログラムコードはこのファイルにほとんど集約されています。
それ以外はライブラリであったり、ライブラリ的な内容のコードを含んでいます。


== プロジェクトのレイアウト

Go言語でパッケージを公開する場合、その目的は次の四つのいずれかに当てはまるでしょう。

    1. バイナリのみを成果物とする
    2. ライブラリのみを成果物とする
    3. バイナリを主な成果物とし、同時にその一部をライブラリとして提供する
    4. ライブラリを主な成果物とし、同時にそれを利用したバイナリを提供する

このうち、バイナリのみを成果物とする場合と、ライブラリのみを成果物にする場合のやり方については、公式サイトで述べられています。@<fn>{official-lib}@<fn>{official-command}
バイナリとライブラリを複合して配布する場合は、コミュニティによって慣習的に定められたレイアウトに従います。
//footnote[official-command][https://golang.org/doc/code.html#Command]
//footnote[official-lib][https://golang.org/doc/code.html#Library]

fodの場合、ライブラリを主な成果物として提供しつつ、もっともシンプルな実装例としてのバイナリを提供を目指します。
つまり、4の形をとるように改修を加えていきます。

== バイナリ用のディレクトリを切る

ライブラリがメインで、バイナリが副次的な要素の場合、慣例的に @<code>{パッケージのルートディレクトリ>/cmd/<バイナリ名>}とします。
今回はバイナリをこれまでどおりfodとしたいので、@<code>{$GOPATH/src/github.com/ykanda/fod/cmd/fod}というディレクトリを作成します。
ここに、fod.goを移します。


//emlist[バイナリコマンド用にコードを切り分ける]{
$ cd $GOPATH/src/github.com/ykanda/fod
$ mkdir -p cmd/fod
$ git mv ./fod.go ./cmd/fod/fod.go
$ git status
On branch c91
Your branch is up-to-date with 'origin/c91'.
Changes to be committed:
  (use "git reset HEAD <file>..." to unstage)

        renamed:    fod.go -> cmd/fod/fod.go
//}


== パッケージ名の修正

さて、@<code>{/cmd/fod/fod.go}とファイルを配置したわけですが、これはバイナリファイルとなるため（オフィシャルルールに準拠して）パッケージ名を@<code>{main}とします。
一方、これまでリポジトリの直下にあり、それまでの@<code>{fod.go}から直接参照されていたものはライブラリとしての扱いになるため、パッケージ名を@<code>{fod}とする形に変更します。
そして、fod.goはこれをimportするようにします。

また、これにともないfod.goの中でこれまでmainという同じパッケージ名であったために参照できていた部分が参照できなくなるため、
適宜パッケージ名fodを解決するようにプログラムを修正しました。
（必要と判断される部分を除き）修正前後のdiffを全て掲載するわけにはいきませんが、そこはぜひともGitHub上のfodのリポジトリをご覧下さい。@<fn>{github-fod}


//footnote[github-fod][https://github.com/ykanda/fod]


== この章のまとめ

この章では、プロジェクトのファイル配置を変更しました。
How to Write Go Code@<fn>{how-to-write-go-code}でも触れられているとおり、Go言語ではパッケージを作る際のディレクトリレイアウトが標準で定められていますし、さらにその中にコミュニティのコンセンサスに基づくレイアウトがあります。
Ruby on Railsなどの界隈では「設定より規約」という言葉などで語られているとおり、このような取り決めによって、無作為にコーディングすればただカオスのように広がる複雑さをある程度制御することができます。
とまあ大げさな物言いをしてみましたが、このようにすることで最初は窮屈に思えるかもしれないこの仕組みが意外に親しみやすく思えてくるはずです。


//footnote[how-to-write-go-code][https://golang.org/doc/code.html]