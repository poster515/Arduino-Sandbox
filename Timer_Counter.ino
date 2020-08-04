//#define COUNT_CYCLES
#define SLOW_SAMPLE

bool l = false;
volatile bool ISR_done;

// simply set the ISR variable to true
//void TC3_Handler(){ ISR_done = true; }

// setup TC2 in capture mode - just use it to grab counter values
void TC2_setup() {
  PMC->PMC_PCER0 |= PMC_PCER0_PID29;  // TC2 power ON : Timer Counter 0 channel 2 IS TC2

//        TIMER_CLOCK1 Clock: internal MCK/2 clock signal (from PMC)
//        TIMER_CLOCK2 Clock: internal MCK/8 clock signal (from PMC)
//        TIMER_CLOCK3 Clock: internal MCK/32 clock signal (from PMC)
//        TIMER_CLOCK4 Clock: internal MCK/128 clock signal (from PMC)

  TC0->TC_CHANNEL[2].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK2;
  TC0->TC_CHANNEL[2].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;// Software trigger TC2 counter and enable
}

// setup TC3 in waveform mode - use it generate 25 us interrupts
void TC3_setup() {
  PMC->PMC_PCER0 |= PMC_PCER0_PID30;  // TC3 power ON : Timer Counter 1 channel 0 IS TC3
  TC1->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1   // MCK/2, clk on rising edge
                              | TC_CMR_WAVE                // Waveform mode
                              | TC_CMR_WAVSEL_UP_RC;       // UP mode with automatic trigger on RC Compare

  // e.g., an RC value of 42 cycles, a clock of 84MHz/2 = 42 MHz, the sampling frequency is 1 MHz
  #ifdef SLOW_SAMPLE
    TC1->TC_CHANNEL[0].TC_RC = 10500;  // Frequency = (Mck/2)/TC_RC = 4000 Hz
  #else 
    TC1->TC_CHANNEL[0].TC_RC = 1050;  // Frequency = (Mck/2)/TC_RC = 40,000 Hz
  #endif
  TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
  TC1->TC_CHANNEL[0].TC_IDR = ~(TC_IER_CPCS);
//  NVIC_EnableIRQ(TC3_IRQn);
  TC1->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;// reset counter and enable clock
}

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(38400);
  ISR_done = false;
  
  #ifdef COUNT_CYCLES
    TC2_setup();
  #endif
  TC3_setup();
}

void loop(){
  // reset counter
  #ifdef COUNT_CYCLES
    TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_SWTRG; //software trigger to reset the counter
  #endif

  // perform ADC, and transmit value
    Serial.println(analogRead(A0), DEC);

  // measure time taken to perform ADC and transmit value
    #ifdef COUNT_CYCLES
      uint32_t Time = TC0->TC_CHANNEL[2].TC_CV; //grab the counter value in real time
      Serial.print("Conversion time: ");
      Serial.println(Time, DEC);
    #endif
    
  // wait for ISR interrupt, and clear value
//    while(!ISR_done){}
    while(TC1->TC_CHANNEL[0].TC_SR & TC_SR_CPCS == 0){}
//    ISR_done = false;

}
