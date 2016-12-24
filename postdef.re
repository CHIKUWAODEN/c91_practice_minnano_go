= あとがき

さて、いかがでしたでしょうか。
少しは楽しめる内容になっていたら幸いです。
今回はネタが固まるのがかなり遅くなってしまい、スケジュールが押していく中で焦りながら仕上げました。
お見苦しい部分があればご容赦いただければと思います。


== ボツになったSquirrelネタの話し

弊サークルは、プログラミング言語としてはGo言語とSquirrel言語を扱うことがこれまで多かったのですが、C91のネタとして検討していたものの一つにSquirrel言語のGo言語バインディングというのがありました。
これが率直に言ってとても面倒というか、割に合わないというか、筋が悪いというか、実装に取り組んでみたところあまり進みが良くありませんでした。
同じ組み込み型スクリプト言語とGo言語をバインディングするというもので、Rubyの組み込み向け実装であるmrubyの組み込みAPIをGo言語から扱うgo-mrubyというものがあるのですが、これの実装を見て意外と簡単に実装できるかもと思ったのが間違いでした。

Squirrelは基本的にC++で実装されており、一方mrubyはC（C99）で実装されています。
そのため、mrubyはcgoを用いることで比較的容易に実装できているように見えましたが、もともとC++としてコンパイルされるSquirrelは、単にライブラリをリンクするという方法ではなかなか上手くいきませんでした。
いろいろと試してはみたものの、結局のところ思わしい実装が仕上がるまでに至らず、結果として見送りました。
改めて機会をうかがいつつ再チャレンジできないかとは考えていますが、どうなるかはわかりません。


== 謝辞

本書の制作にあたり、Re:VIEW (https://github.com/kmuto/review) を利用させていただきました。
武藤Re:VIEWの開発者各位にこの場を借りて感謝の意を表すとともに、お礼申し上げる次第です。
@<br>{}
@<br>{}
また、本書の表紙として、Renee French氏によるhttps://github.com/egnelbre/gophersの画像をCC0 licenseのもとに利用させて頂きました。
ここに感謝の意を評します。


== おわりに

紙幅の都合もあるので、そろそろおしまいにしたいと思います。
次はもうすこし余裕のあるスケジュールで取り組みたいですね（これも毎回書いている気がしますが）。
それでは、良いお年を。
@<br>{}
@<br>{}
2016年12月29日 @kandayasu