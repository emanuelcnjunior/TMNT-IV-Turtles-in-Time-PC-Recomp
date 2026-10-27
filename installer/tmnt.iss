#ifndef StageDir
#error StageDir is required
#endif
#ifndef OutputDir
#define OutputDir "..\dist"
#endif
#ifndef ReleaseVersion
#define ReleaseVersion "1.0.2"
#endif
[Setup]
AppId={{D31700C9-4DD9-49E3-B1CF-A83C60969E40}
AppName=TMNT IV — Turtles in Time PC Recomp
AppVersion={#ReleaseVersion}
DefaultDirName={localappdata}\Programs\TMNTIVRecomp
DefaultGroupName=TMNT IV — Turtles in Time PC Recomp
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.18362
OutputDir={#OutputDir}
OutputBaseFilename=TMNT-IV-Turtles-in-Time-PC-Setup-v{#ReleaseVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupIconFile=tmnt.ico
UninstallDisplayIcon={app}\TmntIvTurtlesInTimeSNESRecomp.exe
CloseApplications=yes
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
[Tasks]
Name: "desktopicon"; Description: "Desktop shortcut / Atalho na Área de Trabalho"; Flags: unchecked
[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Excludes: "*.sfc,*.smc,*.rom,*.bin,rom.cfg"; Flags: ignoreversion recursesubdirs createallsubdirs
[Messages]
english.WelcomeLabel2=This installer does not include a game ROM. During installation, select your own original USA ROM of Teenage Mutant Ninja Turtles IV - Turtles in Time (SNES).
brazilianportuguese.WelcomeLabel2=Este instalador não inclui a ROM do jogo. Durante a instalação, selecione sua própria ROM americana (USA) original de Teenage Mutant Ninja Turtles IV - Turtles in Time (SNES).
[INI]
Filename: "{localappdata}\TMNTIVRecomp\config.ini"; Section: "Graphics"; Key: "Fullscreen"; String: "1"
[Icons]
Name: "{group}\TMNT IV — Turtles in Time PC Recomp"; Filename: "{app}\TmntIvTurtlesInTimeSNESRecomp.exe"
Name: "{autodesktop}\TMNT IV — Turtles in Time PC Recomp"; Filename: "{app}\TmntIvTurtlesInTimeSNESRecomp.exe"; Tasks: desktopicon
[Run]
Filename: "{app}\TmntIvTurtlesInTimeSNESRecomp.exe"; Description: "Jogar / Play TMNT IV"; Flags: nowait postinstall skipifsilent
; User data is deliberately outside {app}. No UninstallDelete wildcard.


[Code]
var
  RomPage: TInputFileWizardPage;
  SelectedRom: String;

function ValidRom(const Path: String): Boolean;
begin
  Result := False;
  if not FileExists(Path) then Exit;
  try
    Result := CompareText(GetSHA256OfFile(Path),
      '5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4') = 0;
  except
    Result := False;
  end;
end;

function InitializeSetup: Boolean;
begin
  Result := True;
  if WizardSilent then begin
    Result := ValidRom(ExpandConstant('{param:ROM|}'));
    if not Result then Log('USA ROM required: supply /ROM="C:\path\game.sfc"');
  end;
end;

procedure InitializeWizard;
begin
  RomPage := CreateInputFilePage(wpSelectDir,
    'ROM americana / USA ROM',
    'Selecione sua ROM original / Select your original ROM',
    'Forneça sua própria ROM americana (USA) de TMNT IV - Turtles in Time para SNES, sem cabeçalho ou patches. A ROM não acompanha este instalador.' + #13#10#13#10 +
    'Supply your own original USA SNES ROM, without a copier header or patches. This installer does not include a ROM.');
  RomPage.Add('ROM USA (*.sfc):', 'SNES ROM (*.sfc)|*.sfc|Todos os arquivos / All files|*.*', '.sfc');
  RomPage.Values[0] := ExpandConstant('{param:ROM|}');
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if CurPageID = RomPage.ID then begin
    Result := ValidRom(RomPage.Values[0]);
    if not Result then
      MsgBox('Selecione a ROM americana (USA) original, sem cabeçalho ou patches.' + #13#10 +
        'Select the original USA ROM, without a copier header or patches.', mbError, MB_OK);
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  SelectedRom := RomPage.Values[0];
  Result := '';
  if not ValidRom(SelectedRom) then
    Result := 'ROM USA obrigatória / USA ROM required. Selecione uma ROM original válida. / Select a valid original ROM. Silent: /ROM="C:\path\game.sfc"';
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ExitCode: Integer;
begin
  if CurStep = ssPostInstall then begin
    if not Exec(ExpandConstant('{app}\TmntIvTurtlesInTimeSNESRecomp.exe'),
      '--import-rom "' + SelectedRom + '"', '', SW_HIDE, ewWaitUntilTerminated, ExitCode) then
      RaiseException('Não foi possível importar a ROM. / Could not import the ROM.');
    if ExitCode <> 0 then
      RaiseException('Falha ao validar ou salvar a ROM. / Failed to validate or save the ROM.');
  end;
end;
