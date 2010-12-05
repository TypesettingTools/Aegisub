■Aegisub 2.1.6(SVN r2496)日本語言語ファイル Ver0.25

本体のコンパイルとかよくわかんない。
「wxstd.mo」ほぼすべて日本語化完了。
「aegisub.mo」現在96％ほど日本語化した。
※1237行中48行は未翻訳

翻訳の間違いやヘンなところがありましたら、お気軽にメールください。
博：h-h23@edogawa.home.ne.jp


■Aegisub 使用上の注意！

字幕ファイルの関連付けですが、私のPCでは関連付けすると解除が上手くいきません。
ASSファイルの沢山入ったフォルダを開くと、目玉アイコンがイッパイで…
見た目が怖いです。

目玉アイコンが気持ち悪いと感じる人は、
本体のバグが直るまで字幕の関連付けは避けた方が良いでしょう。


■日本語化

「aegisub.mo」と「wxstd.mo」の入った「ja」フォルダを、
Aegisubインストールフォルダ内の「locale」フォルダに入れてください。

デフォルトでは、
C:\Program Files\Aegisub\locale
です。

Aegisubのメニュー「Language」を選択、
Languageウィンドウに「Japanese」が現れるので指定してOKを押してください。

「Restart Aegisub?」と書かれたダイアログが表示されます。

「Aegisub needs to be restarted so that the new language can be applied. Restart now?」
　↑翻訳↓
「新たな言語を適用するにはAegisubを再起動する必要があります、Aegisubを再起動しますか？」

「はい」を押せばAegisubが再起動して日本語ユーザーインターフェースに替わります。


■簡単な言語ファイルの仕組み

「aegisub.po」と「aegisub.mo」：Aegisub固有の言語ファイル。

「wxstd.po」と「wxstd.mo」：ソフト動作基本の言語ファイル（他のソフトと共用可能）。

「*.po」はソース記述の言語ファイル。
「*.mo」はコンパイル済みの言語ファイル。

poEdit
http://www.google.co.jp/search?hl=ja&q=poedit&btnG=Google+%E6%A4%9C%E7%B4%A2&lr=lang_ja
ってソフトで同梱の「aegisub.po」と「wxstd.po」を開くとイジれる、
保存すると「aegisub.mo」と「wxstd.mo」を自動生成して保存してくれる。

イジるだけならUTF-8に対応してるテキストエディタで「*.po」ファイルを書き換え、
poEditで読み込み、保存すればOK！

※wxstd.moが優先されるので、wxstd.moの的確でない翻訳部分はわざと空白にしています。
※テキストエディタ：フリーのTeraPadや有料の秀丸など



■翻訳環境

WindowsXP Pro SP3


■更新履歴


▼「aegisub.mo」

●2009/01/08
Hiroshi更新

●2008/12/04
Hiroshi更新

●2008/08/31
Hiroshi更新

●2008/08/17
Hiroshi更新

●2008/08/16
Hiroshi更新

●2008/08/05
Hiroshi、新たなSVNサーバーからソース取得後にaegisub.potにてカタログファイル更新

●2008/07/31
Hiroshi更新

●2008/07/31
Hiroshi更新分と准643更新分をマージ

●2008/07/30
准643更新

●2008/07/30
Hiroshi更新

●2008/07/29
Hiroshi更新

●2008/07/28
Hiroshi更新

●2008/07/27
Hiroshi更新

●2008/07/27
Hiroshi更新

●2008/07/26
Hiroshi更新分とElec更新分をマージ

●2008/07/25
Elec更新

●2008/07/23
Hiroshi更新

●2008/07/23
Hiroshi更新

●2008/07/22
Hiroshi更新

●2008/07/21
Hiroshi更新

●2008/07/20
Hiroshi更新

●2008/07/15
Hiroshi更新

●2007/06/07
Hibiki更新

●2007/05/13
Hiroshi作成


▼「wxstd.mo」

●2007/07/23
Hiroshi更新

●2008/07/20
鈴見咲君高さんのソースを参考にほぼ完了、Hiroshi更新

●2007/05/13
Hiroshi作成




Hiroshi Haga
mail：h-h23@edogawa.home.ne.jp
WEB：http://g-mark.jpn.org/

Hibiki
mail：hibikiotemae@gmail.com

Elec
mail：
WEB：

准643
mail：
WEB：http://www13.atwiki.jp/cc/




■ローカライズ用カタログファイルの作成

Subversion管理用ツールを用いる。
お薦めは「TortoiseSVN」
http://tortoisesvn.net/

AegisubのSVNサーバーからチェックアウト

poEditを起動して、
「ファイル」→「POT ファイルを元に新しいカタログを作成します」
でSVNサーバーから落とした「po」フォルダ内の「aegisub.pot」でカタログ作成。
