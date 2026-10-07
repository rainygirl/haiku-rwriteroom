<img src="icon.png" width="64" align="left" alt="">

# Haiku 用 R WriteRoom

[English](README.md) · [한국어](README.ko.md)

書くことだけに集中するための全画面エディターです。黒いページに緑の文字と緑のカーソルだけが表示され、メニューはポインターを画面のいちばん上に動かすまで隠れています。

![R WriteRoom で書いているところ](screenshots/writing.png)

![ポインターを上端に動かすと出てくるメニュー](screenshots/menu.png)

- ポインターを**画面の上端**に動かすとメニューが下りてきます。開く、保存、名前を付けて保存、新規、文字の大きさ、終了を選べます。ポインターを離すとメニューはまた隠れます。**Esc** でメニューを常に表示するか、また隠すかを切り替えます。
- メニューバーにはファイル名、未保存の変更の有無、語数も表示されます。
- メニューが隠れていてもショートカットが使えます。**Alt+N** 新規、**Alt+O** 開く、**Alt+S** 保存、**Shift+Alt+S** 名前を付けて保存、**Alt+Q** 終了、**Alt+Plus/Minus** 文字の大きさ。
- ファイルは UTF-8 のプレーンテキストです。未保存のまま終了しようとすると確認します。
- メニューは英語、イタリア語、韓国語、日本語に対応し、Locale 設定に従います。

## pkgman でインストール

| Haiku | コマンド |
| --- | --- |
| 32 ビット x86 (x86_gcc2) | `pkgman add-repo https://pkgman.rainygirl.com/x86_gcc2`<br>`pkgman install rwriteroom_x86` |
| x86_64 | `pkgman add-repo https://pkgman.rainygirl.com/x86_64`<br>`pkgman install rwriteroom` |
| arm64 | `pkgman install rwriteroom` (RENKU イメージにはリポジトリが登録済みです) |

インストール後、Deskbar → Applications から **R WriteRoom** を起動するか、Tracker の **Open with** メニューでテキストファイルを開いてください。

## ソースからビルド

Haiku 上でビルドします。

```sh
make                # x86_64、arm64
setarch x86 make    # 32 ビットの x86_gcc2 システム
```

ほかのシステムからのクロスビルドは `tools/cross-build.sh` を参照してください。

## ライセンス

MIT

## AI の利用について

R WriteRoom の一部は AI コーディングツール (Anthropic の Claude) の支援を受けて開発されました。すべてのコードは作者がレビューし、テストしています。
