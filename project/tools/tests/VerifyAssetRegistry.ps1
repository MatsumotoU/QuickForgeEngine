$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$generated = Join-Path $repo 'generated'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
$testProject = Join-Path $generated 'asset-registry-test.vcxproj'
@"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{C088FC07-B045-4B29-98BB-63F19044F132}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>`$(MSBuildThisFileDirectory)asset-registry-test\bin\</OutDir><IntDir>`$(MSBuildThisFileDirectory)asset-registry-test\obj\</IntDir><TargetName>AssetRegistryTest</TargetName></PropertyGroup>
  <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp20</LanguageStandard><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError><AdditionalIncludeDirectories>..\project\engine;..\project\externals;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories></ClCompile><Link><GenerateDebugInformation>false</GenerateDebugInformation></Link></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="..\project\tools\tests\AssetRegistrySmokeTest.cpp" /><ClCompile Include="..\project\engine\assetfactory\game-data\AssetRegistry.cpp" /></ItemGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@ | Set-Content -LiteralPath $testProject -Encoding utf8
& (Join-Path $repo 'project/tools/BuildProject.bat') $testProject /p:Configuration=Release /p:Platform=x64 /m:1 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$testRoot = Join-Path $generated ('asset-registry-check-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
try {
    & (Join-Path $generated 'asset-registry-test/bin/AssetRegistryTest.exe') $testRoot
    if ($LASTEXITCODE -ne 0) { throw "Asset registry checks failed: $LASTEXITCODE" }
    Write-Host 'Asset registry checks passed.'
} finally {
    $resolved = (Resolve-Path -LiteralPath $testRoot).Path
    if ((Split-Path -Parent $resolved) -ne (Resolve-Path -LiteralPath $generated).Path -or -not (Split-Path -Leaf $resolved).StartsWith('asset-registry-check-')) { throw 'Unexpected test directory.' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
