param([string]$Model = 'game-projects/sample/assets/models/TwistBoxTwoBones.glb', [string]$ObjModel = 'game-projects/sample/assets/models/ship.obj')
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$generated = Join-Path $repo 'generated'
$testProject = Join-Path $generated 'raw-asset-import-test.vcxproj'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
@"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Development|x64"><Configuration>Development</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{EB2CEDF3-D52C-4267-BE17-547DD94E6ACB}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>`$(MSBuildThisFileDirectory)raw-asset-import-test\bin\</OutDir><IntDir>`$(MSBuildThisFileDirectory)raw-asset-import-test\obj\</IntDir><TargetName>RawAssetImportTest</TargetName></PropertyGroup>
  <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp20</LanguageStandard><RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary><DebugInformationFormat>None</DebugInformationFormat><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError><AdditionalIncludeDirectories>..\project;..\project\engine;..\project\engine\core;..\project\externals;..\project\externals\assimp;..\project\externals\assimp\upstream\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories></ClCompile><Link><GenerateDebugInformation>false</GenerateDebugInformation><AdditionalLibraryDirectories>`$(MSBuildThisFileDirectory)outputs\Development;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories><AdditionalDependencies>EngineAssetsFactory.lib;EngineCore.lib;Assimp.lib;%(AdditionalDependencies)</AdditionalDependencies></Link></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="..\project\tools\tests\RawAssetImportSmokeTest.cpp" /></ItemGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@ | Set-Content -LiteralPath $testProject -Encoding utf8
& (Join-Path $repo 'project/tools/BuildProject.bat') $testProject /p:Configuration=Development /p:Platform=x64 /m:1 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$unicodePart = [string][char]0x65e5 + [char]0x672c + [char]0x8a9e
$testRoot = Join-Path $generated ('raw-asset-import-check-' + $unicodePart + '-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
try {
    & (Join-Path $generated 'raw-asset-import-test/bin/RawAssetImportTest.exe') $testRoot (Resolve-Path -LiteralPath (Join-Path $repo $Model)).Path (Resolve-Path -LiteralPath (Join-Path $repo $ObjModel)).Path
    if ($LASTEXITCODE -ne 0) { throw "Raw asset import checks failed: $LASTEXITCODE" }
    Write-Host 'Asset manager checks passed.'
} finally {
    $resolved = (Resolve-Path -LiteralPath $testRoot).Path
    if ((Split-Path -Parent $resolved) -ne (Resolve-Path -LiteralPath $generated).Path -or -not (Split-Path -Leaf $resolved).StartsWith('raw-asset-import-check-')) { throw 'Unexpected test directory.' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
