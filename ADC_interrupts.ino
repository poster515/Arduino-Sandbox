
/**********************************************************************************************/
/*   ADC conversions of 1 analog input triggered at 3 KHz by TC0 channel 2 TIOA2 pulse        */
/**********************************************************************************************/
//#define SLOW_SAMPLE

volatile boolean AnalogConversion;
volatile uint16_t Result;
const int num_samples = 15000;
static uint16_t results[num_samples];
int count = 0;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  adc_setup();
  tc_setup();
}

void loop()
{
  if (count == num_samples){
    // then we've obtained all samples, disable ADC interrupts
    NVIC_DisableIRQ((IRQn_Type)ADC_IRQn);
    // could also turn the timer counter off...

    // now to transmit all the values
    Serial.begin(38400);
    bool state = false;
    while(1){
      for(int i = 0; i < num_samples; ++i){
        Serial.println(results[i], DEC);
      }
      // toggle the LED at 1 Hz to show that the data is being sent
      delay(500);
      digitalWrite(LED_BUILTIN, state = !state);
    }
  }
}

/*************  Configure ADC function  *******************/
void adc_setup() {

  PMC->PMC_PCER1  |= PMC_PCER1_PID37;            // ADC power on
  ADC->ADC_CR      = ADC_CR_SWRST;               // Reset ADC
  ADC->ADC_MR     |= ADC_MR_TRGEN_EN             // Hardware trigger select
                  | ADC_MR_TRANSFER(1)           // Transfer Period = (TRANSFER * 2 + 3) ADCClock periods
                  | ADC_MR_PRESCAL(1)           // ADCClock = MCK / ( (PRESCAL+1) * 2 ). clock = 21 MHz
//                  | ADC_MR_LOWRES              // 1 = 10 bit sample, 0 = 12 bit sample
//                  TRGSEL_ADC_TRIG1: TIOA Output of the Timer Counter Channel 0
//                  TRGSEL_ADC_TRIG2: TIOA Output of the Timer Counter Channel 1
//                  TRGSEL_ADC_TRIG3: TIOA Output of the Timer Counter Channel 2
                  | ADC_MR_TRGSEL_ADC_TRIG3;     // Trigger by TIOA2
// per page 1403:
//1. Use ADC_ACR.IBCTL = 00 for sampling frequency below 500 kHz.
//2. Use ADC_ACR.IBCTL = 01 for sampling frequency between 500 kHz and 1 MHz.
  #ifndef SLOW_SAMPLE
  ADC->ADC_ACR = ADC_ACR_IBCTL(0b01);     // For frequency > 500 KHz
  #endif
  ADC->ADC_IER = ADC_IER_EOC7;            // Interrupt on End Of Conversions channel 7
  NVIC_EnableIRQ((IRQn_Type)ADC_IRQn);    // Enable ADC interrupt
  ADC->ADC_CHER = ADC_CHER_CH7;           // Enable Channels 7 = A0
}

void ADC_Handler () {
//  if(ADC->ADC_ISR & 0x80){} //just need to read ADC ISR to clear bit
//  NVIC_DisableIRQ((IRQn_Type)ADC_IRQn);
  results[count++] = 0000000000000000 | ADC->ADC_CDR[7];
//  NVIC_EnableIRQ((IRQn_Type)ADC_IRQn);
}

/*************  Timer Counter 0 Channel 2 to generate pulses via TIOA2  ************/
void tc_setup() {

  PMC->PMC_PCER0 |= PMC_PCER0_PID29;                      // TC2 power ON : Timer Counter 0 channel 2 IS TC2
  // choose clock as follows:
//        TIMER_CLOCK1 Clock: internal MCK/2 clock signal (from PMC)
//        TIMER_CLOCK2 Clock: internal MCK/8 clock signal (from PMC)
//        TIMER_CLOCK3 Clock: internal MCK/32 clock signal (from PMC)
//        TIMER_CLOCK4 Clock: internal MCK/128 clock signal (from PMC)
  TC0->TC_CHANNEL[2].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK2   // MCK/8, clk on rising edge
                              | TC_CMR_WAVE                // Waveform mode (required to use for ADC trigger)
                              | TC_CMR_WAVSEL_UP_RC        // UP mode with automatic trigger on RC Compare
                              | TC_CMR_ACPA_CLEAR          // Clear TIOA2 on RA compare match
                              | TC_CMR_ACPC_SET;           // Set TIOA2 on RC compare match

  // now choose sample frequency.
  // for example, with an RC value of 42 cycles, a clock of 84MHz/2 = 42 MHz, the sampling frequency is 1 MHz
  // for a slower sample frequency of 3 kHz = (84 MHz/X)/RC, choose X = 8, RC = 3500
  #ifdef SLOW_SAMPLE
    TC0->TC_CHANNEL[2].TC_RC = 3500;  // Frequency = (Mck/8)/TC_RC = 3000 Hz
    TC0->TC_CHANNEL[2].TC_RA = 1750;  // Any Duty cycle in between 1 and (RC-1), just toggles TIOA2 low
  #else 
    TC0->TC_CHANNEL[2].TC_RC = 210;  // Frequency = (Mck/8)/TC_RC = 50,000 Hz
    TC0->TC_CHANNEL[2].TC_RA = 105;  // Any Duty cycle in between 1 and (RC-1), just toggles TIOA2 low
  #endif
  TC0->TC_CHANNEL[2].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;// Software trigger TC2 counter and enable
}
