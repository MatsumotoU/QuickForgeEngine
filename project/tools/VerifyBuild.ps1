<#
.SYNOPSIS
    Verifies and builds every project in the QuickForgeEngine solution.

.DESCRIPTION
    The script is intended to be used before merging a branch into master.
    It checks that every C++ project in the solution has an active and build
    mapping for every requested configuration, then rebuilds the whole
    solution once per configuration.

    By default Premake is run first so that the solution and project files are
    regenerated in the same way as the GitHub Actions build. Use
    -SkipGenerate when the generated files are already up to date.

.EXAMPLE
    pwsh -NoProfile -File .\project\tools\VerifyBuild.ps1

.EXAMPLE
    pwsh -NoProfile -File .\project\tools\VerifyBuild.ps1 -SkipGenerate -ValidateOnly

.EXAMPLE
    pwsh -NoProfile -File .\project\tools\VerifyBuild.ps1 -Configuration Debug
#>

[CmdletBinding()]
param(
    [string]$SolutionPath,
    [string[]]$Configuration = @('Debug', 'Development', 'Release'),
    [string]$Platform = 'x64',
    [string]$MsBuildPath,
    [string]$LogDirectory,
    [switch]$SkipGenerate,
    [switch]$Restore,
    [switch]$ValidateOnly,
    [switch]$NoBinaryLog
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-FullPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (-not [IO.Path]::IsPathRooted($Path)) {
        $Path = Join-Path (Get-Location) $Path
    }

    return [IO.Path]::GetFullPath($Path)
}

function Get-CommandPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($null -eq $command) {
        return $null
    }

    if ($command.PSObject.Properties['Path']) {
        return $command.Path
    }

    return $command.Source
}

function Find-MSBuild {
    param(
        [string]$RequestedPath
    )

    if (-not [string]::IsNullOrWhiteSpace($RequestedPath)) {
        $resolvedPath = Resolve-FullPath $RequestedPath
        if (-not (Test-Path -LiteralPath $resolvedPath -PathType Leaf)) {
            throw "MSBuild was not found at '$resolvedPath'."
        }

        return $resolvedPath
    }

    $pathFromEnvironment = Get-CommandPath 'msbuild.exe'
    if (-not [string]::IsNullOrWhiteSpace($pathFromEnvironment)) {
        return $pathFromEnvironment
    }

    $programFilesX86 = [Environment]::GetEnvironmentVariable('ProgramFiles(x86)')
    if (-not [string]::IsNullOrWhiteSpace($programFilesX86)) {
        $vsWherePath = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
        if (Test-Path -LiteralPath $vsWherePath -PathType Leaf) {
            $foundPath = & $vsWherePath -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' 2>$null | Select-Object -First 1
            if (-not [string]::IsNullOrWhiteSpace($foundPath) -and (Test-Path -LiteralPath $foundPath -PathType Leaf)) {
                return $foundPath.Trim()
            }
        }
    }

    throw 'MSBuild.exe was not found. Run this script from a Visual Studio Developer PowerShell, install Visual Studio with the C++ workload, or pass -MsBuildPath.'
}

function Invoke-PremakeGeneration {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $premakeScript = Join-Path $ProjectRoot 'premake5.bat'
    if (-not (Test-Path -LiteralPath $premakeScript -PathType Leaf)) {
        throw "Premake script was not found at '$premakeScript'."
    }

    Write-Host '[1/4] Generating Visual Studio projects with Premake...' -ForegroundColor Cyan

    $previousGithubActions = $env:GITHUB_ACTIONS
    try {
        # premake5.bat pauses at the end for interactive local use. Setting this
        # only for the child process keeps the verification command non-blocking.
        $env:GITHUB_ACTIONS = 'true'
        Push-Location $ProjectRoot
        try {
            $premakeCommand = '"' + $premakeScript + '"'
            & $env:ComSpec /d /c $premakeCommand
            if ($LASTEXITCODE -ne 0) {
                throw "Premake failed with exit code $LASTEXITCODE."
            }
        }
        finally {
            Pop-Location
        }
    }
    finally {
        if ($null -eq $previousGithubActions) {
            Remove-Item Env:GITHUB_ACTIONS -ErrorAction SilentlyContinue
        }
        else {
            $env:GITHUB_ACTIONS = $previousGithubActions
        }
    }
}

