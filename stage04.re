= アクセスログをいい感じに取り込んでみよう
== ログを取り込むまでのステップ
ログをよしなに取り込むにはFILTERでログフォーマットに合わせて定義をする必要があります。
なので、この章ではどのようにログを取り込むかをステップを踏んで解説していきたいと思います。

1. 取り込むログフォーマットを調べる
2. Grokのお時間
3. Grok Constructorでテスト
4. logstashのconfファイルで動かしてみる

結構、ステップ面倒いなーって思う人もいるかと思いますが、一つ一つクリアしていくことが大切だと思ってます。
地味ーな作業が盛りだくさんですが、自分の思った通りにFILTERがかかった時が最高に嬉しい瞬間です！


それでは一つ一つ見ていきたいとおもいまするー

== 取り込むログフォーマットを調べる

今回取り込むログは、Apacheのログフォーマットがcommonですね。
Apacheのサイトからログフォーマットについて調べると色々と記載されてます。

Apacheのアクセスログのログフォーマットは以下な感じです。


* LogFormat "%h %l %u %t \"%r\" %>s %b" common
** %h: サーバへリクエストしたクライアントIP
** %l: クライアントのアイデンティティ情報ですが、デフォルト取得しない設定になっているため、”-”（ハイフン）で表示される
** %u: HTTP認証によるリクエストしたユーザID（認証していない場合は、"-"）
** %t: サーバがリクエストを受け取った時刻
** \"%r\": メソッド、パス、プロトコルなど
** %>s: ステータスコード
** %b: クライアントに送信されたオブジェクトサイズ（送れなかった時は、"-"）

== フィールド定義していくよ

アクセスログのログフォーマットがわかったので、フィールド名を定義していきたいと思いますー


* %hは、クライアントIPということで"clientip"
* %lは、アイデンティティ情報なので、"ident"
* %uは、認証なので、"auth"
* %tは、時刻なので"date"
* \"%r\"は、いくつかに分割したいので、メソッドは、"method"、パスは、"path"、んでHTTPバージョンは、"httpversion"
* %>sは、ステータスコードなので、"response"
* %bは、オブジェクトサイズなので、"bytes"


仮に上記がマッピングされると以下のようにいい感じになるはず！


* 5.10.83.30 - - [10/Oct/2000:13:55:36 -0700] "GET /test.html HTTP/1.0" 200 2326
** clientip: 5.10.83.30
** ident: -
** auth: -
** date: 10/Oct/2000:13:55:36 -0700
** method: GET
** path: /test.html
** httpversion: 1.0
** response: 200
** bytes: 2326

== GrokPattern

Grokは、様々なログを正規表現を駆使していい感じにフィールド分割して、マッチさせるためのプラグインです。
Grokは、GrokPatternという形であらかじめ正規表現のパターン定義が用意されているので、ふんだんに使っていきたいと思います。
ただ、GrokPatternにないものは自分で作成する必要がありますー
そこも含めて解説できればと思いますm(_ _)m

