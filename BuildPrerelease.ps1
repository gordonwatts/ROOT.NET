#
# Coordinate building a pre-release version
# The version shoudl be "5.33.02" or "5.33.02-rc1" or similar.
# Wrapper version can be something like "2.2-dev" or similar.
#
[CmdletBinding()]
param (
  [Parameter(mandatory=$true)]
  [string] $rootVersion,
  [Parameter(mandatory=$true)]
  [string] $wrapperVersion
)

#
# First, find the full info for the ROOT version we want to go after.
#

Import-Module -DisableNameChecking -Force ./ROOTRepositoryAccess
$vinfo = Parse-ROOT-Version $rootVersion

$goodRs = Get-All-ROOT-Downloads | ? {$_.VersionMajor -eq $vinfo.VersionMajor} `
	| ? {$_.VersionMinor -eq $vinfo.VersionMinor} `
	| ? {$_.VersionSubMinor -eq $vinfo.VersionSubMinor} `
	| ? {$_.VersionExtraInfo -eq $vinfo.VersionExtraInfo} `
	| ? {$_.DownloadType -eq "win32.vc10"} `
	| ? {$_.FileType -eq "tar.gz"}

if ($goodRs.length > 1)
{
	throw New-Object System.InvalidOperationException "There are more than one root's availible for version $rootVersion."
}
$goodR = $goodRs

#
# Next, build the thing
#

Import-Module -DisableNameChecking -Force ./BuildControl
Build-ROOT-Dot-Net -Version $wrapperVersion -BuildLocation $PWD.Path -ROOTURL $goodR.URL -showLog $true
