@rem msbuild.exe dotNetMF.proj /t:clean /p:flavor=debug;memory=flash;platform=GR_ROSE 2>&1 | tee build_clean_flash.txt
msbuild.exe dotNetMF.proj /t:clean /p:flavor=debug;memory=flash;platform=GR_ROSE
