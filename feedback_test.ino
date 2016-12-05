
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

int hall = 0;
int enc_last = 0;
int enc_count = 0;
int pwm_middle = 768;
int pwm_delta = 100;
int pwm0 = 0;
unsigned long time, prev;

void setup() {
  Serial.begin(9600);
  Serial.println("enc,hall");

  //pinMode(A0, INPUT);

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  pwm.begin();
  pwm.setPWMFreq(125);
  pwm.setPin(0, 0);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  pwm0 = pwm_middle;
}

void loop() {
  int e = enc_count;
  int h = hall;

  // cw 0 2 3 1
  int enc_next = digitalRead(2) * 2 + digitalRead(3);
  if (enc_last == 0 && enc_next == 2) enc_count++;
  if (enc_last == 2 && enc_next == 3) enc_count++;
  if (enc_last == 3 && enc_next == 1) enc_count++;
  if (enc_last == 1 && enc_next == 0) enc_count++;
  if (enc_next == 0 && enc_last == 2) enc_count--;
  if (enc_next == 2 && enc_last == 3) enc_count--;
  if (enc_next == 3 && enc_last == 1) enc_count--;
  if (enc_next == 1 && enc_last == 0) enc_count--;
  enc_last = enc_next;

  int nh = analogRead(A0);
  if (nh - 30 > hall) hall = nh - 30;
  if (nh + 30 < hall) hall = nh + 30;
  hall=nh;

  time = millis();

  if (time > prev + 100) {
    if (pwm0 < pwm_middle + pwm_delta / 10) {
      pwm0++;
      pwm.setPin(0, pwm0);
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    prev = time;
  }

  if (e != enc_count) {
    Serial.print(enc_count, DEC);
    Serial.print(F(","));
    Serial.print(hall, DEC);
    Serial.println("");
  }

}
