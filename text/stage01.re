= はじめに

ゆるふわエンジニアの@micci184ですー

世の中には様々なデータがあり、そのデータがどう活用されているかを知るために、いろんな人たちがログの可視化をしています。
ただ、ログの可視化を本格的に実施するには、何を目的としてログの可視化をしたいかを定義する必要があります。
ログを可視化したいということは、何かしらの意思決定をするための材料を与えたいということだからです。
可視化するには、対象となるログの整理を実施し、そのログを抽出、変換、取り込み、そこから、どのようなダッシュボードを作成するかも考える必要があります。
更に非機能面も加味してデザインする必要がでてきます。

ここまで本格的に取り組むとなると大変な労力と途中で挫折をしてしまう可能性があるので、
まずは、大いなる一歩目のログの抽出、変換、取り込みの楽しさに触れられればと思います。

本章では、ログの取り込みまでを手助けしてくれるLogstashの使い方や、心が折れやすいGrok Filterについて書いていきます。
と記載してますが、主にGrok Filterです！w
この本を通じてGrokの苦手意識をなくし、楽しいロギング生活を遅れることを願ってます。

あ、ここまで堅苦しく書いちゃってますが、
ここからは、ゆるふわーな感じで書きますので、よろしくお願いしますーヽ(*ﾟдﾟ)ノ

 = 環境について

以下のAWSサービスを利用してログの可視化をしちゃいます。

* Cloudfront
* Elastic Load Balancing
** Application Load Balancer
** Classic Load Balancer
** Cloudtrail
* S3

可視化するためのサーバ環境は以下の感じです。

* OS
** Amazon Linux AMI 2017.03.1 (HVM)
* ElasticStack
** Logstash 6.0(Beta)
** Elasticsearch 6.0(Beta)
** Kibana 6.0(Beta)
* Tool
** Grok Constructor

本の内容は公式ドキュメントや参考書籍などを元に動作確認を行ってはいますが、
この本の情報を用いた開発・制作・運用に対して発生した全ての結果に対して責任は負うことはできません。
必ずご自身の環境でよく検証してから導入をお願いします。

みっちー(Github:@micci184)S
