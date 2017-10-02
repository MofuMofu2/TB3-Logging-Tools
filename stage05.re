= AWSサービスのログを取り込むよ！
== 何を取り込むか
AWSサービスはログを出力する機能をもったサービスがあります。
そのなかでも今回は以下のサービスのログに対してLogstashでINPUT・FILTER・OUTPUTするところまでを実施したいと思いますー

* ELBアクセスログ
* VPC FlowLogs

この他にもCloudtrailやS3などもログを出力し、構造化することが可能です
ページ数の制限により記載できませんが、どこかでお披露目できればと考えてますヽ(*ﾟдﾟ)ノ

== ELBのログを取り込むよ！
第4章で説明したログ取り込みフローに乗っ取って進めたいと思います。

=== ログフォーマットを調べる
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


=== フィールド定義
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
** method: GET (string)
** proto: http (string)
** urihost: www.example.com:80 (string)
** uripath: - (string)
** httpversion: 1.1 (string)
** user_agent: curl/7.38.0 (string)
** ssl_cipher: - (string)
** ssl_protocol: - (string)
