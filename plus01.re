= fluentdでApacheのログをパースする

では、これをfluentdで取得する場合、どのように記述すれば良いのでしょうか。
データの取得元はLogstashのときと同様に、@<code>{httpd_access.log}とします。

== データの読み取り部を指定

まず、@<code>{<source>}を作成します。

//list[plus-list01][httpd_access.logを取得するための<source>]{
<source>
  @type tail
  path /var/log/apache/httpd_access.log
  pos_file /var/log/td-agent/tmp/apache.log.pos
  format json
  tag apache
</source>
//}


@<code>{httpd_access.log}はファイルなので、@<code>{@type}は@<code>{tail}とします。
@<code>{path}にはログファイルの配置先を指定しています。ファイルはjson形式なので@<code>{format}は
@<code>{json}を指定しています。

== データの出力先を指定

次に@<code>{match}を作成します。

//list[plus-list02][パースしたログの出力先を設定]{
<match>
	@type stdout
	output_type hash
</match>
//}

@<code>{@type}はLogstashと合わせるため、@<code>{stdout}としています。
ファイル出力を希望する場合、@<code>{file}を設定し、@<code>{path}オプションを追加してください。

@<code>{output_type}は標準出力の形式を指定するオプションです。@<code>{json}と@<code>{hash}の2種類が設定でき、
@<code>{hash}を設定すると、Rubyのハッシュオブジェクト形式で標準出力されます。

では、この時点でデータを出力してみるとどうなるでしょうか。
#@#コンフィグ名合わせる

//list[plus-list03][現時点のfluentd.conf]{
<source>
  @type tail
  path /var/log/apache/httpd_access.log
  pos_file /var/log/td-agent/tmp/apache.log.pos
  format json
  tag apache
</source>
<match>
	@type stdout
	output_type hash
</match>
//}

fluentdを起動すると、次のように出力されます。

//cmd{
出力結果を記載
//}