function Get-ProjectConfigurations {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ProjectPath
    )

    try {
        [xml]$projectXml = Get-Content -LiteralPath $ProjectPath -Raw
        $configurationNodes = $projectXml.SelectNodes("//*[local-name()='ProjectConfiguration']")
        return @($configurationNodes | ForEach-Object { $_.Include })
    }
    catch {
        throw "Failed to read project configuration from '$ProjectPath': $($_.Exception.Message)"
    }
}

function Read-SolutionModel {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SolutionFile
    )

    $lines = Get-Content -LiteralPath $SolutionFile
    $vcProjectTypeGuid = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
    $projectsByGuid = @{}
    $solutionConfigurations = @()
    $projectMappings = @{}
    $section = $null

    foreach ($line in $lines) {
        if ($line -match '^Project\("(?<type>[^" ]+)"\)\s*=\s*"(?<name>[^"]+)",\s*"(?<path>[^"]+)",\s*"(?<guid>\{[^}]+\})"') {
            if ($matches.type -ieq $vcProjectTypeGuid) {
                $guid = $matches.guid.ToUpperInvariant()
                $relativePath = $matches.path.Replace('/', '\')
                $fullPath = Resolve-FullPath (Join-Path (Split-Path $SolutionFile -Parent) $relativePath)
                $projectsByGuid[$guid] = [pscustomobject]@{
                    Name         = $matches.name
                    Guid         = $guid
                    RelativePath = $relativePath
                    FullPath     = $fullPath
                }
            }
            continue
        }

        if ($line -match '^\s*GlobalSection\(SolutionConfigurationPlatforms\)') {
            $section = 'solution-configurations'
            continue
        }

        if ($line -match '^\s*GlobalSection\(ProjectConfigurationPlatforms\)') {
            $section = 'project-configurations'
            continue
        }

        if ($line -match '^\s*EndGlobalSection') {
            $section = $null
            continue
        }

        if ($section -eq 'solution-configurations' -and $line -match '^\s*(?<configuration>[^|=\s]+)\|(?<platform>[^=\s]+)\s*=') {
            $solutionConfigurations += [pscustomobject]@{
                Configuration = $matches.configuration.Trim()
                Platform      = $matches.platform.Trim()
            }
            continue
        }

        if ($section -eq 'project-configurations' -and $line -match '^\s*(?<guid>\{[^}]+\})\.(?<configuration>[^|]+)\|(?<platform>[^.]+)\.(?<kind>ActiveCfg|Build\.0)\s*=\s*(?<mapped>.+?)\s*$') {
            $guid = $matches.guid.ToUpperInvariant()
            $configurationKey = "$($matches.configuration.Trim())|$($matches.platform.Trim())"

            if (-not $projectMappings.ContainsKey($guid)) {
                $projectMappings[$guid] = @{}
            }
            if (-not $projectMappings[$guid].ContainsKey($configurationKey)) {
                $projectMappings[$guid][$configurationKey] = @{}
            }

            $projectMappings[$guid][$configurationKey][$matches.kind] = $matches.mapped.Trim()
        }
    }

    return [pscustomobject]@{
        Projects               = @($projectsByGuid.Values)
        SolutionConfigurations = @($solutionConfigurations)
        ProjectMappings        = $projectMappings
    }
}

