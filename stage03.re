
= Logstashに触れてみる
== 環境について
Logstash6.0betaがすでにインストールされていることを前提とします。
インストール方法については、りまりま団のもふもふ（Twitter:@froakie0021）に記載してあるので、そちらを参考にして頂ければと思います！
（決してサボってるわけじゃないですよwページの有効活用）

== 動かす前のLogstash準備
早速ですが、Logstashを動かしたいと思います！

Logstashを動かすには、confファイルという設定ファイルを読み込ませる必要があります。
このconfファイルにINPUT・FILTER・OUTPUTの定義することで、Logstashが実行します。

=== Logstashのディレクトリ構造
Logstashの一部のディレクトリ構造について記載してますー
　※RPMでLogstashをインストールしてます

/etc/logstash/
├ conf.d　(Logstashに実行させたいINPUT・FILTER・OUTPUTをディレクトリ配下に配置する)
├ jvm.options (ヒープサイズの割り当てなどを定義する)
├ log4j2.properties (ロギング設定)
├ logstash.yml (Logstashの設定ファイル)
└ startup.options (Logstash起動設定)

=== confファイルの配置場所について
logstashがconf.dを読み込みにいくのは、logstash.ymlの64行目に記載してあるためです。
設定内容をみてわかるとおり、*.confとなっているため、先ほど作成したtest01も.confとしているのです。

//cmd{
$ vim logstash.yml
64 path.config: /etc/logstash/conf.d/*.conf
//}


=== confファイルの準備
Logstashを動かす前に簡単なconfファイルを作成します。
confファイルの名前は、test01.confとします。
また、カレントディレクトリは/etc/logstash/です。


//cmd{
$ vim conf.d/test01.conf
input {
  stdin {}
}
output {
  stdout { codec => rubydebug }
}
//}


== 動かすよ！Logstash
logstashの起動スクリプトは以下に配置されてます。

* Logstash起動スクリプト
** /usr/share/logstash/bin/logstash

logstashをサービス起動で実行させることも可能なのですが、テストとして動かしたいため、今回は起動スクリプトから実行します。

では、早速実行してみます。

//cmd{
$ /usr/share/logstash/bin/logstash -f conf.d/test01
###　実行すると入力を待ってるぜ！って言われるので
The stdin plugin is now waiting for input:
### Helloと入力
Hello
{
      "@version" => "1",
          "host" => "0.0.0.0",
    "@timestamp" => 2017-10-01T04:49:23.282Z,
       "message" => "Hello"
}
//}

Helloと入力したらmessageというフィールドに入ってますね！
これでLogstashの環境が整いましたーヽ(*ﾟдﾟ)ノ


== とりま。Apacheのアクセスログを取り込んでみよう
最近、Nginxを使用しているmicci184です。
でも、今回はApacheのアクセスログをLogstashで取り込んで、ごにょごにょしてみたいと思いますー
Nginxでもいいんですけど、まだApacheのが需要があるのかなー？って思った感じで選んでみました。
 　※GoogleトレンドでApache VS Nginxやってみましたが、軍配はApacheでした

以下のサンプルのアクセスログで試していきたいと思いますー
ログフォーマットは、commnを利用します。

* 127.0.0.1 - - [10/Oct/2000:13:55:36 -0700] "GET /test.html HTTP/1.0" 200 2326

=== アクセスログを取り込んでみる
では、さっきの要領でLogstashを動かしてみるよ！
まずは、先ほど同様にtest02.confというファイルを作成します。

//cmd{
$ conf.d/test02.conf
input {
  file {
    path => "/etc/logstash/log/httpd_access.log
    start_position => "beginning"
  }
}
output {
  stdout { codec => rubydebug }
}
//}

今回作成したtest02.confですが、inputにfileというプラグインを記載してます。
このプラグインは、インプットデータとしてファイルを指定することができます。
また、ログファイルを読み込み指定をするために"start_position"のオプションを利用してます。
デフォルトだとendですが、logstashが起動されてから追記されたログを取り込み対象としたいので、biginningを定義してます。
その他にもオプションがあるので、詳しくは公式サイトを確認して頂ければと思います。

(File input plugin)[https://www.elastic.co/guide/en/logstash/current/plugins-inputs-file.html]
