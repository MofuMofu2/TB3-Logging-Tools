= fluentdとLogstashをはじめるまえの準備
コンフィグや動作の比較をするのであれば、実際に動かしながらの方がわかりやすいですよね。
というわけで、事前準備としてインストール方法やプラグインの探し方などを記載します。
今回はXX上で動作検証を行っていますので、別のOSを利用する場合や詳しく知りたい場合は
公式ドキュメントや技術書を参照してください@<fn>{mofumofu_tweet}。

//footnote[mofumofu_tweet][もふもふちゃんの同人誌を買ってもらってもいいんですよー（ただしLogstashに限る）]

== インストール
=== fluentd
=== Logstash

==== Javaのインストール
LogstashはJRubyでプラグインが作成されているため、動作にはJavaが必要です。
バージョン8以上が動作要件となりますが、こだわりがなければJava8の最新版をインストールすれば良いでしょう。

//cmd{
# Javaがインストールされていない場合
$ sudo yum install java

$ java -version
java version "1.8.0_45"
Java(TM) SE Runtime Environment (build 1.8.0_45-b14)
Java HotSpot(TM) 64-Bit Server VM (build 25.45-b02, mixed mode)
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
