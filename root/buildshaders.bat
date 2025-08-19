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
    "$v=''; $f=''; $mode=''; $base=[IO.Path]::GetDirectoryName($src);" ^
    "foreach($line in [IO.File]::ReadLines($src)){" ^
    "  if($line -like '#shader*'){" ^
    "    if($line -match 'vertex'){ $mode='v' } elseif($line -match 'fragment'){ $mode='f' } else { $mode='' }" ^
    "  } elseif($mode -eq 'v'){" ^
    "    if($line -match '^\s*#include\s+(\S+)\s*$'){" ^
    "      $inc = Join-Path $base $Matches[1]; if(Test-Path $inc){ $v += [IO.File]::ReadAllText($inc) + [Environment]::NewLine } else { throw 'Include not found: ' + $inc }" ^
    "    } else { $v += $line + [Environment]::NewLine }" ^
    "  } elseif($mode -eq 'f'){" ^
    "    if($line -match '^\s*#include\s+(\S+)\s*$'){" ^
    "      $inc = Join-Path $base $Matches[1]; if(Test-Path $inc){ $f += [IO.File]::ReadAllText($inc) + [Environment]::NewLine } else { throw 'Include not found: ' + $inc }" ^
    "    } else { $f += $line + [Environment]::NewLine }" ^
    "  }" ^
    "}" ^
    "$utf8NoBom = New-Object System.Text.UTF8Encoding($false);" ^
    "[IO.File]::WriteAllText( (Join-Path $outDir ($name+'V.shader')), $v, $utf8NoBom );" ^
    "[IO.File]::WriteAllText( (Join-Path $outDir ($name+'F.shader')), $f, $utf8NoBom );"
)

echo Done. Output: %OUT_DIR%
endlocal