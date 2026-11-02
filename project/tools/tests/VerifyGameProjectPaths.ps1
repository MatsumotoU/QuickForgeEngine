$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$generated = Join-Path $repo 'generated'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
$testProject = Join-Path $generated 'project-paths-test.vcxproj'
@"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{61E3266B-7F93-491B-AE8C-1725B43D35A4}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>`$(MSBuildThisFileDirectory)project-paths-test\bin\</OutDir><IntDir>`$(MSBuildThisFileDirectory)project-paths-test\obj\</IntDir><TargetName>ProjectPathsTest</TargetName></PropertyGroup>
  <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp20</LanguageStandard><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError></ClCompile><Link><GenerateDebugInformation>false</GenerateDebugInformation></Link></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="..\project\tools\tests\GameProjectPathsSmokeTest.cpp" /></ItemGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@ | Set-Content -LiteralPath $testProject -Encoding utf8
& (Join-Path $repo 'project/tools/BuildProject.bat') $testProject /p:Configuration=Release /p:Platform=x64 /m:1 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $generated 'project-paths-test/bin/ProjectPathsTest.exe') $generated
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host 'Game project path checks passed.'
