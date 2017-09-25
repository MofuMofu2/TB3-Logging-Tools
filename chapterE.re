= 比較その3：実際に動かしてみる
今度は実際にfluentdとLogstashを動作させてみます。
同じデータを処理させたとき、出力結果やサーバーの負荷がどのようになるかを比較してみました。

== 動作環境
今回、動作環境として使用したサーバーはテコラス社のCloudGarageを利用しました。
2017年の夏からサービスを開始した出来立ての国産クラウドサービスです。
1ヶ月でサーバーがまとめて借りられることと、一定のお値段であることがお財布に優しいですね@<fn>{cloud_services}。

//footnote[cloud_services][ちょこっとミドルウェアの動作確認するのに、AWSでネットワーク構築したりするのはめんどかったというのは内緒なのです。]


また、ソフトウェア動作時のサーバーにかかる負荷を調べてみたかったので、
軽量データジッパーのBeats（@<href>{https://www.elastic.co/jp/products/beats}）を使用しました。
Elastic社が提供するOSSで、特定のデータを加工し送付できるソフトウェアです。
今回はサーバーに対する負荷情報を収集することに特化したMetricbeatを使用しました。
MetricbeatのデータはElasticsearch@<fn>{Elasticsearch}に入れ、Kibana@<fn>{Kibana}で可視化しています。

//footnote[Elasticsearch][Elastic社が提供する検索エンジンです。Githubとかでも使われています。https://www.elastic.co/jp/products/elasticsearch]

//footnote[Kibana][こちらもElastic社が提供するデータ可視化ツールです。グラフがみやすい。https://www.elastic.co/jp/products/kibana]



サーバー構成図を示しますので、参考にしてみてください。

== fluentdでjsonを読み取る

=== fluentdの起動

=== 読み取ったデータと出力結果

=== fluentd動作サーバーにかかった負荷

== Logstashでjsonを読み取る

=== Logstashの起動

=== 読み取ったデータと出力結果

=== fluentd動作サーバーにかかった負荷
