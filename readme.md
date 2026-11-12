[![DebugBuild](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/DebugBuild.yml)
[![DevelopmentBuild](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/DevelopmentBuild.yml/badge.svg)](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/DevelopmentBuild.yml)
[![ReleaseBuild](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/ReleaseBuild.yml)

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=MatsumotoU_QuickForgeEngine&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=MatsumotoU_QuickForgeEngine)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=MatsumotoU_QuickForgeEngine&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=MatsumotoU_QuickForgeEngine)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=MatsumotoU_QuickForgeEngine&metric=duplicated_lines_density)](https://sonarcloud.io/summary/new_code?id=MatsumotoU_QuickForgeEngine)

[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=MatsumotoU_QuickForgeEngine&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=MatsumotoU_QuickForgeEngine)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=MatsumotoU_QuickForgeEngine&metric=bugs)](https://sonarcloud.io/summary/new_code?id=MatsumotoU_QuickForgeEngine)

[![Doxygen Documentation](https://github.com/MatsumotoU/QuickForgeEngine/actions/workflows/doxygen.yml/badge.svg)](https://MatsumotoU.github.io/QuickForgeEngine/)

## 作業開始ツール

Windows でルートの `StartWork.bat` を実行すると、`origin` のブランチ一覧を取得します。ブランチを選んで「取得して Premake を実行」を押すと、ローカルブランチへの切り替え、fast-forward のみの pull、Visual Studio 2022 用のプロジェクト生成を順に実行します。Git と Windows PowerShell が必要です。初回の Git 認証や Premake のダウンロードが必要になる場合があります。

未コミット・未追跡のファイルがある場合や、ローカルとリモートが分岐している場合は処理を中止します。変更をコミットまたは退避するなどしてから再実行してください。Visual Studio は事前に閉じてください。通信中や生成中は画面の反応が止まることがあります。
