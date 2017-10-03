= まだいくよ！Grok
== 今度は何を取り込む？
前章ではApacheのアクセスログを取り込めるようになりました。
そこで既存のGrokPatternだけではどうにもならない系のログを対象にGrokしていきたいと思います。

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





