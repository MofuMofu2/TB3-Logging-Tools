= fluentdでApacheのログをパースする

#@#Logstashが先に来る場合

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
