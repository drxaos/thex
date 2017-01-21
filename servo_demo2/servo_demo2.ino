
#include "pwm.h"
#include "kem.h"
#include "math.h"


void setup() {

  // LED
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  kem_init();

  setPwmFrequency(9, 256);
  analogWrite(9, 0);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  // SPI
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);
}

byte SRV0 = 70; // +-8
byte srv = 70;
byte srv_prev = 71;
byte srv_next = 70;
byte srv_mode = 0; // 0-free, 1-hold
byte srv_dir = 1; // 1 по часовой, -1 против
double srv_pos = 0;
double srv_target = 0;
unsigned long srv_change = millis();
int hall1, hall2;
byte show_mode = 9;
unsigned long show_time;
byte calibrate = 0;


void upd_hall() {
  hall1 = analogRead(A0);
  hall2 = analogRead(A1);
}

void upd_srv() {
  srv_next = srv;
  if (srv_next != srv_prev && millis() - srv_change > 30) {
    if (srv_next > srv_prev) {
      srv_next = srv_prev + 1;
    }
    if (srv_next < srv_prev) {
      srv_next = srv_prev - 1;
    }
    analogWrite(9, srv_next);
    srv_prev = srv_next;
    srv_change = millis();
  }
}

int sx, sy, cx, cy;
long round_time;
int px[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
int py[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
float pa[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};


double find_pos(int x, int y) {
  double a = atan2(y - cy, x - cx);
  while (a < pa[0]) {
    a += PI * 2;
  }
  double d = 1000000;
  byte n = -1;
  for (byte i = 0; i < 8; i++) {
    if (a > pa[i]) {
      n = i;
    }
  }
  double range = pa[n + 1] - pa[n];
  a -= pa[n];
  double res = double(n) + a / range; // 0-7 + часть
  return res;
}

void srv_hold() {
  if (srv_target < 0 || srv_target >= 8) {
    srv_target = 0;
  }
  double dif = srv_pos - srv_target;
  if (dif < 0) {
    dif += 8;
  }
  if (7.96 < dif) {
    srv = SRV0;
  }
  if (7.5 < dif && dif <= 7.97) {
    srv = SRV0 + 1 * srv_dir;
  }
  if (7 < dif && dif <= 7.5) {
    srv = SRV0 + 2 * srv_dir;
  }
  if (6 < dif && dif <= 7) {
    srv = SRV0 + 5 * srv_dir;
  }
  if (5 < dif && dif <= 6) {
    srv = SRV0 + 7 * srv_dir;
  }
  if (4 <= dif && dif <= 5) {
    srv = SRV0 + 8 * srv_dir;
  }
  if (3 <= dif && dif < 4) {
    srv = SRV0 - 8 * srv_dir;
  }
  if (2 <= dif && dif < 3) {
    srv = SRV0 - 7 * srv_dir;
  }
  if (1 <= dif && dif < 2) {
    srv = SRV0 - 5 * srv_dir;
  }
  if (0.5 <= dif && dif < 1) {
    srv = SRV0 - 2 * srv_dir;
  }
  if (0.04 <= dif && dif < 0.5) {
    srv = SRV0 - 1 * srv_dir;
  }
  if (dif < 0.04) {
    srv = SRV0;
  }
}

void loop() {
  boolean spi_comm = spi_handler();

  if (calibrate == 1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);

    do_calibrate();
    calibrate = 0;
  }

  upd_hall();

  srv_pos = find_pos(hall1, hall2);

  if (srv_mode == 1) {
    srv_hold();
  }

  upd_srv();

  if (millis() - show_time > 100 || millis() - show_time < 0) {
    show_time = millis();

    float a, sa;
    int seg;
    double dsq;
    long x, y, dx, dy;
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
      case 4:
        kem_show(srv_prev % 1000);
        break;
      case 5:
        x = hall1;
        y = hall2;
        dx = long(x) - long(sx);
        dy = long(y) - long(sy);
        dsq = sqrt(dx * dx + dy * dy);
        kem_show(int(dsq) % 1000);
        break;
      case 6:
        kem_show(int(srv_target * 100) % 1000);
        break;
      case 7:
        kem_show(int(find_pos(hall1, hall2) * 100) % 1000);
        break;
      case 8:
        kem_show(int(pa[0] * 100) % 1000);
        break;
      case 9:
        kem_show(int(pa[2] * 100) % 1000);
        break;
      case 10:
        kem_show(int(pa[4] * 100) % 1000);
        break;
      case 11:
        kem_show(int(pa[6] * 100) % 1000);
        break;
    }
  }
}


