# Raspberry PI 3 <- SPI -> Freeduino 2009 (Duemilanove)

make avrdude+linuxspi
- !!! disable reset pin unexport
- https://github.com/kcuzner/avrdude

copy and check config
```
programmer
  id = "linuxspi";
  desc = "Use Linux SPI device in /dev/spidev*";
  type = "linuxspi";
  reset = 25;
  baudrate=400000;
;
```

export reset pin on boot
```
echo 25 > /sys/class/gpio/export
```

get bootloader
```
/home/pi/arduino-1.6.13/hardware/tools/avr/bin/avrdude -C/home/pi/arduino-1.6.13/hardware/tools/avr/etc/avrdude.conf -v -patmega168p -clinuxspi -P /dev/spidev0.0 -Uflash:r:freeduino.hex:i
```

programmers.txt
```
spi.name=SPI
spi.communication=linuxspi
spi.protocol=linuxspi
spi.program.protocol=linuxspi
spi.program.tool=avrdude
spi.program.extra_params=-P /dev/spidev0.0
```

boards.txt
```
## Arduino Duemilanove or Diecimila w/ ATmega168p
## ---------------------------------------------
diecimila.menu.cpu.atmega168p=ATmega168P

diecimila.menu.cpu.atmega168p.upload.maximum_size=14336
diecimila.menu.cpu.atmega168p.upload.maximum_data_size=1024
diecimila.menu.cpu.atmega168p.upload.speed=19200

diecimila.menu.cpu.atmega168p.bootloader.high_fuses=0xdd
diecimila.menu.cpu.atmega168p.bootloader.extended_fuses=0x00
diecimila.menu.cpu.atmega168p.bootloader.file=atmega/ATmegaBOOT_168_diecimila.hex

diecimila.menu.cpu.atmega168p.build.mcu=atmega168p
```

wiring

//TODO

Note: Arduino works on 3.3v now (3.3v on "5V" pin and 3.3v on "3V3" pin)

