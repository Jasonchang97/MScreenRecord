; MScreenRecord Setup Script
; Requires Inno Setup

#define MyAppName "MScreenRecord"
#define MyAppVersion "1.4.1"
#define MyAppPublisher "zhangjisheng"
#define MyAppExeName "MScreenRecord.exe"
#define MyBuildDir "dist\MScreenRecord_v1.4.1_x86"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
AppId={{5E4F8B2A-3D1C-4E5F-9A0B-7C8D9E0F1G2H}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=.
OutputBaseFilename=MScreenRecord_Setup_v{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\{#MyAppExeName}
UsePreviousTasks=no

;[Languages]
;Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#MyBuildDir}\audio_sniffer.dll"; DestDir: "{app}"; Flags: regserver ignoreversion
Source: "{#MyBuildDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; 强制包含 VC++ 运行时 (如果有)
; Source: "vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{group}\{#MyAppName} (Debug Mode)"; Filename: "{app}\launcher.bat"; WorkingDir: "{app}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; WorkingDir: "{app}"; IconFilename: "{app}\{#MyAppExeName}"


[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Messages]
BeveledLabel=Author: {#MyAppPublisher}

[Code]
// 检测旧版本并提示卸载
function GetUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstPathOld: String;
begin
  sUnInstPath := '';
  sUnInstPathOld := '';
  // 32-bit registry view
  if RegQueryStringValue(HKLM, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#MyAppName}_is1', 'UninstallString', sUnInstPath) then
    Result := sUnInstPath
  else
    Result := '';
end;

function InitializeSetup(): Boolean;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin
  Result := True;
  sUnInstallString := GetUninstallString();
  if sUnInstallString <> '' then begin
    if MsgBox('检测到已安装旧版本，是否先卸载？' + #13#10 + '如果不卸载，可能会导致文件冲突。', mbConfirmation, MB_YESNO) = IDYES then begin
      sUnInstallString := RemoveQuotes(sUnInstallString);
      Exec(sUnInstallString, '/SILENT', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
    end;
  end;
end;
