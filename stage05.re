= まだいくよ！Grok
== 今度は何を取り込む？
前章ではApacheのアクセスログを取り込めるようになりました。
そこで既存のGrokPatternだけではどうにもならない系のログを対象にGrokしていきたいと思います。

今回は、CISCOのファイアウォール製品であるASAのログを取り込みたいと思いますー
やっぱり企業を守っているファイアウォールがどんなログを出しているか気になりますよね！？（薄っぺらいw）

といことで今回は以下のログを対象にしたいと思います。
 ※IPアドレスは、適当なプライベートIPアドレスを割り当てています

* Jun 20 10:21:34 ASA-01 : %ASA-6-606001: ASDM session number 0 from 192.168.1.254 started
* Jun 20 13:36:43 ASA-01 : %ASA-3-713167: Group = Future-VPN, IP = 10.0.0.1, Remote peer has failed user authentication -  check configured username and password


ということで、いつも通りに以下のログ取り込みフローで進めたいと思います！

<ログ取り込みフロー>
1. ログフォーマットを調べる
2. フィールド定義
3. GrokPatternをつくる
4. Grok Constructorでテスト
5. logstashのconfファイルで動かしてみる

=== ログフォーマットを調べる
まずは、ログフォーマットを調べる！
Ciscoさんは丁寧にログフォーマットが掲載されていることがわかります。

@<href>{https://www.cisco.com/c/en/us/td/docs/security/asa/syslog/b_syslog.html}

...よく見るとわかりますが、数が多いw
まぁ、Ciscoって世界最大ですからねー(*ﾟ∀ﾟ)/



