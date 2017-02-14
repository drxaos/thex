
// ##### UTILS #####

void swap(float *x, float *y)
{
  float t;
  t = *x;
  *x = *y;
  *y = t;
}

union {
  byte asByte;
  char asChar;
} temp8;

union {
  byte asBytes[2];
  char asChars[2];
  uint16_t asUInt;
  int asInt;
  float asFloat;
} temp16;


#define DBG_EN
#ifdef DBG_EN
#define DBG1(x) {Serial.println(x);}
#define DBG2(x,y) {Serial.print(x);Serial.println(y);}
#else
#define DBG1(x)
#define DBG2(x,y)
#endif

// ##### INIT #####

#include <Servo.h>
Servo _srv;

void init_spi() {
  // SPI slave
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);
}

void setup() {
  cli();
  _srv.attach(9);
  init_spi();
  sei();

#ifdef DBG_EN
  Serial.begin(9600);
#endif
}


// ##### DATA #####

uint16_t srv_base = 91; // нулевая скорость
int sp[] = { // уровни скорости (получаются эмпирически)
  -25, -18, -15, -12, -10, -8, -6, -4, -2, -1,
  0, 1, 2, 4, 6, 8, 10, 12, 15, 18, 25
};
byte srv_dir = 1; // 1 по часовой, -1 против
int cx, cy; // середина, от нее меряются углы
float pa[] = {0, 1, 2, 3, 4, 5, 6, 7, 8}; // здесь будут углы после калибровки

int srv = 0; // скорость относительно srv_base
uint16_t srv_prev = srv_base + 1;
uint16_t srv_next = srv_base;
byte srv_mode = 0; // 0-free, 1-hold
float srv_target = 0;
byte show_mode = 9;

float srv_pos = 0;
unsigned long srv_change = millis();
int hall1, hall2;
unsigned long show_time;
byte calibrate = 0;



// ##### CONTROL #####

void upd_hall() {
  hall1 = analogRead(A0);
  hall2 = analogRead(A1);
}

int spd(char level) { // -10..10
  return sp[10 + level];
}

boolean upd_srv() {
  srv_next = srv_base + srv;
  if (srv_next == srv_prev) {
    return true;
  }
  if (srv_next != srv_prev && millis() - srv_change > 20) {
    if (srv_next > srv_prev) {
      if (srv_next - srv_prev >= 4) srv_next = srv_prev + (srv_next - srv_prev) / 2;
      else srv_next = srv_prev + 1;
    }
    if (srv_next < srv_prev) {
      if (srv_prev - srv_next >= 4) srv_next = srv_prev - (srv_prev - srv_next) / 2;
      else srv_next = srv_prev - 1;
    }
    _srv.write(srv_next);
    srv_prev = srv_next;
    srv_change = millis();
  }
  return false;
}

float find_pos(int x, int y) {
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
  return float(res);
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
    srv = spd(0);
  }
  if (7.75 < dif && dif <= 7.97) {
    srv = spd(2 * srv_dir);
  }
  if (7 < dif && dif <= 7.75) {
    srv = spd(3 * srv_dir);
  }
  if (7 < dif && dif <= 7.5) {
    srv = spd(4 * srv_dir);
  }
  if (6 < dif && dif <= 7) {
    srv = spd(7 * srv_dir);
  }
  if (5 < dif && dif <= 6) {
    srv = spd(9 * srv_dir);
  }
  if (4 <= dif && dif <= 5) {
    srv = spd(10 * srv_dir);
  }
  if (3 <= dif && dif < 4) {
    srv = spd(-10 * srv_dir);
  }
  if (2 <= dif && dif < 3) {
    srv = spd(-9 * srv_dir);
  }
  if (1 <= dif && dif < 2) {
    srv = spd(-7 * srv_dir);
  }
  if (0.5 <= dif && dif < 1) {
    srv = spd(-4 * srv_dir);
  }
  if (0.25 <= dif && dif < 0.5) {
    srv = spd(-3 * srv_dir);
  }
  if (0.04 <= dif && dif < 0.25) {
    srv = spd(-2 * srv_dir);
  }
  if (dif < 0.04) {
    srv = spd(0);
  }
}

void loop() {
  boolean spi_comm = spi_handler();

  if (calibrate == 1) {
    do_calibrate();
    calibrate = 0;
  }

  upd_hall();

  srv_pos = find_pos(hall1, hall2);

  switch (srv_mode) {
    case 1:
      srv_hold();
      break;
  }

  upd_srv();
}


