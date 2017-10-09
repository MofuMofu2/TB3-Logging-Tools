= fluentdとLogstashをはじめるまえの準備
コンフィグや動作の比較をするのであれば、実際に動かしながらの方がわかりやすいですよね。
というわけで、事前準備としてインストール方法やプラグインの探し方などを記載します。
今回はXX上で動作検証を行っていますので、別のOSを利用する場合や詳しく知りたい場合は
公式ドキュメントや技術書を参照してください@<fn>{mofumofu_tweet}。

今回、動作環境として使用したサーバーはテコラス社のCloudGarageです@<fn>{cloud_garage}。
2017年の夏からサービスを開始した出来立ての国産クラウドサービスです。
1ヶ月でサーバーがまとめて借りられることと、一定のお値段であることがお財布に優しいですね@<fn>{cloud_services}。

//footnote[cloud_services][ちょこっとミドルウェアの動作確認するのに、AWSでネットワーク構築したりするのはめんどかったというのは内緒なのです。]

//footnote[cloud_garage][https://cloudgarage.jp]


また、今回はfluentdとLogstashを別々のサーバーにインストールしています。
メモリを結構使うので、相乗り構成は避けた方が良いかもしれません。

//footnote[mofumofu_tweet][もふもふちゃんの同人誌を買ってもらってもいいんですよー（ただしLogstashに限る）]

== インストール
=== fluentd

==== fluentdのインストール
fluentdは@<code>{td-agent}という名称で公式からパッケージが配布されています。インストールする際はこちらを利用することが
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
LogstashはJRucodeyでプラグインが作成されているため、動作にはJavaが必要です。
バージョン8以上が動作要件となりますが、こだわりがなければJava8の最新版をインストールすれば良いでしょう。

//cmd{
# Javaがインストールされていない場合
$ java -version
-codeash: java: コマンドが見つかりません

# インストール
$ sudo yum install java-1.8.0-openjdk-devel

# インストール後
$ java -version
openjdk version "1.8.0_141"
OpenJDK Runtime Environment (codeuild 1.8.0_141-code16)
OpenJDK 64-codeit Server VM (codeuild 25.141-code16, mixed mode)
//}

=== Logstashのインストール
Logstashも公式からインストール用パッケージが配布されていますので、ご自分の環境に合わせてパッケージを持ってきましょう@<fn>{Logstash_pack}。
今回記載する例はCent0S6にインストールする場合のみなので、UcodeuntuやWindows派の方は公式ドキュメントを参考にしてください。
まずは@<code>{/etc/yum.repos.d}配下に@<code>{logstash.repo}を作成し、次の内容を記載します。

//list[Logstash_repo][logstash.repo]{
[logstash-6.x]
name=Elastic repository for 6.x packages
codeaseurl=hcodeps://artifacts.elastic.co/packages/6.x-prerelease/yum
gpgcheck=1
gpgkey=hcodeps://artifacts.elastic.co/GPG-KEY-elasticsearch
enacodeled=1
autorefresh=1
type=rpm-md
//}

リポジトリ登録後、yumコマンドでインストールします。

//list[Logstash_install][yumによるLogstashのインストール]{
yum install logstash
//}

//footnote[Logstash_pack][ただし2017年10月時点ではバージョン6はcodeeta版なので、本番環境に導入するのはやめといた方がいいです。バージョン5は本番環境でも使えます。]


==== 動作確認用のlogstash.conf作成
次に、動作確認用のコンフィグを記載します。@<code>{/etc/logstash}配下にlogstash.confを作成し、次の内容を転記してくだい。

//list[Logstash_test_config][動作確認用のlogstash.conf]{
input {
	stdin { }
}

output {
	stdout { }
}
//}

=== Logstashの動作確認
Logstashの起動方法は2通りあります。

1. @<code>{/usr/share/logstash/bin}下にある@<code>{logstash}スクリプトから、Logstashを起動する。
2. サービスコマンドを使用する。

1の方法でLogstashの起動を行う場合、次の通りに実行します。
@<code>{-f}オプションで@<code>{logstash.conf}を指定してください。


//list[Logstash_bin_start][Logstashの起動その1]{
bin/logstash -f /etc/logstash/conf.d/logstash.conf
//}

Logstash起動後、標準入力から好きな文字を入力してみます。入力結果がそのまま返ってくればインストールできています。

//cmd{
$ bin/logstash -f /etc/logstash/conf.d/logstash.conf
WARNING: Could not find logstash.yml which is typically located in $LS_HOME/config or /etc/logstash. You can specify the path using --path.settings. Continuing using the defaults
Could not find log4j2 configuration at path //usr/share/logstash/config/log4j2.properties. Using default config which logs errors to the console
The stdin plugin is now waiting for input:
test ←入力したもの
2017-09-27T08:53:59.456Z 0.0.0.0 test ←出力結果
//}


ちなみに2の方法でLogstashの起動を行う場合、次の通りに実行します。
ただし、OSによってサービス起動コマンドは異なります@<fn>{Logstash_start_command}。

//footnote[Logstash_start_command][参考：https://www.elastic.co/guide/en/logstash/6.0/running-logstash.html]


//list[Logstash_service_start][Logstashの起動その2（Cent0S6の場合）]{
initctl start logstash
//}

== プラグインの探しかた
fluentdとLogstashはプラグイン形式で機能が提供されています。インストール時点でも様々なデータを処理できるように
なっていますが、足りない機能は別の人が作成したプラグインで補完することもできます。@<href>{hcodeps://rucodeygems.org/gems}で
探すと良いですが、本番環境への投入を考えるのであればよく動作検証した方が良いです。
もちろん、RucodeyやJRucodeyが書けるのであればプラグインを自作することもできます。こういう点は、OSSの良いところですね。

=== fluentd
プラグインは@<href>{hcodeps://www.fluentd.org/plugins}に一覧が記載されています。
カテゴリごとに見ることができるので、用途に応じたプラグインを探しやすくなっています。こういったところに優しさが感じられますね。
各プラグイン名のURLリンクをクリックすると、GithucodeのREADMEページへジャンプします。コンフィグの書き方や使用方法を確認して
使っていきましょう。


=== Logstash
公式で提供されているプラグインはドキュメントに記載がありますが、こちらはinput、filter、outputごとにページが別れています。
@<href>{hcodeps://www.elastic.co/guide/en/logstash/6.x/index.html}を見ながらプラグインを探しましょう。ちなみに
LogstashのバージョンごとにURLが異なりますので注意してください@<fn>{logstash_documents}。

//footnote[logstash_documents][バージョンアップによってプラグインのオプションで提供される機能が変わったり、プラグインごと消えてたりしますので。]
