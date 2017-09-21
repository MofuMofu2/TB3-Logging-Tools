= fluentdとLogstashをはじめるまえの準備
コンフィグや動作の比較をするのであれば、実際に動かしながらの方がわかりやすいですよね。
というわけで、事前準備としてインストール方法やプラグインの探し方などを記載します。
今回はXX上で動作検証を行っていますので、別のOSを利用する場合や詳しく知りたい場合は
公式ドキュメントや技術書を参照してください@<fn>{mofumofu_tweet}。


また、今回はfluentdとLogstashを別々のサーバーにインストールしています。
メモリを結構使うので、相乗り構成は避けた方が良いかもしれません。

//footnote[mofumofu_tweet][もふもふちゃんの同人誌を買ってもらってもいいんですよー（ただしLogstashに限る）]

== インストール
=== fluentd

==== fluentdのインストール
fluentdは@<b>{td-agent}という名称で公式からパッケージが配布されています。インストールする際はこちらを利用することが
推奨されているので、ありがたく使わせていただきましょう。提供されているシェルスクリプトをcurlコマンドでGETすると
インストールまで完了します。

//cmd{
fluentdのインストールコマンド
//}


==== 動作確認
インストール後は@<code>{/etc/init.d/td-agent}にデーモンスクリプトが配置されますので、起動できるかを確認しましょう。

//cmd{
デーモンスクリプトの起動
//}


最後に、テストデータをfluentdに投げて処理ができるかも確認しておきましょう。ポートが空いているかは事前に確認しておくと良いです。

//cmd{
テストデータの確認
//}


=== Logstash

==== Javaのインストール
LogstashはJRubyでプラグインが作成されているため、動作にはJavaが必要です。
バージョン8以上が動作要件となりますが、こだわりがなければJava8の最新版をインストールすれば良いでしょう。

//cmd{
# Javaがインストールされていない場合
$ sudo yum install java
（動作ログに入れ替える）

$ java -version
入れる
//}

==== 動作確認用のlogstash.conf作成
logstash.confはこの後の章で中身を見ていきますので、ここでは動作確認用のコンフィグを
記載します。任意のディレクトリにlogstash.confを作成し、次の内容を転記してくだい。

次に、logstash.confを編集します。

//cmd{
# logstash.confの編集
$ vi logstash.conf

# 下記の通り編集
-----------------------------------------------------
# 標準入力を受け付ける
input {
  stdin { }
}
# 標準出力を行う
output {
  stdout { codec => rubydebug }
}
//}

=== Logstashの動作確認

@<tt>{/bin}下にある@<tt>{logstash}スクリプトから、Logstashを起動します。
OSがWindowsの場合、同階層に@<tt>{logstash.bat}が配置されているのでそちらを起動します。

Logstashの起動をコマンドで行う場合、次の通りに実行します。
@<code>{-f}オプションで@<tt>{logstash.conf}の名前を指定します。

//emlist[Logstashの起動]{
$ /bin/logstash -f logstash.conf
//}

@<tt>{logstash.conf}にタイプミスがない場合、<@<tt>{"Logstash startup completed"}と出力されます。
コマンドプロンプトに好きな文字列を打ち込んでみましょう。
文字列がそのまま返り値として出力された場合、正しくセットアップできています。

//cmd{
logstash-5.2.2 mofumofu$ bin/logstash -f logstash.conf

# 標準入力で"hello world"を入力
hello world
{
    "@timestamp" => 2017-03-05T07:22:40.899Z,
      "@version" => "1",
          "host" => "mofumofu-no-MacBook-Pro.local",
       "message" => "hello world"
}
//}


== プラグインの探しかた
fluentdとLogstashはプラグイン形式で機能が提供されています。インストール時点でも様々なデータを処理できるように
なっていますが、足りない機能は別の人が作成したプラグインで補完することもできます。@<href>{https://rubygems.org/gems}で
探すと良いですが、本番環境への投入を考えるのであればよく動作検証した方が良いです。
もちろん、RubyやJRubyが書けるのであればプラグインを自作することもできます。こういう点は、OSSの良いところですね。

=== fluentd
プラグインは@<href>{https://www.fluentd.org/plugins}に一覧が記載されています。
カテゴリごとに見ることができるので、用途に応じたプラグインを探しやすくなっています。こういったところに優しさが感じられますね。
各プラグイン名のURLリンクをクリックすると、GithubのREADMEページへジャンプします。コンフィグの書き方や使用方法を確認して
使っていきましょう。


=== Logstash
公式で提供されているプラグインはドキュメントに記載がありますが、こちらはinput、filter、outputごとにページが別れています。
@<href>{https://www.elastic.co/guide/en/logstash/6.x/index.html}を見ながらプラグインを探しましょう。ちなみに
LogstashのバージョンごとにURLが異なりますので注意してください@<fn>{logstash_documents}。

//footnote[logstash_documents][バージョンアップによってプラグインのオプションで提供される機能が変わったり、プラグインごと消えてたりしますので。]
