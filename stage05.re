= まだいくよ！Grok
== 今度は何を取り込む？
前章ではApacheのアクセスログを取り込めるようになりました。
そこで既存のGrokPatternだけではどうにもならない系のログを対象にGrokしていきたいと思います。

ちなみにですが、FireWallという括りでCISCOのASAのGrokPatternが用意されているものもあります。
なので、イベントIDと照らし合わせてすでにあるものは利用する形がよいです。
ただ、今回取り上げるのは、先にも伝えたようにGrokPatternにないものを選定していますが、
使えるものは使っていきたいと思います。

今回は、CISCOのファイアウォール製品であるASAのログを取り込みたいと思いますー
やっぱり企業を守っているファイアウォールがどんなログを出しているか気になりますよね！？（薄っぺらいw）

といことで今回は以下のログを対象にしたいと思います。
 ※IPアドレスは、適当なプライベートIPアドレスを割り当てています

* Jun 20 10:21:34 ASA-01 : %ASA-6-606001: ASDM session number 0 from 192.168.1.254 started
* Jun 20 11:21:34 ASA-01 : %ASA-6-606002: ASDM session number 0 from 192.168.1.254 ended

ということで、いつも通りに以下のログ取り込みフローで進めたいと思います！

<ログ取り込みフロー>
1. ログフォーマットを調べる
2. フィールド定義
3. GrokPatternをつくる
4. Grok Constructorでテスト
5. logstashのconfファイルで動かしてみる

=== ログフォーマットを調べる
まずは、ログフォーマットを調べる！
Ciscoさんは丁寧にログフォーマットを掲載してます。

