$ArmToolchainBin = "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin"
$MsysUsrBin = "C:\msys64\usr\bin"
$MsysMingwBin = "C:\msys64\mingw64\bin"
$SeggerJLinkBin = "C:\Program Files\SEGGER\JLink_V932"

$env:Path = "$ArmToolchainBin;$MsysUsrBin;$MsysMingwBin;$SeggerJLinkBin;$env:Path"
