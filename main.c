/* MAIN.C file
 * 
 * Copyright (c) 2002-2005 STMicroelectronics
 */
 
 //#define Green_LED GPIOA, GPIO_PIN_3
 //#define Test_LED GPIOC, GPIO_PIN_5
 #define DRIVER_ADDR_PIN GPIOD, GPIO_PIN_3
 #define DAT_GND GPIOC, GPIO_PIN_3
 #define CLK_GND GPIOC, GPIO_PIN_4
 #define CS_POT GPIOD, GPIO_PIN_3
 #define CS_WG GPIOA, GPIO_PIN_2
 //#define TEST_LED GPIOA, GPIO_PIN_3
 #define MUTE GPIOA, GPIO_PIN_1
 
 #define CHANGE_FREQ GPIOA, GPIO_PIN_3
 #define FREQ_DOWN GPIOD, GPIO_PIN_5
 #define FREQ_UP GPIOD, GPIO_PIN_6
 
 #define FREQ_SELECT GPIOD, GPIO_PIN_1
 
 #define TIM4_PERIOD       255//124
 
 //This is for the display only. Must change for the audio manually in initFreq()
 #define INIT_FREQ 100	
 
 #define FAST_COUNT_LIMIT 15
 #define SFAST_COUNT_LIMIT 15
 #define FREQ_UP_LIMIT 999999
 #define FREQ_DOWN_LIMIT 1
 
 
 #include "STM8S.h"
 //#include "stm8s103_serial.h"
 #include "stm8s103_adc.h"
 #include "stm8s_tim4.h"
  //#include "stm8s.h"
 
__IO uint8_t VolumeADC = 0;
__IO uint32_t TimeCounter = 0;
__IO int32_t frequency = INIT_FREQ;

int32_t old_frequency = INIT_FREQ;
uint8_t fast_counter = 0;
uint8_t sfast_counter = 0;

uint8_t volume_counter = 0;
uint8_t volume_flag = 1;

void delay (int ms) //Function Definition 
{
int i =0 ;
int j=0;
for (i=0; i<=ms; i++)
{
for (j=0; j<120; j++) // Nop = Fosc/4
_asm("nop"); //Perform no operation //assembly code 
}
}

void divmnu(uint16_t q[], uint16_t r[],
     const uint16_t u[], const uint16_t v[]);


void flipLED(void);

void startLCD(void);

void changePot(uint8_t volumeCommand);

void getDispRam(uint8_t *disp_ram, uint32_t frequency);

void updateDisp(uint32_t frequency);

//void updateFreq(uint32_t frequency);
void updateFreq(void);
void freqButton(int8_t freq_up);

void initFreq(void);
void wgDelay(void);
void sendWg(uint8_t b1, uint8_t b2);

void clkGndHiLo(void);

