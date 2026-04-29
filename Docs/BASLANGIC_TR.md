# JZ-F407VET6 Baslangic Notlari

Bu workspace, `art103/JZ-F407VET6` reposundaki CubeMX tabanli STM32F407VET6 kart projesinden baslatildi. Kart uzerinde LED, buton, USART, RS485, CAN, SD kart, SPI flash, I2C EEPROM, RTC, USB device/host ve Ethernet icin hazir test kodlari var.

## Nereden Baslamali

- Ana uygulama: `Core/Src/main.c`
- Pin isimleri: `Core/Inc/main.h`
- GPIO ayarlari: `Core/Src/gpio.c`
- UART/RS485: `Core/Src/usart.c`
- CAN: `Core/Src/can.c`
- SPI flash kontrolu: `Core/Src/spi.c`

Ilk deneme icin en rahat yol LED ve seri porttur. Kod `printf` cikisini varsayilan olarak USART1 DB9 seri porta yollar. USB CDC kullanmak istersen `Core/Src/main.c` icindeki `USB_DEBUG` tanimini acabilirsin.

## Windows Icin Kurulan Altyapi

Bu makinede proje icin su altyapi kuruldu:

- MSYS2: `C:\msys64`
- GNU Make: `C:\msys64\usr\bin\make.exe`
- dfu-util: `C:\msys64\mingw64\bin\dfu-util.exe`
- Resmi Arm GNU Toolchain: `C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin`
- SEGGER J-Link: `C:\Program Files\SEGGER\JLink_V932\JLink.exe`

VS Code gorevleri bu yollari PATH'e kendisi ekler. Normal PowerShell terminalinde elle derlemek istersen once bu PATH'i ekleyebilirsin:

```powershell
$env:Path = "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin;C:\msys64\usr\bin;C:\msys64\mingw64\bin;C:\Program Files\SEGGER\JLink_V932;$env:Path"
```

Kontrol:

```powershell
arm-none-eabi-gcc --version
make --version
dfu-util --version
JLink.exe -? 
```

## Derleme

```powershell
.\scripts\build.ps1
```

Temiz derleme:

```powershell
.\scripts\build.ps1 -Clean
```

Derleme basarili olursa firmware dosyalari `build/` klasorunde olusur:

- `build/Industrial_Board.elf`
- `build/Industrial_Board.hex`
- `build/Industrial_Board.bin`

VS Code icinden `Terminal > Run Task... > STM32: build` gorevini de kullanabilirsin.

## DFU Ile Yukleme

1. BOOT0 jumper'ini `1` konumuna al.
2. Kartin mini USB portunu PC'ye bagla.
3. Asagidaki komutu calistir:

```powershell
.\scripts\flash-dfu.ps1
```

1. BOOT0 jumper'ini tekrar `0` konumuna al.
2. Reset veya power cycle yap.

VS Code icinden `Terminal > Run Task... > STM32: flash DFU` gorevini de kullanabilirsin.

## J-Link Ile Yukleme

J-Link kullaniyorsan J-Flash projesinde cihaz bos kaldiginda `Unknown device specified ()` hatasi gorulur. Bu projede J-Flash yerine dogrudan J-Link Commander ile yuklemek icin hazir script var:

```powershell
.\scripts\flash-jlink.ps1
```

Varsayilan ayarlar:

- Device: `STM32F407VE`
- Interface: `SWD`
- Speed: `4000`
- Firmware: `build\Industrial_Board.hex`

JTAG kablolamasi kullaniyorsan:

```powershell
.\scripts\flash-jlink.ps1 -Interface JTAG
```

VS Code icinden `Terminal > Run Task... > STM32: flash J-Link` gorevini de kullanabilirsin.

## Onemli Donanim Notu

Repo notuna gore CAN1 transceiver'in GND baglantisi eksik olabilir. CAN1 kullanacaksan C22 altindan C21 altina GND baglantisi yapilmasi gerekiyor. CAN2 icin bu modifikasyon gerekmiyor.

## Uygun Ilk Proje Fikirleri

- Seri porttan komut alan LED/role kontrolcusu
- RS485 Modbus RTU sensor/role modulu
- SD karta veri kaydeden sicaklik veya dijital giris logger'i
- CAN2 uzerinden basit arac/cihaz haberlesme monitörü
- Ethernet + SD kart ile lokal veri toplama cihazi

Benim onerim: once USART1 uzerinden komut alip LED1/LED2/LED3 kontrol eden kucuk bir komut kabugu yapmak. Kartin debug/flash akisini dogrular, sonra RS485 veya CAN tarafina rahatca genisletilir.
