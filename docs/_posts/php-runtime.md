---
layout: note
title: "PHPとは何者？"
tags:
  - php
---

## 何を知りたかったか

PHPとは言語なのか、スクリプトなのか、ツールなのか訳が分からなかったので調べた。

## 分かったこと

- 結論
> PHPは、特にWeb開発で使われるプログラミング言語。サーバー側でリクエストを処理し、HTMLなどの応答を作ってブラウザへ返す用途がある。PHP言語と、それを実行するZend Engine（PHPの標準実装の実行エンジン）は別のもの。

PHPコードは実行時に解析・コンパイルされ、Zend Engineが扱うオペコードに変換された後、実行される。PHPの実装は主にC言語で書かれているが、PHPに関する脆弱性がすべてC言語のメモリ管理に由来するわけではない。アプリケーションの認証・入力検証の不備や設定、依存ライブラリなど、原因はさまざま。

PHPはWebサーバー経由だけでなく、コマンドラインから実行することもできる。また、GitHub Pagesのような静的ホスティングでは、サーバー上でPHPを実行できない。

## 制作への活用

PHPのニュースが自分のページを作るにあたってどのように影響するのか確かめたかった。HTMLやCSSだけを置くGitHub PagesではPHPは実行されないため、PHP固有の実行環境を用意しなくてもページを公開できる。

## 参考資料

- [PHP: Introduction](https://www.php.net/intro)
- [GitHub Docs: Creating a GitHub Pages site](https://docs.github.com/en/pages/getting-started-with-github-pages/creating-a-github-pages-site)
