---
layout: note
title: "ウェブページってどうやって表示されているの？"
tags:
  - browser
  - html
  - https
---

## 何を知りたかったか

自作ツールの説明ページを書くにあたってそもそもどのようにページを表示しているのか気になったので調べた。

## 分かったこと

- 結論
> ブラウザはHTTPリクエストをサーバーへ送り、サーバーからHTMLやCSS、JavaScript、画像などを受け取る。ブラウザはHTMLからページの構造を読み取り、CSSを適用して表示し、JavaScriptがあればそれも実行する。PHPなどのサーバー側プログラムを使うサイトでは、サーバーが応答を作ることもある。静的サイトでは、用意されたファイルをそのまま返す。

- リクエストについて
> HTTPリクエストは、サーバーに対してリソースの取得やデータ送信などを求めるメッセージ。ページを開くとき、ブラウザは通常`GET`でHTMLを取得し、そのHTMLが参照するCSSや画像なども追加で取得する。リンクを押したりフォームを送信したりすると、ブラウザが対応するリクエストを送る。リクエストはブラウザ以外のプログラムからも送れるため、サーバー側では必要に応じて認証・権限確認・入力検証を行う。これはPHPだけに特有の問題ではない。

- HTMLについて
> HTMLはJSONのような単なるデータ形式ではなく、見出し・文章・リンクなど、文書の構造や意味を記述するマークアップ言語。CSSはその要素をどう見せるかを指定するスタイルシートで、HTMLファイル内に書く方法と、別のCSSファイルに分けて読み込む方法がある。JavaScriptはページ上の動作を実装するために使われる。ブラウザがこれらを読み取って表示・実行する。

- HTTPSについて
> HTTPSはHTTP通信をTLSで保護する仕組み。HTTPリクエストそのものとは別の「別方式」ではなく、HTTPを暗号化などで保護して送受信する。ブラウザはHTTPSで接続した場合もHTTPリクエストを送る。

## 制作への活用

自分のエンジン、ツールの使い方を公開するにあたってReadme.mdよりも見やすいページを作るのに使う。

## 参考資料

- [MDN: HTML の基本](https://developer.mozilla.org/ja/docs/Learn_web_development/Getting_started/Your_first_website/Creating_the_content)
- [MDN: CSS の基本](https://developer.mozilla.org/ja/docs/Learn_web_development/Core/Styling_basics/What_is_CSS)
- [MDN: HTTP の概要](https://developer.mozilla.org/ja/docs/Web/HTTP/Overview)
