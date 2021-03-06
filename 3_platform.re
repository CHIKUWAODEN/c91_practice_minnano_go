= マルチプラットフォーム対応（主にパス周り）

//lead{
本章ではマルチプラットフォーム、主にファイルパスまわりの扱いについての対応を行います。
本書冒頭で説明しましたが、fodは@<b>{File Open Dialog}の省略語であり、最終的にシステム上のファイルのパスを返すことを目的にしたプログラムです。
ゆえに、ファイルパスを扱う操作が内部にそれなりに登場しますが、fodはmacOS（書き始めた当時はOS Xと呼ばれていました）上で書かれいて、動作確認もそこでのみ行われています。
他の環境での動作があまり意識されていないというのは否めません。
そこで「みんなのGo言語 2.2 守るべき暗黙のルール OS間の移植をあらかじめ想定する」などを参考に、ファイルパスの扱いを見直していくとともに、移植性の妨げになるような要素を排除できるよう改修してみたいと思います。
//}


//caution[Windows対応について]{
本書ではmacOS・各種Linuxディストリビューション・Windows系のOSを対象に、マルチプラットフォーム対応を目指した内容について取り扱っていますが、筆者の手元に環境が無いWindowsについては動作確認がしっかり取れていません。
もしfodのWindows上での動作に不具合を見つけるなどした場合には、筆者までご連絡ください。
//}

== path/filepathの利用

#@# 執筆にあたりコードを見直してみたところ、それなりの範囲にわたって修正を行う必要がありました。
#@# そのため、全ての修正内容はここでは掲載しないものとして、エッセンスだけが伝わるように修正内容を例示していこうと思います。

とりあえず@<code>{file/filepath}パッケージを現在どのように利用しているのかを確認してみます。
@<code>{git --no-page grep filepath}としてみたところ、次のようになりました。


