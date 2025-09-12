# 層構造
.  
└── Engine/  
    ├── Platform（プラットフォーム層）/  
    │   └── Windows/  
    │       └── DirectXCommonとかの  Windows依存のコード  
    ├── Utility（コアシステム層）/  
    │   └── アサートやデバッグ用ツール、乱数生成器など  
    ├── Core（非依存層）/  
    │   └── プラットフォーム層の純粋仮想関数  
    └── Externals（サードパーティー層）/  
        └── ImGuiなどの外部ライブラリ