# Windows PowerShell 5.1: select a project for the editor's project/ working directory.
Add-Type -AssemblyName System.Windows.Forms
[System.Windows.Forms.Application]::EnableVisualStyles()

$workingDirectory = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$configPath = Join-Path $workingDirectory 'EditorConfig.json'
try {
    if (-not (Test-Path -LiteralPath $configPath -PathType Leaf)) {
        throw "EditorConfig.json が見つかりません: $configPath"
    }
    $config = Get-Content -LiteralPath $configPath -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($null -eq $config -or $config -is [array] -or $config -isnot [PSCustomObject]) {
        throw 'EditorConfig.json は JSON オブジェクトである必要があります。'
    }
    $dialog = New-Object System.Windows.Forms.FolderBrowserDialog
    $dialog.Description = 'ゲームプロジェクトのルートを選択してください。新しいフォルダーも作成できます。'
    $dialog.ShowNewFolderButton = $true
    if ($config.project_root) {
        $initial = [string]$config.project_root
        if (-not [IO.Path]::IsPathRooted($initial)) { $initial = Join-Path $workingDirectory $initial }
        if (Test-Path -LiteralPath $initial -PathType Container) { $dialog.SelectedPath = [IO.Path]::GetFullPath($initial) }
    }
    if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { return }
    $config | Add-Member -NotePropertyName project_root -NotePropertyValue $dialog.SelectedPath -Force
    $json = $config | ConvertTo-Json -Depth 100
    [IO.File]::WriteAllText($configPath, $json + [Environment]::NewLine, (New-Object System.Text.UTF8Encoding($false)))
    [System.Windows.Forms.MessageBox]::Show("プロジェクトを設定しました。エディタを project フォルダーから起動してください。`n$($dialog.SelectedPath)", 'QuickForge', 'OK', 'Information') | Out-Null
} catch {
    [System.Windows.Forms.MessageBox]::Show($_.Exception.Message, 'QuickForge', 'OK', 'Error') | Out-Null
    exit 1
} finally {
    if ($dialog) { $dialog.Dispose() }
}
