= おまけ！！
== 何を取り込むか
本当は、5章で終わろうと思ったのですが、AWSに踏み込んだ話もしたいなーって思って書きました。
知ってる方も多いと思いますが、AWSサービスはログを出力する機能をもったサービスがあるのです。
そのなかでも今回は以下のサービスのログに対してLogstashで取り込んでみたいと思いますー

* ELBアクセスログ

AWSサービスのログを取り込むイメージです。

//image[stage06-01][ELBログ取得イメージ][scale=0.5]{
  ELB
//}

この他にもCloudtrailやS3などもログを出力し、取り込むことが可能です。
どこかでお披露目できればと考えてますヽ(*ﾟдﾟ)ノ


== ELBのログを取り込むよ！
いままで説明した"ログ取り込みフロー"に乗っ取って進めたいと思います。


== ログフォーマットを調べる
ELBのログフォーマットを調べたいと思います。
あ、前提としてELBのロギングは有効化されていることとします！
もし設定されていない方は、公式ドキュメントを確認頂ければと思いますー

公式ドキュメントにアクセスログのフォーマットが記載されています。
（Classic Load Balancerアクセスログ:@<href>{http://docs.aws.amazon.com/ja_jp/elasticloadbalancing/latest/classic/access-log-collection.html}）

うーん。。長い！
ということで一つ一つ分解していきます。


* timestamp elb client:port backend:port request_processing_time backend_processing_time response_processing_time elb_status_code backend_status_code received_bytes sent_bytes "request" "user_agent" ssl_cipher ssl_protocol
** timestamp: ロードバランサーがクライアントからリクエストを受け取った時刻 (ISO 8601 形式)
** elb: ロードバランサーの名前
** client:port: リクエストを送信したクライアントの IP アドレスとポート
** backend:port: ELBにぶら下がっているインスタンス（バックエンド）に対してのIPアドレスとポート番号（リクエストが送信できなかった場合は"-"）
** request_processing_time: ELBがリクエストを受け取ってから、バックエンドのインスタンスに送信するまでの時間(応答がない場合などは"-1")
** backend_processing_time: ELBがバックエンドにリクエストを送信してから、レスポンスが帰ってくるまでの時間(応答がない場合などは"-1")
** response_processing_time: ELBがレスポンスを受け取ってから、クライアントに返すまでの時間(応答がない場合などは"-1")
** elb_status_code: ELBからのレスポンスステータスコード
** backend_status_code: バックエンドのインスタンスからのレスポンスステータスコード
** received_bytes: クライアントから受信したリクエストサイズ
** sent_bytes: クライアントに送信したリクエストサイズ
** request: クライアントからのリクエスト（HTTP メソッド + プロトコル://ホストヘッダー:ポート + パス + HTTP バージョン）
** user_agent: リクエスト元のクライアントを特定する
** ssl_cipher: SSL暗号化(暗号化されていない場合は"-")
** ssl_protocol: SSLプロトコル(暗号化されていない場合は"-")


== フィールド定義
ここからは、フィールド定義するよ！
今回は、Apacheのアクセスログと違ってすでにフィールド名が公式として定義されているので、そのまま使用します。
ただし、client:portのようなフィールドは、clientipとportやbackendも分割します。
また、requestも分割します。

なので、ここでは、フィールドのタイプを決めていきたいと思いますのでサンプルログから当てはめて見たいと思います。
サンプルログは、先ほどのリンクのAWS公式ドキュメントから使ってます。

* 2015-05-13T23:39:43.945958Z my-loadbalancer 5.10.83.30:2817 10.0.0.1:80 0.000073 0.001048 0.000057 200 200 0 29 "GET http://www.example.com:80/ HTTP/1.1" "curl/7.38.0" - -
** timestamp: 2015-05-13T23:39:43.945958Z (date)
** elb: my-loadbalancer (string)
** client_ip: 5.10.83.30 (string)
** client_port: 2817 (string)
** backend_ip: 10.0.0.1 (string)
** backend_port: 2817 (string)
** request_processing_time: 0.000073 (float)
** backend_processing_time: 0.001048 (float)
** response_processing_time: 0.000057 (float)
** elb_status_code: 200 (string)
** backend_status_code: 200 (string)
** received_bytes: 200 (long)
** sent_bytes: 0 (long)
** verb: GET (string)
** proto: http (string)
** urihost: www.example.com:80 (string)
** uripath: - (string)
** httpversion: 1.1 (string)
** user_agent: curl/7.38.0 (string)
** ssl_cipher: - (string)
** ssl_protocol: - (string)


こん感じにマッピングされるようにGrokPatternを作成していきたいと思いますー


== GrokPatternをつくる
前の章でやったようにGrokPatternを作っていきましょう！
実は、AWSのELBは、GrokPatternが用意されているのです。
ただね。
それを使うだけじゃGrok芸人にはなれんのですよ！
ちゃんと理解して、自由にry


=== timestamp
ELBの時刻形式は、ISO8601のフォーマットを利用しています。
そのため、GrokPatternに存在するTIMESTAMP_ISO8601をそのまま使用できるため、こちらを使います。

* %{TIMESTAMP_ISO8601:date}


=== elb
elbの名前ですね！
コレはユーザが任意につける名前なので、GrokPatternの"NOTSPACE"を使用します。

* %{NOTSPACE:elb}


=== client_ip & client_port
Apacheアクセスログと同様に"IPORHOST"を使用したくなりますが、コレはやりすぎです。
なぜかというと"IPORHOST"は、IPだけでなくHOSTも含んでいるためです。
今回のフィールドは、IPのみのため、"client_ip"はGrokPatternのIPとし、client_portは"INT"とします。

* (?:%{IP:client_ip}:%{INT:client_port:int})


=== backend_ip & backend_port
上記のclient_ipとclinet_port同様です！
が、しかし。
バックエンドから応答がない場合は、"-"となるため、|で"-"も記載します。

* (?:%{IP:backend_ip}:%{INT:backend_port:int}|-)


=== リクエストタイム3兄弟
これらすべてGrokPatternの"NUMBER"を使用し、応答がなかったように|で"-1"も記載します。
このフィールドを利用するこで、ELBが受け付けてからのレイテンシを測ることができます。

* (?:%{NUMBER:request_processing_time:double}|-1)
* (?:%{NUMBER:backend_processing_time:double}|-1)
* (?:%{NUMBER:response_processing_time:double}|-1)


=== elb_status_code & backend_status_code
Apacheのアクセスログと同様にステータスコードは、"NUMBER"を使用します。

* (?:%{INT:elb_status_code}|-)
* (?:%{INT:backend_status_code:int}|-)


=== received_bytes & sent_bytes
バイトも同様にNUMBERを使用します。

* %{INT:received_bytes:int}
* %{INT:sent_bytes:int}


=== request
requestの中に複数のフィールドが組み込まれてます。
GrokPatternをみると"ELB_REQUEST_LINE"というのがあります。
このGrokPatternは、"verb" "proto" "urihost" "uripath" "httpversion"を含んでます。
そのため、"ELB_REQUEST_LINE"を呼び出すだけでマッチさせることができます。
察しのいい方は気づいているかもですが、GrokPatternの中で更にGrokPatternを呼び出すことができます。

* ELB_REQUEST_LINE (?:%{WORD:verb} %{ELB_URI:request}(?: HTTP/%{NUMBER:httpversion})?|%{DATA:rawrequest})

上記の"ELB_REQUEST_LINE"内で"ELB_URI"を呼び出しています。

* ELB_URI %{URIPROTO:proto}://(?:%{USER}(?::[^@]*)?@)?(?:%{URIHOST:urihost})?(?:%{ELB_URIPATHPARAM})?

更に、"ELB_URIPATHPARAM"というのを呼び出しているかたちになってます。

* ELB_URIPATHPARAM %{URIPATH:path}(?:%{URIPARAM:params})?


=== user_agent
Apacheアクセスログで使用したGrokPatternの"DATA"を使用します。
"GREEDYDATA"というGrokPatternもあるのですが、最長マッチになってしまうため、想定外のものとマッチしてしまうため、DATAを使用します。

* (?:%{DATA:user_agent}|-)


=== ssl_cipher & ssl_protocol
SSL通信時に使用するフィールドで、使用していない場合は、"-"が付くため|を記載します。

* (?:%{NOTSPACE:ssl_cipher}|-)
* (?:%{NOTSPACE:ssl_protocol}|-)


== Grok Constructorでテスト
個々のテスト結果は省いてますが、慣れるまでは一つ一つクリアしていってください！
あ！ちなみに、今回作成したGrokPattern名がELBではなくCLBなのは、Application Load Balancer（以下、ALB）と区別するためです。
ALBとCLBでは、ログフォーマットが若干違うため、区別してます。

ALB版も合わせてGrokPatternを記載しますー

* CLB_ACCESS_LOG %{TIMESTAMP_ISO8601:date} %{NOTSPACE:elb} (?:%{IP:client_ip}:%{INT:client_port:int}) (?:%{IP:backend_ip}:%{INT:backend_port:int}|-) (?:%{NUMBER:request_processing_time}|-1) (?:%{NUMBER:backend_processing_time}|-1) (?:%{NUMBER:response_processing_time}|-1) (?:%{INT:elb_status_code}|-) (?:%{INT:backend_status_code:int}|-) %{INT:received_bytes:int} %{INT:sent_bytes:int} \"%{ELB_REQUEST_LINE}\" \"(?:%{DATA:user_agent}|-)\" (?:%{NOTSPACE:ssl_cipher}|-) (?:%{NOTSPACE:ssl_protocol}|-)
* ALB_ACCESS_LOG %{NOTSPACE:type} %{TIMESTAMP_ISO8601:date} %{NOTSPACE:elb} (?:%{IP:client_ip}:%{INT:client_port}) (?:%{IP:backend_ip}:%{INT:backend_port}|-) (:?%{NUMBER:request_processing_time}|-1) (?:%{NUMBER:backend_processing_time}|-1) (?:%{NUMBER:response_processing_time}|-1) (?:%{INT:elb_status_code}|-) (?:%{INT:backend_status_code}|-) %{INT:received_bytes} %{INT:sent_bytes} \"%{ELB_REQUEST_LINE}\" \"(?:%{DATA:user_agent}|-)\" (?:%{NOTSPACE:ssl_cipher}|-) (?:%{NOTSPACE:ssl_protocol}|-) %{NOTSPACE:target_group_arn} \"%{NOTSPACE:trace_id}\"

CLBのGrok Constructorの結果です！

//image[grok_constructor08][CLB Grok Constructor結果][scale=0.5]{
  Grok Constructor
//}


== logstashを動かしてみる
ここからconfファイルの作成ですが、Apacheのアクセスログと構造はほぼ一緒です。
ただ、大きく違うのがINPUTがファイルパスではなく、S3からという点です。
なので、S3をINPUTにした取り込み方法について解説していきたいと思います。
FILTERとOUTPUTに関しては、最終的なconfファイルを記載するかたちとしますー

=== Install S3 Plugin
S3をINPUTとしてログを取得するには、Pluginをインストールする必要があります。
インストール手順を以下に記載します。

//cmd{
/usr/share/logstash/bin/logstash-plugin install logstash-input-s3
Validating logstash-input-s3
Installing logstash-input-s3
Installation successful
}

