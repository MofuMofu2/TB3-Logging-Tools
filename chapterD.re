= 比較その2：コンフィグの形式
fluentdとLogstashのコンフィグ形式は、かなり違いがあります。
ここではデータの読み取り・加工・出力に分けて、コンフィグの比較をしてみたいと思います。
なお、今回はjsonデータをcsvに加工・出力する、という事例に沿ってコンフィグを記載しています。


ちなみに、jsonファイルは@<href>{http://www.databasetestdata.com/}を利用して作成しました。
json,csv,xmlから拡張子を選択しデータの中身を項目を指定すれば、自動でテストデータが生成されます。

//list[test_json][今回使用したjsonファイル]{
{"Full Name": "Sheldon Tillman","Country": "Peru","Created At": "1980-11-13T17:37:36.702Z","Id": 0,"Email": "Carolanne_Kub@emmalee.name"}
//}

この章で扱う入力データの例は、全て@<list>{test_json}を使用しています。

== fluentd

=== コンフィグの構造
fluentdのコンフィグ（td-agentを用いてインストールした場合は@<code>{td-agnet.conf}）の構造は、次のようになっています。
また、コンフィグは@<code>{/etc/td-agent/td-agnet.conf}にデフォルトで配置されているので、こちらを編集していきます。


ただし、fluentdは@<code>{source}→@<code>{filter}→@<code>{match}の順に処理を行います。
コンフィグの記述位置と処理の順序は関係ありません。

=== データの読み取り部（source）
fluentdがどのデータを読み取るのかをこのディレクティブで指定します。データの入力元は複数指定することが可能です。
@<code>{<source>}と@<code>{</source>}の間に設定を記述します。この書式は他のディレクティブでも同様です。

//list[fluentd_source][source部分の実装例]{
<source>
  @type tail
  path /var/log/json/*.json
  pos_file /var/log/td-agent/tmp/json.log.pos
  format json
  tag json
</source>
//}

@<code>{@type}は使用するプラグイン名です。今回はサーバー内のjsonファイルを取得することが目的ですので、
@<code>{tail}プラグインを選択しました。@<code>{tail}プラグインは、Linuxコマンドに例えると@<code>{tail -F}と同じような働きをします@<fn>{fluentd_tail}。
具体的には指定したファイルを監視し、ファイルが更新されるとfluentdがデータを取得する、という動きをします。

//footnote[fluentd_tail][https://docs.fluentd.org/v0.12/articles/in_tail]

@<code>{path}はどのファイルを取得対象とするか設定する箇所です。実装例では@<code>{/var/log/json}に存在するjsonファイルを全て取得する設定になっています。
（当たり前かもしれませんが）@<code>{path}は必須設定です。

@<code>{pos_file}はファイルをどこまで読み取ったか記録しておく@<code>{.pos}ファイルを
どこに配置するのか設定しています。@<code>{path}と違って任意設定ですが、設定することを推奨されています。

@<code>{format}は指定した形式にデータを整形します。こちらは必須設定です。

@<code>{tag}は取得したデータに付与するデータの名称を設定します。このtagを用いて
データの送信先の振り分けなどを行うことができます。こちらも必須設定です。

=== データの加工部（filter）
読み取ったデータをどのように加工するのか指定するプラグインです。ただし、@<code>{filter}ディレクティブの中では
tagの変更ができないため、加工後のデータに新しくtagを付与したい場合は@<code>{match}ディレクティブで専用プラグインを使用します。

//list[fluentd_filter][filter部分の実装例]{
//}


=== データの送付部（match）
処理が終わったデータをどこに送付するか指定するディレクティブです。
プラグインを複数記述すれば、複数の出力先へデータを送付することが可能です。

@<list>{fluentd_match}

=== その他特徴など

== Logstash

=== コンフィグの構造
対するLogstashのコンフィグ（@<code>{logstash.conf}）の構造は、次のようになっています。
また、コンフィグは@<code>{/etc/logstash/conf.d}配下に置きます。名称の最後を@<code>{.conf}にして配置します。

特徴として、コンフィグにはデータ処理の内容しか記載しないこと・各データに対するタグ付けが任意であることが挙げられます。
Logstash自体のソフトウェアに関する動作設定は、全て@<code>{/etc}配下にある設定ファイル（@<code>{logstash.yml}）で行います。
ここでは@<code>{logstash.conf}のみに焦点を絞って中身をみていきます。

=== データの読み取り部（input）
Logstashがどのデータを読み取るのか、この部分で指定します。必須設定項目はプラグインごとに異なり、共通のものはありません。
取得したいデータに合わせてどのプラグインを使用するか決定し、使用方法を確認する必要があります。

今回はサーバー内に配置されたjsonファイルを取得するため、@<code>{file}プラグインを使用します（@<href>{https://www.elastic.co/guide/en/logstash/6.0/plugins-inputs-file.html}）。

//list[input_json_logstash][inputプラグインの実装例]{
input {
  file {
  path => "/var/log/json/*.json"
  }
}
//}

@<code>{file}はプラグイン名です。@<code>{path}はどこからデータを取得するか記載しています。この場合@<code>{/var/log/json}から
jsonファイルを全て取得する、という設定になっています。


=== データの加工部（filter）
読み取ったデータをどのように加工・整形するのか指定します@<fn>{logstash_filter}。

//footnote[logstash_filter][ここでの加工・整形とは、データの新規追加・削除・データ型の変更・データ分割などを指します。]

プラグインごとにできることが異なりますので、プラグイン同士を組み合わせてコンフィグを作成します。
基本は取得するデータ形式に応じたプラグインでデータをいい感じに分割し、その後データを煮るなり焼くなりして加工する、という感じです@<fn>{filter_make}。

//footnote[filter_make][こういうの、がらがらぽんってできると楽だと思うんですけど世の中厳しい。]

例えば、今回のようにjsonを取得した場合、@<code>{json}プラグインを使用してjsonを分割します。

//list[filter_json_logstash][filterプラグインの実装例]{
  filter {
    json {
     source => "message"
   }
  }
//}

@<code>{json}はプラグイン名称です。オプション@<code>{source}はどのfield内のデータを加工するか指定する部分です。
取得したデータはデフォルトだと@<code>{message}というfieldに入るので、@<code>{message}内のデータを加工する
指定を行なっています。

=== データの送付部（output）
処理が終わったデータをどこに送付するか指定するプラグインです。データの送付以外にも、別フォーマットへの加工が可能です。
例えばjsonで取得したデータをcsvに変換して保存、といったことができます。
また、複数の宛先にデータを送付したい場合、プラグインを複数記述することで実現可能です。

//list[output_json_logstash][outputプラグインの実装例]{
output{
  csv {
    fields => [
      "Full Name", "Country", "Created At", "Id", "Email"
    ]
    path => "/var/log/csv/test.csv"
  }
}
//}

@<code>{csv}はプラグイン名称です。このプラグインを使うと、データをcsvに変換できます。
@<code>{fields}で指定した文字がcsvの列にあたります。列に一致するデータが見つからない場合、csvは空欄になります。
@<code>{path}は生成したcsvの出力場所を指定します。日付方法をファイル名にすることも可能です。


=== コンフィグ例と出力結果
今まで紹介した、Logstashのコンフィグと出力結果例を記載します。

//list[Logstash_example][Logstashのコンフィグ例まとめ]{
input {
  file {
  path => "/var/log/json/*.json"
    }
}
filter {
  json {
   source => "message"
 }
}
output{
  csv {
    fields => [
      "Full Name", "Country", "Created At", "Id", "Email"
    ]
    path => "/var/log/csv/test.csv"
  }
}
//}

入力データは@<list>{test_json}を参照してください。

//list[Logstash_output][test.csvの出力例]{
Sheldon Tillman,Peru,1980-11-13T17:37:36.702Z,0,Carolanne_Kub@emmalee.name
//}

jsonデータがcsv形式に加工されていることがわかります。

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

なので、Logstashでも複数のデータを同じプロセスで取得する場合、tagをつけてデータを別々に管理する必要があります。
ちなみに、コンフィグを1つにするか複数にするか、どちらがいいのかはElastic社の人も悩みどころらしいです。
個人的には複数に分けて中身を短くする方が管理しやすそうに思いますが、ちょっとめんどいですね。長くてもめんどいのは一緒ですが。

==== add_fieldとremove_field
@<code>{add_field}とは、多くのfilterプラグインについているオプション機能です。
取得したデータに1つfieldを追加できます（複数fieldの追加も可能です）。
@<code>{remove_field}は逆で、指定したfieldを削除することができます。
そのfieldに入っているデータは全部抹消されます。


fieldの数を意図的に操作する、というのは他のElastic製品@<fn>{Elastic_Stack}（特にElasticsearch）との連携を
意識しているものと思われます。Elasticsearchでデータを検索するときや、Kibanaでグラフを描画するときは
field単位でデータを引っ張ってくる感じになるので、意図的にfield数を操作できるようにしているような感じがします。



//footnote[Elastic_Stack][Elastic社製のOSS群はElastic Stackといいます。でもElasticsearch,Logstash,KibanaでELKって言ってる人の方が多いですね。そっちの方が短いし。]

=== Logstashでお便利なプラグイン

Logstashを運用する上でよく出てくるプラグインを紹介します。

==== metrics（filter）
取得したデータの数値型を変換できます。Logstashのバージョンが上がるほど、変換できる型が充実しています。
公式ドキュメントをみると、その充実っぷりをご堪能いただけます。

==== grok（filter）
正規表現を使って、データを細かく分割できるプラグインです。例えばログに含まれるタイムスタンプを
日付データとして使いたいときなどに役立ちます。
○章以降で@micci184さんが使い方を紹介していますので、詳しくはそちらを参照ください。

==== mutate（filter）
指定した条件に一致するデータを変更できます。
ただし、ある程度fieldを分割していないと狙った部分だけ変換、という処理がしにくいので
前に紹介した@<code>{grok}filterをいかに活用し、データを分割できるかが勝負の分かれ目になります。

==== stdout(output)
動作確認時にも使用した@<code>{stdout}プラグインは、加工後のデータを標準出力させることが可能です。
serviceコマンドでプロセス起動を行うと、@<code>{logstash.stdout}という名称で標準出力内容が保存されます。
動作確認時に使用すると便利です。
