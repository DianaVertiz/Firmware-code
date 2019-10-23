//#include "board.h"
#include "prueba.h"

#define N_SAMPLES 256
//static volatile uint8_t Interrupt_Continue_Flag;
//static volatile uint8_t ADC_Interrupt_Done_Flag;
static volatile uint8_t Burst_Mode_Flag = 0;
static volatile uint8_t channelTC;
static uint16_t contador = 0;
static uint16_t adc_buffer[N_SAMPLES];
static uint8_t dmaChannelNum;
uint32_t DMAbuffer;
/*****************************************************************************/

void InitADC_DMA(void){
	static ADC_CLOCK_SETUP_T configADC;

	Chip_ADC_Init(LPC_ADC0,&configADC);
	Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH1,ENABLE);
	/* Initialize GPDMA controller */
	Chip_GPDMA_Init(LPC_GPDMA);
	/* Setting GPDMA interrupt */
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	/* Get the free channel for DMA transfer */
	dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_ADC_0);
	/* Get  adc value until get 'x' character */

}

void StartDMATransfer(void){

	uint16_t dataADC;
	while (contador != N_SAMPLES) {
		/* Start A/D conversion if not using burst mode */
		if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		channelTC = 0;
		Chip_GPDMA_Transfer(LPC_GPDMA, dmaChannelNum,
							  GPDMA_CONN_ADC_0,
							  (uint32_t) &DMAbuffer,
							  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
							  1);

		/* Waiting for reading ADC value completed */
		while (channelTC == 0) {}

		/* Get the ADC value from Data register*/
		dataADC = ADC_DR_RESULT(DMAbuffer);
		adc_buffer[contador-1] = dataADC;
	}
		/* Disable interrupts, release DMA channel */
	Chip_GPDMA_Stop(LPC_GPDMA, dmaChannelNum);
	NVIC_DisableIRQ(DMA_IRQn);
	/* Disable burst mode if any */

	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}


}

void DMA_IRQHandler(void){

	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChannelNum) == SUCCESS) {
			channelTC = 1;
			contador++;
		}
		else {
			/* Process error here */
		}
}


int main(void)
{
	InitADC_DMA();
	StartDMATransfer();
	while (1) {


	}

	return 0;

}
