
#include <avr/pgmspace.h>
#include <Adafruit_PWMServoDriver.h>
#include "pwm.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup() {
  // LED
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  // PCA9685
  pwm.begin();
  TWBR = 10;
  pwm.reset();
  pwm.setPWMFreq(200);

  setPwmFrequency(9, 256);
  analogWrite(9, 0);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  // SPI
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);

  // KEM
  kem_init();
}

byte srv = 42;
int hall1, hall2;
byte show_mode = 1;
unsigned long show_time;

void loop() {
  boolean spi_comm = spi_handler();

  hall1 = analogRead(A0);
  hall2 = analogRead(A1);

  if (millis() - show_time > 100 || millis() - show_time < 0) {
    show_time = millis();

    analogWrite(9, srv);

    switch (show_mode) {
      case 1:
        kem_show(srv % 1000);
        break;
      case 2:
        kem_show(hall1 % 1000);
        break;
      case 3:
        kem_show(hall2 % 1000);
        break;
    }
  }


}

#define kem_p 13
#define kem_pmax 100
#define kem_pall 1300
const PROGMEM byte kem_seg_start[8] = {0, 0, 1, 0, 2, 2, 1, 0};
const PROGMEM byte kem_seg_end[8] = {0, 1, 2, 2, 3, 1, 3, 3};
const PROGMEM byte kem_digits[10] = {
  0b00111111, //0
  0b00000110, //1
  0b01011011, //2
  0b01001111, //3
  0b01100110, //4
  0b01101101, //5
  0b01111101, //6
  0b00000111, //7
  0b01111111, //8
  0b01101111  //9
};

// digit 0-2
// percent 0-100
void kem_brightness(byte digit, byte percent) {
  int start = kem_p * kem_pmax * digit;
  pwm.setPWM(6 + digit, start + kem_p * percent, start);
}

void kem_init() {
  kem_brightness(0, 100);
  kem_brightness(1, 100);
  kem_brightness(2, 100);
}

// part 0-6
// mask 0-7
void kem_seg(byte part, byte mask) {
  pwm.setPWM(9 + part, pgm_read_byte_near(kem_seg_start + mask) * kem_pall, pgm_read_byte_near(kem_seg_end + mask) * kem_pall);
}

// 000 - 999
void kem_show(int num) {
  byte d0 = num % 10;
  byte d1 = (num / 10) % 10;
  byte d2 = (num / 100) % 10;
  for (byte i = 0; i < 7; i++) {
    kem_seg(i,
            ((pgm_read_byte_near(kem_digits + d0) >> i) & 1) |
            ((pgm_read_byte_near(kem_digits + d1) >> i) & 1) << 1 |
            ((pgm_read_byte_near(kem_digits + d2) >> i) & 1) << 2
           );
  }
}

#define SPI_RDY ((SPSR & (1 << SPIF)) != 0)
unsigned long spi_time;
boolean spi_fail = false;

void spi_send_byte(byte v) {
  SPDR = v;
  spi_time = millis();
  spi_fail = false;
  while (!SPI_RDY) {
    if (millis() - spi_time > 100) {
      spi_fail = true;
      return;
    }
  }
}

void spi_send_int(int v) {
  spi_send_byte(v & 0xFF);
  spi_send_byte((v >> 8) & 0xFF);
}

byte spi_recv_byte() {
  SPDR = 0;
  spi_time = millis();
  spi_fail = false;
  while (!SPI_RDY) {
    if (millis() - spi_time > 100) {
      spi_fail = true;
      return 0;
    }
  }
  return SPDR;
}

boolean spi_handler()
{
  boolean comm = false;
  //digitalWrite(LED_BUILTIN, HIGH);
  if (SPI_RDY) {
    boolean done = false;
    comm = true;
    while (!done) {
      spi_time = millis();
      spi_fail = false;
      while (!SPI_RDY) {
        if (millis() - spi_time > 100) {
          spi_fail = true;
          return true;
        }
      }
      byte spi_in = SPDR;
      byte recv = 0;
      switch (spi_in) {
        case 0x01: // ack
          SPDR = 0xAC;
          break;
        case 0xFF: // end
          done = true;
          break;
        case 0x02: // send hall1
          spi_send_int(hall1);
          break;
        case 0x03: // send hall2
          spi_send_int(hall2);
          break;
        case 0x04: // show mode
          recv = spi_recv_byte();
          if (!spi_fail) {
            show_mode = recv;
          }
          break;
        case 0x05: // set speed
          recv = spi_recv_byte();
          if (!spi_fail) {
            srv = recv;
          }
          break;
        default: // idle
          SPDR = 0x00;
          break;
      }
      if (spi_fail) {
        return true;
      }
    }
  }
  //digitalWrite(LED_BUILTIN, LOW);
  return comm;
}
