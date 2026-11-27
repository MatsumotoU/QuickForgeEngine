# ゲームの操作設定

エディターの `Window > Game Config > Game Input Actions` でアクションを追加・削除し、各アクションにキー、マウスボタン、ゲームパッドボタンを割り当てます。キーは複数登録できます。`Save Game Config` を押すとプロジェクトの `project.json` に保存され、次回のゲーム起動から適用されます。未設定のプロジェクトには、従来の移動・射撃などのキー割り当てが用意されます。

`InputMovementComponent` の各方向のアクションと、`InputBulletEmitterTriggerComponent` の `Input Action` で、登録した名前を選びます。移動の左スティックは移動コンポーネントの方向アクションに対応します。従来の射撃コンポーネントに直接設定されたマウス・ゲームパッドボタンも、その `Input Action` が禁止されると使えません。名前を `None` にした直接ボタンは、個別のアクション名と対応しないため、全操作禁止の場合のみ無効になります。

ゲーム全体の操作を切り替えるには、シーン内のエンティティに `InputControlComponent` を付けます。現在のメインシーンでは Player に付いています。

- `inputEnabled` がオンなら、このコンポーネントは操作を禁止しません。オフにすると操作を禁止します。
- `affectAllActions` がオンなら、オフにしたとき全操作を禁止します。オフなら `actions` で選んだ操作だけを禁止します。
- 入力制御コンポーネントが複数ある場合、読み込み中のどのシーンであっても、いずれかが禁止した操作は受け付けません。

Collision Trigger や Event の Parameter で `InputControlComponent` の `inputEnabled` を `false` にすると禁止、`true` にすると解除できます。例えばトリガーが接触相手の Player に付いている設定を操作するなら、`Target = Other`、`Component = InputControlComponent`、`Property Path = inputEnabled`、`Operation = Set` にします。値を変える対象のエンティティに `InputControlComponent` が必要です。

操作禁止は新しい入力を受け付けなくする設定です。既に物理コンポーネントに残っている速度や、イベント・AI による移動は止めません。アクションを削除してもシーンの名前は自動変更されず、名前が残った設定は再登録するか選び直す必要があります。

エディターのカメラ操作とショートカットは ImGui の入力を使います。Scene Viewer では従来どおり中ボタンを押しながら WASD／矢印キーで移動、マウスで回転します。