void do_calibrate() {

  unsigned int x, y;
  long dx, dy;
  double dsq, pdsq, mdsq;
  boolean started;
  boolean done;

  int sx, sy;
  long round_time;
  int px[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  int py[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

  srv_dir = 0;

  DBG1("Calibrating");

  // останов
  srv = spd(0);
  while (!upd_srv()) {}

  // начальная точка
  upd_hall();
  x = sx = hall1;
  y = sy = hall2;

  DBG2("sx: ", sx);
  DBG2("sy: ", sy);

  // round 1 - разгон
  done = false;
  started = false;

  srv = spd(1);
  while (!upd_srv()) {}
  pdsq = 0;
  byte splvl = 0;
  while (done == false) {
    upd_hall();
    x = hall1;
    y = hall2;
    dx = long(x) - long(sx);
    dy = long(y) - long(sy);
    dsq = sqrt(dx * dx + dy * dy);

    srv = spd(5);
    while (!upd_srv()) {}

    // начало пройдено
    if (!started && dsq > 50) {
      started = true;
      mdsq = dsq;
      DBG1("r1, started")
    }

    // ожидание конечной точки
    if (started && dsq < 50) {
      if (mdsq > dsq) { // запоминаем минимум
        mdsq = dsq;
        DBG2("r1, mdsq: ", mdsq)
      }
      if (mdsq < 8) { // достаточно близко
        done = true;
        DBG2("r1, mdsq: ", mdsq)
        DBG1("r1, done")
      }
      if (mdsq + 8 < dsq) { // начали удаляться
        done = true;
        DBG2("r1, mdsq+5 < ", dsq)
        DBG1("r1, done")
      }
    }
  }

  // round 2 - замер времени
  unsigned long round_start = millis();
  round_time = 0;
  byte N = 2;
  for (byte i = 0; i < N; i++) {

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
      if (dsq > 60) {
        started = true;
        mdsq = dsq;
      }

      // ожидание конечной точки
      if (started && dsq < 30) {
        if (mdsq > dsq) { // запоминаем минимум
          mdsq = dsq;
        }
        if (mdsq < 8) { // достаточно близко
          done = true;
        }
        if (mdsq + 8 < dsq) { // начали удаляться
          done = true;
        }
      }
    }
  }
  unsigned long round_end = millis();
  round_time = (round_end - round_start) / N;
  long seg_time = round_time / 8;
  DBG2("r2, time: ", round_time)
  DBG2("r2, seg_time: ", seg_time)

  // round 3 - разметка
  round_start = millis();

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
      DBG2("r3, seg: ", n)
      DBG2("    x: ", x)
      DBG2("    y: ", x)
      DBG2("    round_start: ", round_start)
      px[n] = x;
      py[n] = y;
      round_start += seg_time;
      DBG2("    round_end: ", round_start)
      n++;
    }

    // начало пройдено
    if (dsq > 60) {
      started = true;
      mdsq = dsq;
    }

    // ожидание конечной точки
    if (started && dsq < 30) {
      if (mdsq > dsq) { // запоминаем минимум
        mdsq = dsq;
      }
      if (mdsq < 8) { // достаточно близко
        done = true;
      }
      if (mdsq + 8 < dsq) { // начали удаляться
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



// ##### COMMUNICATION #####

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
  temp8.asByte = SPDR;
  return temp8.asByte;
}

int spi_recv_int() {
  temp16.asBytes[0] = spi_recv_byte();
  if (spi_fail) {
    return 0;
  }
  temp16.asBytes[1] = spi_recv_byte();
  if (spi_fail) {
    return 0;
  }
  return temp16.asInt;
}

char spi_recv_char() {
  SPDR = 0;
  spi_time = millis();
  spi_fail = false;
  while (!SPI_RDY) {
    if (millis() - spi_time > 100) {
      spi_fail = true;
      return 0;
    }
  }
  temp8.asByte = SPDR;
  return temp8.asChar;
}

uint16_t spi_recv_uint() {
  temp16.asBytes[0] = spi_recv_byte();
  if (spi_fail) {
    return 0;
  }
  temp16.asBytes[1] = spi_recv_byte();
  if (spi_fail) {
    return 0;
  }
  return temp16.asUInt;
}

boolean spi_handler()
{
  boolean comm = false;
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
      char crecv = 0;
      uint16_t urecv16 = 0;
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
          recv16 = spi_recv_int();
          if (!spi_fail && srv_mode == 0) {
            srv = recv16;
          }
          break;
        case 0x06: // set calibrate
          recv = spi_recv_byte();
          if (!spi_fail) {
            calibrate = recv;
          }
          break;
        case 0x07: // send params
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
        case 0x0C: // set srv base (0-65535)
          urecv16 = spi_recv_uint();
          if (!spi_fail) {
            srv_base = urecv16;
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
  return comm;
}
