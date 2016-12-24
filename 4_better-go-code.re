= Go言語らしいコードにする

//lead{
これまでの@<chapref>{1_lib-and-bin}と@<chapref>{2_makefile}では主にプロジェクトそのものの構成にまつわる修正をおこなってきました。
ここから「みんのGo言語」で示唆されているようなGoらしいコードになるように修正していくということを実践したいと思います。
//}



== cmd/fodをテストしやすくする

現状、アプリケーションのmain直下にはいきなりurfave/cliを呼び出すようになっています。
つまりは@<b>{main() -> cli.App.Run()}という形になっていて、エントリポイントの処理がそのままアプリケーションの一部になっています。
このため、コマンドラインの引数を条件としたテストなどが書きづらくなっています。
そのため、これを「4.5 使いやすく、メンテナンスしやすいツール」などを参考に、@<b>{main() -> アプリケーションのブートストラップ -> cli.App.Run()}となるようにします。


//list[main-the-app][アプリケーションのエントリポイント]{
// main()関数にあった処理をrun()関数に追い出す
func main() {
  os.Exit(run(os.Args))
}


// run()関数にはそれまでmain()に入っていた様々な処理がそのまま移植されている
func run(args []string) int {
  ...
  ...
  app := cli.NewApp()
  app.Run(args)
  ...
  ...
  return ExitCodeOK
}
//}


ここでコマンド実行時の終了ステータスとなる値を@<code>{ExitCode...}という定数として定義するという作業も行いました。
0や1といった単なる値に名前付けをすることで、値の意味をより把握しやすくなるというのは、Go言語にかぎらずプログラミング言語一般に通じるお作法です。


//list[decl-exit-code][ExitCodeを定数として定義する]{
// ExitCode
const (
  ExitCodeOK    int = 0
  ExitCodeError     = 1
)
//}


こうすることで、ブートストラップ部分をアプリケーションの実質的なエントリポイントとして、テストからパラメータを与えやすくなります。
実際にテストをするための擬似的なコードは@<list>{test-run}のようになります。
@<code>{run()}関数は引数として@<code>{os.Args}つまりコマンドライン引数の配列表現をそのままうけとるようになっています。
つまり、擬似的にこれと同じものを渡すことができれば、任意のパラメータでアプリケーションを動かしているのと変わらないチェックをテストフレームワーク上で実現できるわけです。


//list[test-run][run()関数をテストする]{
func TestAppVersion(t *testing.T) {
  args := strings.Split("fod -v", " ")
  if status := run(args); status != 0 {
    t.Errorf("status expectd %d, actual %d", 0, status)
  }
}
//}


@<list>{test-run}の例では、@<code>{fod -v}というバージョン情報を表示させるためのオプションでのfodの呼び出しを表しています。
このオプションをつけてアプリケーションを起動した場合、返されるステータスコードはExitCodeOKであることが期待されるため、もしそうでない場合にはテストを失敗するものとしています。
実際に@<code>{make test}を実行した結果は、次のようになりました。


//emlist[make testを実行した結果]{
$ make test
... 
go test $(glide novendor)
ok      github.com/ykanda/fod/cmd/fod   0.011s
?       github.com/ykanda/fod   [no test files]
//}


世の中には、シェル経由でコマンドラインアプリケーションを起動して標準出力やExitコードを検査するようなアプリケーションテストフレームワークも存在しています。
しかし、この節で解説した手法によって、そのようなものに頼らずともユニットテストフレームワーク上で同じことを可能になり、さらにこの手法はGo言語にかぎらず応用することができそうです。






== lintの結果をもとに警告を修正する

@<hd>{2_makefile|make_practice|lint_and_fmt}では@<code>{make lint}の実行結果により、たくさんの警告が表示されているのを確認しました。
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


