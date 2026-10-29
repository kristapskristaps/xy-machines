@echo off
if exist "%~dp0.venv\Scripts\python.exe" (
  "%~dp0.venv\Scripts\python.exe" "%~dp0tools\dev.py" %*
) else (
  py -3.12 "%~dp0tools\dev.py" %*
)
