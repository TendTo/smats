@echo off
REM Script called upon compilation by Bazel.
REM It stores useful information about the compilation environment
REM in a file called "bazel-out/stable-status.txt".
REM This file is then read by scripts/generate_version_header.sh to
REM generate the version header file.
REM The invocation of this script is defined in the file .bazelrc.
REM See https://bazel.build/docs/user-manual#workspace-status
REM for more information.
REM Note: This script is called by Bazel. So it should not depend on
REM any external tools.
@echo off
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set "CURRENT_TIME=%YYYY%-%MM%-%DD% %HH%:%Min%:%Sec%"
echo CURRENT_TIME %CURRENT_TIME%

for /f %%i in ('git rev-parse HEAD') do set "STABLE_GIT_COMMIT=%%i"
echo STABLE_GIT_COMMIT %STABLE_GIT_COMMIT%

echo STABLE_USER_NAME %USERNAME%

for /f "tokens=*" %%i in ('hostname') do set "STABLE_HOST_NAME=%%i"
echo STABLE_HOST_NAME %STABLE_HOST_NAME%

for /f "tokens=*" %%i in ('git describe --tags --dirty ^|^| echo unknown') do set "STABLE_REPOSITORY_STATUS=%%i"
echo STABLE_REPOSITORY_STATUS %STABLE_REPOSITORY_STATUS%

for /f "tokens=*" %%i in ('git describe --tags --dirty --always ^|^| echo unknown') do set "STABLE_REPOSITORY_STATUS_SHORT=%%i"
echo STABLE_REPOSITORY_STATUS_SHORT %STABLE_REPOSITORY_STATUS_SHORT%

for /f "tokens=*" %%i in ('git describe --tags --dirty --long ^|^| echo unknown') do set "STABLE_REPOSITORY_STATUS_LONG=%%i"
echo STABLE_REPOSITORY_STATUS_LONG %STABLE_REPOSITORY_STATUS_LONG%