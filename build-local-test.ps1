#
# Build a single local release, in the directory we are existing... Just cd to the directory
# of this file, and then in PS just run it, with a single argument - the version of the release:
#   "v5.30.01" for example, and the second parameter is the version (say "2.5-alpha1" or similar).
#
[CmdletBinding()]
param (
  [Parameter(mandatory=$true)]
  [string] $ROOTVersion,
  [Parameter(mandatory=$true)]
  [string] $wrapperVersion,
  [Parameter()]
  [switch] $CleanUp
  )

#
# Capture a few things about our environment before anything happens
#

$baseDir = $PWD.Path

#
# We depend on a number of other modules up and running.
#

Import-Module -Force -DisableNameChecking $baseDir/ROOTRepositoryAccess
Import-Module -DisableNameChecking -Force $baseDir/BuildControl
Import-Module -DisableNameChecking -Force $baseDir/BuildNuGetPacakges

#
# Locations for later work
#

$libDir="$baseDir\Wrappers\AutoBuild"
$nugetDir="$baseDir\nuget"

#
# Clean out anything that has happened already
#

if ($CleanUp)
{
    if (Test-Path $libDir)
    {
		Write-Host "Removing results of previous build in $libDir"
        Remove-Item -Recurse $libDir
        remove-item "$baseDir\Wrapper Generators\*.txt"
    } else
	{
		Write-Host "No previous results found in $libDir"
	}
} else {
    Write-Host "Not trying to clean up previous results in $libDir"
}

#
# Ready to do the build now.
#

$rURL = "ftp://root.cern.ch/root/root_" + $ROOTVersion + ".win32.vc10.tar.gz"
Build-ROOT-Dot-Net -Version $wrapperVersion -BuildLocation $baseDir -ROOTURL $rURL -nugetDir $nugetDir