@<href>{https://www.cisco.com/c/en/us/td/docs/security/asa/syslog/b_syslog.html}

...よく見るとわかりますが、数が多いw
まぁ、Ciscoって世界最大ですからねー(*ﾟ∀ﾟ)/ｱｯｹﾗｶﾝ

まず該当するログフォーマットを探す方法ですが、以下のログに"%ASA-6-606001"という記載がありますので、このメッセージNo.の"606001"で検索することができます。

//image[cisco_grok01][CiscoASAでログ#01][scale=0.5]{
  cisco_log検索画面
//}

@<href>{https://www.cisco.com/c/en/us/td/docs/security/asa/syslog/b_syslog/syslogs6.html}

このログフォーマットは以下のようになっています。

* %ASA-6-606001: ASDM session number number from IP_address started
* %ASA-6-606002: ASDM session number number from IP_address ended

ASDMのセッションを開始した時と終了した時に出力するログです。
ASDMはWebベースの管理インターフェースを提供するツールです。

=== フィールド定義していくよ！
ではでは、フィールド定義ですが、左から順にやっていきます。
先ほど見たログフォーマットには、タイムスタンプとASA-01というのがなかったと思います。
これらは、ログイベントに関わらず必ず記載されるので、こちらも定義します。

* Jun 20 10:21:34 ASA-01 : %ASA-6-606001: ASDM session number 0 from 192.168.1.254 started
** timestamp: Jun 20 10:21:34 (date)
** hostname: ASA-01 (string)
** eventid: ASA-6-606001 (string)
** ASDM-sesion-number: 0 (long)
** src_ip: 192.168.1.254 (string)
** status: started (string)

実際のログに記載されているメッセージ内容のすべてが、フィールドにマッピングされていないことがわかります。
例えば、"ASDM session number"というメッセージに対して意味はなく、そのセッションナンバーが知りたいのです。
そのため、フィールド名に"ASDM session number"とし、値としては取り込まないようにしています。
その他の"from"も同様で、どこからのIPアドレスかを知りたいため、fromを取り除き、src_ip（ソースIP）というフィールドにIPアドレスを値として取り込んでいます。

次のログですが、最後の"ended"しか変わらないということがわかります。
なので、先ほどのフィールド定義を使用します。

* Jun 20 11:21:34 ASA-01 : %ASA-6-606002: ASDM session number 0 from 192.168.1.254 ended

=== GrokPatternをつくるよ
それでは、GrokPatternを作っていきます。

* Jun 20 10:21:34 ASA-01 : %ASA-6-606001: ASDM session number 0 from 192.168.1.254 started

=== 共通部分
タイプスタンプとホスト名、イベントIDはすべてのログに入るメッセージのため、共通部分とします。
それでは、タイムスタンプとホスト名、イベントIDに取り掛かりたいと思います！

タイムスタンプは、GrokPatternに"CISCOTIMESTAMP"を使用します。

* CISCOTIMESTAMP %{MONTH} +%{MONTHDAY}(?: %{YEAR})? %{TIME}

また、ホスト名は、ユーザが自由に付与する名前のため、柔軟性を求めて"NOTSPACE"を使用します。
また、先頭にスペースが必ず入るので"\s"を入れます。

* HOSTNAME \s%{NOTSPACE:hostname}

イベントIDは、GrokPatternに用意されていないので、自分で作成します。
自分でGrokPatternを作成する場合は以下のように作成します。

* (?<hostname>ASA-\d{1}-\d{6})
** GrokPatternを作成したい場合は、(?)で括り、<>内にフィールド名を任意に付与します
** それ以降（ここでいうASAから始まる正規表現）にフィールドに入れたい正規表現を記載します

上記のように作成することで好きなGrokPatternを作成することができます。
これをCustomPatternといいます。

== 固有部分
ここからはイベント毎に異なる固有部分のGrokPatternを作っていきます。
共通部分を取り除いた部分の以下が対象ですね。

* : ASDM session number 0 from 192.168.1.254 ended

=== ASDMセッションNo.
フィールド定義で記載した通りですが、ASDMセッションNo.をフィールドとし、値が取得できればよいわけです。
なので、以下のようになります。

* ASDM session number(?<ASDM-sesion-number>\s[0-9]+)

これも見てわかる通り、CustomPatternで作成しています。
一つ一つみていくと(?)の外に"ASDM session nubber"がありますね。
これは、"ASDM session number"をマッチしても値は取得しない場合に使うやり方です。
なので、隣の"(?<ASDM-session-number>\s[0-9]+)"CustomPatternで取得した値が"ASDM-session-number"というフィールに入るかたちです。
正規表現部分は、"\s"のスペースと"[0-9]"の数字複数並んでも対応できるように"+"を使用しているかたちです。

最終的に先頭の":"とスペースも含むので以下な感じになりますー

* :\sASDM session number(?<ASDM-session-number>\s[0-9]+)

=== ソースIPアドレス
これはApacheのアクセスログでやったことと一緒ですね。
まぁ、IPアドレスのGrokPatternのように他にも確立されているものは積極的に使っていきましょう。
あるものは使う、なければCutomPattern！
当たり前のことですが、GrokPatternにあるのにCustomPatternで頑張ってしまうという経験をしてしまう時期がくるかもなのでw

* from 192.168.1.254

これは、フィールド定義で説明したようにソースIPなので、GrokPatternの"IP"を使用し、不要な部分を取り除く必要があります。
スペースと"from"が不要なのでGrokPatternの外側に出しますが、一つの文字列とするため()で囲います。
結果、以下の感じになりますー

* (\sfrom\s%{IP:src_ip})

=== ステータス
最後にセッションステータスを表す"started"ですね。
これは、CustomPatternで対応します。
先ほどのソースIPとの間にスペースがあるので"\s"を入れます。
また、"started"は文字列なので"\b"を入れて以下な感じです。

* \s(?<session>\bstarted)

ただ、もう一つのイベントID"606002"ですが、ステータスがendedしか変わりません。
なので、先ほどのステータスに"started""ended"のどちらかを選択できるようにします。

* \s(?<session>\bstarted|\bended)

"|"パイプを入れることで選択できるようになります。
これで整ったので、GrokConstructorでテストをしてみたいと思います。

== Grok Constructorで全体テスト
最終形は以下です。

* %{CISCOTIMESTAMP:date}\s%{NOTSPACE:hostname}\s: %(?<EventID>ASA-\d{1}-\d{6}):\sASDM\ssession\snumber(?<ASDM-session-number>\s[0-9]+)(\sfrom\s%{IP:src_ip})\s(?<session>\bstarted|\bended)

//image[grok04][ASA Grok結果#03][scale=0.5]{
  Grokパワポ
//}