(GrokPattern)[https://github.com/elastic/logstash/blob/v1.4.2/patterns/grok-patterns]

それでは、ここからはフィールド一つ一つを見ていってGrokPatternを作成していきたいと思います。
GrokPatternを作成していくには、ログの左から順に攻略していくのが重要です。
ということを念頭において進めていきたいと思います。

ちなみにですが、そもそものGrokFilterの書き方とかはひとまず置いておきます！
後ほど、その辺は詳しく書きます。

=== ClientIP
ClientIPといことで、IPアドレスにマッチさせる必要があります。
まずは、IPアドレスにマッチさせるためのGrokPatternがすでにないかを先ほどのGrokPatternのサイトから確認します。

・・・あるではないか！（茶番劇っぽくてすまそんです）


* IPORHOST (?:%{HOSTNAME}|%{IP})

IPORHOST内に%{HOSTNAME}と%{IP}で構成されており、別に%{HOSTNAME}と%{IP}がGrokPatternとして定義されているので、それらを読み込む様になってます。
また、先ほどGrokPatternサイトで調べてみると...ありますね！


* HOSTNAME \b(?:[0-9A-Za-z][0-9A-Za-z-]{0,62})(?:\.(?:[0-9A-Za-z][0-9A-Za-z-]{0,62}))*(\.?|\b)
* IP (?:%{IPV6}|%{IPV4})

HOSTNAMEに正規表現が記載されていることがわりますね。
また、IPは、IPv6とIPv4に対応できるように構成されてます。
これも同じ様にサイトをみると正規表現で記載されていることがわかると思います。

IPORHOSTでHOSTNAMEとIPが定義されていることがわかったと思いますが、(?:)と|（パイプ）はなんぞや？と思った人もいると思いますが、
この(?:)は、文字列をマッチさせたい&&キャプチャさせたくない場合に使います（キャプチャは使用しないので説明しません）
今回でいう文字列は、%{HOSTNAME}と%{IP}に該当する文字列を指します。
また、|は、どちからが一致したらを意味します。

結果、IPORHOSTは、HOSTNAMEかつ、IPに該当するものをマッチさせます。

上記を踏まえてGrokPatternを記載すると以下な感じになります。

* %{IPORHOST:clientip}

これらを図にすると以下です。

//image[grok01][IPアドレスをGrokするイメージ図#01][scale=0.5]{
  Grokパワポ
//}

それでは、実際にGrokがマッチされるかをGrok Constructorを使って確認してみたいと思います。

== Grok Constructor
(Grok Constructor)[http://grokconstructor.appspot.com/do/match]は、作成したGrokがマッチするかをブラウザベースでテストすることが可能なツールです。
この他にも(GrokDebugger)[https://grokdebug.herokuapp.com/]やKibanaのDevToolで提供しているGrokDebuggerなどがあります。

Grok Constructorの使い方を以下の図に記載します。

//image[grok_constructor01][Grok Constructorでテスト#01][scale=0.5]{
  Grokパワポ
//}

それでは、早速先ほど作成したGrokPatternがうまい具合にマッチするか試したいと思います。

//image[grok_constructor02][Grok Constructorでテスト#02][scale=0.5]{
  Grokパワポ
//}

想定通り、clientipというフィールドに "5.10.83.30"というIPアドレスがマッチしたことがわかります。
この調子で、他のフィールドに対しても定義していきたいと思います！

=== ident
ユーザ名が付与入ってくるのと"-"がマッチできるものをGrokPatternで見ていくと、USERというGrokPatternがあるのでこちらを使用します。

* %{USER:ident}

先ほどの様に、上記のGrokPatternでGrok Constructorでテストを実施するとIPアドレスが引っかかると思います。
なので、%{IPORHOST:clientip}を含んでテストを実施してみてください。

//image[grok_constructor03][Grok Constructorでテスト#03][scale=0.5]{
  Grokパワポ
//}

=== auth
authもUserと同様の定義で良いので、GrokPatternのUSERを使用します。
また、identとauthの間もスペースがあるので\sもしくはスペースを入力する必要があります。
 ※図の記載では\sを¥sで記載してますm(_ _)m

=== date

次は、時刻ですね！
時刻のフォーマットは、"[day/month/year:hour:minute:second zone]"です。
これに当てはまるGrokPatternを探していたいと思いますー

結果、以下のGrokPatternが当てはまることがわかります。

* HTTPDATE %{MONTHDAY}/%{MONTH}/%{YEAR}:%{TIME} %{INT}

なので、こちらを使用してGrok Constructorでテストしてみたいと思います。
先ほど作成したGrok Constructorに連ねてきましょー

//image[grok_constructor04][Grok Constructorでテスト#04][scale=0.5]{
  Grokパワポ
//}

あれ？"NOT MATCHED"ですね。。
そうなんですね。じつは、%{HTTPDATE}に該当しない[]があるのです。
なので、以下の図で示している通り、[]を無効にしてあげる必要があります。
無効するためにエスケープ\（バックスラッシュ）を使用します。

//image[grok02][IPアドレスをGrokするイメージ図#02][scale=0.5]{
  Grokパワポ
//}





APACHE_COMMONLOG
%{IPORHOST:clientip} %{USER:ident} %{USER:auth} \[%{HTTPDATE:dete}\] "(?:%{WORD:method} %{NOTSPACE:path}(?: HTTP/%{NUMBER:httpversion})?|%{DATA:rawrequest})" %{NUMBER:response} (?:%{NUMBER:bytes}|-)
