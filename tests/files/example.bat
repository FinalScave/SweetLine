@echo off
REM Batch file sample
:: This is also a comment

REM Variables
set MY_VAR=Hello World
set /a NUMBER=42
set /p INPUT="Enter input: "

REM Environment variables
echo %PATH%
echo %USERPROFILE%
echo %TEMP%

REM Command line arguments
echo First argument: %1
echo Second argument: %2
echo All arguments: %*
echo Script name: %0

REM Parameter modifiers
echo Drive: %~d0
echo Path: %~p0
echo Name: %~n0
echo Extension: %~x0
echo Full path: %~f0

REM Delayed expansion
setlocal enabledelayedexpansion
set COUNT=0
for /l %%i in (1,1,10) do (
    set /a COUNT+=1
    echo Count: !COUNT!
)
endlocal

REM Conditional statements
if "%1"=="" (
    echo No argument provided
) else (
    echo Argument: %1
)

if exist "C:\Windows\System32\cmd.exe" (
    echo Command prompt exists
)

if defined MY_VAR (
    echo MY_VAR is defined
)

if %ERRORLEVEL% EQU 0 (
    echo Success
) else (
    echo Error: %ERRORLEVEL%
)

REM Comparison operators
if %NUMBER% EQU 42 echo Equal
if %NUMBER% NEQ 0 echo Not equal
if %NUMBER% LSS 100 echo Less than
if %NUMBER% LEQ 42 echo Less or equal
if %NUMBER% GTR 10 echo Greater than
if %NUMBER% GEQ 42 echo Greater or equal

REM Goto and labels
goto :start

:middle
echo Middle section
goto :end

:start
echo Start section
goto :middle

:end
echo End section

REM For loops
for %%i in (1 2 3 4 5) do (
    echo Number: %%i
)

for /l %%i in (1,2,10) do (
    echo Loop: %%i
)

for %%f in (*.txt) do (
    echo File: %%f
)

for /r "C:\Temp" %%f in (*.log) do (
    echo Log file: %%f
)

REM File operations
copy "source.txt" "destination.txt"
xcopy "C:\Source\*" "D:\Destination\" /E /I /Y
move "old.txt" "new.txt"
del "temp.txt"
mkdir "NewDirectory"
rmdir "OldDirectory"

REM Directory operations
cd "C:\Temp"
pushd "C:\Windows"
popd
dir /B /S

REM String operations
set STRING=Hello World
echo %STRING:~0,5%
echo %STRING:World=Universe%
echo %STRING:~=-5%

REM Functions
call :my_function arg1 arg2
echo Returned: %RESULT%
goto :eof

:my_function
echo Function called with: %1 %2
set RESULT=Success
goto :eof

REM Error handling
if errorlevel 1 (
    echo An error occurred
    exit /b 1
)

REM Network commands
ping 127.0.0.1
ipconfig /all
netstat -an

REM Process management
tasklist | find "notepad"
taskkill /IM notepad.exe /F

REM Registry operations
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion" /v ProgramFilesDir
reg add "HKCU\Software\MyApp" /v Setting /t REG_SZ /d "Value" /f

REM System commands
cls
color 0A
ver
vol C:
date /t
time /t

REM Redirection and pipes
dir > output.txt
dir >> output.txt
type input.txt | find "search"
sort < unsorted.txt > sorted.txt

REM Special characters
echo Special: ^& ^| ^< ^> ^^
echo Percent: %%
echo Exclamation: ^!

REM Pause and timeout
pause
timeout /t 5

REM Exit
exit /b 0
