$lastSvnRevision = 6962
$lastSvnHash = '16cd907fe7482cb54a7374cd28b8501f138116be'
$defineNumberMatch = [regex] '^#define\s+(\w+)\s+(\d+)$'
$defineStringMatch = [regex] "^#define\s+(\w+)\s+[`"']?(.+?)[`"']?$"
$semVerMatch = [regex] 'v?(\d+)\.(\d+).(\d+)(?:-(\w+))?'

if (!(git rev-parse --is-inside-work-tree 2>$null)) {
  throw 'git repo not found'
}

$repositoryRootPath = git rev-parse --git-path .  | Join-Path -ChildPath .. | Resolve-Path
$buildPath = Join-Path $repositoryRootPath 'build'
$gitVersionHeaderPath = Join-Path $buildPath 'git_version.h'
$gitVersionXmlPath = Join-Path $buildPath 'git_version.xml'

if (!(Test-Path $gitVersionHeaderPath)) {
  throw "missing git_version.h in ${buildPath}"
}

$version = @{}
Get-Content $gitVersionHeaderPath | %{$_.Trim()} | ?{$_} | %{
  switch -regex ($_) {
    $defineNumberMatch {
      $version[$Matches[1]] = [int]$Matches[2];
    }
    $defineStringMatch {
      $version[$Matches[1]] = $Matches[2];
    }
  }
}

if(!($version.ContainsKey('BUILD_GIT_VERSION_NUMBER') -and $version.ContainsKey('BUILD_GIT_VERSION_STRING'))) {
  throw 'invalid git_version.h'
}

$gitRevision = $lastSvnRevision + ((git log --pretty=oneline "$($lastSvnHash)..HEAD" 2>$null | Measure-Object).Count)
$gitBranch = git symbolic-ref --short HEAD 2>$null
$gitHash = git rev-parse --short HEAD 2>$null
$gitVersionString = $gitRevision, $gitBranch, $gitHash -join '-'
$exactGitTag = git describe --exact-match --tags 2>$null

if ($exactGitTag -match $semVerMatch) {
  $version['TAGGED_RELEASE'] = $true
  $version['RESOURCE_BASE_VERSION'] = $Matches[1..3]
  $version['INSTALLER_VERSION'] = $gitVersionString = ($Matches[1..3] -join '.') + @("-$($Matches[4])",'')[!$Matches[4]]
} else {
  foreach ($rev in (git rev-list --tags 2>$null)) {
    $tag = git describe --exact-match --tags $rev 2>$null
    if ($tag -match $semVerMatch) {#
      $version['TAGGED_RELEASE'] = $false
      $version['RESOURCE_BASE_VERSION'] = $Matches[1..3] + $gitRevision
      $version['INSTALLER_VERSION'] = ($Matches[1..3] -join '.') + "-" + $gitBranch
      break;
    }
  }
}

$version['BUILD_GIT_VERSION_NUMBER'] = $gitRevision
$version['BUILD_GIT_VERSION_STRING'] = $gitVersionString

$version.GetEnumerator() | %{
  $type = $_.Value.GetType()
  $value = $_.Value
  $fmtValue = switch ($type) {
    ([string]) {"`"$value`""}
    ([int]) {$value.ToString()}
    ([bool]) {([int]$value).ToString()}
    ([object[]]) {$value -join ', '}
    default {
      Write-Host "no format known for type '$type' - trying default string conversion" -ForegroundColor Red
      {"`"$($value.ToString())`""}
    }
  }
  "#define $($_.Key) $($fmtValue)"
} | Out-File -FilePath $gitVersionHeaderPath -Encoding utf8

$gitVersionXml = [xml]@'
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <GitVersionNumber></GitVersionNumber>
    <GitVersionString></GitVersionString>
  </PropertyGroup>
</Project>
'@

$gitVersionXml.Project.PropertyGroup.GitVersionNumber = $gitRevision.ToString()
$gitVersionXml.Project.PropertyGroup.GitVersionString = $gitVersionString
$gitVersionXml.Save($gitVersionXmlPath)