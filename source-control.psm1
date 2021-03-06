#
# Module that contains an interface for source control
#

# SVN in order to download rootdotnet
# We depend on svn path from http://sourceforge.net/projects/win32svn/files/

$SVNDir = "C:\Users\gwatts\Documents\ATLAS\Projects\svn\svn-win32-1.6.17\bin"
if (-not (test-path $SVNDir))
{
    $SVNDIR = "D:\users\gwatts\ROOT.NET\svn-win32-1.6.17\bin"
}

if (-not (test-path $SVNDir))
{
    throw "Unable to find the svn executable"
}


#
# Get the revision number from a SVN repository. Use the svnversion executable to
# get this.
#
function get-svn-revision($directory)
{
    $vexe = "$SVNDir\svnversion.exe"
    return & $vexe $directory
}

#
# Update a given directory. Specify revision (or it defaults to HEAD).
#
function update-svn ($directory, [switch] $TopLevelOnly, $revision="HEAD")
{
    $rflag = "infinity"
    if ($TopLevelOnly)
    {
        $rflag = "immediates"
    }
    return & "$SVNDir\svn.exe" update -r $revision --depth $rflag $directory
}

#
# Set the revision for a directory from a repository path.
#
function set-svn-revision ($directory, $repositoryPath, $revision, [Switch] $TopLevelOnly)
{
    $rflag = "infinity"
    if ($TopLevelOnly)
    {
        $rflag = "immediates"
    }
    return & "$SVNDir\svn.exe" checkout -r $revision --depth $rflag $repositoryPath $directory
}

#################################
#
# The functions that we will export - so we can switch out what ss we are using
# as time goes by.
#

#
# Get the current revision number for the repo
#
function get-revision ($directory)
{
    if (-not (test-path $directory))
    {
        return -1
    }
    return get-svn-revision $directory
}

#
# Set a directory to a particular revision. This means co if it hasn't been
# setup already
#
function set-revision ($directory, [Switch] $TopLevelOnly, $repositoryPath = "", $revision = "HEAD")
{
    if (test-path $directory)
    {
        update-svn $directory -revision $revision -TopLevelOnly:$TopLevelOnly
    }
    else
    {
        set-svn-revision $directory -repositoryPath $repositoryPath -revision $revision -TopLevelOnly:$TopLevelOnly
    }
}

export-modulemember get-revision
export-modulemember set-revision
