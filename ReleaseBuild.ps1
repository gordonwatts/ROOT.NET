#
# Download the ROOT.NET software and a version of ROOT.NET and do the complete
# build, including the nuget packages.
#

#
# Helper funciton
#

# download from the web if the file is not already there
function global:DownloadFromWeb($url, $local)
{
    if (-not (test-path $local))
    {
        if (-not $url.Contains("//"))
        {
            copy-item $url $local
        }
        else
        {
            Write-Host "Downloading $url"
            $client = new-object System.Net.WebClient
            $client.DownloadFile( $url, $local )
        }
    }
}

#
# Do the actual build
#
function global:BuildROOTNET ($rootLoc, $rootdotnetloc, $finalbuild, $version, $logDir)
{
    $buildDir = "$rootdotnetloc/Wrapper Generators"
    set-location $buildDir
    Write-Host "Building wrappers..."
    & DoEverythingAtOnce\BuildROOTWrappers.bat "$rootLoc" AutoBuild "$finalbuild" $version 2>&1 | out-file "$logDir/build.log"
    
    #
    # A few log files are generated by that script. We need to copy them over
    # to our logs directory so they can be used by report generation.
    #

    copy-item "$buildDir/bad_headers.txt" $logDir
    copy-item "$buildDir/conversion_errors.txt" $logDir    
    copy-item "$rootdotnetloc\Wrappers\AutoBuild\library_dependencies.txt" $logDir
}

# Uncompress the file - as long as it hasn't been uncompressed already, in
# which case skip this step.
function global:Uncompress($path, $logDir)
{
    if (-not $path.EndsWith(".tar.gz"))
    {
        throw "Only know how to uncompress .tar.gz files - $path"
    }

    $logFile = "$logDir/uncompress.log"
    $uncompressFlag = "$path-uncompressed"
    
    if (-not (test-path $uncompressFlag)) {
      Write-Host "Uncompressing $path"
      "Uncompressing $path" | out-file -filepath $logFile -append

      $tarfileName = split-path -leaf ($path.SubString(0, $path.Length-3))

      if (-not (test-path $tarfileName)) {
        
        & "$SevenZipExe" x -y $path | out-file -filepath $logFile -append
      }
      if (-not (test-path $tarfileName)) {
        throw "Could not find the tar file $tarfile after uncompressoing $path"
      }

      & "$SevenZipExe" x -y $tarfileName | out-file -filepath $logfile -append

      $bogus = new-item $uncompressFlag -type file
    }
}

#
# Down load the ROOTNET tools from some source.
#
function global:DownloadROOTNET ($source, $destpath, $logDir)
{
    if (-not $source)
    {
        throw "The rootdotnet version must be specified"
    }
    
    #
    # Next, see if we already have the right directory done. Note this
    # will fail if we are trying to look at the head, which will force an
    # update to proceed (and we recheck).
    #
    
    $currentRevision = get-revision $destpath
    if ($currentRevision -eq $source)
    {
        return $currentRevision
    }
    
    #
    # Do the update. Either they are different or we are going after the HEAD
    # version.
    #

    Write-Host "Updating ROOT.NET from SVN ($source)"    
    set-revision -directory $destpath -repositoryPath https://rootdotnet.svn.codeplex.com/svn -revision $source | out-file -filepath "$logDir/snvco.log" -append
        
    return (get-revision $destpath)
}

#
# Build a root release in the given spot
#
function global:ReleaseBuild($rootDirectory, $ROOTdotNETSource, $ROOTdotNETVersion, $rootRelease)
{
    #
    # Create the directory if it isn't already
    #

    if (-not (test-path $rootDirectory -PathType container))
    {
        Write-Host $rootDirectory
        $bogus = New-Item $rootDirectory -type directory
    }
    $logDir = "$rootDirectory/logs"
    if (-not (test-path $logDir -PathType container))
    {
        $bogus = new-item $logDir -type directory
    }

    #
    # Next, see if the ROOT file isn't downloaded, then go get it.
    #

    $rootFilename = ($rootRelease -split {$_ -eq "/" -or $_ -eq "\"})[-1]
    $rootArchivePath = "$rootDirectory\$rootFilename"
    DownloadFromWeb $rootRelease $rootArchivePath
    $rootsys = "$rootDirectory/root"
    if (-not (test-path $rootsys))
    {
        $bogus = new-item $rootsys -type directory
    }
    set-location $rootDirectory
    Uncompress $rootArchivePath $logDir

    #
    # Next, download the proper version of ROOT.NET for the build
    #

    $rootdotnetloc = "$rootDirectory/rootdotnet"
    $wrapperRevision = DownloadROOTNET $ROOTdotNETSource $rootdotnetloc $logDir
    
    #
    # By looking at the marker files for the last update to the SVN repostiory
    # and also at the last time a build finished, we can figure out if
    # we need to re-run the build or not.
    #
    
    $lastBuildFile = "$rootDirectory/build/buildRevNumber.txt"
    if (test-path $lastBuildFile)
    {
        $lastBuildRev = get-content -Path $lastBuildFile
        if ($lastBuildRev -eq $wrapperRevision)
        {
            return $lastBuildRev
        }
    }

    #
    # If we are here, then we need to do the build.
    #

    $finalLibraryBuild = "$rootDirectory/build"
    BuildROOTNET "$rootDirectory/root" $rootdotnetloc $finalLibraryBuild  $ROOTdotNETVersion $logDir

    #
    # Generate a log report of some of the stuff we disabled for later use
    #
    
    $disabledItems = & "$rootDirectory\rootdotnet\Wrapper Generators\Release\DumpConfigInfo"
    $disabledItems | out-file -filepath "$logDir\disabled_items.txt"
    
    #
    # Remember we did the build
    #
    
    $wrapperRevision | out-file -filepath $lastBuildFile
    
    return $wrapperRevision 
}

#
# Uncomment the below lines for testing
#
#& MachineConfigTOUCHME.ps1
#$rootDirectory = "c:\users\gwatts\Documents\rootrelease"
#$ROOTdotNETSource = "HEAD"
#$rootRelease = "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz"
#ReleaseBuild $rootDirectory $ROOTdotNETSource $rootRelease
