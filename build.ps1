# build.ps1 の内容 (あなたのプロジェクトのルートディレクトリに配置)

# vcpkg のパスを設定
# ここが "externals\vcpkg" であることを確認！
$vcpkgRoot = (Join-Path $PSScriptRoot "externals\vcpkg")

# vcpkg の bootstrap を実行 (初回のみ)
if (-not (Test-Path (Join-Path $vcpkgRoot "vcpkg.exe"))) {
    Write-Host "Bootstrapping vcpkg..."
    & (Join-Path $vcpkgRoot "bootstrap-vcpkg.bat")
}

# vcpkg を Visual Studio と統合 (初回のみ)
Write-Host "Integrating vcpkg with Visual Studio..."
& (Join-Path $vcpkgRoot "vcpkg.exe") integrate install

# vcpkg を使用してライブラリをインストール
Write-Host "Installing dependencies with vcpkg..."
& (Join-Path $vcpkgRoot "vcpkg.exe") install msdfgen:x64-windows
& (Join-Path $vcpkgRoot "vcpkg.exe") install msdf-atlas-gen:x64-windows # msdf-atlas-genもライブラリとして使いたい場合

Write-Host "Vcpkg dependencies installed. Now open your Visual Studio solution and configure project properties."