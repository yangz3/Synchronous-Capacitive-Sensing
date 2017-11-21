//Timer setup code uses references :timer interrupts
//by Amanda Ghassaei
//https://www.instructables.com/id/Arduino-Timer-Interrupts/

// fast ADC code uses references: http://yaab-arduino.blogspot.com/2015/02/fast-sampling-from-analog-input.html

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define ADC_BUF_SIZE 512
#define SAMPLE_SIZE 128
#define ADC_PIN A0
#define POT_PIN A7

uint16_t samples[SAMPLE_SIZE];
uint16_t input_buffer [ADC_BUF_SIZE];
uint16_t serial_buffer [1];

int curpos = 0;
int latency = 0;

// interupt setup
//storage variables
boolean toggle0 = 0;

void setup(){

  //set pins as outputs
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(13, OUTPUT);

  cli();//stop interrupts

//set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

//set timer1 interrupt at 2kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 780;// = (16*10^6) / (20*1024) - 1 ((must be <65536))
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS01 and CS00 bits for 1024 prescaler
  TCCR1B |= (1 << CS02) | (1 << CS00);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

 //set timer2 interrupt at 2kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 64 prescaler
  TCCR2B |= (1 << CS21) | (1 << CS20);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

sei();//allow interrupts


//adc configuration has to be after timer setup
  Serial.begin(115200);
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  pinMode(ADC_PIN, INPUT);
  pinMode(POT_PIN, INPUT);

}//end setup

ISR(TIMER0_COMPA_vect){ // 2KHz

  if (toggle0){
    digitalWrite(8,HIGH);
    toggle0 = 0;
  }
  else{
    digitalWrite(8,LOW);
    toggle0 = 1;
  }
}

ISR(TIMER1_COMPA_vect){ // 20Hz
 //do other things here
  extractSamples(input_buffer, samples, SAMPLE_SIZE);
  uint16_t rst = (uint16_t)calcMean(samples, SAMPLE_SIZE);
  rst = escapeTerminator(rst);
  serial_buffer[0] = rst;

  Serial.write((uint8_t *)serial_buffer, sizeof(serial_buffer));
  Serial.write(0x80);
  latency = (analogRead(POT_PIN));
}

ISR(TIMER2_COMPA_vect){ // 2KHz

  delayMicroseconds(20);
 if(toggle0 == 0){
      
      // indicator
      digitalWrite(13, HIGH);
      digitalWrite(13, LOW);

      // read samples
      input_buffer[curpos] = analogRead(ADC_PIN);
      curpos = (curpos+1) % ADC_BUF_SIZE;
    }
}
  

void extractSamples(uint16_t *buf, uint16_t *out, int samplecount){
 if(curpos > samplecount){
  memcpy(out, &buf[curpos - samplecount], samplecount*sizeof(uint16_t));
 }else{
  memcpy(out, &buf[ADC_BUF_SIZE - (samplecount - curpos)], (samplecount-curpos)*sizeof(uint16_t));
  memcpy(out + (samplecount - curpos), &buf[0], curpos*sizeof(uint16_t));
 }
}

int calcMean(uint16_t* buf, int len){
  long rst = 0;
  for(int i = 0; i < len; i++){
    rst+= buf[i];
  }
  return rst/len;
}
void loop(){
  /*
  //do other things here
  extractSamples(input_buffer, samples, SAMPLE_SIZE);
  uint16_t rst = (uint16_t)calcMean(samples, SAMPLE_SIZE);
  Serial.write(rst);
  Serial.write(0x80);
  latency = (analogRead(POT_PIN));
  */
  delay(1);
}

static inline uint16_t escapeTerminator (uint16_t v){
  if((v & 0xff) == 0x80){
    v++;
  }

  if((v >> 8) == 0x80){
    v += 0x100 - (v & 0xff); 
  }
  return v;
}