main()
{
	
	

		
	uint8_t volumeCommand = 0;
	uint8_t oldVolumeCommand = 0;

	
	CLK_DeInit();
	
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
	
	TIM4_TimeBaseInit(TIM4_PRESCALER_128, TIM4_PERIOD);
	TIM4_ClearFlag(TIM4_FLAG_UPDATE);
	TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
	
	
	
	
	I2C_Cmd(DISABLE);
  SPI_Cmd(ENABLE);	
	
	I2C_DeInit();
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C , DISABLE);  
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI , DISABLE);
	
	//GPIO_DeInit(GPIOA); //prepare Port A for working 
	//GPIO_Init (Green_LED, GPIO_MODE_OUT_PP_LOW_SLOW);
	
	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOC); //prepare Port C for working 
	GPIO_DeInit(GPIOD);
	//GPIO_Init (Test_LED, GPIO_MODE_OUT_PP_LOW_SLOW);
	//GPIO_WriteLow(GPIOC,GPIO_PIN_5);
	
	GPIO_Init (CLK_GND, GPIO_MODE_OUT_OD_HIZ_SLOW);
	GPIO_WriteHigh(CLK_GND);
	
	GPIO_Init (DAT_GND, GPIO_MODE_OUT_OD_HIZ_SLOW);
	GPIO_WriteHigh(DAT_GND);
	
	GPIO_Init (CS_POT, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_WriteHigh(CS_POT);
	
	GPIO_Init (CS_WG, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_WriteHigh(CS_WG);

	//GPIO_Init (TEST_LED, GPIO_MODE_OUT_OD_HIZ_SLOW);
	//GPIO_WriteLow(TEST_LED);
	
	
	GPIO_Init (MUTE, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_WriteHigh(MUTE);
	
	GPIO_Init (CHANGE_FREQ, GPIO_MODE_IN_FL_IT);
	GPIO_Init (FREQ_UP, GPIO_MODE_IN_FL_NO_IT);
	GPIO_Init (FREQ_DOWN, GPIO_MODE_IN_FL_NO_IT);
	GPIO_Init (FREQ_SELECT, GPIO_MODE_IN_FL_NO_IT);
	
	//GPIO_DeInit(GPIOD); //prepare Port D for working 
	GPIO_Init (DRIVER_ADDR_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_WriteHigh(GPIOD,GPIO_PIN_3);
	
	
	
	
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, EXTI_SENSITIVITY_FALL_ONLY);
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);
	
	
	
	
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C , ENABLE);  
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI , ENABLE);
	
	I2C_Cmd(ENABLE);
	
	//I2C_ITConfig((I2C_IT_EVT |I2C_IT_BUF | I2C_IT_ERR), ENABLE);
	
	I2C_Init(1000, 0x48, I2C_DUTYCYCLE_2, I2C_ACK_CURR, 
              I2C_ADDMODE_7BIT, CLK_GetClockFreq()/1000 );
							
	I2C_AcknowledgeConfig(I2C_ACK_CURR);
	
	
	
	
	
	SPI_DeInit();
	SPI_Init(SPI_FIRSTBIT_MSB, 
           SPI_BAUDRATEPRESCALER_256, 
           SPI_MODE_MASTER, SPI_CLOCKPOLARITY_LOW, 
           SPI_CLOCKPHASE_1EDGE , 
					 SPI_DATADIRECTION_2LINES_FULLDUPLEX, 
				   SPI_NSS_SOFT,0x00);
	SPI_Cmd(ENABLE);
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
	
		
	
	
	//changePot(0x1F);	
	

	
	startLCD();
	

	enableInterrupts();
	TIM4_Cmd(ENABLE);

	
	updateDisp(frequency);
	

	initFreq();
	
	
	
	while (1){		
	
		if(!GPIO_ReadInputPin(FREQ_UP) || !GPIO_ReadInputPin(FREQ_DOWN)){
			delay(3000);
			if(!GPIO_ReadInputPin(FREQ_UP)){
				freqButton(1);
			}
			if(!GPIO_ReadInputPin(FREQ_DOWN)){
				freqButton(-1);
			}
		}
		else{
			fast_counter=0;
			sfast_counter=0;
		}
		
		
			if(VolumeADC-volumeCommand>=20 || VolumeADC-volumeCommand<=-20){
				volumeCommand=VolumeADC;
			}


			if(oldVolumeCommand!=volumeCommand){
				oldVolumeCommand=volumeCommand;
				GPIO_WriteLow(MUTE);
				changePot(volumeCommand);
				delay(4000);
				GPIO_WriteHigh(MUTE);
				//volume_flag = 0;
				//volume_counter = 0;
				//GPIO_WriteLow(MUTE);
			}

	}
	
}




























