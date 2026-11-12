# Windows PowerShell 5.1 / Windows Forms
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
[System.Windows.Forms.Application]::EnableVisualStyles()

$repo = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Set-Location -LiteralPath $repo

$form = New-Object System.Windows.Forms.Form
$form.Text = 'QuickForgeEngine - 作業開始'
$form.ClientSize = New-Object System.Drawing.Size(700, 480)
$form.StartPosition = 'CenterScreen'
$form.MinimumSize = New-Object System.Drawing.Size(600, 450)

$label = New-Object System.Windows.Forms.Label
$label.Text = 'origin のブランチを選択してください'
$label.Location = New-Object System.Drawing.Point(12, 15)
$label.AutoSize = $true
$form.Controls.Add($label)

$branches = New-Object System.Windows.Forms.ComboBox
$branches.Location = New-Object System.Drawing.Point(15, 40)
$branches.Size = New-Object System.Drawing.Size(540, 25)
$branches.Anchor = 'Top,Left,Right'
$branches.DropDownStyle = 'DropDownList'
$form.Controls.Add($branches)

$refresh = New-Object System.Windows.Forms.Button
$refresh.Text = '一覧を更新'
$refresh.Location = New-Object System.Drawing.Point(565, 38)
$refresh.Size = New-Object System.Drawing.Size(120, 28)
$refresh.Anchor = 'Top,Right'
$form.Controls.Add($refresh)

$start = New-Object System.Windows.Forms.Button
$start.Text = '取得して Premake を実行'
$start.Location = New-Object System.Drawing.Point(15, 78)
$start.Size = New-Object System.Drawing.Size(230, 32)
$start.Enabled = $false
$form.Controls.Add($start)

$log = New-Object System.Windows.Forms.TextBox
$log.Location = New-Object System.Drawing.Point(15, 125)
$log.Size = New-Object System.Drawing.Size(670, 340)
$log.Anchor = 'Top,Bottom,Left,Right'
$log.Multiline = $true
$log.ReadOnly = $true
$log.ScrollBars = 'Both'
$log.WordWrap = $false
$form.Controls.Add($log)

function Write-Log([string]$message) {
    $log.AppendText($message + [Environment]::NewLine)
    $log.Refresh()
}

function Invoke-GitCommand([string[]]$gitArgs) {
    Write-Log ("`r`n> git " + ($gitArgs -join ' '))
    # Native stderr must not become a terminating PowerShell error.
    $output = @(& git @gitArgs 2>&1)
    $code = $LASTEXITCODE
    foreach ($line in $output) { Write-Log ([string]$line) }
    if ($code -ne 0) { throw "Git コマンドが失敗しました (終了コード: $code)。" }
    return $output
}

function Set-Busy([bool]$busy) {
    $refresh.Enabled = -not $busy
    $branches.Enabled = -not $busy
    $start.Enabled = (-not $busy -and $branches.SelectedIndex -ge 0)
    $form.UseWaitCursor = $busy
    $form.Refresh()
}

function Update-Branches {
    Set-Busy $true
    try {
        $previous = [string]$branches.SelectedItem
        [void](Invoke-GitCommand @('fetch', 'origin', '--prune'))
        $refs = @(Invoke-GitCommand @('for-each-ref', '--format=%(refname:strip=3)', 'refs/remotes/origin/'))
        $branches.Items.Clear()
        foreach ($ref in $refs) {
            $name = [string]$ref
            if ($name -and $name -ne 'HEAD') { [void]$branches.Items.Add($name) }
        }
        if ($previous -and $branches.Items.Contains($previous)) {
            $branches.SelectedItem = $previous
        } else {
            $current = (& git branch --show-current 2>$null | Select-Object -First 1)
            if ($current -and $branches.Items.Contains($current)) { $branches.SelectedItem = $current }
            elseif ($branches.Items.Count -gt 0) { $branches.SelectedIndex = 0 }
        }
        Write-Log 'ブランチ一覧を更新しました。'
    } catch {
        Write-Log ("エラー: " + $_.Exception.Message)
        [System.Windows.Forms.MessageBox]::Show($_.Exception.Message, '一覧の更新に失敗', 'OK', 'Error') | Out-Null
    } finally { Set-Busy $false }
}

$refresh.Add_Click({ Update-Branches })
$branches.Add_SelectedIndexChanged({ $start.Enabled = $branches.SelectedIndex -ge 0 })
$start.Add_Click({
    $branch = [string]$branches.SelectedItem
    if (-not $branch) { return }
    Set-Busy $true
    try {
        $status = @(Invoke-GitCommand @('status', '--porcelain=v1', '--untracked-files=normal'))
        if ($status.Count -gt 0) { throw '未コミットまたは未追跡のファイルがあります。保存・コミット・退避してから再実行してください。' }

        # An existing local branch may track a different remote branch.
        & git show-ref --verify --quiet "refs/heads/$branch"
        $exists = $LASTEXITCODE -eq 0
        if ($exists) {
            $upstream = @(& git for-each-ref '--format=%(upstream)' "refs/heads/$branch")
            if ($upstream.Count -gt 0 -and $upstream[0] -and $upstream[0] -ne "refs/remotes/origin/$branch") {
                throw "ローカルブランチ '$branch' は別の追跡先を持っています。Git で追跡先を確認してください。"
            }
            [void](Invoke-GitCommand @('checkout', $branch))
        } else {
            [void](Invoke-GitCommand @('checkout', '--track', '-b', $branch, "origin/$branch"))
        }
        [void](Invoke-GitCommand @('pull', '--ff-only', 'origin', $branch))

        $premake = Join-Path $repo 'project\premake5.bat'
        if (-not (Test-Path -LiteralPath $premake)) { throw 'project\premake5.bat が見つかりません。ブランチの取得は完了しています。' }
        Write-Log "`r`n> project\premake5.bat"
        # Supply a newline so the existing batch file's pause does not block the GUI.
        $output = @('' | & $env:ComSpec /d /c "`"$premake`"" 2>&1)
        $code = $LASTEXITCODE
        foreach ($line in $output) { Write-Log ([string]$line) }
        if ($code -ne 0) { throw "Premake に失敗しました (終了コード: $code)。ブランチの取得は完了しています。" }
        Write-Log "`r`n完了しました。"
        [System.Windows.Forms.MessageBox]::Show('ブランチの取得と Premake が完了しました。', '完了', 'OK', 'Information') | Out-Null
    } catch {
        Write-Log ("エラー: " + $_.Exception.Message)
        [System.Windows.Forms.MessageBox]::Show($_.Exception.Message, '処理を中止', 'OK', 'Error') | Out-Null
    } finally { Set-Busy $false }
})

$form.Add_Shown({ Update-Branches })
[void]$form.ShowDialog()
