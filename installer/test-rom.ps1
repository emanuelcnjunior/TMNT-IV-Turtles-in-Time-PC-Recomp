[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Rom, [string]$Exe = 'build/TmntIvTurtlesInTimeSNESRecomp.exe')
$ErrorActionPreference = 'Stop'
$Exe = (Resolve-Path $Exe).Path
$Rom = (Resolve-Path $Rom).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('tmnt-rom-test-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $temp | Out-Null
$results = @()
function Probe([string]$Name, [string]$Path, [int]$Expected) {
 $process = Start-Process $Exe -ArgumentList ('--validate-rom "' + $Path + '"') -WindowStyle Hidden -Wait -PassThru
 if ($process.ExitCode -ne $Expected) { throw "$Name returned $($process.ExitCode), expected $Expected" }
 $script:results += [pscustomobject]@{ test=$Name; result='PASS'; exit=$process.ExitCode }
}
# Real positive fixture is supplied privately, never committed or packaged.
Probe 'Original USA ROM' $Rom 0
$renamed = Join-Path $temp 'nome diferente 日本.sfc'
Copy-Item $Rom $renamed
Probe 'Renamed valid content and Unicode filename' $renamed 0
$bytes = [IO.File]::ReadAllBytes($Rom)
$bytes[12345] = $bytes[12345] -bxor 1
$bad = Join-Path $temp 'wrong-hash-USA.sfc'
[IO.File]::WriteAllBytes($bad, $bytes)
Probe 'USA ROM with one corrupt byte' $bad 2
# Synthetic region fixture: identity must reject regardless of SNES region tag.
$bytes[0x7fd9] = 2
$region = Join-Path $temp 'other-region.sfc'
[IO.File]::WriteAllBytes($region, $bytes)
Probe 'Different region content (synthetic)' $region 2
$invalid = Join-Path $temp 'invalid.sfc'
[IO.File]::WriteAllText($invalid, 'not a ROM')
Probe 'Invalid file and incorrect size' $invalid 2
$header = Join-Path $temp 'headered.sfc'
[IO.File]::WriteAllBytes($header, ([byte[]]::new(512) + [IO.File]::ReadAllBytes($Rom)))
Probe 'Copier header rejected (entire-file policy)' $header 2
Probe 'Missing file' (Join-Path $temp 'missing.sfc') 2
$results | Format-Table
$results | ConvertTo-Json | Set-Content -Encoding UTF8 (Join-Path (Split-Path $Exe) 'rom-validation-results.json')
# Remove only the individually named temporary ROM fixtures.
foreach ($file in @($renamed,$bad,$region,$invalid,$header)) { Remove-Item -LiteralPath $file }
Remove-Item -LiteralPath $temp
