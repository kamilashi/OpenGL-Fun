@echo off
setlocal ENABLEDELAYEDEXPANSION

REM --- Where the combined .shader files live (relative to this .bat) ---
set "SRC_DIR=%CD%\assets\shaders"

REM --- Output root is arg #1 (usually the target's directory). Default: .\out\build ---
if "%~1"=="" ( set "OUT_ROOT=%CD%\out\build" ) else ( set "OUT_ROOT=%~1" )
set "OUT_DIR=%OUT_ROOT%\assets\shaders"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

for %%F in ("%SRC_DIR%\*.shader") do (
  echo Splitting %%~nxF
  powershell -NoLogo -NoProfile -ExecutionPolicy Bypass ^
    -Command ^
    "$src='%%~fF'; $outDir='%OUT_DIR%'; $name=[IO.Path]::GetFileNameWithoutExtension($src);" ^
    "$v=''; $f=''; $mode='';" ^
    "foreach($line in [IO.File]::ReadLines($src)){" ^
    "  if($line -like '#shader*'){" ^
    "    if($line -match 'vertex'){ $mode='v' } elseif($line -match 'fragment'){ $mode='f' } else { $mode='' }" ^
    "  } elseif($mode -eq 'v'){ $v += $line + \"`n\" } elseif($mode -eq 'f'){ $f += $line + \"`n\" }" ^
    "}" ^
    "Set-Content -NoNewline -Encoding UTF8 (Join-Path $outDir ($name+'V.shader')) $v;" ^
    "Set-Content -NoNewline -Encoding UTF8 (Join-Path $outDir ($name+'F.shader')) $f;"
)

echo Done. Output: %OUT_DIR%
endlocal