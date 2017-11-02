Rebol [
	Title:   "Make VS project"
	Purpose: "Builds Visual Studio project files for building Rebol3"
	Author:  "Oldes"
]

VS: context [
	guid-project-type: none
	LibraryPath-x86: ""
	IncludePath-x86: "..\..\..\src\include;"

	LibraryPath-x64: ""
	IncludePath-x64: IncludePath-x86

	PreprocessorDefinitions-x86: ""
    PreprocessorDefinitions-x64: ""

    Prebuild-x86: none
    Prebuild-x64: none

	SourcePath:   ""

	AdditionalDependencies: "wsock32.lib;comdlg32.lib;"

	Sources: []


	make-guid: func[name [string!] /local id] [
		;using `name` variable for better randomness
		id: form checksum/method to binary! (append (copy name) now/precise) 'md5
		remove back tail remove/part id 2
		insert at id 9 "-"
		insert at id 14 "-"
		insert at id 19 "-"
		insert at id 24 "-"
		id
	]
	prepare-dir: func[dir [file! string!] /confirm][
		dir: dirize dir
		if all [confirm exists? dir][
			print ["Target directory:" mold dir "already exists!"]
			print  "Files inside may be modified!"
			ask    "^/Press ENTER to continue."
			print  ""
		]
		unless exists? dir [
			print ["Making dir: " mold dir]
			if error? try [make-dir/deep dir][
				print ["Cannot make directory: " mold dir]
				halt
			]
		]
		dir
	]
	write-file: func[file [file! block!] data][
		if block? file [file: rejoin file]
		prin ["Writing: " mold file "..."]
		print either error? try [write file data]["FAILED!"]["DONE"]
	]
	make-project: func[
		name   [string!]
		dir    [file! string!]
		/guid
			id [string!] "Visual studio project type GUID"
		/type subsystem "[Console Windows]"
		/local
			tmp flt dir-vs file
	][
		guid-project-type: any [id {8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}]
		subsystem: any [subsystem "Windows"]

		dir:    prepare-dir dir
		dir-vs: prepare-dir/confirm dir/:name



		guid: make-guid
		tmp: replace/all copy sln     "#PROJECT-TYPE-GUID#"       guid-project-type
		tmp: replace/all      tmp     "#PROJECT-GUID#"            guid
		tmp: replace/all      tmp     "#PROJECT-NAME#"            name

		write-file [dir name %.sln] tmp

		tmp: replace/all copy vcxproj "#PROJECT-GUID#"            guid
		tmp: replace/all      tmp     "#PROJECT-NAME#"            name
		tmp: replace/all      tmp     "#SUBSYSTEM#"               subsystem
		tmp: replace/all      tmp     "#LIBRARY-PATH-32#"         LibraryPath-x86
		tmp: replace/all      tmp     "#LIBRARY-PATH-64#"         LibraryPath-x64
		tmp: replace/all      tmp     "#INCLUDE-PATH-32#"         IncludePath-x86
		tmp: replace/all      tmp     "#INCLUDE-PATH-64#"         IncludePath-x64
		tmp: replace/all      tmp     "#DEFINITIONS-32#"          PreprocessorDefinitions-x86
		tmp: replace/all      tmp     "#DEFINITIONS-64#"          PreprocessorDefinitions-x64
		tmp: replace/all      tmp     "#SOURCE-PATH#"             SourcePath
		tmp: replace/all      tmp     "#ADDITIONAL-DEPENDENCIES#" AdditionalDependencies
		
		

		flt: copy vcxproj.filters
		foreach [filter files] Sources [
			if not find flt rejoin [{Include="} filter {"}][
				insert find head flt "</ItemGroup>" rejoin [
					{^-<Filter Include="} filter {">}
      				{^/^-  <UniqueIdentifier>^{} make-guid filter {^}</UniqueIdentifier>}
    				{^/^-</Filter>^/}
				] 
			]
			flt: find/tail/last flt {</ItemGroup>}
			tmp: find/tail      tmp {</ItemGroup>}
			flt: insert flt {^/  <ItemGroup>}
			tmp: insert tmp {^/  <ItemGroup>}
			foreach file files [
				flt: insert flt rejoin [
					{^/    <ClCompile Include="} to-local-file file {">}
					{<Filter>} filter {</Filter></ClCompile>}
				]
				tmp: insert tmp rejoin [
					{^/    <ClCompile Include="} to-local-file file {" />}
				]
			]
			flt: insert flt {^/  </ItemGroup>}
			tmp: insert tmp {^/  </ItemGroup>}
			flt: head flt
			tmp: head tmp
		]

		write-file [dir-vs name %.vcxproj.filters] flt
		write-file [dir-vs name %.vcxproj] tmp

		tmp: replace/all copy build-vs-release-x86 "#PROJECT-NAME#" name
		write-file [dir %build-vs-release-x86.bat] tmp

		tmp: replace/all copy build-vs-release-x64 "#PROJECT-NAME#" name
		write-file [dir %build-vs-release-x64.bat] tmp

		if all [Prebuild-x86 not exists? dir/prebuild.bat] [
			write-file dir-vs/prebuild.bat Prebuild-x86
		]
		if all [Prebuild-x64 not exists? dir/prebuild64.bat] [
			write-file dir-vs/prebuild64.bat Prebuild-x64
		]

		tmp: replace/all rc-file "#PROJECT-NAME#" name
		write-file [dir-vs name %.rc] tmp

	]

	sln: {Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 15
VisualStudioVersion = 15.0.26730.12
MinimumVisualStudioVersion = 10.0.40219.1
Project("{#PROJECT-TYPE-GUID#}") = "#PROJECT-NAME#", "#PROJECT-NAME#\#PROJECT-NAME#.vcxproj", "{#PROJECT-GUID#}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Release|x86 = Release|x86
		Release|x64 = Release|x64
		Debug|x86 = Debug|x86
		Debug|x64 = Debug|x64
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{#PROJECT-GUID#}.Release|x86.ActiveCfg = Release|Win32
		{#PROJECT-GUID#}.Release|x86.Build.0 = Release|Win32
		{#PROJECT-GUID#}.Release|x64.ActiveCfg = Release|x64
		{#PROJECT-GUID#}.Release|x64.Build.0 = Release|x64
		{#PROJECT-GUID#}.Debug|x86.ActiveCfg = Debug|Win32
		{#PROJECT-GUID#}.Debug|x86.Build.0 = Debug|Win32
		{#PROJECT-GUID#}.Debug|x64.ActiveCfg = Debug|x64
		{#PROJECT-GUID#}.Debug|x64.Build.0 = Debug|x64
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {919B196B-B79C-4142-8672-1BF24F9B7183}
	EndGlobalSection
EndGlobal
}

	vcxproj: {<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
	<ProjectConfiguration Include="Debug|Win32">
	  <Configuration>Debug</Configuration>
	  <Platform>Win32</Platform>
	</ProjectConfiguration>
	<ProjectConfiguration Include="Release|Win32">
	  <Configuration>Release</Configuration>
	  <Platform>Win32</Platform>
	</ProjectConfiguration>
	<ProjectConfiguration Include="Debug|x64">
	  <Configuration>Debug</Configuration>
	  <Platform>x64</Platform>
	</ProjectConfiguration>
	<ProjectConfiguration Include="Release|x64">
	  <Configuration>Release</Configuration>
	  <Platform>x64</Platform>
	</ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
	<VCProjectVersion>15.0</VCProjectVersion>
	<ProjectGuid>{#PROJECT-GUID#}</ProjectGuid>
	<RootNamespace>SDLtest</RootNamespace>
	<WindowsTargetPlatformVersion>10.0.14393.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
	<ConfigurationType>Application</ConfigurationType>
	<UseDebugLibraries>true</UseDebugLibraries>
	<PlatformToolset>v141</PlatformToolset>
	<CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
	<ConfigurationType>Application</ConfigurationType>
	<UseDebugLibraries>false</UseDebugLibraries>
	<PlatformToolset>v141</PlatformToolset>
	<WholeProgramOptimization>true</WholeProgramOptimization>
	<CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
	<ConfigurationType>Application</ConfigurationType>
	<UseDebugLibraries>true</UseDebugLibraries>
	<PlatformToolset>v141</PlatformToolset>
	<CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
	<ConfigurationType>Application</ConfigurationType>
	<UseDebugLibraries>false</UseDebugLibraries>
	<PlatformToolset>v141</PlatformToolset>
	<WholeProgramOptimization>true</WholeProgramOptimization>
	<CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Label="Shared">
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
	<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
	<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
	<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
	<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
	<IncludePath>#INCLUDE-PATH-64#$(IncludePath)</IncludePath>
	<LibraryPath>#LIBRARY-PATH-64#$(LibraryPath)</LibraryPath>
	<SourcePath>#SOURCE-PATH#$(SourcePath)</SourcePath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
	<IncludePath>#INCLUDE-PATH-64#;$(IncludePath)</IncludePath>
	<LibraryPath>#LIBRARY-PATH-64#$(LibraryPath)</LibraryPath>
	<SourcePath>#SOURCE-PATH#$(SourcePath)</SourcePath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
	<LibraryPath>#LIBRARY-PATH-32#$(LibraryPath)</LibraryPath>
	<IncludePath>#INCLUDE-PATH-32#$(IncludePath)</IncludePath>
	<SourcePath>#SOURCE-PATH#$(SourcePath)</SourcePath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
	<LibraryPath>#LIBRARY-PATH-32#$(LibraryPath)</LibraryPath>
	<IncludePath>#INCLUDE-PATH-32#$(IncludePath)</IncludePath>
    <SourcePath>#SOURCE-PATH#$(SourcePath)</SourcePath>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
	<ClCompile>
	  <WarningLevel>Level3</WarningLevel>
	  <Optimization>Disabled</Optimization>
	  <SDLCheck>true</SDLCheck>
	  <PreprocessorDefinitions>#DEFINITIONS-32#%(PreprocessorDefinitions)</PreprocessorDefinitions>
	</ClCompile>
	<Link>
	  <AdditionalDependencies>#ADDITIONAL-DEPENDENCIES#%(AdditionalDependencies)</AdditionalDependencies>
	  <SubSystem>#SUBSYSTEM#</SubSystem>
	</Link>
	<PreBuildEvent>
	  <Command>prebuild.bat</Command>
	</PreBuildEvent>
	<PostBuildEvent>
      <Command>COPY /Y "$(TargetDir)$(ProjectName).exe" "$(ProjectDir)..\..\..\build\win-x86\$(ProjectName)_debug.exe"</Command>
      <Message>Copy resulted EXE</Message>
    </PostBuildEvent>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
	<ClCompile>
	  <WarningLevel>Level3</WarningLevel>
	  <Optimization>Disabled</Optimization>
	  <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>#DEFINITIONS-64#%(PreprocessorDefinitions)</PreprocessorDefinitions>
	</ClCompile>
	<Link>
	  <AdditionalDependencies>#ADDITIONAL-DEPENDENCIES#%(AdditionalDependencies)</AdditionalDependencies>
	  <SubSystem>#SUBSYSTEM#</SubSystem>
	</Link>
	<PreBuildEvent>
	  <Command>prebuild64.bat</Command>
	</PreBuildEvent>
	<PostBuildEvent>
      <Command>COPY /Y "$(TargetDir)$(ProjectName).exe" "$(ProjectDir)..\..\..\build\win-x64\$(ProjectName)_x64-debug.exe"</Command>
      <Message>Copy resulted EXE</Message>
    </PostBuildEvent>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
	<ClCompile>
	  <WarningLevel>Level3</WarningLevel>
	  <Optimization>Full</Optimization>
	  <FunctionLevelLinking>true</FunctionLevelLinking>
	  <IntrinsicFunctions>true</IntrinsicFunctions>
	  <SDLCheck>true</SDLCheck>
	  <PreprocessorDefinitions>#DEFINITIONS-32#NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
	</ClCompile>
	<Link>
	  <EnableCOMDATFolding>true</EnableCOMDATFolding>
	  <OptimizeReferences>true</OptimizeReferences>
	  <AdditionalDependencies>#ADDITIONAL-DEPENDENCIES#%(AdditionalDependencies)</AdditionalDependencies>
	  <SubSystem>#SUBSYSTEM#</SubSystem>
	</Link>
	<PreBuildEvent>
	  <Command>prebuild.bat</Command>
	</PreBuildEvent>
	<PostBuildEvent>
	  <Command>COPY /Y "$(TargetDir)$(ProjectName).exe" "$(ProjectDir)..\..\..\build\win-x86\$(ProjectName).exe"</Command>
	  <Message>Copy resulted EXE</Message>
	</PostBuildEvent>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
	<ClCompile>
	  <WarningLevel>Level3</WarningLevel>
	  <Optimization>Full</Optimization>
	  <FunctionLevelLinking>true</FunctionLevelLinking>
	  <IntrinsicFunctions>true</IntrinsicFunctions>
	  <SDLCheck>true</SDLCheck>
	  <PreprocessorDefinitions>#DEFINITIONS-64#NDEBUG;(PreprocessorDefinitions)</PreprocessorDefinitions>
	</ClCompile>
	<Link>
	  <EnableCOMDATFolding>true</EnableCOMDATFolding>
	  <OptimizeReferences>true</OptimizeReferences>
	  <AdditionalDependencies>#ADDITIONAL-DEPENDENCIES#%(AdditionalDependencies)</AdditionalDependencies>
	  <SubSystem>#SUBSYSTEM#</SubSystem>
	</Link>
	<PreBuildEvent>
	  <Command>prebuild64.bat</Command>
	</PreBuildEvent>
	<PostBuildEvent>
      <Command>COPY /Y "$(TargetDir)$(ProjectName).exe" "$(ProjectDir)..\..\..\build\win-x64\$(ProjectName)_x64.exe"</Command>
      <Message>Copy resulted EXE</Message>
    </PostBuildEvent>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ResourceCompile Include="#PROJECT-NAME#.rc" />
  </ItemGroup>
  <ItemGroup>
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>}

	vcxproj.filters: {<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
	<Filter Include="Source Files">
	  <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
	  <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>
	</Filter>
	<Filter Include="Header Files">
	  <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>
	  <Extensions>h;hh;hpp;hxx;hm;inl;inc;xsd</Extensions>
	</Filter>
	<Filter Include="Resource Files">
	  <UniqueIdentifier>{67DA6AB6-F800-4c08-8B7A-83BB121AAD01}</UniqueIdentifier>
	  <Extensions>rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav;mfcribbon-ms</Extensions>
	</Filter>
  </ItemGroup>
</Project>}

	build-vs-release-x86: {@echo off
call "c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cd %~dp0 & cd VisualC
msbuild "#PROJECT-NAME#.sln" /p:Configuration=Release /p:Platform="x86"
cd %~dp0
pause}
	
	build-vs-release-x64: {@echo off
call "c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd %~dp0 & cd VisualC
msbuild "Rebol3.sln" /p:Configuration=Release /p:Platform="x64"
cd %~dp0
pause}

	rc-file: {
IDI_APPICON ICON "../../r3-icon.ico"

1 VERSIONINFO
FILEVERSION     3,0,0,0
PRODUCTVERSION  3,0,0,0
BEGIN
  BLOCK "StringFileInfo"
  BEGIN
    BLOCK "040904E4"
    BEGIN
      VALUE "FileDescription", "REBOL (relative expression based object language) console."
      VALUE "FileVersion", "3.0"
      VALUE "InternalName", "Rebol 3 (Oldes branch)"
      VALUE "LegalCopyright", "2012 REBOL Technologies"
      VALUE "OriginalFilename", "#PROJECT-NAME#.exe"
      VALUE "ProductName", "#PROJECT-NAME#"
      VALUE "ProductVersion", "3.0"
    END
  END
  BLOCK "VarFileInfo"
  BEGIN
    VALUE "Translation", 0x409, 1252
  END
END}
]


do %../../src/tools/file-base.r

forall core     [change core join %../../../src/core/ core/1]
forall os       [change os   join %../../../src/os/ os/1]
forall os-win32 [change os-win32 join %../../../src/os/win32/ os-win32/1]
core:     head core
os:       head os
os-win32: head os-win32

vs/Sources: compose/only [
	"Source Core Files" (core)
	"Source Host Files" (append os os-win32)
]
vs/IncludePath-x86:
vs/IncludePath-x64: "..\..\..\src\include;"
vs/PreprocessorDefinitions-x86: {TO_WIN32;REB_CORE;REB_EXE;ENDIAN_LITTLE;_FILE_OFFSET_BITS=64;_CRT_SECURE_NO_WARNINGS;_UNICODE;UNICODE;}
vs/PreprocessorDefinitions-x64: {TO_WIN32_X64;__LLP64__;REB_CORE;REB_EXE;ENDIAN_LITTLE;_FILE_OFFSET_BITS=64;_CRT_SECURE_NO_WARNINGS;_UNICODE;UNICODE;}
vs/Prebuild-x86: {
set REBOL=..\..\prebuild\r3-make-win.exe
set T=../../../src/tools
set OS_ID=0.3.1

%REBOL% %T%/make-headers.r
%REBOL% %T%/make-boot.r %OS_ID%
%REBOL% %T%/make-host-init.r
%REBOL% %T%/make-os-ext.r
%REBOL% %T%/make-host-ext.r
%REBOL% %T%/make-reb-lib.r
}

vs/Prebuild-x64: {
set REBOL=..\..\prebuild\r3-make-win.exe
set T=../../../src/tools
set OS_ID=0.3.40

%REBOL% %T%/make-headers.r
%REBOL% %T%/make-boot.r %OS_ID%
%REBOL% %T%/make-host-init.r
%REBOL% %T%/make-os-ext.r
%REBOL% %T%/make-host-ext.r
%REBOL% %T%/make-reb-lib.r
}

vs/make-project "Rebol3" %./

ask "^/Press ENTER to end."