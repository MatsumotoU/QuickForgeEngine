# ステータスと接触トリガー

`StatusComponent` はプレイヤーにも敵にも追加できます。`health` は現在の HP、`attackPower` は攻撃力です。`Enemy` タグを持つエンティティは HP が 0 になると削除されます。攻撃やダメージは別途設定してください。

接触で値を変更するには、トリガーの `CollisionTriggerComponent` に Action を追加し、次のように設定します。

1. `Timing` を `Enter`、`Action Type` を `Parameter` にします。
2. 接触相手の値を変える場合は `Target` を `Other` にします。トリガー自身は `Self`、指定したエンティティは `EntityReference` です。
3. `Component` に `StatusComponent`、`Property Path` に `health` または `attackPower` を選びます。相手が `Other` の場合、シーン内に同コンポーネントを持つエンティティがあれば項目が表示されます。なければ名前を入力できます。
4. たとえば HP を 1 減らすには `Operation` を `Add`、`Value` を `-1` にします。HP・攻撃力は 0 未満にならず、整数の範囲を超えません。

`Stay` は接触中の毎フレームに実行されます。1 回だけ変更したい場合は `Enter` を使ってください。タグやマスクによる接触条件も必要に応じて設定します。

ゲーム固有の Action Type として `IncreaseHealth` と `DecreaseHealth` も選べます。`Target` に HP を変更する対象を指定し、`Amount Source` で固定値または接触相手の攻撃力を選びます。例えばトリガーを持つプレイヤーが敵の攻撃力分のダメージを受ける場合、`DecreaseHealth`、`Target = Self`、`Amount Source = Other's attack power` にします。接触相手が `StatusComponent` を持たない場合、攻撃力を使う処理は実行されません。
