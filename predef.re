= まえがき

== はじめのごあいさつ

こんにちは、あるいははじめまして。@kandayasuです。
ひさしぶりに冬コミに出られることになりました、
C90はもともと出展しないことにしていましたし、C89は落選していたので、C88以来二回のインターバルをまたいでのC91への参加となります。

C88ではGo言語を扱った本をつくりましたが、今回も同じくGo言語ネタでお送りします（Gopherくんかわいいですね）。
先だって技術評論社より発売された「みんなのGo言語[現場で使える実践テクニック]」という本について、この本をお求めになった方ならご存知であるかと思います。
実際にアプリを書くにあたって、しっかりと足場を踏んでいくためのTIPSが多く掲載された、良い本だと思いました。

今回はこの本の内容を参考に改修を施してみようというものです。
題材となるのは、拙作の fod（file open dialog）というプログラムで、これはしばらく前に自分が書いた、ファイルを開くダイアログを実現する簡単な CLI アプリケーションです。@<fn>{github-fod}
それほど大きくもなく、かといって改善の余地もないほどシンプルすぎることもなく、ネタとしてはちょうど良いものが手元にあったという感じです。


== 動作環境

本書におけるプログラムの動作確認はmacOS Sierra (10.12.1)上でのみ行っています。
また、Go言語のバージョン情報は@<code>{go version go1.7.1 darwin/amd64}となっています。

Linuxなどの環境でも、そのままビルドしたり動作させたりすることは可能かと思いますが、著者自身確認を行っておりませんし、これはWindowsも同様です。
もし動作不良を確認したのであれば、http://github.com/ykanda/fodにIssueを投げてくださるか、奥付に記載している連絡先宛にお声かけいただければと思います。


== 免責事項

本書に登場するプログラムコードおよびその解説について、引用・二次利用について一切の制限を設けるものではないことをここに明記いたしますが、引用・二次利用の結果生じ不利益については、一切の責任を負いません。


さて、それでは本編にいってみましょう。

//footnote[github-fod][https://github.com/ykanda/fod]