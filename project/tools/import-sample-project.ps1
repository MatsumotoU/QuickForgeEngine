param([string]$Name = 'sample')
$ErrorActionPreference = 'Stop'
if ($Name -cnotmatch '^[a-z0-9_-]{1,80}$') { throw 'Use lowercase letters, numbers, hyphens or underscores for the project name.' }
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$base = Join-Path $repo 'game-projects'
$root = Join-Path $base $Name
if (Test-Path -LiteralPath $root) { throw "Project already exists: $root" }
New-Item -ItemType Directory -Path $base -Force | Out-Null
New-Item -ItemType Directory -Path $root | Out-Null
$assets = Join-Path $root 'assets'
New-Item -ItemType Directory -Path $assets | Out-Null
Get-ChildItem -LiteralPath (Join-Path $repo 'project/resources') | Copy-Item -Destination $assets -Recurse
Rename-Item -LiteralPath (Join-Path $assets 'scene') -NewName 'scenes'
# Windows is case-insensitive; rename through an intermediate name.
Rename-Item -LiteralPath (Join-Path $assets 'Scripts') -NewName 'scripts-temp'
Rename-Item -LiteralPath (Join-Path $assets 'scripts-temp') -NewName 'scripts'
foreach ($dir in 'entities','models','images','audio','shaders','animations') { New-Item -ItemType Directory -Path (Join-Path $assets $dir) -Force | Out-Null }
@{ name = $Name; version = 1; start_scene = 'stage_japan.json' } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $root 'project.json') -Encoding utf8
Write-Host "Created $root"
