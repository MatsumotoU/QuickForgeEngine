# コミットルール
## コミットの種類
- feat（機能の追加）
- fix（バグの修正）
- update（バグ以外の更新）
- chore（非コードタスク）
- release（masterブランチに統合する前の準備）
> ※ 破壊的な変更を含む場合、コミットメッセージに BREAKING CHANGE と書くこと。ブランチに絶対書かない。

## ブランチ名規約
### 通常コミットの規約
- 小文字の英数字、ハイフン、ドットを使用
- 連続、先頭、末尾のハイフンやドットは禁止
- GitのIssuesのチケットを変更の種類の後に記載すること

> 例: feat/issues-999-load-dds（Issues#999 DDSファイルを読み込めるようにしますよというブランチ）

### リリースコミットの規約
- release/v破壊的変更.新機能.バグ修正 この形で書くこと

> 例: release/v1.0.0

## コミットメッセージ規約
- チケットがあるなら Closes #5 （5番のチケット）と書いて問題をクローズすること
- 破壊的変更を含む場合フッターに BREAKING CHANGE (内容)と書くこと
- 作業中のコミットは wip: (内容) と書くこと

>例: <br>
コミットの種類: タイトル<br>
(空行)<br>
本文<br>
(空行)<br>
フッター

### 参考サイト

https://conventional-branch.github.io/ja/

https://www.conventionalcommits.org/ja/v1.0.0/