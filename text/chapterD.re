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

//image[Logstash_config][Logstashのコンフィグ構造][scale=0.3]{
//}

特徴として、コンフィグにはデータ処理の内容しか記載しないこと・各データに対するタグ付けが任意であることが挙げられます。
Logstash自体のソフトウェアに関する動作設定は、全て@<code>{/etc}配下にある設定ファイル（@<code>{logstash.yml}）で行います。
ここでは@<code>{logstash.conf}のみに焦点を絞って中身をみていきます。

=== データの読み取り部（input）
=== データの加工部（filter）
=== データの送付部（output）
=== その他特徴など
