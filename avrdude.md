# avrdude config
```
programmer
  id    = "linuxgpio";
  desc  = "Use the Linux sysfs interface to bitbang GPIO lines";
  type  = "linuxgpio";
  reset = 8;
  sck   = 11;
  mosi  = 10;
  miso  = 9;
;
```

# programmers.txt
```
gpio.name=GPIO
gpio.communication=linuxgpio
gpio.protocol=linuxgpio
gpio.program.protocol=linuxgpio
gpio.program.tool=avrdude
```