#@# //note{
#@# Go言語のソースコードを見ていると、十分に短い関数にシンプルな（時として一文字などもある）変数名を使うという風潮があるように思います。
#@# カルチャーとしては良いのですが、これが時たま好ましくない形であらわれているのを目にします。
#@# たとえば、省略することにこだわるあまり、意図を損ねかねない形になってしまっていることもあります（適当な名前付けで怒られている自分が言うのもおかしな話しですが）。
#@# 
#@# *SelectorCommon型のレシーバの名前にselとつけるよりは、selectorと書いてしまった方が具体的だと思うのです。
#@# また、一文字単語も名前の重複が発生するような場合は少し気まずいことになってしまいます。
#@# 同じスコープの中にselectorとswitcherという要素があった場合に、単純にsと略すと名前がかぶってしまいますね。
#@# //}



#@# === 不要な型名
#@# 
#@# @<code>{var}キーワードで変数を宣言すると同時に初期値を指定するばあい、初期値の型によって変数の方が定められるため、型名の記述は不要です。
#@# これは、次のように警告されます。
#@# 
#@# 
#@# //list[lint-should-omit-type][]{
#@# filter_directory.go:8:21: should omit type *DirectoryFilter from declaration of var directoryFilter; it will be inferred from the right-hand side
#@# //}
#@# 
#@# 
#@# 次のようにコードを修正しました。
#@# 
#@# 
#@# //list[lint-should-omit-type-fix][]{
#@# +++ b/filter_directory.go
#@# @@ -5,7 +5,7 @@ type DirectoryFilter struct {
#@#  }
#@#  
#@# -var directoryFilter *DirectoryFilter = &DirectoryFilter{}
#@# +var directoryFilter = &DirectoryFilter{}
#@# //}




=== if節がreturnで終わる場合にはelse節を省く

次の警告は、if文に伴うブロックがreturn文で終わる場合にelse節は不要である旨を警告するものです。


//list[lint-else-return][不要なelse節に関する警告]{
termbox.go:37:9: if block ends with a return statement, so drop this else and outdent its block
//}


具体的には、次のようなコードについて警告されています。
このようなコードではelse節を省くようにします。


//list[lint-marked][不要なelse節を含むコード]{
func marked(m bool) string {
  if m == true {
    return "*"
  } else { // この else 節がいらない
    return " "
  }
}
//}

修正後のコードは次のようになります。
単に引数の値によって返す値を決めるような関数を書く場合、このコードのような形に自然となるのではないかと思います。


//list[lint-marked-fixed][不要なelse節を省いたコード]{
func marked(m bool) string {
  if m == true {
    return "*"
  }
  return " "
}
//}


このコードでは戻り値を決めるだけの簡単なコードですが、仮にelse節の中に処理が書いてあるようなコードをの場合はどうなるでしょうか。
もしelse節の中にまたコードが続き、さらにその中にif文やfor文を含むような場合、インデントが無駄に深くなっていってしまうのは容易に想像できるでしょう。
可読性という点からみて、インデントの深いコードは避けられるべきであるというのは、一般に認められていることかと思います。
この警告にしたがった修正を施すことで、むやみにインデントが深くならないようにすることができるでしょう。


ちなみに、Go言語には三項演算子（:?演算子）がありません（Go言語にたしなみのある人には説くまでもありませんが）。
故に、他の言語では単に一つの式で済むような処理も、このような形で書くことになります。


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
-  for i, _ := range selector.Entries {
+  for i := range selector.Entries {
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
-  for i := 0; i < len(runes); i += 1 {
+  for i := 0; i < len(runes); i++ {
     termbox.SetCell(x+i, y, runes[i], fgColor, bgColor)
   }
 }
//}


この違いの是非はともかくとして、この差分には意味合いの違いがあります。
それは@<code>{+=}が演算子である一方、@<code>{++}は文（ステートメント）であるということです。
@<code>{++}は「型を持たない定数1によってオペランドを加算する」という意味で解釈されます。
しかし、実際に評価されると結果としては変わらないようです。@<fn>{why-use-inc-stmt}


//footnote[why-use-inc-stmt][なんでこれが推奨されるか良く分からなくてオフィシャルを調べたけど結局良くわからなかったので誰か教えてください（時間がなくてひよってきてる）]