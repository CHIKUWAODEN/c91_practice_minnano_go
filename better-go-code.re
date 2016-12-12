= Go言語らしいコードにする

//lead{
これまでの@<chapref>{lib-and-bin}と@<chapref>{makefile}では主にプロジェクトそのものの構成にまつわる修正をおこなってきました。
ここから「みんのGo言語」で示唆されているようなGoらしいコードになるよう、具体田的なコードそのものとなるよう、修正していくということを実践したいと思います。
//}



== cmd/fodをテストしやすくする

現状、アプリケーションのmain直下にはいきなりurfave/cliを呼び出すようになっています。
つまりは@<b>{main() -> cli.App.Run()}という形になっていて、エントリポイントの処理がそのままアプリケーションの一部になっています。
このため、コマンドラインの引数を条件としたテストなどが書きづらくなっています。
そのため、これを@<b>{main() -> アプリケーションのブートストラップ -> cli.App.Run()}となるようにします。


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


こうすることで、ブートストラップ部分をアプリケーションの実質的なエントリポイントとして、テストなどからパラメータを与えやすくなるでしょう。
実際にテストをするための擬似的なコードは次のようになります。
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
このオプションをつけてアプリを起動した場合、返されるステータスコードはExitCodeOKであることが期待されるため、もしそうでない場合にはテストを失敗するものとしています。
実際に@<code>{make test}を実行した結果は、次のようになりました。


//emlist[make testを実行した結果]{
$ make test
... 
go test $(glide novendor)
ok      github.com/ykanda/fod/cmd/fod   0.011s
?       github.com/ykanda/fod   [no test files]
//}


//tip[コマンドライン引数をパラメータとしたテスト]{
世の中には、シェル経由でコマンドラインアプリケーションを起動して標準出力やExitコードを検査するようなアプリケーションテストフレームワークも存在していますが、このようにすることでユニットテストフレームワーク上で同じことが可能です。
そして、この手法はGo言語にかぎらず応用することができそうです。
//}


== レシーバ名にselfは使わない

fodのコードをVisual Studio Codeで開いていたところ、次のような警告が大量に発生していました。


//list[][]{
receiver name should be a reflection of its identity; don't use generic names such as "me", "this", or "self"
//}


「レシーバの名前にジェネリックな名前は使うな、その特性を反映した名前にしろ」ということのようです。
これは「みんなのGo言語」で言及されている内容ではないのですが「名前重要」というのはプログラマに広く認識されたベタープラクティスの一つというのは疑いのないものでしょうから、警告にしたがって修正してみましょう。
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
func (selector *SelectorCommon) MoveCursorDown() {
	if selector.Cursor < (len(self.GetEntries()) - 1) {
		selector.Cursor++
	}
}
//}


これまでのselfよりは冗長になりましたが、レシーバが具体的にどんなオブジェクトなのかということが具体的に分かりやすくなりました。
また、直してみると分かるのですが、@<code>{self.Hoge}などと書くとオブジェクト自身のHogeなんだな、という風に見えてしまいますが、レシーバ名が具体的になることによって、そのフィールドは本当にそのレシーバに持たせるのが適当なのか？という不自然さに気づくことがあります。
コードの良くないにおいに早期に気づけるようにするという意味で、レシーバの名前を具体的なものとすることは良いプラクティスといえるのではないでしょうか。


//note{
ところで筆者は、レシーバや引数名にあまり省略形は使わないようにしています。
Go言語のソースコードを見ていると、十分に短い関数にシンプルな（時として一文字などもある）変数名を使うという風潮があるように思います。
カルチャーとしては良いのですが、これが時たま好ましくない形であらわれているのを目にします。
たとえば、省略することにこだわるあまり、意図を損ねかねない形になってしまっていることもあります。
*SelectorCommon型のレシーバの名前にselとつけるよりは、selectorと書いてしまった方が具体的だと思うのです。
また、一文字単語も名前の重複が発生するような場合は少し気まずいことになってしまいます。
同じスコープの中にselectorとswitcherという要素があった場合に、単純にsと略すと名前がかぶってしまいますね。
//}


== 全体的なテストの追加
