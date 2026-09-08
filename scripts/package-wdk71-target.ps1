param(
    [Parameter(Mandatory = $true)]
    [string] $TargetName,

    [Parameter(Mandatory = $true)]
    [string] $RepositoryRoot
)

$ErrorActionPreference = 'Stop'

$packages = @{
    'windows-xp-x86' = @{
        Inf = 'driver/Dist/uata_xp.inf'; SourceSection = 'UNIATA_HDC.NTx86.5.1'
        Decoration = 'NTx86.5.1'; Block = @('NTx86.5.2', 'NTx86.6.0', 'NTx86.6.1', 'NTx86.6.2', 'NTx86.6.3', 'NTx86.10.0')
        DisplayName = 'Windows XP x86'
    }
    'server-2003-x86' = @{
        Inf = 'driver/Dist/2k3/uata_2k3.inf'; SourceSection = 'UNIATA_HDC.NTx86.5.2'
        Decoration = 'NTx86.5.2'; Block = @('NTx86.6.0', 'NTx86.6.1', 'NTx86.6.2', 'NTx86.6.3', 'NTx86.10.0')
        DisplayName = 'Windows Server 2003 x86'
    }
    'windows-xp-server-2003-x64' = @{
        Inf = 'driver/Dist/2k3/uata_2k3.inf'; SourceSection = 'UNIATA_HDC.NTx86.5.2'
        Decoration = 'NTamd64.5.2'; Block = @('NTamd64.6.0', 'NTamd64.6.1', 'NTamd64.6.2', 'NTamd64.6.3', 'NTamd64.10.0')
        DisplayName = 'Windows XP x64 / Server 2003 x64'
    }
    'vista-server-2008-x86' = @{
        Inf = 'driver/Dist/uata_w7.inf'; SourceSection = 'UNIATA_HDC.NTx86.6.1'
        Decoration = 'NTx86.6.0'; Block = @('NTx86.6.1', 'NTx86.6.2', 'NTx86.6.3', 'NTx86.10.0')
        DisplayName = 'Windows Vista / Server 2008 x86'
    }
    'vista-server-2008-x64' = @{
        Inf = 'driver/Dist/uata_w7.inf'; SourceSection = 'UNIATA_HDC.NTamd64.6.1'
        Decoration = 'NTamd64.6.0'; Block = @('NTamd64.6.1', 'NTamd64.6.2', 'NTamd64.6.3', 'NTamd64.10.0')
        DisplayName = 'Windows Vista / Server 2008 x64'
    }
    'windows-7-x86' = @{
        Inf = 'driver/Dist/uata_w7.inf'; SourceSection = 'UNIATA_HDC.NTx86.6.1'
        Decoration = 'NTx86.6.1'; Block = @('NTx86.6.2', 'NTx86.6.3', 'NTx86.10.0')
        DisplayName = 'Windows 7 x86'
    }
    'windows-7-x64' = @{
        Inf = 'driver/Dist/uata_w7.inf'; SourceSection = 'UNIATA_HDC.NTamd64.6.1'
        Decoration = 'NTamd64.6.1'; Block = @('NTamd64.6.2', 'NTamd64.6.3', 'NTamd64.10.0')
        DisplayName = 'Windows 7 x64'
    }
}

if (-not $packages.ContainsKey($TargetName)) {
    throw "Unknown package target '$TargetName'."
}

$package = $packages[$TargetName]
$artifactDirectory = Join-Path $RepositoryRoot "artifact/$TargetName"
$driver = Join-Path $artifactDirectory 'uniata.sys'
if (-not (Test-Path -LiteralPath $driver)) {
    throw "Cannot package $TargetName because $driver does not exist."
}

$sourceInf = Join-Path $RepositoryRoot $package.Inf
$inf = [IO.File]::ReadAllText($sourceInf)
$manufacturerDecorations = @($package.Decoration) + @($package.Block)
$manufacturer = "%ALTERWARE%=UNIATA_HDC, $($manufacturerDecorations -join ', ')"
$inf = [regex]::Replace(
    $inf,
    '(?m)^%ALTERWARE%=UNIATA_HDC(?:,[^\r\n]*)?\r?$',
    $manufacturer
)
$sourceHeader = [regex]::Escape("[$($package.SourceSection)]")
$targetHeader = "[UNIATA_HDC.$($package.Decoration)]"
$sectionRegex = [regex]::new("(?m)^$sourceHeader\r?$")
$inf = $sectionRegex.Replace($inf, $targetHeader, 1)
if ($inf -notmatch [regex]::Escape($targetHeader)) {
    throw "$sourceInf does not contain model section [$($package.SourceSection)]."
}
$blockedSections = @($package.Block | ForEach-Object {
    "`r`n[UNIATA_HDC.$_]`r`n; Intentionally empty: this package is locked to $($package.Decoration).`r`n"
}) -join ''
$inf += $blockedSections

if ($package.Decoration.StartsWith('NTamd64')) {
    $inf = $inf.Replace('[SourceDisksNames.x86]', '[SourceDisksNames.amd64]')
}
$infPath = Join-Path $artifactDirectory 'uniata.inf'
[IO.File]::WriteAllText($infPath, $inf, [Text.Encoding]::ASCII)

$hardwareIds = [regex]::Matches(
    $inf,
    '(?im)^[^;\r\n]*?=\s*uniata_Inst\s*,+\s*(PCI\\[^\s;]+)'
) | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique

$hardwareLines = @($hardwareIds | ForEach-Object { 'id = "{0}", "uniata"' -f $_ })
$txtsetup = @"
[Disks]
d1 = "UniATA 0.48 - $($package.DisplayName)", \txtsetup.oem, \

[Defaults]
scsi = uniata

[scsi]
uniata = "UniATA storage controller ($($package.DisplayName))", uniata

[Files.scsi.uniata]
driver = d1, uniata.sys, uniata
inf = d1, uniata.inf

[HardwareIds.scsi.uniata]
$($hardwareLines -join "`r`n")

[Config.scsi.uniata]
value = Parameters\PnpInterface, 5, REG_DWORD, 1
"@
[IO.File]::WriteAllText(
    (Join-Path $artifactDirectory 'txtsetup.oem'),
    $txtsetup,
    [Text.Encoding]::ASCII
)

$readme = @"
UniATA 0.48 package for $($package.DisplayName)

Files:
  uniata.sys   - target-specific driver binary
  uniata.inf   - PnP installation file ($($package.Decoration))
  txtsetup.oem - text-mode Setup integration metadata

The INF platform decoration prevents installation on the wrong CPU architecture
and declares the target platform as $($package.Decoration).
On NT 5.x, txtsetup.oem can be used for text-mode/F6 integration.  Windows
Vista and newer use offline INF/SYS injection (for example from WinPE) instead;
txtsetup.oem is included there only for package-layout consistency.
The INF deliberately retains CatalogFile=uniata.cat.  Generate that catalog
from this final package and sign both the catalog and driver before release.
In particular, 64-bit Windows normally requires a properly signed package.
"@
[IO.File]::WriteAllText(
    (Join-Path $artifactDirectory 'README.txt'),
    $readme,
    [Text.Encoding]::ASCII
)