function Validate-SolutionCoverage {
    param(
        [Parameter(Mandatory = $true)]
        [pscustomobject]$SolutionModel,
        [Parameter(Mandatory = $true)]
        [string[]]$RequestedConfigurations,
        [Parameter(Mandatory = $true)]
        [string]$RequestedPlatform
    )

    $selectedConfigurations = @($SolutionModel.SolutionConfigurations | Where-Object {
        $_.Platform -ieq $RequestedPlatform -and $_.Configuration -iin $RequestedConfigurations
    })

    if ($selectedConfigurations.Count -eq 0) {
        throw "No solution configuration matched platform '$RequestedPlatform' and configurations '$($RequestedConfigurations -join ', ')'."
    }

    $issues = @()
    $projectConfigurationCache = @{}

    foreach ($solutionConfiguration in $selectedConfigurations) {
        $expectedConfiguration = "$($solutionConfiguration.Configuration)|$($solutionConfiguration.Platform)"

        foreach ($project in $SolutionModel.Projects) {
            if (-not (Test-Path -LiteralPath $project.FullPath -PathType Leaf)) {
                $issues += "$($project.Name): project file was not found at '$($project.RelativePath)'."
                continue
            }

            $projectKey = $project.FullPath.ToUpperInvariant()
            if (-not $projectConfigurationCache.ContainsKey($projectKey)) {
                try {
                    $projectConfigurationCache[$projectKey] = @(Get-ProjectConfigurations $project.FullPath)
                }
                catch {
                    $issues += $_.Exception.Message
                    continue
                }
            }

            if ($expectedConfiguration -notin $projectConfigurationCache[$projectKey]) {
                $issues += "$($project.Name): project does not define '$expectedConfiguration'."
            }

            $guid = $project.Guid.ToUpperInvariant()
            $mapping = $null
            if ($SolutionModel.ProjectMappings.ContainsKey($guid) -and $SolutionModel.ProjectMappings[$guid].ContainsKey($expectedConfiguration)) {
                $mapping = $SolutionModel.ProjectMappings[$guid][$expectedConfiguration]
            }

            if ($null -eq $mapping) {
                $issues += "$($project.Name): solution mapping '$expectedConfiguration' is missing."
                continue
            }

            if (-not $mapping.ContainsKey('ActiveCfg')) {
                $issues += "$($project.Name): ActiveCfg mapping '$expectedConfiguration' is missing."
            }
            elseif ($mapping['ActiveCfg'] -ne $expectedConfiguration) {
                $issues += "$($project.Name): ActiveCfg '$($mapping['ActiveCfg'])' does not match '$expectedConfiguration'."
            }

            if (-not $mapping.ContainsKey('Build.0')) {
                $issues += "$($project.Name): Build.0 mapping '$expectedConfiguration' is missing."
            }
            elseif ($mapping['Build.0'] -ne $expectedConfiguration) {
                $issues += "$($project.Name): Build.0 '$($mapping['Build.0'])' does not match '$expectedConfiguration'."
            }
        }
    }

    if ($issues.Count -gt 0) {
        Write-Host "Coverage validation failed with $($issues.Count) issue(s):" -ForegroundColor Red
        $issues | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
        throw 'The solution does not contain a buildable mapping for every requested project and configuration.'
    }

    Write-Host "Coverage validation passed: $($SolutionModel.Projects.Count) project(s) x $($selectedConfigurations.Count) configuration(s)." -ForegroundColor Green
    return $selectedConfigurations
}

function Invoke-Restore {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SolutionFile
    )

    $dotnetPath = Get-CommandPath 'dotnet.exe'
    if ([string]::IsNullOrWhiteSpace($dotnetPath)) {
        throw 'dotnet.exe was not found. Remove -Restore or install the .NET SDK.'
    }

    Write-Host '[2/4] Restoring solution dependencies...' -ForegroundColor Cyan
    & $dotnetPath restore $SolutionFile
    if ($LASTEXITCODE -ne 0) {
        throw "dotnet restore failed with exit code $LASTEXITCODE."
    }
}