また、S3にアクセスできるようにIAM Roleの設定がされていることを前提としてます。

=== logstash.conf
準備が整ったので以下にlogstash.confを記載します。

//cmd{
input {
  file {
    path => "/Users/micci/project/logstash-5.5.2/clb.log"
    start_position => "beginning"
  }
}
filter {
  grok {
    patterns_dir => ["/Users/micci/project/logstash-5.5.2/patterns/clb_patterns"]
    match => { "message" => "%{CLB_ACCESS_LOG}" }
  }
  date {
    match => [ "date", "ISO8601" ]
    timezone => "Asia/Tokyo"
    target => "@timestamp"
  }
  geoip {
    source => "client_ip"
  }
  mutate {
    remove_field => [ "date", "message", "path"m ]
  }
}
output {
  stdout { codec => rubydebug }
}
}

実行結果です。

//cmd{
{
              "received_bytes" => 0,
                     "request" => "http://www.example.com:80/",
                  "backend_ip" => "10.0.0.1",
                  "ssl_cipher" => "-",
                "backend_port" => 80,
                  "sent_bytes" => 29,
                 "client_port" => 2817,
         "backend_status_code" => 200,
                    "@version" => "1",
                        "host" => "122x208x2x42.ap122.ftth.ucom.ne.jp",
                         "elb" => "my-loadbalancer",
                   "client_ip" => "5.10.83.30",
     "backend_processing_time" => "0.001048",
                  "user_agent" => "curl/7.38.0",
                "ssl_protocol" => "-",
                       "geoip" => {
              "timezone" => "Europe/Amsterdam",
                    "ip" => "5.10.83.30",
              "latitude" => 52.35,
        "continent_code" => "EU",
             "city_name" => "Amsterdam",
          "country_name" => "Netherlands",
         "country_code2" => "NL",
         "country_code3" => "NL",
           "region_name" => "North Holland",
              "location" => {
            "lon" => 4.9167,
            "lat" => 52.35
        },
           "postal_code" => "1091",
           "region_code" => "NH",
             "longitude" => 4.9167
    },
             "elb_status_code" => "200",
                        "verb" => "GET",
     "request_processing_time" => "0.000073",
                     "urihost" => "www.example.com:80",
    "response_processing_time" => "0.000057",
                  "@timestamp" => 2015-05-13T23:39:43.945Z,
                        "port" => "80",
                       "proto" => "http",
                 "httpversion" => "1.1"
}
}

如何でしたでしょうか？
AWSのサービスに対してもログを取り込めるということがわかったのではないでしょうか(dﾟωﾟd)ｵｩｲｪｰ
この他のサービスに対してもトライして頂ければと思います！
