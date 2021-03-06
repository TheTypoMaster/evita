@if "%_echo%"=="" echo off
setlocal
setlocal enabledelayedexpansion
set config=debug
if "%cd:~-11,-4%"=="release" set config=release_x64

for %%x in (%*) do (
  set targets=!targets! %%x
)
if "%targets%"=="" set targets=evita
set/a number_of_jobs=%number_of_processors% / 2
echo config=%config% number_of_jobs=%number_of_jobs% targets=%targets%
set start=%TIME%
ninja -j%number_of_jobs% -C ..\out\%config% %targets%
set end=%TIME%
echo Start %start%
echo End   %end%
endlocal
