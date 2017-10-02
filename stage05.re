= AWSサービスのログを取り込むよ！
== 何を取り込むか
AWSサービスはログを出力する機能をもったサービスがあります。
そのなかでも今回は以下のサービスのログに対してLogstashでINPUT・FILTER・OUTPUTするところまでを実施したいと思いますー

* ELBアクセスログ
* VPC FlowLogs

AWSサービスのログを取り込むイメージです。

//image[aws_log01][AWS Log取得構成#01][scale=0.5]{
  Grokパワポ
//}

この他にもCloudtrailやS3などもログを出力し、構造化することが可能です
ページ数の制限により記載できませんが、どこかでお披露目できればと考えてますヽ(*ﾟдﾟ)ノ



== ELBのログを取り込むよ！
第4章で説明したログ取り込みフローに乗っ取って進めたいと思います。

== ログフォーマットを調べる
ELBのログフォーマットを調べたいと思います。
あ、前提としてELBのロギングは有効化されていることとします！
もし設定されていない方は、公式ドキュメントを確認頂ければと思いますー

公式ドキュメントにアクセスログのフォーマットが記載されています。
(Classic Load Balancerアクセスログ)[http://docs.aws.amazon.com/ja_jp/elasticloadbalancing/latest/classic/access-log-collection.html]

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
サンプルログは、先ほどのリンクのAWS公式ドキュメントから使ってますー

* 2015-05-13T23:39:43.945958Z my-loadbalancer 192.168.131.39:2817 10.0.0.1:80 0.000073 0.001048 0.000057 200 200 0 29 "GET http://www.example.com:80/ HTTP/1.1" "curl/7.38.0" - -
** timestamp: 2015-05-13T23:39:43.945958Z (date)
** elb: my-loadbalancer (string)
** client_ip: 192.168.131.39 (string)
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

== GrokPatternをつくるよ
前の章でやったようにGrokPatternを作っていきましょう！
実は、AWSのELBは、GrokPatternが用意されているのです。
ただね。
それを使う！だけじゃGrok芸人にはなれんのですよ！
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
上記のClient同様です！
が、しかし。
バックエンドから応答がない場合は、"-"となるため、|で"-"も記載します。

* (?:%{IP:backend_ip}:%{INT:backend_port:int}|-)

=== リクエストタイム3兄弟
これらすべてGrokPatternの"NUMBER"を使用し、応答がなかったように|で"-1"も記載します。

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

=== Grok Constructorでテスト
個々のテスト結果は省いてますが、慣れるまでは一つ一つクリアしていってください！
あ！ちなみに、今回作成したGrokPattern名がELBではなくCLBなのは、Application Load Balancer（以下、ALB）と区別するためです。
ALBとCLBでは、ログフォーマットが若干違うため、区別してます。
ALB版も合わせてGrokPatternを記載しますー

* CLB_ACCESS_LOG %{TIMESTAMP_ISO8601:date} %{NOTSPACE:elb} (?:%{IP:client_ip}:%{INT:client_port:int}) (?:%{IP:backend_ip}:%{INT:backend_port:int}|-) (?:%{NUMBER:request_processing_time}|-1) (?:%{NUMBER:backend_processing_time}|-1) (?:%{NUMBER:response_processing_time}|-1) (?:%{INT:elb_status_code}|-) (?:%{INT:backend_status_code:int}|-) %{INT:received_bytes:int} %{INT:sent_bytes:int} \"%{ELB_REQUEST_LINE}\" \"(?:%{DATA:user_agent}|-)\" (?:%{NOTSPACE:ssl_cipher}|-) (?:%{NOTSPACE:ssl_protocol}|-)
* ALB_ACCESS_LOG %{NOTSPACE:type} %{TIMESTAMP_ISO8601:date} %{NOTSPACE:elb} (?:%{IP:client_ip}:%{INT:client_port}) (?:%{IP:backend_ip}:%{INT:backend_port}|-) (:?%{NUMBER:request_processing_time}|-1) (?:%{NUMBER:backend_processing_time}|-1) (?:%{NUMBER:response_processing_time}|-1) (?:%{INT:elb_status_code}|-) (?:%{INT:backend_status_code}|-) %{INT:received_bytes} %{INT:sent_bytes} \"%{ELB_REQUEST_LINE}\" \"(?:%{DATA:user_agent}|-)\" (?:%{NOTSPACE:ssl_cipher}|-) (?:%{NOTSPACE:ssl_protocol}|-) %{NOTSPACE:target_group_arn} \"%{NOTSPACE:trace_id}\"

といことで、Grok Constructorの結果です！

//image[grok_constructor07][Grok Constructorでテスト#07][scale=0.5]{
  Grokパワポ
//}

ちなみに、ALBは、以下な感じですー

//image[grok_constructor08][Grok Constructorでテスト#08][scale=0.5]{
  Grokパワポ
//}
