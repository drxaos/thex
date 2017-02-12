void setup()
{
  cli();
  DDRB |= 1<<1 | 1<<2;         
  PORTB &= ~(1<<1 | 1<<2);
  TCCR1A = 0b00000010; 
  //TCCR1A = 0b10100010;  
  TCCR1B = 0b00011001;  
  ICR1H = 255;
  ICR1L = 255;
  sei(); 
  Serial.begin(9600);
  Serial.println("PWM resolution: 16-bit");
  Serial.println("PWM frequency: 244 Hz");
  Serial.println("PWM duty OC1A(D9): 0");
  Serial.println("PWM duty OC1B(D10): 0");
  Serial.println();
}

void loop() 
{
  if(Serial.available()) 
  {
    char c = Serial.read();
    uint16_t pwm = Serial.parseInt();
    if(c == 'A')
    { 
      pwm ? TCCR1A|=1<<7 : TCCR1A&=~(1<<7);
      OCR1AH = highByte(pwm);  
      OCR1AL = lowByte(pwm);
      Serial.print("PWM duty OC1A(D9): ");
      Serial.println(pwm);
    }
    if(c == 'B')
    { 
      pwm ? TCCR1A|=1<<5 : TCCR1A&=~(1<<5);
      OCR1BH = highByte(pwm);  
      OCR1BL = lowByte(pwm);
      Serial.print("PWM duty OC1B(D10): ");
      Serial.println(pwm);
    }
    if(c == 'R')
    {
      Serial.println();
      Serial.print("PWM resolution: ");
      Serial.print(pwm);
      Serial.println("-bit");
      pwm = pow(2, pwm);
      ICR1H = highByte(pwm);  
      ICR1L = lowByte(pwm);
      Serial.print("PWM frequency: ");
      Serial.print(16000000/(pwm+1UL));
      Serial.println(" Hz");
      Serial.println();
    }
  }
}
