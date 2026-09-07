Param(
    [Parameter(Position = 0)]
    [string]$VcxprojPath,
    [string]$OutputPath,
    [string]$ConfigPath,
    [string]$Profile,
    [switch]$ShowAssignments
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

. "$PSScriptRoot\VcxprojFilters.Common.ps1"

function Show-Assignments {
    param(
        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [object[]]$Items
    )

    Write-Host "[Expected assignments] Files by filter"
    $groups = $Items |
        Select-Object Type, Include, Filter |
        Group-Object -Property { if ($_.Filter) { $_.Filter } else { "(none)" } } |
        Sort-Object Name

    foreach ($g in $groups) {
        Write-Host ("== {0} ==" -f $g.Name)
        $g.Group | Sort-Object Type, Include | ForEach-Object {
            Write-Host ("  {0}  {1}" -f $_.Type, $_.Include)
        }
    }
}

function Key-Item {
    param(
        [Parameter(Mandatory = $true)]
        [object]$Item
    )

    return "{0}|{1}" -f $Item.Type, $Item.Include
}

$context = Resolve-VcxprojFiltersContext `
    -VcxprojPath $VcxprojPath `
    -OutputPath $OutputPath `
    -ConfigPath $ConfigPath `
    -Profile $Profile

$expected = Build-ExpectedFiltersDocument `
    -ResolvedVcxprojPath $context.VcxprojPath `
    -ProjectDirectory $context.ProjectDirectory `
    -StripPrefixes $context.StripPrefixes `
    -RootFileNames $context.RootFileNames

$namespaceUri = $expected.NamespaceUri
$expectedXml = $expected.Document

Write-Host ("Target: {0}" -f $context.VcxprojPath)
if ($context.Profile) {
    Write-Host ("Profile: {0}" -f $context.Profile)
}
Write-Host ("Filters file: {0}" -f $context.OutputPath)

if (-not (Test-Path -LiteralPath $context.OutputPath)) {
    Write-Host "Result: .filters does not exist. Generate-VcxprojFilters.ps1 will create it."
    $expectedFilters = @(Get-FiltersDefinitionList -FiltersXml $expectedXml -NamespaceUri $namespaceUri)
    $expectedItems = @(Get-ItemEntries -FiltersXml $expectedXml -NamespaceUri $namespaceUri)
    Write-Host ("Expected: {0} filter definitions / {1} items" -f $expectedFilters.Count, $expectedItems.Count)

    if ($ShowAssignments) {
        Show-Assignments -Items $expectedItems
    }

    exit 1
}

[xml]$actualXml = Get-Content -LiteralPath $context.OutputPath -Raw

$expectedFilters = @(Get-FiltersDefinitionList -FiltersXml $expectedXml -NamespaceUri $namespaceUri)
$actualFilters = @(Get-FiltersDefinitionList -FiltersXml $actualXml -NamespaceUri $namespaceUri)

$expectedItems = @(Get-ItemEntries -FiltersXml $expectedXml -NamespaceUri $namespaceUri)
$actualItems = @(Get-ItemEntries -FiltersXml $actualXml -NamespaceUri $namespaceUri)

if ($ShowAssignments) {
    Show-Assignments -Items $expectedItems
}

$filterDiff = @()
if ($actualFilters.Count -gt 0 -or $expectedFilters.Count -gt 0) {
    $filterDiff = @(Compare-Object `
        -ReferenceObject @($actualFilters | Sort-Object -Unique) `
        -DifferenceObject @($expectedFilters | Sort-Object -Unique))
}

$actualByKey = @{}
foreach ($it in $actualItems) {
    $k = Key-Item -Item $it
    if (-not $actualByKey.ContainsKey($k)) { $actualByKey[$k] = @() }
    $actualByKey[$k] += $it
}

$expectedByKey = @{}
foreach ($it in $expectedItems) {
    $k = Key-Item -Item $it
    if (-not $expectedByKey.ContainsKey($k)) { $expectedByKey[$k] = @() }
    $expectedByKey[$k] += $it
}

$allKeys = @($actualByKey.Keys + $expectedByKey.Keys) | Sort-Object -Unique

$itemsAdded = @()
$itemsRemoved = @()
$itemsMoved = @()

foreach ($k in $allKeys) {
    $a = @()
    if ($actualByKey.ContainsKey($k)) { $a = $actualByKey[$k] }
    $e = @()
    if ($expectedByKey.ContainsKey($k)) { $e = $expectedByKey[$k] }

    if ($a.Count -eq 0 -and $e.Count -gt 0) {
        foreach ($x in $e) { $itemsAdded += $x }
        continue
    }
    if ($a.Count -gt 0 -and $e.Count -eq 0) {
        foreach ($x in $a) { $itemsRemoved += $x }
        continue
    }

    foreach ($ai in $a) {
        $match = $e | Where-Object { $_.Filter -eq $ai.Filter } | Select-Object -First 1
        if (-not $match) {
            $new = $e | Select-Object -First 1
            $itemsMoved += [PSCustomObject]@{
                Type = $ai.Type
                Include = $ai.Include
                From = $ai.Filter
                To = $new.Filter
            }
        }
    }
}

$hasChanges = $false

if ($filterDiff.Count -gt 0) {
    $hasChanges = $true
    $adds = @($filterDiff | Where-Object SideIndicator -eq '=>' | Select-Object -ExpandProperty InputObject)
    $removes = @($filterDiff | Where-Object SideIndicator -eq '<=' | Select-Object -ExpandProperty InputObject)

    if ($adds.Count -gt 0) {
        Write-Host "[Filter definitions] To add:"
        $adds | Sort-Object | ForEach-Object { Write-Host ("  + {0}" -f $_) }
    }
    if ($removes.Count -gt 0) {
        Write-Host "[Filter definitions] To remove:"
        $removes | Sort-Object | ForEach-Object { Write-Host ("  - {0}" -f $_) }
    }
}

if ($itemsAdded.Count -gt 0) {
    $hasChanges = $true
    Write-Host "[Items] To add:"
    $itemsAdded | Sort-Object Type, Include | ForEach-Object {
        $f = if ($_.Filter) { $_.Filter } else { "(none)" }
        Write-Host ("  + {0}  Include={1}  Filter={2}" -f $_.Type, $_.Include, $f)
    }
}

if ($itemsRemoved.Count -gt 0) {
    $hasChanges = $true
    Write-Host "[Items] To remove:"
    $itemsRemoved | Sort-Object Type, Include | ForEach-Object {
        $f = if ($_.Filter) { $_.Filter } else { "(none)" }
        Write-Host ("  - {0}  Include={1}  Filter={2}" -f $_.Type, $_.Include, $f)
    }
}

if ($itemsMoved.Count -gt 0) {
    $hasChanges = $true
    Write-Host "[Items] Filter changes:"
    $itemsMoved | Sort-Object Type, Include | ForEach-Object {
        $from = if ($_.From) { $_.From } else { "(none)" }
        $to = if ($_.To) { $_.To } else { "(none)" }
        Write-Host ("  * {0}  Include={1}  {2} -> {3}" -f $_.Type, $_.Include, $from, $to)
    }
}

if (-not $hasChanges) {
    Write-Host "Result: No changes. Generate-VcxprojFilters.ps1 would keep the same content."
    exit 0
}

Write-Host "Result: Changes found. Generate-VcxprojFilters.ps1 will update the .filters file."
exit 1
