import spidev
import time
import turtle
import ctypes
import math
from Tkinter import *
import time

millis = lambda: int(round(time.time() * 1000))
master = Tk()
w = Scale(master, from_=60, to=80, length=500, orient=HORIZONTAL)
w.set(70)
w.pack()
w1 = Scale(master, from_=0, to=20, length=500, orient=HORIZONTAL)
w1.set(7)
w1.pack()
btn = Button(master, text = "calibrate")
btn.pack();
btn1 = Button(master, text = "free")
btn1.pack();
btn2 = Button(master, text = "hold0")
btn2.pack();
btn3 = Button(master, text = "hold2")
btn3.pack();
btn4 = Button(master, text = "hold4")
btn4.pack();
btn5 = Button(master, text = "hold6")
btn5.pack();
btn6 = Button(master, text = "hold55")
btn6.pack();
btn7 = Button(master, text = "toggle demo")
btn7.pack();
g = turtle.Turtle()
g.tracer(0, 0)
g.speed(0)
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
	time.sleep(0.01)
	resp = spi.xfer2([0x00])
	res = resp[0]
	resp = spi.xfer2([0x00])
	res = res + (resp[0] << 8)
	res = ctypes.c_short(res).value
	return res
	
def spi_get_int_next():
	resp = [0]
	res = 0
	time.sleep(0.01)
	resp = spi.xfer2([0x00])
	res = resp[0]
	resp = spi.xfer2([0x00])
	res = res + (resp[0] << 8)
	res = ctypes.c_short(res).value
	return res
	
def spi_get_byte(cmd):
	resp = [0]
	res = 0
	resp = spi.xfer2([cmd])
	time.sleep(0.01)
	resp = spi.xfer2([0x00])
	res = resp[0]
	return res
	
def spi_set_byte(cmd, value):
	resp = [0]
	res = 0
	resp = spi.xfer2([cmd])
	resp = spi.xfer2([value])

def spi_set_int(cmd, value):
	resp = [0]
	res = 0
	resp = spi.xfer2([cmd])
	resp = spi.xfer2([value & 0xFF])
	resp = spi.xfer2([(value>>8) & 0xFF])
#	resp = spi.xfer2([100])
#	resp = spi.xfer2([0])

def spi_get_hall1():
	return spi_get_int(0x02)

def spi_get_hall2():
	return spi_get_int(0x03)

def spi_get_srv_pos():
	return spi_get_int(0x0A)

def spi_get_srv_target():
	return spi_get_int(0x0B)

def spi_set_mode(mode):
	spi_set_byte(0x04, mode)

def spi_set_srv(speed):
	spi_set_byte(0x05, speed)

def spi_set_srv_free():
	spi_set_byte(0x08, 0)

def spi_set_srv_hold():
	spi_set_byte(0x08, 1)

def spi_set_srv_target(trg):
	spi_set_int(0x09, trg)

def spi_calibrate():
	spi_set_byte(0x06, 1)

def spi_params():
	return {
		'sx': spi_get_int(0x07),
		'sy': spi_get_int_next(),
		'px0': spi_get_int_next(),
		'py0': spi_get_int_next(),
		'px1': spi_get_int_next(),
		'py1': spi_get_int_next(),
		'px2': spi_get_int_next(),
		'py2': spi_get_int_next(),
		'px3': spi_get_int_next(),
		'py3': spi_get_int_next(),
		'px4': spi_get_int_next(),
		'py4': spi_get_int_next(),
		'px5': spi_get_int_next(),
		'py5': spi_get_int_next(),
		'px6': spi_get_int_next(),
		'py6': spi_get_int_next(),
		'px7': spi_get_int_next(),
		'py7': spi_get_int_next(),
		'px8': spi_get_int_next(),
		'py8': spi_get_int_next(),
		'pa0': spi_get_int_next(),
		'pa1': spi_get_int_next(),
		'pa2': spi_get_int_next(),
		'pa3': spi_get_int_next(),
		'pa4': spi_get_int_next(),
		'pa5': spi_get_int_next(),
		'pa6': spi_get_int_next(),
		'pa7': spi_get_int_next(),
		'pa8': spi_get_int_next(),
		'cx': spi_get_int_next(),
		'cy': spi_get_int_next(),
	}

def spi_end():
	spi.xfer2([0xFF])
	return

calibrating = False;
sfree = False;
shold = False;
sdemo = False;
sdt = 0;
starget = 0;

def calibrate(e):
	global calibrating;
	calibrating = True
btn.bind("<Button-1>", calibrate)

def srv_free(e):
	global sfree;
	sfree = True
btn1.bind("<Button-1>", srv_free)

def srv_hold0(e):
	global shold;
	global starget;
	global sdt;
	sdt = millis() + 3000
	shold = True
	starget = 000
btn2.bind("<Button-1>", srv_hold0)

def srv_hold2(e):
	global shold;
	global starget;
	global sdt;
	sdt = millis() + 3000
	shold = True
	starget = 200
btn3.bind("<Button-1>", srv_hold2)

def srv_hold4(e):
	global shold;
	global starget;
	global sdt;
	sdt = millis() + 3000
	shold = True
	starget = 400
btn4.bind("<Button-1>", srv_hold4)

def srv_hold6(e):
	global shold;
	global starget;
	global sdt;
	sdt = millis() + 3000
	shold = True
	starget = 600
btn5.bind("<Button-1>", srv_hold6)

def srv_hold55(e):
	global shold;
	global starget;
	global sdt;
	sdt = millis() + 3000
	shold = True
	starget = 550
btn6.bind("<Button-1>", srv_hold55)

def srv_demo(e):
	global sdemo;
	sdemo = not sdemo
btn7.bind("<Button-1>", srv_demo)

