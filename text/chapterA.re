= データ収集ツールとは

　データ収集ツールとは、ファイルやアプリケーションなどからデータを集め、別のサービスに転送する
ソフトウェアのことです。下記に示すような用途として使われています。

* サービス監視用途
** アプリケーションやサーバーのログの収集
** サーバーの負荷情報の収集
* データ収集用途
** Webから収集したデータをストレージへ転送
** アプリケーションやストレージから特定のデータを収集
* サービス間のデータ転送

　データ収集ツールは単体で使用できます。しかし、監視用途の場合
データ可視化ツールと一緒に使用されることが多いです。代表的なサービス例としては、Kibana,Grafanaなどがあります。
この本では詳しく扱いませんが、Kibanaによるデータの可視化を行った画面を一例として載せておきます。

///image[example_kibana][データ可視化ツール（Kibana）の例][scale=0.5]{
  KibanaのDashboardを挿入
//}
