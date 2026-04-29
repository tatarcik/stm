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

## Windows Icin Gerekenler

Bu makinede su an `arm-none-eabi-gcc`, `make`, `dfu-util`, `pio` ve `STM32_Programmer_CLI` bulunamadi. Derlemek/yuklemek icin asagidaki iki yoldan biri yeterli:

1. MSYS2/Chocolatey/Scoop ile `arm-none-eabi-gcc`, `make` ve `dfu-util` kur.
2. ST'nin STM32CubeCLT paketini kur; sonra `make` ve `arm-none-eabi-gcc` PATH icinde gorunmeli.

Kontrol:

```powershell
arm-none-eabi-gcc --version
make --version
dfu-util --version
```

## Derleme

```powershell
make
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
make flash
```

1. BOOT0 jumper'ini tekrar `0` konumuna al.
2. Reset veya power cycle yap.

VS Code icinden `Terminal > Run Task... > STM32: flash DFU` gorevini de kullanabilirsin.

## Onemli Donanim Notu

Repo notuna gore CAN1 transceiver'in GND baglantisi eksik olabilir. CAN1 kullanacaksan C22 altindan C21 altina GND baglantisi yapilmasi gerekiyor. CAN2 icin bu modifikasyon gerekmiyor.

## Uygun Ilk Proje Fikirleri

- Seri porttan komut alan LED/role kontrolcusu
- RS485 Modbus RTU sensor/role modulu
- SD karta veri kaydeden sicaklik veya dijital giris logger'i
- CAN2 uzerinden basit arac/cihaz haberlesme monitörü
- Ethernet + SD kart ile lokal veri toplama cihazi

Benim onerim: once USART1 uzerinden komut alip LED1/LED2/LED3 kontrol eden kucuk bir komut kabugu yapmak. Kartin debug/flash akisini dogrular, sonra RS485 veya CAN tarafina rahatca genisletilir.