void freqButton(int8_t freq_up){
	//Second condition s.t. when the freq gets low it doesn't
	//race below zero
	int16_t freq_scaler = freq_up;
	if(GPIO_ReadInputPin(FREQ_SELECT)){
			freq_scaler *= 1000;
	}
	
	if(fast_counter==FAST_COUNT_LIMIT){
			if(sfast_counter==FAST_COUNT_LIMIT)
				frequency+=freq_scaler*20;
			else
				frequency+=freq_scaler*5;
	}
	//Conditions:
	//1) If the change does not go below zero
	//2) If the freq.  needs to go up (so that we are not stuck at 1 Hz)
	else if (frequency>FREQ_DOWN_LIMIT || freq_up==1){
		frequency+=freq_scaler*1;
	}
		
		
	if(frequency>FREQ_UP_LIMIT){
		frequency=FREQ_UP_LIMIT;
	}
	if(frequency<FREQ_DOWN_LIMIT){
		frequency=FREQ_DOWN_LIMIT;
	}
		
		
		updateDisp(frequency);
		
		delay(1);
		
		
		//If button still pressed, increment progressively quicker
		if(!GPIO_ReadInputPin(FREQ_UP) || !GPIO_ReadInputPin(FREQ_DOWN)){
			//delay(50);
			//if(!GPIO_ReadInputPin(FREQ_UP) || !GPIO_ReadInputPin(FREQ_DOWN)){
				
				if(fast_counter<FAST_COUNT_LIMIT){
					fast_counter+=1;
				}
				
				else if(sfast_counter<SFAST_COUNT_LIMIT){
					sfast_counter+=1;
				}
					
			//}	
		}
		
}




void flipLED(void)
{
	//if(volume_flag){
		VolumeADC = ADC_Read(AIN3)/4;
	//}
	//else{
	//	volume_counter++;
	//}
		
	
	//if(volume_counter==255){
	//	volume_flag = 1;
	//	GPIO_WriteHigh(MUTE);
	//}
	
}

void sendWg(uint8_t b1, uint8_t b2){
	
	SPI_SendData(b1);
	while(!SPI_GetFlagStatus(SPI_FLAG_TXE));
	SPI_SendData(b2);
	while(!SPI_GetFlagStatus(SPI_FLAG_TXE));
	
}

void wgDelay(void){
	delay(1);
	
	GPIO_WriteHigh(CS_WG);
	
	delay(1);
	
	GPIO_WriteLow(CS_WG);
	
	delay(1);
}



void initFreq(void){
	while(SPI_GetFlagStatus(SPI_FLAG_BSY));
	GPIO_WriteLow(CS_WG);
	delay(5);
	
	sendWg(0b00100000,0b00000000);
	wgDelay();
	
	sendWg(0b00100001,0b00000000);
	wgDelay();
	
	sendWg(0b00100001,0b00000000);
	wgDelay();
	
	//LSB
	sendWg(0b01000100,0b00110010);
	wgDelay();
	
	//MSB
	sendWg(0b01000000,0b00000000);
	wgDelay();
	
	sendWg(0b00100001,0b00000000);
	wgDelay();
	
	sendWg(0b10101001,0b11110001);
	wgDelay();
	
	sendWg(0b10000000,0b00000000);
	wgDelay();
	
	sendWg(0b11000000,0b00000000);
	wgDelay();
	
	sendWg(0b11100000,0b00000000);
	wgDelay();
	
	sendWg(0b00100001,0b00000000);
	wgDelay();
	
	sendWg(0b00100000,0b00000000);
	wgDelay();
	
	sendWg(0b00100000,0b00000000);
	wgDelay();
	
	sendWg(0b00100000,0b00000000);
	wgDelay();
	
	sendWg(0b00100000,0b00000000);	
	
	
	delay(5);
	
	GPIO_WriteHigh(CS_WG);
	delay(1);	
}