//emlist[]{
$ git --no-pager grep filepath      
entry.go:       "path/filepath"
entry.go:               if _abs, err := filepath.Abs(path + "/" + fi.Name()); err == nil {
selector.go:    "path/filepath"
selector.go:    if filepath.IsAbs(path) {
selector.go:    } else if abs, err := filepath.Abs(self.CurrentDir + "/" + path); err == nil {
//}

直接的に@<code>{path/filepath}を利用している箇所をファイル単位でみれば@<code>{entry.go}と@<code>{selector.go}の二つのファイルとなっています。
この@<code>{git grep}の結果のみをみてみると、ディレクトリ区切り文字としての@<code>{"/"}を直接リテラルで記述しているところがあからさまによろしくなさそうです。
これを改修してみることにしましょう。

端的にいうと、これは@<code>{filepath.Abs()}の使い方を間違っている例です。
@<code>{fi}という変数は、指定されたパスから@<code>{ioutil.ReadDir()}によって取得した
@<code>{[]os.FileInfo}を格納した変数です。
このコードは@<list>{better-file-abs}のようにするだけで問題なく動作します。


//list[better-file-abs][filepath.Abs()のより良い使い方]{
abs, err := filepath.Abs(fi.Name())
//}


不要なコードが削除できてシンプルになった上、プラットフォームへの依存性も低減できました。
標準パッケージの機能をよく調べて、その性能を把握しておくのも大切ですね。


=== ディレクトリセパレータを抽象化する

パスの扱いでOSの違いで扱いが異なりやすいものの一つがディレクトリ区切り文字列です。
fodでは@<list>{bad-ds}ように、ディレクトリ区切り文字列が文字列リテラルとして記述されていました。


//list[bad-ds][ディレクトリ区切り文字列のダメな使用例]{
...
} else if abs, err := filepath.Abs(self.CurrentDir + "/" + path); err == nil {
...
//}





Go言語では@<code>{path/filepath}パッケージの中で、ディレクトリ区切り文字を表す定数@<code>{filepath.Separator}が@<list>{separator}のように定義されています（ちなみに、複数のパスのリストにおいてパスとパスを区切る文字列は@<code>{filepath.ListSeparator}です）。


//list[separator][パスとパスリスト区切り文字の定義]{
const (
  Separator     = os.PathSeparator
  ListSeparator = os.PathListSeparator
)
//}


ディレクトリ区切り文字をこれで連結することにしましょう。
しかし、この@<code>{filepath.Separator}はrune型の定数のため、そのまま文字列との結合ができませんが、
便利なことに@<code>{Join()}関数を利用することで簡単に済ませることができます。


//list[][]{
abs, err := filepath.Abs(filepath.Join(self.CurrentDir, path))
//}


@<code>{Join()}関数は、可変個の文字列を引数として受け取り、それらをOSごとに適切なディレクトリ区切り文字で連結した文字列を返します。
そして内部で@<code>{filepath.Separator}を利用しているので、システムに合わせたディレクトリ区切り文字を用いて文字列を連結してくれるようになっています。


=== ベースディレクトリを指定するオプション

fodのコマンドラインアプリ実装には@<code>{--base}というオプションがあります。
これは、ディレクトリパスを指定することで、そのディレクトリを基準に処理を開始するというもので、現状では@<code>{"./"}という文字列リテラルが使われています。（@<list>{flag-base-before}）


//list[flag-base-before][修正前の--baseオプションの定義]{
// Flags : options for urfave/cli
var Flags = []cli.Flag{
  ...
  cli.StringFlag{
    Name:  "base, b",
    Value: "./",
    Usage: "base dir",
  },
  ...
}
//}


これも特定のプラットフォームへの依存のもととなるコードであり、あまり好ましいとは言えません。
@<code>{os.Getwd()}関数を使うようにしてみましょう。
ランタイムで関数呼び出しを行う必要が出てくるため、配列自体を返すような関数に変えてみましょう。


//list[flag-base-after][修正後の--baseオプションの定義]{
func flags() ([]cli.Flag, error) {

  dir, err := os.Getwd()
  if err != nil {
    return nil, err
  }

  // Flags : options for urfave/cli
  flags := []cli.Flag{
    ...
    cli.StringFlag{
      Name:  "base, b",
      Value: dir,         // os.Getwd() で取得した値を使う
      Usage: "base dir",
    },
    ...
  }
  return flags, nil
}
//}


これまで@<code>{Flags}を直接代入していた箇所も修正する必要がありそうです。
@<list>{run-flag-base-after}のように修正してみました。


//list[run-flag-base-after][--baseオプションの修正にあわせたrun()関数]{
func run(args []string) int {
  ...
  flags, err := flags()
  if err != nil {
    return ExitCodeError
  }
  app := cli.NewApp()
  app.Flags = flags
  ...
}
//}


オプションのデフォルト値をランタイムで決める方法は、たとえば設定ファイルを読み込む場合などにも使います。
アプリケーションの設定ファイルを読み込む場所として、ユーザーのホームディレクトリ直下にある@<code>{$HOME/.conf/.fodrc}というファイルをデフォルトにしたいとします。
ホームディレクトリはもちろんユーザー毎に異なるため、特定の値として決めることができません。

そこでよく取られるアプローチが、@<code>{os/user}パッケージにある機能を利用する方法です。
これを使うことで、コマンドを実行したユーザーのホームディレクトリを取得することができます。
現状fodにコンフィグファイルというものは存在しないのですが、ありがちなパターンとして解説したいと思います。


//list[homedir-os-user][os/userパッケージでホームディレクトリ取得する]{
user, err := user.Current()
if err != nil {
  return err
}

conf := filepath.Join(user.HomeDir, ".conf/.fodrc")
fmt.Println(conf)
//}


コード@<list>{homedir-os-user}を実行すると、次のような結果となります。

//list[output-homedir-os-user][ホームディレクトリを取得した結果]{
/Users/kandayasu/.conf/.fodrc
//}


しかし、この@<code>{os/file}を使うアプローチには、プラットフォームによる落とし穴があります。
それはクロスコンパイルが出来なくなってしまうというものです。
そこで、この問題を解消するために@<code>{mitchellh/go-homedir}というパッケージを使います。@<fn>{go-homedir}


//footnote[go-homedir][https://github.com/mitchellh/go-homedir]


//list[use-go-homedir][mitchellh/go-homedirを使う]{
h, err := homedir.Dir()
if err != nil {
  t.Fail()
}

e, err := homedir.Expand("~/.conf/.fodrc")
if err != nil {
  t.Fail()
}

fmt.Println(h)
fmt.Println(e)
//}


このコードを実行した結果が@<list>{output-use-go-homedir}となります。
@<code>{os/user}を使った場合と同じようにホームディレクトリのパスを取得したり、チルダをホームディレクトリのパスに展開
できていますね。


//list[output-use-go-homedir][mittchellh/go-homedirを使った結果]{
/Users/kandayasu
/Users/kandayasu/.conf/.fodrc
//}


このパッケージの特徴とするところは、先述したクロスコンパイルができなくなるという問題を避けるため、Go言語のみで実装されているという点です。
（試してはいませんが）Windows系のOSにも対応しているみたいです。
クロスコンパイルが自由にできれば、手元の開発機でビルドしたバイナリを実行環境にそのまま配布して動作させることもできるでしょう。
そういった利便性をむやみに損なわないためにも、こうした小技を覚えておくと良いかもしれません。


== 本章のまとめ

本章ではfodの中心的な機能であるファイルパスの扱いを見直すといったことを主にやりました。
単一の実行バイナリやクロスコンパイルなど、Go言語はマルチプラットフォームを意識した作りになっていますが、それでもこのようなところは注意しないとその特性をスポイルしかねません。
ファイルパスの扱いはプラットフォームごとに異なる部分の代表的なそれ故に注意したい部分となりますが、先人の知恵に従えばそれもずいぶんラクになるでしょう。

本当はWindows環境での場合についていろいろ書きたかったですが、自由に使えるWindowsマシンが手元になかったため、そこは片手落ちとなってしまいました。
どなたか優しい人がいたら、@<ami>{マサカリを投げる}フィードバックしてくださるとうれしいです。


#@# == ディレクトリの相対的なパス
#@# 
#@# 
#@# == フォルダの最上位階層の扱い
#@# 
#@# Unix系のファイルシステムでは、ディレクトリ階層の最上位はただ一つのルートディレクトリとされ、これは"/"というパス#@# で表されます。
#@# 一方、Windows系のシステムでは、システムにマウントされている各ボリュームごとに最上位の階層に分かれていて、それぞ#@# れのボリュームに対して"C:"などのドライブレターと呼ばれる名前が付いています。