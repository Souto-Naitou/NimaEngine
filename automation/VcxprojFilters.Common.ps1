Set-StrictMode -Version Latest

$script:VcxprojFiltersNamespaceUri = "http://schemas.microsoft.com/developer/msbuild/2003"

function Get-OptionalPropertyValue {
    param(
        [object]$InputObject,
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    if (-not $InputObject) {
        return $null
    }

    $property = $InputObject.PSObject.Properties[$Name]
    if (-not $property) {
        return $null
    }

    return $property.Value
}

function Resolve-VcxprojFiltersContext {
    param(
        [string]$VcxprojPath,
        [string]$OutputPath,
        [string]$ConfigPath,
        [string]$Profile
    )

    if (-not $ConfigPath) {
        $ConfigPath = Join-Path -Path $PSScriptRoot -ChildPath "Generate-VcxprojFilters.config.json"
    }

    $resolvedConfigPath = $null
    $config = $null
    if (Test-Path -LiteralPath $ConfigPath) {
        $resolvedConfigPath = (Resolve-Path -LiteralPath $ConfigPath).Path
        $config = Get-Content -LiteralPath $resolvedConfigPath -Raw | ConvertFrom-Json
    }

    $profileConfig = $null
    $defaultProfile = Get-OptionalPropertyValue -InputObject $config -Name "DefaultProfile"
    if (-not $VcxprojPath -and -not $Profile -and $defaultProfile) {
        $Profile = [string]$defaultProfile
    }

    if ($Profile) {
        if (-not $config) {
            throw "Profile '$Profile' was specified, but config file was not found: $ConfigPath"
        }

        $profiles = Get-OptionalPropertyValue -InputObject $config -Name "Profiles"
        $profileConfig = Get-OptionalPropertyValue -InputObject $profiles -Name $Profile
        if (-not $profileConfig) {
            throw "Profile '$Profile' was not found in config file: $resolvedConfigPath"
        }
    }

    $profileVcxprojPath = Get-OptionalPropertyValue -InputObject $profileConfig -Name "VcxprojPath"
    if (-not $VcxprojPath -and $profileVcxprojPath) {
        $VcxprojPath = [string]$profileVcxprojPath
        if ($resolvedConfigPath -and -not [System.IO.Path]::IsPathRooted($VcxprojPath)) {
            $configDirectory = Split-Path -Parent $resolvedConfigPath
            $VcxprojPath = Join-Path -Path $configDirectory -ChildPath $VcxprojPath
        }
    }

    if (-not $VcxprojPath) {
        throw "Specify -VcxprojPath, -Profile, or DefaultProfile in the config file."
    }

    $resolvedVcxprojPath = (Resolve-Path -LiteralPath $VcxprojPath).Path
    $profileOutputPath = Get-OptionalPropertyValue -InputObject $profileConfig -Name "OutputPath"
    if (-not $OutputPath -and $profileOutputPath) {
        $OutputPath = [string]$profileOutputPath
        if ($resolvedConfigPath -and -not [System.IO.Path]::IsPathRooted($OutputPath)) {
            $configDirectory = Split-Path -Parent $resolvedConfigPath
            $OutputPath = Join-Path -Path $configDirectory -ChildPath $OutputPath
        }
    }

    if (-not $OutputPath) {
        $OutputPath = "$resolvedVcxprojPath.filters"
    }

    $stripPrefixes = @()
    $profileStripPrefixes = Get-OptionalPropertyValue -InputObject $profileConfig -Name "StripPrefixes"
    if ($profileStripPrefixes) {
        $stripPrefixes = @($profileStripPrefixes)
    }

    $rootFileNames = @()
    $profileRootFileNames = Get-OptionalPropertyValue -InputObject $profileConfig -Name "RootFileNames"
    if ($profileRootFileNames) {
        $rootFileNames = @($profileRootFileNames)
    }

    return [PSCustomObject]@{
        VcxprojPath = $resolvedVcxprojPath
        ProjectDirectory = Split-Path -Parent $resolvedVcxprojPath
        OutputPath = $OutputPath
        ConfigPath = $resolvedConfigPath
        Profile = $Profile
        StripPrefixes = $stripPrefixes
        RootFileNames = $rootFileNames
    }
}

function Normalize-IncludePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$IncludePath,
        [Parameter(Mandatory = $true)]
        [string]$ProjectDirectory
    )

    $normalized = $IncludePath.Trim()
    if ([string]::IsNullOrWhiteSpace($normalized)) {
        return $null
    }

    $normalized = $normalized -replace '/', '\'

    foreach ($macro in @('$(ProjectDir)', '$(SolutionDir)')) {
        if ($normalized.StartsWith($macro, [System.StringComparison]::OrdinalIgnoreCase)) {
            $normalized = $normalized.Substring($macro.Length).TrimStart('\')
            break
        }
    }

    while ($normalized.StartsWith(".\", [System.StringComparison]::OrdinalIgnoreCase)) {
        $normalized = $normalized.Substring(2)
    }

    if ([System.IO.Path]::IsPathRooted($normalized)) {
        $projectPrefix = $ProjectDirectory.TrimEnd('\')
        if ($normalized.StartsWith($projectPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            $normalized = $normalized.Substring($projectPrefix.Length).TrimStart('\')
        }
    }

    while ($normalized.Contains("\.\")) {
        $normalized = $normalized.Replace("\.\", "\")
    }

    return $normalized
}

function Resolve-ActualRelativePathCasing {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath,
        [Parameter(Mandatory = $true)]
        [string]$ProjectDirectory
    )

    $parts = @($RelativePath -split '\\' | Where-Object { $_ })
    if ($parts.Count -eq 0) {
        return $RelativePath
    }

    $currentDirectory = $ProjectDirectory
    $resolvedParts = @()
    foreach ($part in $parts) {
        $match = $null
        if (Test-Path -LiteralPath $currentDirectory -PathType Container) {
            $match = Get-ChildItem -LiteralPath $currentDirectory -Force |
                Where-Object { $_.Name -ieq $part } |
                Select-Object -First 1
        }

        if ($match) {
            $resolvedParts += $match.Name
            $currentDirectory = $match.FullName
        } else {
            $resolvedParts += $part
            $currentDirectory = Join-Path -Path $currentDirectory -ChildPath $part
        }
    }

    return ($resolvedParts -join '\')
}

function Get-FilterPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$IncludePath,
        [Parameter(Mandatory = $true)]
        [string]$ProjectDirectory,
        [AllowEmptyCollection()]
        [string[]]$StripPrefixes = @(),
        [AllowEmptyCollection()]
        [string[]]$RootFileNames = @()
    )

    $normalized = Normalize-IncludePath -IncludePath $IncludePath -ProjectDirectory $ProjectDirectory
    if (-not $normalized) {
        return $null
    }

    $normalized = Resolve-ActualRelativePathCasing -RelativePath $normalized -ProjectDirectory $ProjectDirectory
    $fileName = [System.IO.Path]::GetFileName($normalized)
    if ($RootFileNames | Where-Object { $_ -ieq $fileName }) {
        return $null
    }

    $directory = [System.IO.Path]::GetDirectoryName($normalized)
    if ([string]::IsNullOrWhiteSpace($directory)) {
        return $null
    }

    foreach ($prefix in $StripPrefixes) {
        $prefix = ($prefix -replace '/', '\').Trim('\')
        if ([string]::IsNullOrWhiteSpace($prefix)) {
            continue
        }

        if ($directory -ieq $prefix) {
            $directory = $null
            break
        }

        $prefixWithSlash = "$prefix\"
        if ($directory.StartsWith($prefixWithSlash, [System.StringComparison]::OrdinalIgnoreCase)) {
            $directory = $directory.Substring($prefixWithSlash.Length)
            break
        }
    }

    if ([string]::IsNullOrWhiteSpace($directory)) {
        return $null
    }

    return $directory
}

function Add-FilterWithParents {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilterPath,
        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [System.Collections.Generic.HashSet[string]]$FilterSet
    )

    $parts = $FilterPath -split '\\'
    for ($i = 0; $i -lt $parts.Count; $i++) {
        $path = ($parts[0..$i] -join '\')
        $null = $FilterSet.Add($path)
    }
}

function Get-ExistingFilterIds {
    param(
        [string]$OutputPath
    )

    $existingFilterIds = @{}
    if (-not (Test-Path -LiteralPath $OutputPath)) {
        return $existingFilterIds
    }

    try {
        [xml]$existingFiltersXml = Get-Content -LiteralPath $OutputPath -Raw
    }
    catch {
        Write-Warning ("Existing filters file is not valid XML; regenerating all filter GUIDs. ({0}): {1}" -f $OutputPath, $_.Exception.Message)
        return $existingFilterIds
    }
    $namespaceManager = New-Object System.Xml.XmlNamespaceManager($existingFiltersXml.NameTable)
    $namespaceManager.AddNamespace("msb", $script:VcxprojFiltersNamespaceUri)
    $existingFilters = $existingFiltersXml.SelectNodes("/msb:Project/msb:ItemGroup/msb:Filter[@Include]", $namespaceManager)
    foreach ($filter in $existingFilters) {
        $filterName = $filter.GetAttribute("Include")
        $uniqueIdNode = $filter.SelectSingleNode("msb:UniqueIdentifier", $namespaceManager)
        if ($uniqueIdNode) {
            $existingFilterIds[$filterName] = $uniqueIdNode.InnerText
        }
    }

    return $existingFilterIds
}

function Build-ExpectedFiltersDocument {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ResolvedVcxprojPath,
        [Parameter(Mandatory = $true)]
        [string]$ProjectDirectory,
        [AllowEmptyCollection()]
        [string[]]$StripPrefixes = @(),
        [AllowEmptyCollection()]
        [string[]]$RootFileNames = @(),
        [hashtable]$ExistingFilterIds = @{}
    )

    [xml]$projectXml = Get-Content -LiteralPath $ResolvedVcxprojPath -Raw
    $namespaceManager = New-Object System.Xml.XmlNamespaceManager($projectXml.NameTable)
    $namespaceManager.AddNamespace("msb", $script:VcxprojFiltersNamespaceUri)

    $excludedNames = @("ProjectConfiguration", "ProjectReference")
    $projectItems = $projectXml.SelectNodes("/msb:Project/msb:ItemGroup/*[@Include]", $namespaceManager) |
        Where-Object {
            $excludedNames -notcontains $_.Name -and
            $_.GetAttribute("Include") -notmatch "\|"
        }

    $items = @()
    $typeOrder = @()
    $filters = New-Object System.Collections.Generic.HashSet[string]

    foreach ($item in $projectItems) {
        $include = $item.GetAttribute("Include")
        $filterPath = Get-FilterPath `
            -IncludePath $include `
            -ProjectDirectory $ProjectDirectory `
            -StripPrefixes $StripPrefixes `
            -RootFileNames $RootFileNames

        if ($filterPath) {
            Add-FilterWithParents -FilterPath $filterPath -FilterSet $filters
        }

        if ($typeOrder -notcontains $item.Name) {
            $typeOrder += $item.Name
        }

        $items += [PSCustomObject]@{
            Type = $item.Name
            Include = $include
            Filter = $filterPath
        }
    }

    $orderedTypes = @()
    if ($typeOrder -contains "ClCompile") {
        $orderedTypes += "ClCompile"
    }
    $orderedTypes += $typeOrder | Where-Object { $_ -ne "ClCompile" }

    $filtersDoc = New-Object System.Xml.XmlDocument
    $xmlDeclaration = $filtersDoc.CreateXmlDeclaration("1.0", "utf-8", $null)
    $null = $filtersDoc.AppendChild($xmlDeclaration)

    $projectNode = $filtersDoc.CreateElement("Project", $script:VcxprojFiltersNamespaceUri)
    $projectNode.SetAttribute("ToolsVersion", "4.0")
    $null = $filtersDoc.AppendChild($projectNode)

    foreach ($itemType in $orderedTypes) {
        $groupNode = $filtersDoc.CreateElement("ItemGroup", $script:VcxprojFiltersNamespaceUri)
        foreach ($entry in $items | Where-Object { $_.Type -eq $itemType }) {
            $itemNode = $filtersDoc.CreateElement($entry.Type, $script:VcxprojFiltersNamespaceUri)
            $itemNode.SetAttribute("Include", $entry.Include)
            if ($entry.Filter) {
                $filterNode = $filtersDoc.CreateElement("Filter", $script:VcxprojFiltersNamespaceUri)
                $filterNode.InnerText = $entry.Filter
                $null = $itemNode.AppendChild($filterNode)
            }
            $null = $groupNode.AppendChild($itemNode)
        }
        $null = $projectNode.AppendChild($groupNode)

        if ($itemType -eq "ClCompile") {
            Add-FiltersItemGroup -Document $filtersDoc -ProjectNode $projectNode -Filters $filters -ExistingFilterIds $ExistingFilterIds
        }
    }

    if (-not $orderedTypes) {
        Add-FiltersItemGroup -Document $filtersDoc -ProjectNode $projectNode -Filters $filters -ExistingFilterIds $ExistingFilterIds
    }

    return [PSCustomObject]@{
        Document = $filtersDoc
        NamespaceUri = $script:VcxprojFiltersNamespaceUri
        Items = $items
        Filters = @($filters)
    }
}

function Add-FiltersItemGroup {
    param(
        [Parameter(Mandatory = $true)]
        [System.Xml.XmlDocument]$Document,
        [Parameter(Mandatory = $true)]
        [System.Xml.XmlElement]$ProjectNode,
        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [System.Collections.Generic.HashSet[string]]$Filters,
        [hashtable]$ExistingFilterIds = @{}
    )

    $filterGroup = $Document.CreateElement("ItemGroup", $script:VcxprojFiltersNamespaceUri)
    $sortedFilters = @($Filters) | Sort-Object `
        @{ Expression = { ($_ -split '\\').Count } }, `
        @{ Expression = { $_ } }

    foreach ($filter in $sortedFilters) {
        $filterNode = $Document.CreateElement("Filter", $script:VcxprojFiltersNamespaceUri)
        $filterNode.SetAttribute("Include", $filter)
        $idNode = $Document.CreateElement("UniqueIdentifier", $script:VcxprojFiltersNamespaceUri)

        if ($ExistingFilterIds.ContainsKey($filter)) {
            $idNode.InnerText = $ExistingFilterIds[$filter]
        } else {
            $idNode.InnerText = ([guid]::NewGuid().ToString("B"))
        }

        $null = $filterNode.AppendChild($idNode)
        $null = $filterGroup.AppendChild($filterNode)
    }

    $null = $ProjectNode.AppendChild($filterGroup)
}

function Get-FiltersDefinitionList {
    param(
        [Parameter(Mandatory = $true)]
        [xml]$FiltersXml,
        [Parameter(Mandatory = $true)]
        [string]$NamespaceUri
    )

    $ns = New-Object System.Xml.XmlNamespaceManager($FiltersXml.NameTable)
    $ns.AddNamespace("msb", $NamespaceUri)

    $nodes = $FiltersXml.SelectNodes("/msb:Project/msb:ItemGroup/msb:Filter[@Include]", $ns)
    $list = @()
    foreach ($n in $nodes) {
        $list += $n.GetAttribute("Include")
    }
    return $list
}

function Get-ItemEntries {
    param(
        [Parameter(Mandatory = $true)]
        [xml]$FiltersXml,
        [Parameter(Mandatory = $true)]
        [string]$NamespaceUri
    )

    $ns = New-Object System.Xml.XmlNamespaceManager($FiltersXml.NameTable)
    $ns.AddNamespace("msb", $NamespaceUri)

    $nodes = $FiltersXml.SelectNodes("/msb:Project/msb:ItemGroup/*[@Include and local-name() != 'Filter']", $ns)
    $items = @()
    foreach ($n in $nodes) {
        $filterNode = $n.SelectSingleNode("msb:Filter", $ns)
        $filterText = $null
        if ($filterNode) {
            $filterText = $filterNode.InnerText
        }

        $items += [PSCustomObject]@{
            Type = $n.LocalName
            Include = $n.GetAttribute("Include")
            Filter = $filterText
        }
    }

    return $items
}

function Save-FiltersDocument {
    param(
        [Parameter(Mandatory = $true)]
        [System.Xml.XmlDocument]$Document,
        [Parameter(Mandatory = $true)]
        [string]$OutputPath
    )

    $writerSettings = New-Object System.Xml.XmlWriterSettings
    $writerSettings.Indent = $true
    $writerSettings.Encoding = New-Object System.Text.UTF8Encoding($false)
    $writer = [System.Xml.XmlWriter]::Create($OutputPath, $writerSettings)
    try {
        $Document.Save($writer)
    } finally {
        $writer.Close()
    }
}