void do_calibrate() {

  unsigned int x, y;
  long dx, dy;
  double dsq, pdsq, mdsq;
  boolean started;
  boolean done;

  kem_show(000);

  // останов
  srv = SRV0;
  upd_srv();
  delay(3);

  // начальная точка
  upd_hall();
  x = sx = hall1;
  y = sy = hall2;


  // round 1 - разгон
  kem_show(100);
  done = false;
  started = false;

  srv = SRV0 + 1;
  upd_srv();
  pdsq = 0;
  while (done == false) {
    upd_hall();
    x = hall1;
    y = hall2;
    dx = long(x) - long(sx);
    dy = long(y) - long(sy);
    dsq = sqrt(dx * dx + dy * dy);

    // разгон ступенями
    if (pdsq + 20 < dsq && srv < SRV0 + 8) {
      srv++;
      upd_srv();
      pdsq = dsq;
    }

    // начало пройдено
    if (dsq > 100) {
      started = true;
      mdsq = dsq;
    }

    // ожидание конечной точки
    if (started && dsq < 50) {
      if (mdsq > dsq) { // запоминаем минимум
        mdsq = dsq;
      }
      if (mdsq < 5) { // достаточно близко
        done = true;
      }
      if (mdsq + 5 < dsq) { // начали удаляться
        done = true;
      }
    }
  }

  // round 2 - замер времени
  unsigned long round_start = millis();
  round_time = 0;
  byte N = 1;
  for (byte i = 0; i < N; i++) {

    kem_show(200 + i);
    done = false;
    started = false;

    while (done == false) {
      upd_hall();
      x = hall1;
      y = hall2;
      dx = long(x) - long(sx);
      dy = long(y) - long(sy);
      dsq = sqrt(dx * dx + dy * dy);

      // начало пройдено
      if (dsq > 100) {
        started = true;
        mdsq = dsq;
      }

      // ожидание конечной точки
      if (started && dsq < 50) {
        if (mdsq > dsq) { // запоминаем минимум
          mdsq = dsq;
        }
        if (mdsq < 5) { // достаточно близко
          done = true;
        }
        if (mdsq + 5 < dsq) { // начали удаляться
          done = true;
        }
      }
    }
  }
  unsigned long round_end = millis();
  round_time = (round_end - round_start) / N;
  long seg_time = round_time / 8;


  // round 3 - разметка
  round_start = millis();

  kem_show(300);
  done = false;
  started = false;

  byte n = 1;
  px[0] = sx;
  py[0] = sy;

  while (done == false) {
    upd_hall();
    x = hall1;
    y = hall2;
    dx = long(x) - long(sx);
    dy = long(y) - long(sy);
    dsq = sqrt(dx * dx + dy * dy);

    if (n < 9 && millis() - round_start >= seg_time) {
      px[n] = x;
      py[n] = y;
      round_start += seg_time;
      n++;
    }

    // начало пройдено
    if (dsq > 100) {
      started = true;
      mdsq = dsq;
    }

    // ожидание конечной точки
    if (started && dsq < 50) {
      if (mdsq > dsq) { // запоминаем минимум
        mdsq = dsq;
      }
      if (mdsq < 5) { // достаточно близко
        done = true;
      }
      if (mdsq + 5 < dsq) { // начали удаляться
        done = true;
      }
    }
  }

  cx = 0;
  cy = 0;
  for (byte i = 0; i < 8; i++) { // ищем центр
    cx += px[i];
    cy += py[i];
  }
  cx /= 8;
  cy /= 8;
  for (byte i = 0; i < 9; i++) { // вычисляем углы
    pa[i] = atan2(py[i] - cy, px[i] - cx);
  }

  // сдвигаем p0 в положительные числа
  while (pa[0] <= PI * 2) {
    pa[0] += PI * 2;
  }
  while (pa[8] <= pa[0] + PI) {
    pa[8] += PI * 2;
  }

  double range = pa[8] - pa[0]; // должно быть 360гр, но обычно меньше

  for (byte i = 1; i < 9; i++) { // дополняем до 360гр пропорционально
    pa[i] += (PI * 2 - range) * i / 8;
  }

  for (byte i = 1; i < 8; i++) { // смещаем все вправо от pa[0]
    while (pa[i] <= pa[0]) {
      pa[i] += PI * 2;
    }
  }

  srv_dir = 1;
  if (pa[2] > pa[6]) { // инверсное подключение
    srv_dir = -1;
    swap(&pa[1], &pa[7]);
    swap(&pa[2], &pa[6]);
    swap(&pa[3], &pa[5]);
  }


  // возврат в 0
  srv_mode = 1;
  srv_target = 0;
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

int spi_recv_int() {
  int l = spi_recv_byte();
  if (spi_fail) {
    return 0;
  }
  int h = spi_recv_byte();
  if (spi_fail) {
    return 0;
  }
  int res = ((h & 0xFF) << 8) | (l & 0xFF);
  return res;
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
      int recv16 = 0;
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
          if (!spi_fail && srv_mode == 0) {
            srv = recv;
          }
          break;
        case 0x06: // set calibrate
          recv = spi_recv_byte();
          if (!spi_fail) {
            calibrate = recv;
          }
          break;
        case 0x07: // send params
          spi_send_int((int) sx);
          spi_send_int((int) sy);
          spi_send_int((int) px[0]);
          spi_send_int((int) py[0]);
          spi_send_int((int) px[1]);
          spi_send_int((int) py[1]);
          spi_send_int((int) px[2]);
          spi_send_int((int) py[2]);
          spi_send_int((int) px[3]);
          spi_send_int((int) py[3]);
          spi_send_int((int) px[4]);
          spi_send_int((int) py[4]);
          spi_send_int((int) px[5]);
          spi_send_int((int) py[5]);
          spi_send_int((int) px[6]);
          spi_send_int((int) py[6]);
          spi_send_int((int) px[7]);
          spi_send_int((int) py[7]);
          spi_send_int((int) px[8]);
          spi_send_int((int) py[8]);
          spi_send_int((int) int(pa[0] * 100));
          spi_send_int((int) int(pa[1] * 100));
          spi_send_int((int) int(pa[2] * 100));
          spi_send_int((int) int(pa[3] * 100));
          spi_send_int((int) int(pa[4] * 100));
          spi_send_int((int) int(pa[5] * 100));
          spi_send_int((int) int(pa[6] * 100));
          spi_send_int((int) int(pa[7] * 100));
          spi_send_int((int) int(pa[8] * 100));
          spi_send_int((int) cx);
          spi_send_int((int) cy);
          break;
        case 0x08: // set srv mode
          recv = spi_recv_byte();
          if (!spi_fail) {
            srv_mode = recv;
          }
          break;
        case 0x09: // set srv target (000-799)
          recv16 = spi_recv_int();
          if (!spi_fail) {
            srv_target = double(recv16) / 100;
          }
          break;
        case 0x0A: // get srv pos (000-799)
          spi_send_int(int(find_pos));
          break;
        case 0x0B: // get srv target (000-799)
          spi_send_int(int(srv_target * 100));
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
