@ECHO OFF
REM Build the root wrappers
REM argument 1: where ROOTSYS should point
REM argument 2: The output directory name
REM argument 3: Where to copy the results. If blank, no copy is done.
REM
REM Expect to be run from the Wrapper Generators directory
REM

REM
REM Check arguments
REM

if "%1EMPTY" == "EMPTY" (
  echo The first argument must be the ROOTSYS value.
  goto :EOF
)

if "%2EMPTY" == "EMPTY" (
  echo The second argument must the name of the output directory
  goto :EOF
)

set "RdNVersion= "
if not "%4EMPTY" == "EMPTY" (
  set "RdNVersion=-v %4"
)

REM
REM Setup the environment
REM

set "ROOTSYS=%1"
set "PATH=%1\bin;%PATH%"
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\vsvars32.bat"

REM Build everything so we can actually run programs! :-)
echo Building code to do the translation...
devenv /nologo "Wrapper Generators.sln" /project "FindBadRootHeaders" /build "Release|Win32"
devenv /nologo "Wrapper Generators.sln" /project "ROOT.NET Library Converter" /build "Release|Win32"
devenv /nologo "Wrapper Generators.sln" /project "ROOT.NET Addon Library Converter" /build "Release|Win32"
devenv /nologo "Wrapper Generators.sln" /project "DumpConfigInfo" /build "Release|Win32"

REM Find all bad headers in this distro of ROOT.

if not exist FindBadRootHeadersStatus.txt (
  echo Looking for bad ROOT headers - this will take a few minutes...
  release\FindBadRootHeaders > FindBadRootheaders.log
  echo Done with finding headers > FindBadRootHeadersStatus.txt
  echo Done looking for bad ROOT headers
) else (
  echo Using header scan results from last run. Remove FindBadRootHeadersStatus.txt to force re-run"
  )

REM Copy down the templates
copy release\cpp_template_project.vcxproj .
copy release\solution_template.sln .

REM Next, build the wrapers!
echo Building the wrappers
"release\ROOT.NET Library Converter" -d "%CD%\..\Wrappers\%2" %RdNVersion%

REM Copy over the property sheet...
copy "ROOT Directories.props" ..\Wrappers\%2\

REM And build the thing!
devenv "..\Wrappers\%2\ROOT.NET Lib.sln" /build "Release"

REM Now, see if we are going to copy the thing to a nice resting place.
:justcopy
if "%3"=="" goto :EOF
if not exist "%3" mkdir "%3"
if not exist "%3\lib" mkdir "%3\lib"
if not exist "%3\bin" mkdir "%3\bin"
if not exist "%3\include" mkdir "%3\include"
copy ..\Wrappers\%2\Release\*.dll "%3\lib"
copy ..\Wrappers\WrapperPlumbingLibrary\NetArrayTranslator.hpp "%3\include"
copy ..\Wrappers\WrapperPlumbingLibrary\VectorObject.hpp "%3\include"
copy ..\Wrappers\%2\converted_items.txt "%3"
copy "..\Wrapper Generators\Release\*" "%3\bin\"
