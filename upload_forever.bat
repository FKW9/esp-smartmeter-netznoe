@echo off
set /p env="Env:" 
cls
:start
pio run -e %env% -t upload
goto start