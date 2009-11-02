; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Psycle Modular Music Creation Studio
AppVerName=Psycle 1.8.6.1
AppPublisher=psycledelics
AppPublisherURL=http://psycle.sourceforge.net/
AppSupportURL=http://psycle.pastnotecut.org/
AppUpdatesURL=http://psycle.sourceforge.net/
DefaultDirName={pf}\Psycle
DefaultGroupName=Psycle
AllowNoIcons=true
InfoBeforeFile=..\..\psycle\doc\for-end-users\readme.txt
InfoAfterFile=..\..\psycle\doc\for-end-users\whatsnew.txt
OutputBaseFilename=PsycleInstallervc2005
SetupIconFile=..\..\psycle\pixmaps\psycle.ico
Compression=lzma
SolidCompression=true
VersionInfoVersion=1.8.6.1
VersionInfoCompany=psycledelics
VersionInfoDescription=Psycle Installer
VersionInfoCopyright=2000-2009 psycledelics
SetupLogging=false
AppCopyright=� psycledelics 2000-2009
PrivilegesRequired=poweruser
AllowRootDirectory=true
ShowLanguageDialog=auto
AppVersion=1.8.6.1
AppID={{8E7D0A7F-B85F-44DC-8C1C-2A2C27BAEA0B}
UninstallDisplayIcon={app}\psycle.exe
ChangesAssociations=true

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: Delete_plugins_cache; Description: deletes the cache of plugin names to force it to be regenerated.; Flags: unchecked; Components: ; Languages: 
Name: Delete_registry_settings; Description: deletes the existing settings in the registry, allowing psycle to regenerate the defaults.

[Files]
Source: ..\msvc\output\release\bin\psycle.exe; DestDir: {app}; Flags: ignoreversion; Tasks: ; Languages: ; Components: Application
Source: ..\msvc\output\release\bin\boost_filesystem-vc80-mt-1_33_1.dll; DestDir: {app}; Flags: ignoreversion; Components: Application; Tasks: ; Languages: 
Source: ..\msvc\output\release\bin\boost_thread-vc80-mt-1_33_1.dll; DestDir: {app}; Flags: ignoreversion; Components: Application
Source: ..\msvc\output\release\bin\universalis.dll; DestDir: {app}; Flags: ignoreversion; Components: Application
Source: ..\msvc\output\release\bin\psycle-plugins\*.dll; DestDir: {app}\psycle-plugins; Flags: ignoreversion; Components: Open_Source_Plugins; Excludes: crasher.dll
Source: ..\..\psycle-plugins\closed-source\*; DestDir: {app}\psycle-plugins\closed-source; Flags: ignoreversion recursesubdirs createallsubdirs; Components: Closed_Source_Plugins
Source: ..\..\psycle\doc\for-end-users\*; DestDir: {app}\docs; Flags: ignoreversion recursesubdirs createallsubdirs; Components: Documentation; Excludes: .svn, Log1.log
Source: ..\..\psycle-plugins\presets\*.prs; DestDir: {app}\PsyclePlugins; Flags: ignoreversion onlyifdoesntexist; Components: Presets
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: ..\..\psycle\doc\*.psy; DestDir: {app}\Songs; Flags: ignoreversion replacesameversion; Components: " Songs"
Source: ..\..\psycle-plugins\src\psycle\plugins\*.txt; DestDir: {app}\Docs; Excludes: license.txt; Flags: recursesubdirs ignoreversion
Source: {app}\Microsoft Visual Studio 8\SDK\v2.0\BootStrapper\Packages\vcredist_x86\vcredist_x86.exe; DestDir: {tmp}; DestName: vcredist_x86.exe; Flags: deleteafterinstall ignoreversion replacesameversion; Components: Application

[INI]
Filename: {app}\psycle.url; Section: InternetShortcut; Key: URL; String: http://psycle.pastnotecut.org; Flags: uninsdeleteentry uninsdeletesectionifempty; Tasks: ; Languages: 

[Icons]
Name: {group}\Psycle; Filename: {app}\psycle.exe
Name: {group}\{cm:UninstallProgram,Psycle}; Filename: {uninstallexe}
Name: {group}\{cm:ProgramOnTheWeb,Psycle}; Filename: {app}\psycle.url
Name: {commondesktop}\Psycle; Filename: {app}\psycle.exe; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Psycle; Filename: {app}\psycle.exe; Tasks: quicklaunchicon
Name: {group}\Documents\How to Skin Psycle; Filename: {app}\Docs\how to skin psycle.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documentation
Name: {group}\Documents\Version History; Filename: {app}\Docs\whatsnew.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documentation
Name: {group}\Documents\Pattern Keys; Filename: {app}\Docs\keys.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documentation
Name: {group}\Documents\MIDI Help; Filename: {app}\Docs\MIDI Help\psyclemidi.html; IconIndex: 0; WorkingDir: {app}\Docs\MIDI input Docs; Components: Documentation
Name: {group}\Documents\Upwego Tutorial; Filename: {app}\Docs\Upwego\upwego5.html; IconIndex: 0; WorkingDir: {app}\Docs\MIDI input Docs; Components: Documentation
Name: {group}\Documents\Tweakings And Commands; Filename: {app}\Docs\tweakings and commands.txt; IconIndex: 0
Name: {group}\{cm:UninstallProgram, Psycle Modular Music Creation Studio}; Filename: {uninstallexe}
Name: {group}\{cm:UninstallProgram, Psycle Modular Music Creation Studio}; Filename: {uninstallexe}

[Run]
Filename: {app}\psycle.exe; Description: {cm:LaunchProgram,Psycle}; Flags: nowait postinstall skipifsilent runasoriginaluser; Components: Application
Filename: {tmp}\Vst-Bundle.exe; WorkingDir: {tmp}; Components: " VstPack"; Flags: runasoriginaluser; StatusMsg: Select the location of your VST Plugins Dir (use Psycle\VstPlugins if in doubt); Tasks: ; Languages: 
Filename: {tmp}\vcredist_x86.exe; WorkingDir: {tmp}; Flags: runascurrentuser; Components: ; Tasks: ; Languages: 
[InstallDelete]
Name: {userappdata}\..\.psycle\psycle.plugin-scan.cache; Type: files; Tasks: " Delete_plugins_cache"; Languages: 
Name: {win}\Psyclekeys.ini; Type: files; Tasks: " Delete_registry_settings"; Components: 
[Components]
Name: Closed_Source_Plugins; Description: Install those plugins of which we don't have the sources; Languages: ; Types: custom full
Name: Open_Source_Plugins; Description: Install those plugins which the developer made their sources available; Types: custom compact full
Name: Application; Description: Main Application and needed dlls; Types: custom compact full
Name: Documentation; Description: Install the documentation of the program; Types: custom full
Name: Microsoft_dlls; Description: Install the needed microsoft runtime dlls; Types: custom full
Name: Presets; Description: Install presets for selected plugins (does not overwrite existing files); Types: custom full
Name: VstPack; Description: Download and Install VST plugins pack; Types: custom full; Languages: ; ExtraDiskSpaceRequired: 19038208
Name: Songs; Description: Install Demo songs; Types: custom full; Languages: 
[Registry]
Root: HKCU; Subkey: software\psycle; Components: ; Tasks: " Delete_registry_settings"; Flags: deletekey; Languages: 
Root: HKCU; Subkey: software\psycle; Flags: uninsdeletekey
Root: HKCU; Subkey: software\AAS\Psycle; Flags: deletekey; Tasks: " Delete_registry_settings"
Root: HKCR; SubKey: .psy; ValueType: string; ValueData: Psycle.Music.File; Flags: uninsdeletekey
Root: HKCR; SubKey: Psycle.Music.File; ValueType: none; Flags: uninsdeletekey
Root: HKCR; SubKey: Psycle.Music.File\Shell\Open\Command; ValueType: string; ValueData: """{app}\psycle.exe"" ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Psycle.Music.File\DefaultIcon; ValueType: string; ValueData: """{app}\psycle.exe"",0"; Flags: uninsdeletevalue
[_ISTool]
UseAbsolutePaths=false
[Dirs]
Name: {app}\Skins
Name: {app}\Songs; Tasks: ; Languages: 
Name: {app}\VstPlugins
Name: {app}\PsyclePlugins
Name: {app}\Docs; Components: Documentation
[Code]
// Function generated by ISTool.
function NextButtonClick(CurPage: Integer): Boolean;
begin
	Result := istool_download(CurPage);
end;
[_ISToolDownload]
Source: http://downloads.sourceforge.net/project/psycle/psycle/1.8.5/psycle-1.8.5-vst-bundle.exe?use_mirror=heanet; DestDir: {tmp}; DestName: Vst-Bundle.exe; Components: " VstPack"; Tasks: ; Languages: 
