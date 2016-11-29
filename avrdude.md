# avrdude

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
