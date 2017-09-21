= 比較その2：コンフィグの形式
fluentdとLogstashのコンフィグ形式は、かなり違いがあります。
ここではデータの読み取り・加工・出力に分けて、コンフィグの比較をしてみたいと思います。
なお、今回のコンフィグ例ではTwitterから取得したjsonデータをcsvに加工する、というユースケースを想定しています。
Twitterから取得できるjsonの例をあげます@<fn>{example_twitter}。

//footnote[example_twitter][これはもふもふのTweetをいくつか取得したものです。大したことはつぶやいていませんね。]

//list[twitter_json][Twitterから取得したつぶやきデータの例]{
  jsonを挿入
//}

== fluentd

=== コンフィグの構造

=== データの読み取り部（）
=== データの加工部（）
=== データの送付部（）
=== その他特徴など

== Logstash

=== コンフィグの構造
対するLogstashのコンフィグ（@<code>{logstash.conf}）の構造は、次のようになっています。
また、コンフィグは@<code>{/etc/logstash/conf.d}配下に置きます。名称の最後を@<code>{.conf}にして配置します。

//image[Logstash_config][Logstashのコンフィグ構造][scale=0.3]{
//}

特徴として、コンフィグにはデータ処理の内容しか記載しないこと・各データに対するタグ付けが任意であることが挙げられます。
Logstash自体のソフトウェアに関する動作設定は、全て@<code>{/etc}配下にある設定ファイル（@<code>{logstash.yml}）で行います。
ここでは@<code>{logstash.conf}のみに焦点を絞って中身をみていきます。

=== データの読み取り部（input）
Logstashがどのデータを読み取るのか、この部分で指定します。必須設定項目はプラグインごとに異なり、共通のものはありません。
取得したいデータに合わせてどのプラグインを使用するか決定し、使用方法を確認する必要があります。

今回はサーバー内に配置されたjsonファイルを取得するため、@<code>{file}プラグインを使用します（@<href>{https://www.elastic.co/guide/en/logstash/6.0/plugins-inputs-file.html}）。

//list[input_json_logstash][inputプラグインの実装例]{
  jsonを取得するためのプラグインを記載
//}

=== データの加工部（filter）
読み取ったデータをどのように加工するのかを指定します。こちらも処理内容によってプラグインが異なります。

//list[filter_json_logstash][filterプラグインの実装例]{
  jsonを加工するためのプラグインを記載
//}



=== データの送付部（output）
=== その他特徴など
==== 複数のlogstash.confは両立できるが、注意が必要
Logstashは取得するデータごとに@<code>{logstash.conf}を作成し、@<code>{/etc/logstash/conf.d}配下に置いて動作させることができます。

//list[etc_logstash_conf][複数のコンフィグを作成し配置した例]{
  ディレクトリの中を入れる
//}

ただし、Logstashは動作時にコンフィグを全て結合し動作するという特性があります。
なので、各コンフィグ内では読み取ったデータにtag付けを行う処理を入れておかないと意図した処理ができない場合があります。

//list[example_some_logstash_conf][複数のコンフィグを動作させる場合：tag付けを行わなかった場合]{


//}

このコンフィグを動作させると、データは次のように出力されます。

//cmd{
lsした後に起動＆データ重複されていることを確認
//}

なので、Logstashでも複数のデータを同じプロセスで取得する場合、tagをつけてデータを別々に管理する必要があります@<fn>{some_logstash_conf}。

//footnote[some_logstash_conf][コンフィグを1つにするか複数にするか、どちらがいいのかはElastic社の人も悩みどころらしいです。
個人的には複数に分けて中身を短くする方が管理しやすそうに思いますが、ちょっとめんどいですね。長くてもめんどいのは一緒ですが。]
