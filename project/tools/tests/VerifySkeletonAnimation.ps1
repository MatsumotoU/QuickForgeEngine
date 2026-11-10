$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$generated = Join-Path $repo 'generated'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
$testProject = Join-Path $generated 'skeleton-animation-test.vcxproj'
@"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Development|x64"><Configuration>Development</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{C79A0173-AD67-45B1-8150-488889552A12}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>`$(MSBuildThisFileDirectory)skeleton-animation-test\bin\</OutDir><IntDir>`$(MSBuildThisFileDirectory)skeleton-animation-test\obj\</IntDir><TargetName>SkeletonAnimationTest</TargetName></PropertyGroup>
  <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp20</LanguageStandard><RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary><DebugInformationFormat>None</DebugInformationFormat><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError><AdditionalIncludeDirectories>..\project\engine;..\project\engine\core;..\project\externals;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories></ClCompile><Link><GenerateDebugInformation>false</GenerateDebugInformation><AdditionalLibraryDirectories>`$(MSBuildThisFileDirectory)outputs\Development;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories><AdditionalDependencies>EngineCore.lib;%(AdditionalDependencies)</AdditionalDependencies></Link></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="..\project\tools\tests\SkeletonAnimationSmokeTest.cpp" /></ItemGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@ | Set-Content -LiteralPath $testProject -Encoding utf8
& (Join-Path $repo 'project/tools/BuildProject.bat') $testProject /p:Configuration=Development /p:Platform=x64 /m:1 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $generated 'skeleton-animation-test/bin/SkeletonAnimationTest.exe')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host 'Skeleton animation checks passed.'
