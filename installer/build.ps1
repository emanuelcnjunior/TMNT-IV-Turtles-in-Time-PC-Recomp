[CmdletBinding()]
param(
 [string]$BuildDir = 'build',
 [string]$Iscc = 'ISCC.exe',
 [string]$CrtDir = '',
 [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location $root
$build = [IO.Path]::GetFullPath((Join-Path $root $BuildDir))
function Invoke-Native([string]$Program, [string[]]$Arguments) {
 $saved = $ErrorActionPreference
 try {
  # Windows PowerShell 5 turns redirected native stderr warnings into errors.
  # The process exit code, rather than stderr text, determines build success.
  $ErrorActionPreference = 'Continue'
  & $Program @Arguments
  $code = $LASTEXITCODE
 } finally { $ErrorActionPreference = $saved }
 if ($code -ne 0) { throw "$Program failed: $code" }
}
if (!$SkipBuild) {
 Invoke-Native cmake @('-S',$root,'-B',$build,'-DCMAKE_BUILD_TYPE=Release','-DTMNT_RETAIL_WINDOWS=ON','-DSNESRECOMP_SETUP_HOST=OFF','-DSNESRECOMP_BUILD_VERSION=1.0.0')
 Invoke-Native cmake @('--build',$build,'--config','Release','--parallel')
}
$cache = Get-Content (Join-Path $build 'CMakeCache.txt') -Raw
if ($cache -notmatch 'TMNT_RETAIL_WINDOWS:BOOL=ON' -or $cache -notmatch 'SNESRECOMP_SETUP_HOST:BOOL=OFF') { throw 'Retail playable build required, not a setup host.' }
if ($cache -notmatch 'CMAKE_BUILD_TYPE:STRING=Release' -and $cache -notmatch 'CMAKE_CONFIGURATION_TYPES:STRING=.*Release') { throw 'Release build required.' }
$exe = Join-Path $build 'Release/TmntIvTurtlesInTimeSNESRecomp.exe'
if (!(Test-Path $exe)) { $exe = Join-Path $build 'TmntIvTurtlesInTimeSNESRecomp.exe' }
if (!(Test-Path $exe)) { throw 'Release executable missing.' }
# Unique staging directory: never package the build tree or arbitrary files.
$stage = Join-Path $root ('dist/stage-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item $exe $stage
$assets = Join-Path (Split-Path $exe) 'assets'
if (!(Test-Path $assets)) { $assets = Join-Path $build 'assets' }
$required = @('fonts/LatoLatin-Regular.ttf','fonts/LatoLatin-Bold.ttf','fonts/NotoSansSymbols2-Regular.ttf','fonts/OpenMoji-black-glyf.ttf','img/brand_mark.tga','img/pad.tga','img/flags.png','img/verdict_ok.tga','img/verdict_bad.tga','img/verdict_none.tga','img/verdict_warn.tga','img/boxart.tga')
foreach ($item in $required) {
 $src = Join-Path $assets $item
 if (!(Test-Path $src)) { throw "Missing launcher resource: $item" }
 $dst = Join-Path (Join-Path $stage 'assets') $item
 New-Item -ItemType Directory -Force (Split-Path $dst) | Out-Null
 Copy-Item $src $dst
}
# Include the existing launcher cover and its source attribution.
$licenses = Join-Path $stage 'licenses'
New-Item -ItemType Directory $licenses | Out-Null
Copy-Item LICENSE (Join-Path $licenses 'project.txt')
Copy-Item launcher_assets/img/BOXART_SOURCE.txt (Join-Path $licenses 'BOXART_SOURCE.txt')
Copy-Item snesrecomp/LICENSE (Join-Path $licenses 'framework.txt')
Copy-Item snesrecomp/THIRD_PARTY_ATTRIBUTION.md $licenses
$licenseRoots = @((Join-Path $root 'recomp-ui'), (Join-Path $build '_deps/sdl3-src')) | Where-Object { Test-Path $_ }
Get-ChildItem $licenseRoots -Recurse -File -Include '*LICENSE*','*COPYING*','*OFL*' | ForEach-Object {
 $name = ($_.FullName.Substring($root.Length+1) -replace '[:\\/]', '_')
 Copy-Item $_.FullName (Join-Path $licenses $name)
}
if (!$CrtDir) {
 if ($env:VCToolsRedistDir) { $CrtDir = Join-Path $env:VCToolsRedistDir 'x64/Microsoft.VC143.CRT' }
 else { throw 'Pass -CrtDir pointing to the licensed Visual C++ x64 redistributable CRT directory.' }
}
foreach ($dll in @('msvcp140.dll','vcruntime140.dll','vcruntime140_1.dll')) {
 if (!(Test-Path (Join-Path $CrtDir $dll))) { throw "Missing redistributable: $dll" }
}
Copy-Item (Join-Path $CrtDir '*.dll') $stage
Copy-Item recomp-ui/assets/common/fonts/NOTICE.md (Join-Path $licenses 'fonts-NOTICE.md')
Copy-Item recomp-ui/assets/common/img/NOTICE.md (Join-Path $licenses 'images-NOTICE.md')
Invoke-Native cmake @("-DEXE=$(Join-Path $stage (Split-Path $exe -Leaf))", "-DCRT=$CrtDir", "-DSTAGE=$stage", '-P', 'installer/dependencies.cmake')
$forbidden = Get-ChildItem $stage -File -Recurse | Where-Object {
 $_.Extension -in @('.sfc','.smc','.rom','.bin','.bps','.ips','.py','.rs','.obj','.pdb') -or
 $_.Name -in @('rom.cfg','CMakeCache.txt') -or
 (Get-FileHash $_.FullName -Algorithm SHA256).Hash -eq '5B82CDD6F2DA56F43680D6A5021FAEBE2E06036D30602C1A7917AA414CF8B5F4'
}
if ($forbidden) { throw 'Forbidden ROM, extracted data, patch or development file in stage.' }
$manifest = Get-ChildItem $stage -File -Recurse | ForEach-Object {
 [ordered]@{ path=$_.FullName.Substring($stage.Length+1); bytes=$_.Length; sha256=(Get-FileHash $_.FullName -Algorithm SHA256).Hash }
}
$manifest | ConvertTo-Json | Set-Content -Encoding UTF8 (Join-Path $stage 'manifest.json')
Invoke-Native $Iscc @("/DStageDir=$stage", "/DOutputDir=$(Join-Path $root 'dist')", 'installer/tmnt.iss')
$setup = Join-Path $root 'dist/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.0.exe'
(Get-FileHash $setup -Algorithm SHA256).Hash + '  ' + (Split-Path $setup -Leaf) | Set-Content ($setup + '.sha256')
Write-Output $setup