void updateFreq(void)
{
	
	uint16_t q[2], r[1];
	uint16_t *u, *v;
	uint16_t entire_u[]={0x0000,(uint16_t)((frequency<<12)&0xFFFF),(uint16_t)((frequency>>4)&0xFFFF)};
	uint16_t entire_v[]={0x7840,0x017D};

	u=&entire_u[0];
  v=&entire_v[0];
	
	divmnu(q, r, u, v);

	while(SPI_GetFlagStatus(SPI_FLAG_BSY));
	
	GPIO_WriteLow(CS_WG);
	delay(5);
	
	
	sendWg(0b00100000,0b00000000);
	wgDelay();

	if(r[0]>0xBF){
	sendWg((uint8_t)(((q[0]>>8)&0x3F)+0x40),(uint8_t)((q[0]&0xFF)+0b1));
	}
	else{
	sendWg((uint8_t)(((q[0]>>8)&0x3F)+0x40),(uint8_t)(q[0]&0xFF));
	}
	wgDelay();
	
	sendWg((uint8_t)(((q[1]>>6)&0x3F)+0x40),(uint8_t)(((q[1]&0x3F)<<2)+((q[0]>>14)&0b11)));
	
	delay(5);
	
	GPIO_WriteHigh(CS_WG);
	delay(1);	
	
}







void changePot(uint8_t volumeCommand){
	while(SPI_GetFlagStatus(SPI_FLAG_BSY));
	
	GPIO_WriteLow(CS_POT);
	delay(5);
	
	SPI_SendData(0b00010011);
	while(!SPI_GetFlagStatus(SPI_FLAG_TXE));
	
	SPI_SendData(volumeCommand);
	while(!SPI_GetFlagStatus(SPI_FLAG_TXE));
	
	delay(5);
	
	GPIO_WriteHigh(CS_POT);
	delay(1);	
}








void updateDisp(uint32_t frequency){
	
	
	uint8_t disp_ram[16]={0};
	int i=0;
	
	getDispRam(disp_ram,frequency);
	
	
	
	
	I2C_GenerateSTART(ENABLE);
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
			
	I2C_Send7bitAddress((uint8_t)0b01110000,I2C_DIRECTION_TX);
		
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
			
	
	
	I2C_SendData(0b00000000);	//data address

	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	
	for(i=0; i<16; i++){
		
		I2C_SendData(disp_ram[i]);
		
		while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  }
	
			
	I2C_GenerateSTOP(ENABLE);
			
	(void)I2C->SR1;
	(void)I2C->SR3;
	
}


void clkGndHiLo(void){
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
}


void startLCD(void){
	
	uint8_t i = 0;
	
	//start
	GPIO_WriteLow(DAT_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	for(i=0; i<8; i++){
		clkGndHiLo();
	}

	/*
	//b1
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b2
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b3
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b4
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b5
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b6
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b7
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	
	//b8
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteLow(CLK_GND);
	delay(1);
	*/
	
	//stop
	GPIO_WriteHigh(CLK_GND);
	delay(1);
	GPIO_WriteHigh(DAT_GND);
	delay(10);
	
	
	
	
	//welcome message
	
	I2C_GenerateSTART(ENABLE);
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
			
	I2C_Send7bitAddress((uint8_t)0b01110000,I2C_DIRECTION_TX);
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	I2C_SendData(0b11000000); //mode

	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
			
	I2C_SendData(0b10000000); //data pointer
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	
	
	I2C_SendData(0b11100000); //device select
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	
	I2C_SendData(0b11111000); //bank select
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b01110000);	//disable blinking

	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	
	
	/*
	I2C_SendData(0b11100000); //display info : pin 1-2
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b11100100); //display info : pin 3-4
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b11100000); //display info : pin 5-6
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b11100000); //display info : pin 7-8
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b11100000); //display info : pin 9-10
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b01100001); //display info : pin 11-12
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b11100100); //display info : pin 13-14
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b00000000); //display info : pin 15-16
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b10000000); //display info : pin 21-22
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b10010010); //display info : pin 23-24
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b1110000); //display info : pin 25-26
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b01110000); //display info : pin 27-28
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b00010000); //display info : pin 29-30
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b00000000); //display info : pin 31-32
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b00010010); //display info : pin 33-34
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(0b01101000); //display info : pin 35-36
	
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	*/
	
	
			
	I2C_GenerateSTOP(ENABLE);
			
	(void)I2C->SR1;
	(void)I2C->SR3;
	
	
	
	delay(10);
	
	
	
	
	I2C_GenerateSTART(ENABLE);
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
			
	I2C_Send7bitAddress((uint8_t)0b01110000,I2C_DIRECTION_TX);
		
	while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
			
	I2C_SendData(0b01001000);

	while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
			
	I2C_GenerateSTOP(ENABLE);
			
	(void)I2C->SR1;
	(void)I2C->SR3;
}





