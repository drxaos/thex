import spidev
import time
import turtle

g = turtle.Turtle()
g.penup()

spi = spidev.SpiDev()
spi.open(0,0)

def spi_handshake():
	resp = [0]
	while resp[0] != 0xAC:
		resp = spi.xfer2([0x01])
	return

def spi_get_int(cmd):
	resp = [0]
	res = 0
	resp = spi.xfer2([cmd])
	resp = spi.xfer2([0x00])
	res = resp[0]
	resp = spi.xfer2([0x00])
	res = res + (resp[0] << 8)
	return res
	
def spi_get_byte(cmd):
	resp = [0]
	res = 0
	resp = spi.xfer2([cmd])
	resp = spi.xfer2([0x00])
	res = resp[0]
	return res
	
def spi_set_byte(cmd, value):
	resp = [0]
	res = 0
	resp = spi.xfer2([cmd])
	resp = spi.xfer2([value])

def spi_get_hall1():
	return spi_get_int(0x02)

def spi_get_hall2():
	return spi_get_int(0x03)

def spi_set_mode(mode):
	spi_set_byte(0x04, mode)

def spi_end():
	spi.xfer2([0xFF])
	return

m = 1;
while True:
	spi_handshake()
	hall1 = spi_get_hall1()
	hall2 = spi_get_hall2()
	#m = 3-m;
	#spi_set_mode(m);
	spi_end()
	print "Hall1: " + str(hall1) + ", Hall2: " + str(hall2)
	g.setpos((hall1-500)/2, (hall2-500)/2);
	g.dot(2, "blue");
	#time.sleep(0.05)
