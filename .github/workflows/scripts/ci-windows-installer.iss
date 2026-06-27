#define MyAppName "JS8Call-CN"
#define MyAppVersion "3.0.1.1-CN-Beta1"
#define MyAppPublisher "BG7IPB"
#define MyAppURL "https://github.com/bg7ipb/JS8Call-improved/"
#define MyAppExeName "JS8CALL-CN.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
AppId={{67820EE0-ABE4-4386-8DC4-0637D0B94601}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
VersionInfoVersion=3.0.1.1
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
LicenseFile=D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\LICENSE
DefaultDirName={autopf}\{#MyAppName}
UsePreviousAppDir=no
; UninstallDisplayIcon={app}\{#MyAppExeName}
; "ArchitecturesAllowed=x64compatible" specifies that Setup cannot run
; on anything but x64 and Windows 11 on Arm.
ArchitecturesAllowed=x64compatible
; "ArchitecturesInstallIn64BitMode=x64compatible" requests that the
; install be done in "64-bit mode" on x64 or Windows 11 on Arm,
; meaning it should use the native 64-bit Program Files directory and
; the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only).
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call
; This can be changed from the ci-windows.yml for release builds
OutputBaseFilename=JS8Call-installer
SetupIconFile=D:\a\JS8Call-improved\JS8Call-improved\icons\windows-icons\js8call.ico
UninstallDisplayIcon=D:\a\JS8Call-improved\JS8Call-improved\icons\windows-icons\js8call.ico
SolidCompression=yes
; WizardStyle introduced in 6.6.0, Github has 6.5.4
;WizardStyle=modern dynamic

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\multimedia\*"; DestDir: "{app}\multimedia"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\a\JS8Call-improved\JS8Call-improved\icons\windows-icons\js8call.ico"; DestDir: "{app}"; Flags: ignoreversion
; ILC codebook: runtime loads via QFile from applicationDirPath()/codebook_cn.csv (JS8_Main/main.cpp); ship next to exe.
Source: "D:\a\JS8Call-improved\JS8Call-improved\build\JS8Call\codebook_cn.csv"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\js8call.ico"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\js8call.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
