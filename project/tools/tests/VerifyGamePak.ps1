$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$generated = Join-Path $repo 'generated'
$testProject = Join-Path $generated 'game-pak-test.vcxproj'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
@"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{1271F3C7-189D-4F7F-9F44-9BC42BD7EB35}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>`$(MSBuildThisFileDirectory)game-pak-test\bin\</OutDir><IntDir>`$(MSBuildThisFileDirectory)game-pak-test\obj\</IntDir><TargetName>GamePakTest</TargetName></PropertyGroup>
  <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp20</LanguageStandard><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError><AdditionalIncludeDirectories>..\project\externals;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories></ClCompile><Link><GenerateDebugInformation>false</GenerateDebugInformation></Link></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="..\project\tools\tests\GamePakSmokeTest.cpp" /><ClCompile Include="..\project\engine\assetfactory\game-data\GamePak.cpp" /><ClCompile Include="..\project\engine\assetfactory\game-data\AssetRegistry.cpp" /></ItemGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@ | Set-Content -LiteralPath $testProject -Encoding utf8
& (Join-Path $repo 'project/tools/BuildProject.bat') $testProject /p:Configuration=Release /p:Platform=x64 /m:1 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$testRoot = Join-Path $generated ('game-pak-check-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
try {
    & (Join-Path $generated 'game-pak-test/bin/GamePakTest.exe') $testRoot
    if ($LASTEXITCODE -ne 0) { throw "Package checks failed: $LASTEXITCODE" }
    Write-Host 'Game package checks passed.'
} finally {
    $resolved = (Resolve-Path -LiteralPath $testRoot).Path
    if ((Split-Path -Parent $resolved) -ne (Resolve-Path -LiteralPath $generated).Path -or -not (Split-Path -Leaf $resolved).StartsWith('game-pak-check-')) { throw 'Unexpected test directory.' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
