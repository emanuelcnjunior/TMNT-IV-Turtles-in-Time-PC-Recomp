[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Rom)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$setup = Join-Path $root 'dist/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.0.exe'
$install = Join-Path $root ('build/install-test-' + [guid]::NewGuid().ToString('N'))
$data = Join-Path $env:LOCALAPPDATA 'TMNTIVRecomp'
if (Test-Path $data) { throw 'Use a fresh Windows test account. Existing user data will not be altered.' }
$Rom = (Resolve-Path $Rom).Path
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class TmntWindowTest {
 public delegate bool Callback(IntPtr h, IntPtr p);
 [DllImport("user32.dll")] public static extern bool EnumWindows(Callback c, IntPtr p);
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
 [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
 public static IntPtr Find(uint pid) { IntPtr found=IntPtr.Zero; EnumWindows((h,p)=>{uint id;GetWindowThreadProcessId(h,out id);if(id==pid)found=h;return true;},IntPtr.Zero);return found; }
}
'@
function Install {
 $p = Start-Process $setup -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/NOICONS',('/ROM="'+$Rom+'"'),('/DIR="'+$install+'"')) -WindowStyle Hidden -Wait -PassThru
 if ($p.ExitCode) { throw "Install failed: $($p.ExitCode)" }
}
function Uninstall {
 $p = Start-Process (Join-Path $install 'unins000.exe') -ArgumentList '/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART' -WindowStyle Hidden -Wait -PassThru
 if ($p.ExitCode) { throw 'Uninstall failed' }
}
Install
$exe = Join-Path $install 'TmntIvTurtlesInTimeSNESRecomp.exe'
$forbidden = Get-ChildItem $install -Recurse -File | Where-Object {
 $_.Extension -in @('.sfc','.smc','.rom','.bin') -or $_.Name -eq 'rom.cfg' -or
 (Get-FileHash $_.FullName -Algorithm SHA256).Hash -eq '5B82CDD6F2DA56F43680D6A5021FAEBE2E06036D30602C1A7917AA414CF8B5F4'
}
if ($forbidden) { throw 'Installer included a ROM or ROM cache.' }
'PASS: installed payload contains no ROM or ROM cache'
foreach ($dll in @('msvcp140.dll','vcruntime140.dll','vcruntime140_1.dll')) {
 if (!(Test-Path (Join-Path $install $dll))) { throw "Missing $dll" }
}
$canonical = Join-Path $data 'Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc'
$expected = '5B82CDD6F2DA56F43680D6A5021FAEBE2E06036D30602C1A7917AA414CF8B5F4'
if ((Get-FileHash $canonical).Hash -ne $expected) { throw 'Stored ROM mismatch' }
'PASS: import stores the canonical file outside install directory'
Uninstall
if (Test-Path $exe) { throw 'Uninstaller left executable' }
if ((Get-FileHash $canonical).Hash -ne $expected) { throw 'Uninstall modified user ROM' }
'PASS: uninstall removes program and preserves user ROM'
Install
$p = Start-Process $exe -PassThru
Start-Sleep -Seconds 5
if ($p.HasExited) { throw "Game exited prematurely: $($p.ExitCode)" }
$p.Refresh(); $window = $p.MainWindowHandle
if ($window -eq [IntPtr]::Zero) { throw 'Game window did not open after reinstall' }
[TmntWindowTest]::SendMessage($window, 0x10, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
if (!$p.WaitForExit(10000)) { $p.Kill(); throw 'Game did not close' }
if ($p.ExitCode) { throw "Game failed: $($p.ExitCode)" }
'PASS: first run after reinstall finds cached ROM and starts the game'
Uninstall
'User ROM remains in LocalAppData, as designed. Delete explicitly in this test account if desired.'
