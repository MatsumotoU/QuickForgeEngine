$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$generated = Join-Path $repo 'generated'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
$testProject = Join-Path $generated 'texture-descriptor-test.vcxproj'
@"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Development|x64"><Configuration>Development</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{EAC39A28-21EA-410F-8DF8-C62D6457151A}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>`$(MSBuildThisFileDirectory)texture-descriptor-test\bin\</OutDir><IntDir>`$(MSBuildThisFileDirectory)texture-descriptor-test\obj\</IntDir><TargetName>TextureDescriptorTest</TargetName></PropertyGroup>
  <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp20</LanguageStandard><RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary><DebugInformationFormat>None</DebugInformationFormat><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError><AdditionalIncludeDirectories>..\project\engine;..\project\engine\core;..\project\externals;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories></ClCompile><Link><GenerateDebugInformation>false</GenerateDebugInformation><AdditionalLibraryDirectories>`$(MSBuildThisFileDirectory)outputs\Development;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories><AdditionalDependencies>GraphicEngine.lib;EngineCore.lib;DirectXTex.lib;windowscodecs.lib;d3d12.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)</AdditionalDependencies></Link></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="..\project\tools\tests\TextureDescriptorSmokeTest.cpp" /></ItemGroup>
  <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@ | Set-Content -LiteralPath $testProject -Encoding utf8
& (Join-Path $repo 'project/tools/BuildProject.bat') $testProject /p:Configuration=Development /p:Platform=x64 /m:1 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $generated 'texture-descriptor-test/bin/TextureDescriptorTest.exe') (Resolve-Path -LiteralPath (Join-Path $repo 'game-projects/sample/assets/images/ship.png')).Path
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