m = 1;
sp = 0;
def loop():
	global calibrating;
	global sfree;
	global shold;
	global starget;
	global sdemo;
	global sdt;
	if(sfree):
		spi_handshake()
		spi_set_srv_free();
		spi_end()
		sfree=False
		
	if(shold):
		spi_handshake()
		spi_set_srv_hold();
		spi_set_srv_target(starget);
		print "Target=" + str(spi_get_srv_target());
		print "Pos=" + str(spi_get_srv_pos());
		spi_end()
		shold=False
		
	if(calibrating):
		calibrating = False;
		print "calibrating"
		spi_handshake()
		spi_calibrate()
		spi_end()
		print "params"
		spi_handshake()
		p = spi_params()
		print p
		g.setpos(p['px0']-500, p['py0']-500);
		g.dot(5, "black");
		g.setpos(p['px8']-500, p['py8']-500);
		g.dot(5, "pink");
		g.setpos(p['px1']-500, p['py1']-500);
		g.dot(5, "green");
		g.setpos(p['px2']-500, p['py2']-500);
		g.dot(5, "red");
		g.setpos(p['px3']-500, p['py3']-500);
		g.dot(5, "green");
		g.setpos(p['px4']-500, p['py4']-500);
		g.dot(5, "red");
		g.setpos(p['px5']-500, p['py5']-500);
		g.dot(5, "green");
		g.setpos(p['px6']-500, p['py6']-500);
		g.dot(5, "red");
		g.setpos(p['px7']-500, p['py7']-500);
		g.dot(5, "green");

		g.pen(pencolor="red", pensize=1)
		g.setpos(p['px0']-500, p['py0']-500);
		g.pendown()
		g.setpos(p['px4']-500, p['py4']-500);
		g.penup()
		g.setpos(p['px2']-500, p['py2']-500);
		g.pendown()
		g.setpos(p['px6']-500, p['py6']-500);
		g.penup()

		g.pen(pencolor="green", pensize=1)
		g.setpos(p['px3']-500, p['py3']-500);
		g.pendown()
		g.setpos(p['px7']-500, p['py7']-500);
		g.penup()
		g.setpos(p['px1']-500, p['py1']-500);
		g.pendown()
		g.setpos(p['px5']-500, p['py5']-500);
		g.penup()

		cx = (p['px0']+p['px1']+p['px2']+p['px3']+p['px4']+p['px5']+p['px6']+p['px7'])/8
		cy = (p['py0']+p['py1']+p['py2']+p['py3']+p['py4']+p['py5']+p['py6']+p['py7'])/8
		g.setpos(cx-500, cy-500);
		g.dot(10, "blue");

		spi_end()
		print "done"
		
		print "pa0: " + str(p['pa0']*1.80/math.pi - 360) + " (" + str(p['pa0']) + ")"
		print "pa1: " + str(p['pa1']*1.80/math.pi - 360) + " (" + str(p['pa1']) + ")"
		print "pa2: " + str(p['pa2']*1.80/math.pi - 360) + " (" + str(p['pa2']) + ")"
		print "pa3: " + str(p['pa3']*1.80/math.pi - 360) + " (" + str(p['pa3']) + ")"
		print "pa4: " + str(p['pa4']*1.80/math.pi - 360) + " (" + str(p['pa4']) + ")"
		print "pa5: " + str(p['pa5']*1.80/math.pi - 360) + " (" + str(p['pa5']) + ")"
		print "pa6: " + str(p['pa6']*1.80/math.pi - 360) + " (" + str(p['pa6']) + ")"
		print "pa7: " + str(p['pa7']*1.80/math.pi - 360) + " (" + str(p['pa7']) + ")"
		print "pa8: " + str(p['pa8']*1.80/math.pi - 360) + " (" + str(p['pa8']) + ")"

		g.pen(pencolor="red", pensize=1)
		g.penup()
		g.setpos(p['cx']-500, p['cy']-500);
		g.pendown()
		g.setpos(p['cx']+math.cos(p['pa0']*0.01)*200-500, p['cy']+math.sin(p['pa0']*0.01)*200-500);
		g.pen(pencolor="green", pensize=1)
		g.penup()
		g.setpos(p['cx']-500, p['cy']-500);
		g.pendown()
		g.setpos(p['cx']+math.cos(p['pa2']*0.01)*200-500, p['cy']+math.sin(p['pa2']*0.01)*200-500);
		g.pen(pencolor="blue", pensize=1)
		g.penup()
		g.setpos(p['cx']-500, p['cy']-500);
		g.pendown()
		g.setpos(p['cx']+math.cos(p['pa4']*0.01)*200-500, p['cy']+math.sin(p['pa4']*0.01)*200-500);
		g.pen(pencolor="aqua", pensize=1)
		g.penup()
		g.setpos(p['cx']-500, p['cy']-500);
		g.pendown()
		g.setpos(p['cx']+math.cos(p['pa6']*0.01)*200-500, p['cy']+math.sin(p['pa6']*0.01)*200-500);
		g.pen(pencolor="black", pensize=1)
		g.penup()

	
	if(sdemo):
		if(millis() > sdt):
			starget = starget + 50
			if(starget >= 800):
				starget = 0
			spi_handshake()
			spi_set_srv_hold();
			spi_set_srv_target(starget);
			print "Target=" + str(spi_get_srv_target());
			print "Pos=" + str(spi_get_srv_pos());
			spi_end()
			sdt = millis() + 1000

	
	spi_handshake()
	hall1 = spi_get_hall1()
	hall2 = spi_get_hall2()
	spi_set_mode(w1.get());
	spi_set_srv(w.get());
	spi_end()
	g.setpos((hall1-500), (hall2-500));
	g.dot(2, "blue");
	turtle.update()
	master.after(1, loop);

master.after(1, loop);
master.mainloop();