function Invoke-SolutionBuild {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildTool,
        [Parameter(Mandatory = $true)]
        [string]$SolutionFile,
        [Parameter(Mandatory = $true)]
        [string]$BuildConfiguration,
        [Parameter(Mandatory = $true)]
        [string]$BuildPlatform,
        [Parameter(Mandatory = $true)]
        [string]$BuildLogDirectory,
        [switch]$DisableBinaryLog
    )

    $safeConfiguration = $BuildConfiguration -replace '[^A-Za-z0-9._-]', '_'
    $binaryLogPath = Join-Path $BuildLogDirectory "QuickForgeEngine-$safeConfiguration-$BuildPlatform.binlog"
    $arguments = @(
        $SolutionFile,
        '/t:Rebuild',
        '/m',
        '/nologo',
        '/verbosity:minimal',
        "/p:Configuration=$BuildConfiguration",
        "/p:Platform=$BuildPlatform"
    )

    if (-not $DisableBinaryLog) {
        $arguments += "/bl:$binaryLogPath"
    }

    Write-Host "[BUILD] $BuildConfiguration|$BuildPlatform" -ForegroundColor Yellow
    $stopwatch = [Diagnostics.Stopwatch]::StartNew()
    # Forward MSBuild output to the console without letting it become part of
    # the result object returned by this function.
    & $BuildTool @arguments 2>&1 | ForEach-Object { Write-Host $_ }
    $exitCode = $LASTEXITCODE
    $stopwatch.Stop()

    return [pscustomobject]@{
        Configuration = $BuildConfiguration
        Platform      = $BuildPlatform
        ExitCode      = $exitCode
        Duration      = $stopwatch.Elapsed
        BinaryLog     = if ($DisableBinaryLog) { $null } else { $binaryLogPath }
    }
}

try {
    $scriptRoot = Split-Path $PSScriptRoot -Parent
    if ([string]::IsNullOrWhiteSpace($SolutionPath)) {
        $SolutionPath = Join-Path $scriptRoot 'QuickForgeEngine.sln'
    }
    $SolutionPath = Resolve-FullPath $SolutionPath
    if (-not (Test-Path -LiteralPath $SolutionPath -PathType Leaf)) {
        throw "Solution file was not found at '$SolutionPath'."
    }

    $projectRoot = Split-Path $SolutionPath -Parent
    if ([string]::IsNullOrWhiteSpace($LogDirectory)) {
        $LogDirectory = Join-Path (Split-Path $projectRoot -Parent) 'generated\build-logs'
    }
    $LogDirectory = Resolve-FullPath $LogDirectory

    if ($Configuration.Count -eq 0) {
        throw 'At least one configuration must be specified.'
    }

    Write-Host 'QuickForgeEngine build verification' -ForegroundColor White
    Write-Host "Solution: $SolutionPath"
    Write-Host "Configurations: $($Configuration -join ', ')"
    Write-Host "Platform: $Platform"

    if (-not $SkipGenerate) {
        Invoke-PremakeGeneration $projectRoot
    }
    else {
        Write-Host '[1/4] Premake generation skipped.' -ForegroundColor DarkGray
    }

    $solutionModel = Read-SolutionModel $SolutionPath
    $selectedConfigurations = @(Validate-SolutionCoverage $solutionModel $Configuration $Platform)

    if ($ValidateOnly) {
        Write-Host 'Validation completed successfully. No build was requested.' -ForegroundColor Green
        exit 0
    }

    if ($Restore) {
        Invoke-Restore $SolutionPath
    }
    else {
        Write-Host '[2/4] Dependency restore skipped.' -ForegroundColor DarkGray
    }

    $buildTool = Find-MSBuild $MsBuildPath
    New-Item -ItemType Directory -Path $LogDirectory -Force | Out-Null
    Write-Host "[3/4] Using MSBuild: $buildTool" -ForegroundColor Cyan
    Write-Host "Build logs: $LogDirectory"

    $results = @()
    foreach ($selectedConfiguration in $selectedConfigurations) {
        $results += Invoke-SolutionBuild -BuildTool $buildTool -SolutionFile $SolutionPath -BuildConfiguration $selectedConfiguration.Configuration -BuildPlatform $selectedConfiguration.Platform -BuildLogDirectory $LogDirectory -DisableBinaryLog:$NoBinaryLog
    }

    Write-Host '[4/4] Build summary' -ForegroundColor Cyan
    $results | Format-Table Configuration, Platform, ExitCode, Duration, BinaryLog -AutoSize

    $failedResults = @($results | Where-Object { $_.ExitCode -ne 0 })
    if ($failedResults.Count -gt 0) {
        Write-Host "$($failedResults.Count) configuration(s) failed." -ForegroundColor Red
        exit 1
    }

    Write-Host "All $($results.Count) configuration(s) built successfully." -ForegroundColor Green
    exit 0
}
catch {
    Write-Error $_.Exception.Message
    exit 1
}
