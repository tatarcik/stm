# EXITI_CONTROL.hex Analiz Notlari

`C:\Users\qq\Desktop\EXITI_CONTROL.hex` dosyasi Intel HEX formatinda bir Cortex-M firmware olarak incelendi.

## Bulgular

- Flash araligi: `0x08000000 - 0x0801D853`
- Veri boyutu: `120916` byte
- Ilk stack pointer: `0x2000A140`
- Reset handler: `0x080001A1`
- Aktif vektorler STM32F4 ailesiyle uyumlu gorunuyor.
- Firmware icinde LwIP 1.3.1 HTTP sunucu izleri var.
- Bulunan sayfalar/endpoints:
  - `/index.html`
  - `/settings.shtml`
  - `/info.shtml`
  - `/enter.cgi`
  - `/deneme.cgi`
- Login formunda `user` ve `pwd` alanlari var.
- Binary icinde gorulen sabitler `admin` ve `signa`.

## Uyarlama Karari

Kaynak kod HEX'ten birebir geri elde edilemez. Bu nedenle davranis mevcut STM32F407VET6 projesinde yeniden kuruldu:

- Ethernet/LwIP etkinlestirildi.
- Port 80 uzerinde raw TCP tabanli basit HTTP sunucusu eklendi.
- `/index.html` login sayfasi eklendi.
- `/enter.cgi` login kontrolu eklendi.
- `/settings.shtml` LED1/LED2/LED3 kontrol sayfasi eklendi.
- `/info.shtml` bilgi sayfasi eklendi.

Ham analiz ciktisi `analysis/` klasorunde yerel tutulur ve Git'e eklenmez.
