Param(
    [Parameter(Position = 0)]
    [string]$VcxprojPath,
    [string]$OutputPath,
    [string]$ConfigPath,
    [string]$Profile
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

. "$PSScriptRoot\VcxprojFilters.Common.ps1"

$context = Resolve-VcxprojFiltersContext `
    -VcxprojPath $VcxprojPath `
    -OutputPath $OutputPath `
    -ConfigPath $ConfigPath `
    -Profile $Profile

$existingFilterIds = Get-ExistingFilterIds -OutputPath $context.OutputPath
$expected = Build-ExpectedFiltersDocument `
    -ResolvedVcxprojPath $context.VcxprojPath `
    -ProjectDirectory $context.ProjectDirectory `
    -StripPrefixes $context.StripPrefixes `
    -RootFileNames $context.RootFileNames `
    -ExistingFilterIds $existingFilterIds

Save-FiltersDocument -Document $expected.Document -OutputPath $context.OutputPath

Write-Host ("Generated filters: {0}" -f $context.OutputPath)