//ADAPTED FROM: https://github.com/hcs0/Hackers-Delight/blob/master/divmnu.c.txt
void divmnu(uint16_t q[], uint16_t r[],
     const uint16_t u[], const uint16_t v[]) {

   const uint32_t b = 65536; // Number base (16 bits).
   //uint16_t *un, *vn;  // Normalized form of u, v.
	 uint16_t vn[4];
   uint16_t un[8];
   uint32_t qhat;            // Estimated quotient digit.
   uint32_t rhat;            // A remainder.
   uint32_t p;               // Product of two digits.
   uint8_t i;
   int8_t j;
   int32_t t, k;

    /*
   if (m < n || n <= 0 || v[n-1] == 0)
      return 1;              // Return if invalid param.

   if (n == 1) {                        // Take care of
      k = 0;                            // the case of a
      for (j = m - 1; j >= 0; j--) {    // single-digit
         q[j] = (k*b + u[j])/v[0];      // divisor here.
         k = (k*b + u[j]) - q[j]*v[0];
      }
      if (r != NULL) r[0] = k;
      return 0;
   }
   */

   // Normalize by shifting v left just enough so that
   // its high-order bit is on, and shift u left the
   // same amount.  We may have to append a high-order
   // digit on the dividend; we do that unconditionally.

    const int8_t s = 7;        // 0 <= s <= 15.
   
   //vn = (uint16_t *)alloca(2*n);

   vn[1] = (v[1] << s) | (v[0] >> 16-s);

   vn[0] = v[0] << s;

   //un = (uint16_t *)alloca(2*(m + 1));
   un[3] = u[2] >> 16-s;
   for (i = 2; i > 0; i--)
      un[i] = (u[i] << s) | (u[i-1] >> 16-s);
   un[0] = u[0] << s;

   for (j = 1; j >= 0; j--) {       // Main loop.
      // Compute estimate qhat of q[j].
      qhat = (un[j+2]*b + un[j+1])/vn[1];
      rhat = (un[j+2]*b + un[j+1]) - qhat*vn[1];
again:
      if (qhat >= b || qhat*vn[0] > b*rhat + un[j])
      { qhat = qhat - 1;
        rhat = rhat + vn[1];
        if (rhat < b) goto again;
      }

      // Multiply and subtract.
      k = 0;
      for (i = 0; i < 2; i++) {
         p = qhat*vn[i];
         t = un[i+j] - k - (p & 0xFFFF);
         un[i+j] = t;
         k = (p >> 16) - (t >> 16);
      }
      t = un[j+2] - k;
      un[j+2] = t;

      q[j] = qhat;              // Store quotient digit.
      if (t < 0) {              // If we subtracted too
         q[j] = q[j] - 1;       // much, add back.
         k = 0;
         for (i = 0; i < 2; i++) {
            t = un[i+j] + vn[i] + k;
            un[i+j] = t;
            k = t >> 16;
         }
         un[j+2] = un[j+2] + k;
      }
   } // End j.
   // If the caller wants the remainder, unnormalize
   // it and pass it back.
	 /*
   if (r != NULL) {
      for (i = 0; i < n; i++)
         r[i] = (un[i] >> s) | (un[i+1] << 16-s);
   }*/
   //Only return most significant 16 bits of remainder
	 r[0] = (un[1] >> s) | (un[2] << 16-s);
   //return 0;
}