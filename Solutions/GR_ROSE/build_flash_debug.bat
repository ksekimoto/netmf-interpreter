@rem msbuild.exe dotNetMF.proj /v:d /t:build /p:flavor=debug;memory=flash;platform=GR_ROSE 2>&1 | tee build_flash_debug.txt
msbuild.exe dotNetMF.proj /t:build /p:flavor=debug;memory=flash 2>&1 | tee build_flash_debug.txt
