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
