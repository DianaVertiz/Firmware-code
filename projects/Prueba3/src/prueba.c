//#include "board.h"
#include "prueba.h"

#define N_SAMPLES 1000
#define N_SAMPLES2 10
#define N_SAMPLES3 10

//static volatile uint8_t Interrupt_Continue_Flag;
//static volatile uint8_t ADC_Interrupt_Done_Flag;
static volatile uint8_t Burst_Mode_Flag = 0;
static volatile uint8_t channelTC, TC;
static volatile uint8_t Interrupt_Continue_Flag;
static volatile uint8_t ADC_Interrupt_Done_Flag;
static volatile uint8_t Interrupt_Continue_Flag2;
static volatile uint8_t ADC_Interrupt_Done_Flag2;
static volatile uint8_t Interrupt_Continue_Flag3;
static volatile uint8_t ADC_Interrupt_Done_Flag3;
static uint16_t contador = 0;
static uint16_t counter = 0;
static uint16_t counter2 = 0;
static uint16_t counter3 = 0;
static uint16_t adc_buffer[N_SAMPLES];
static uint16_t adc_buffer2[N_SAMPLES2];
static uint16_t adc_buffer3[N_SAMPLES3];
static uint8_t dmaChannelNum;
uint32_t DMAbuffer;
/*****************************************************************************/

void InitADC_DMA(void){
	static ADC_CLOCK_SETUP_T configADC;

	Chip_ADC_Init(LPC_ADC0,&configADC);
	Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH1,ENABLE);
	Chip_ADC_SetSampleRate(LPC_ADC0, &configADC, 100);
	/* Initialize GPDMA controller */
	Chip_GPDMA_Init(LPC_GPDMA);
	/* Setting GPDMA interrupt */


	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	/* Get the free channel for DMA transfer */
	dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_ADC_0);
	/* Get  adc value until get 'x' character */

}

void StartDMATransfer(void){

	uint16_t dataADC;
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);
		if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		channelTC = 0;
		Led_On(YELLOW_LED);
		Chip_GPDMA_Transfer(LPC_GPDMA, dmaChannelNum,
							  GPDMA_CONN_ADC_0,
							  (uint32_t) &DMAbuffer,
							  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
							  1);

		/* Waiting for reading ADC value completed */
		while (channelTC == 0) {}

		/* Get the ADC value from Data register*/
		dataADC = ADC_DR_RESULT(DMAbuffer);
		adc_buffer[contador-1]=dataADC;

		if(contador == N_SAMPLES){
			contador = 0;
			Led_Off(YELLOW_LED);
		}
	/* Disable burst mode if any */

	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}


}

void DMA_IRQHandler(void){

	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChannelNum) == SUCCESS) {
			channelTC = 1;
			contador++;
			/* Disable interrupts, release DMA channel */
				Chip_GPDMA_Stop(LPC_GPDMA, dmaChannelNum);
				NVIC_DisableIRQ(DMA_IRQn);

		}
		else {
			/* Process error here */
		}
}



void init_ADC(void){
	static ADC_CLOCK_SETUP_T configADC;

	Chip_ADC_Init(LPC_ADC0,&configADC);
	Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH1,ENABLE);
	Chip_ADC_Init(LPC_ADC0,&configADC);
	Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH2,ENABLE);
	Chip_ADC_Init(LPC_ADC1,&configADC);
	Chip_ADC_EnableChannel(LPC_ADC1,ADC_CH3,ENABLE);
}


void App_Interrupt_Test(void)
{
	/* Enable ADC Interrupt */
	NVIC_EnableIRQ(ADC0_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);

	Interrupt_Continue_Flag = 1;
	ADC_Interrupt_Done_Flag = 1;
	while (Interrupt_Continue_Flag) {
		if (ADC_Interrupt_Done_Flag) {
			ADC_Interrupt_Done_Flag = 0;
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
	}
	/* Disable ADC interrupt */
	NVIC_DisableIRQ(ADC0_IRQn);
}

void IRQHandler(void){

	uint16_t dataADC;

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, DISABLE);
	Chip_ADC_ReadValue(LPC_ADC0, ADC_CH1, &dataADC);
	ADC_Interrupt_Done_Flag = 1;
	adc_buffer[counter] = dataADC;
	TC = 1;
	if(counter != N_SAMPLES-1){
		counter++;
		NVIC_EnableIRQ(ADC0_IRQn);
		Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	}
	else{
		counter = 0;
		Interrupt_Continue_Flag = 0;
	}
}


void App_Interrupt_Test2(void)
{
	/* Enable ADC Interrupt */
	NVIC_EnableIRQ(ADC0_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, ENABLE);

	Interrupt_Continue_Flag2 = 1;
	ADC_Interrupt_Done_Flag2 = 1;
	while (Interrupt_Continue_Flag2) {
		if (ADC_Interrupt_Done_Flag2) {
			ADC_Interrupt_Done_Flag2 = 0;
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
	}
	/* Disable ADC interrupt */
	NVIC_DisableIRQ(ADC0_IRQn);
}

void IRQHandler2(void){

	uint16_t dataADC;

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, DISABLE);
	Chip_ADC_ReadValue(LPC_ADC0, ADC_CH2, &dataADC);
	ADC_Interrupt_Done_Flag2 = 1;
	adc_buffer2[counter2] = dataADC;
	TC = 1;
	if(counter2 != N_SAMPLES2-1){
		counter2++;
		NVIC_EnableIRQ(ADC0_IRQn);
		Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, ENABLE);
	}
	else{
		counter2 = 0;
		Interrupt_Continue_Flag2 = 0;
	}
}

void App_Interrupt_Test3(void)
{
	/* Enable ADC Interrupt */
	NVIC_EnableIRQ(ADC1_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC1, ADC_CH3, ENABLE);

	Interrupt_Continue_Flag3 = 1;
	ADC_Interrupt_Done_Flag3 = 1;
	while (Interrupt_Continue_Flag3) {
		if (ADC_Interrupt_Done_Flag3) {
			ADC_Interrupt_Done_Flag3 = 0;
			Chip_ADC_SetStartMode(LPC_ADC1, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
	}
	/* Disable ADC interrupt */
	NVIC_DisableIRQ(ADC1_IRQn);
}

void IRQHandler3(void){

	uint16_t dataADC;

	Chip_ADC_Int_SetChannelCmd(LPC_ADC1, ADC_CH3, DISABLE);
	Chip_ADC_ReadValue(LPC_ADC1, ADC_CH3, &dataADC);
	ADC_Interrupt_Done_Flag3 = 1;
	adc_buffer3[counter3] = dataADC;
	TC = 1;
	if(counter3 != N_SAMPLES3-1){
		counter3++;
		NVIC_EnableIRQ(ADC1_IRQn);
		Chip_ADC_Int_SetChannelCmd(LPC_ADC1, ADC_CH3, ENABLE);
	}
	else{
		counter3 = 0;
		Interrupt_Continue_Flag3 = 0;
	}
}



int main(void)
{
	SystemCoreClockUpdate();
	Init_Leds();
	InitADC_DMA();

	//init_ADC();
	//App_Interrupt_Test();
	//App_Interrupt_Test2();
	//App_Interrupt_Test3();
	while (1) {
		StartDMATransfer();


	}

	return 0;

}
