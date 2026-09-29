<#
  gate.ps1 - talk to a BreakBeam gate over USB without opening an IDE.

  Examples (run from a PowerShell prompt in the repo folder):
    .\tools\gate.ps1 -Port COM3                       # just listen for 8 seconds
    .\tools\gate.ps1 -Port COM3 -Send "role finish"   # send a command, then listen
    .\tools\gate.ps1 -Port COM4 -Send "a" -Listen 15  # ALIGN mode numbers for 15 s
    .\tools\gate.ps1 -Port COM3 -Listen 120           # watch a timing session for 2 min

  Opening the port resets the Uno (normal Arduino behaviour), so you see the boot
  banner first. Boot takes ~5 s because of the radio self-test and commands typed
  during it are ignored, so this script waits for the help text before sending.
#>
param(
  [Parameter(Mandatory=$true)][string]$Port,
  [string[]]$Send = @(),
  [int]$Listen = 8,
  [int]$BootWaitMs = 12000,
  [switch]$Raw          # also show the raw 12-byte radio frames (lines starting with @)
)
$Send = ($Send -join ",") -split "," | Where-Object { $_ -ne "" }
$p = New-Object System.IO.Ports.SerialPort $Port, 115200, 'None', 8, 'One'
$p.DtrEnable = $true; $p.ReadTimeout = 250; $p.NewLine = "`n"
$p.Open()
try {
  # Wait for the board to finish booting (it prints "h   this help" at the end of setup).
  $deadline = (Get-Date).AddMilliseconds($BootWaitMs); $booted = $false
  while ((Get-Date) -lt $deadline -and -not $booted) {
    try { $l = $p.ReadLine().TrimEnd("`r"); if ($Raw -or -not $l.StartsWith("@")) { Write-Output ("<- " + $l) }; if ($l -match "this help") { $booted = $true } } catch {}
  }
  Start-Sleep -Milliseconds 300
  foreach ($c in $Send) {
    Write-Output ("-> " + $c); $p.WriteLine($c); Start-Sleep -Milliseconds 300
    try { while ($true) { $l = $p.ReadLine().TrimEnd("`r"); if ($Raw -or -not $l.StartsWith("@")) { Write-Output ("<- " + $l) } } } catch {}
  }
  $deadline = (Get-Date).AddSeconds($Listen)
  while ((Get-Date) -lt $deadline) { try { $l = $p.ReadLine().TrimEnd("`r"); if ($Raw -or -not $l.StartsWith("@")) { Write-Output ("<- " + $l) } } catch {} }
} finally { $p.Close() }
