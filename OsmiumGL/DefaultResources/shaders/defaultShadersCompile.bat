SETLOCAL ENABLEDELAYEDEXPANSION
set CompilerPath=!VULKAN_SDK!/Bin/glslangValidator.exe
echo !CompilerPath!
set q=^"
forfiles /s /m *.vert /c "cmd /c \"\"%CompilerPath%\" @path -gVS -V -o @fname.vert.spv\""
forfiles /s /m *.frag /c "cmd /c \"\"%CompilerPath%\" @path -gVS -V -o @fname.frag.spv\""
ENDLOCAL