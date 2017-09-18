= fluentdとLogstashをはじめるまえの準備
コンフィグや動作の比較をするのであれば、実際に動かしながらの方がわかりやすいですよね。
というわけで、事前準備としてインストール方法やプラグインの探し方などを記載します。
今回はXX上で動作検証を行っていますので、別のOSを利用する場合や詳しく知りたい場合は
公式ドキュメントや技術書を参照してください@<fn>{mofumofu_tweet}。

//footnote[mofumofu_tweet][もふもふちゃんの同人誌を買ってもらってもいいんですよー（ただしLogstashに限る）]

== インストール
=== fluentd
=== Logstash
Logstashの起動にはJava（バージョン8以上）が必要です。
必要に応じてインストールしましょう。

//cmd{
$ java -version
java version "1.8.0_45"
Java(TM) SE Runtime Environment (build 1.8.0_45-b14)
Java HotSpot(TM) 64-Bit Server VM (build 25.45-b02, mixed mode)
//}
