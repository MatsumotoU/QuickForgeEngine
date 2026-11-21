$ErrorActionPreference = 'Stop'

function Read-Section([string]$Name) {
    Write-Host "`n${Name}を入力してください。終了するには、単独の行に . を入力します。"
    Write-Host '空行や Markdown をそのまま入力できます。'
    $lines = [System.Collections.Generic.List[string]]::new()
    while ($true) {
        $line = Read-Host
        if ($line -ceq '.') { break }
        $lines.Add($line)
    }
    return ($lines -join "`n").Trim()
}

try {
    Write-Host '技術備忘録を作成します。'
    do {
        $title = (Read-Host 'タイトル').Trim()
    } while (-not $title)

    $posts = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\docs\_posts'))
    do {
        $slug = (Read-Host 'URL用の名前（半角英数字とハイフン、例: build-process）').Trim().ToLowerInvariant()
        if ($slug -notmatch '^[a-z0-9]+(?:-[a-z0-9]+)*$') {
            Write-Host '半角英数字とハイフンで入力してください。先頭と末尾にハイフンは使えません。'
            $slug = ''
        } elseif (@(Get-ChildItem -LiteralPath $posts -File | Where-Object { $_.Name -match ('^\d{4}-\d{2}-\d{2}-' + [regex]::Escape($slug) + '\.(md|markdown)$') }).Count -gt 0) {
            Write-Host '同じURL用の名前が既にあります。別の名前を入力してください。'
            $slug = ''
        }
    } while (-not $slug)

    $tagsInput = Read-Host 'タグ（カンマ区切り、省略可）'
    $tags = @($tagsInput -split '[,、]' | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    $sections = @(
        @{ Name = '何を知りたかったか'; Text = (Read-Section '何を知りたかったか') }
        @{ Name = '分かったこと'; Text = (Read-Section '分かったこと') }
        @{ Name = '制作への活用'; Text = (Read-Section '制作への活用') }
        @{ Name = '参考資料'; Text = (Read-Section '参考資料') }
    )

    $content = [System.Collections.Generic.List[string]]::new()
    $content.Add('---')
    $content.Add('layout: note')
    $content.Add('title: ' + (ConvertTo-Json -InputObject $title -Compress))
    if ($tags.Count -gt 0) {
        $content.Add('tags:')
        foreach ($tag in $tags) { $content.Add('  - ' + (ConvertTo-Json -InputObject $tag -Compress)) }
    }
    $content.Add('---')
    foreach ($section in $sections) {
        $content.Add('')
        $content.Add('## ' + $section.Name)
        $content.Add('')
        $content.Add($section.Text)
    }
    $path = Join-Path $posts ((Get-Date -Format 'yyyy-MM-dd') + '-' + $slug + '.md')
    $stream = [System.IO.File]::Open($path, [System.IO.FileMode]::CreateNew, [System.IO.FileAccess]::Write)
    try {
        $writer = [System.IO.StreamWriter]::new($stream, [System.Text.UTF8Encoding]::new($false))
        try { $writer.Write(($content -join "`n") + "`n") } finally { $writer.Dispose() }
    } finally { $stream.Dispose() }
    Write-Host "`n作成しました: $path"
} catch {
    Write-Host "`n作成できませんでした: $($_.Exception.Message)"
    exit 1
}
