#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();



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


// part 0-6
// mask 0-7
void kem_seg(byte part, byte mask) {
  pwm.setPWM(9 + part, pgm_read_byte_near(kem_seg_start + mask) * kem_pall, pgm_read_byte_near(kem_seg_end + mask) * kem_pall);
}

void kem_reset() {
  kem_brightness(0, 100);
  kem_brightness(1, 100);
  kem_brightness(2, 100);

  for (byte i = 0; i < 6; i++) {
    kem_seg(i, 0);
  }
  kem_seg(6, 7);
}

void kem_init() {
  // PCA9685
  pwm.begin();
  TWBR = 10;
  pwm.reset();
  pwm.setPWMFreq(200);

  // KEM
  kem_reset();
}

// 000 - 999
void kem_show(int num) {
  num = abs(num);
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
