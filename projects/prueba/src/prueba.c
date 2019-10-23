//#include "board.h"
#include "prueba.h"
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#if (defined(BOARD_KEIL_MCB_4357) || defined(BOARD_KEIL_MCB_1857))
#define ADC_CH1 ADC_CH1
#define LPC_ADC0 LPC_ADC0
#define _LPC_ADC_IRQ ADC0_IRQn
#define GPDMA_CONN_ADC_0 GPDMA_CONN_ADC_0
#elif defined(BOARD_NXP_LPCXPRESSO_4337)
#define ADC_CH1 ADC_CH3
#define LPC_ADC0 LPC_ADC0
#define _LPC_ADC_IRQ ADC0_IRQn
#define GPDMA_CONN_ADC_0 GPDMA_CONN_ADC_0
#elif (defined(BOARD_HITEX_EVA_4350) || defined(BOARD_HITEX_EVA_1850))
#define ADC_CH1 ADC_CH2
#define LPC_ADC0 LPC_ADC1
#define _LPC_ADC_IRQ ADC1_IRQn
#define GPDMA_CONN_ADC_0 GPDMA_CONN_ADC_1
#endif

static const char *WelcomeMenu = "\r\nHello NXP Semiconductors \r\n"
							"ADC DEMO \r\n"
							"Sample rate : 400kHz \r\n"
							"Bit accuracy : 10 bits \r\n"
							"Press \'c\' to continue or \'x\' to quit\r\n"
							"Press \'o\' or \'p\' to set Sample rate\r\n"
							"Press \'k\' or \'l\' to set Bit accuracy "
							"(valid only when Burst mode is enabled)\r\n"
							"Press \'b\' to ENABLE or DISABLE Burst Mode\r\n";
static const char *SelectMenu = "\r\nPress number 1-3 to choose ADC running mode:\r\n"
						   "\t1: Polling Mode \r\n"
						   "\t2: Interrupt Mode \r\n"
						   "\t3: DMA Mode \r\n";
#define N_SAMPLES   1024
static ADC_CLOCK_SETUP_T ADCSetup;
static volatile uint8_t Burst_Mode_Flag = 1, Interrupt_Continue_Flag;
static volatile uint8_t ADC_Interrupt_Done_Flag, channelTC, dmaChannelNum;
uint32_t DMAbuffer;
static uint32_t dma_buffer[N_SAMPLES];
static uint16_t adc_buffer[N_SAMPLES];
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Print ADC value and delay */
static void App_print_ADC_value(uint16_t data)
{
	volatile uint32_t j;
	j = 5000000;
	//char* mensaje[]="ADC value is : 0x%04x\r\n";

	//EnviarMensaje(mensaje,23);
	/* Delay */
	while (j--) {}
}

/* DMA routine for ADC example */
static void App_DMA_Test(void)
{
	//uint16_t dataADC;
	uint16_t i = 0;
	/* Initialize GPDMA controller */
	Chip_GPDMA_Init(LPC_GPDMA);
	/* Setting GPDMA interrupt */
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	/* Get the free channel for DMA transfer */
	dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_ADC_0);
	/* Enable burst mode if any, the AD converter does repeated conversions
	   at the rate selected by the CLKS field in burst mode automatically */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	}
	/* Get  adc value until get 'x' character */
	while (Leer_intUART() != 'x') {
		/* Start A/D conversion if not using burst mode */
		if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		channelTC = 0;
		Chip_GPDMA_Transfer(LPC_GPDMA, dmaChannelNum,
						  GPDMA_CONN_ADC_0,
						  (uint32_t) &dma_buffer,
						  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
						  N_SAMPLES);

		/* Waiting for reading ADC value completed */
		while (channelTC == 0) {}

		/* Get the ADC value fron Data register*/
		//dataADC = ADC_DR_RESULT(DMAbuffer);
		//WriteInt(dataADC);
		//uartWriteByte('\n');

		for (i = 0; i < N_SAMPLES; i++) {
		/* Get the ADC value from Data register*/
			adc_buffer[i] = ADC_DR_RESULT(dma_buffer[i]);
			WriteInt(adc_buffer[i]);
			uartWriteByte('\n');
		}


	}
	/* Disable interrupts, release DMA channel */
	Chip_GPDMA_Stop(LPC_GPDMA, dmaChannelNum);
	NVIC_DisableIRQ(DMA_IRQn);
	/* Disable burst mode if any */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}
}

/* Interrupt routine for ADC example */
static void App_Interrupt_Test(void)
{
	/* Enable ADC Interrupt */
	NVIC_EnableIRQ(ADC0_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	/* Enable burst mode if any, the AD converter does repeated conversions
	   at the rate selected by the CLKS field in burst mode automatically */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	}
	Interrupt_Continue_Flag = 1;
	ADC_Interrupt_Done_Flag = 1;
	while (Interrupt_Continue_Flag) {
		if (!Burst_Mode_Flag && ADC_Interrupt_Done_Flag) {
			ADC_Interrupt_Done_Flag = 0;
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
	}
	/* Disable burst mode if any */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}
	/* Disable ADC interrupt */
	NVIC_DisableIRQ(ADC0_IRQn);
}

/* Polling routine for ADC example */
static void App_Polling_Test(void)
{
	uint16_t dataADC;

	/* Select using burst mode or not */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	}
	else {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}

	/* Get  adc value until get 'x' character */
	while (uartReadByte() != 'x') {
		/* Start A/D conversion if not using burst mode */
		if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		/* Waiting for A/D conversion complete */
		while (Chip_ADC_ReadStatus(LPC_ADC0, ADC_CH1, ADC_DR_DONE_STAT) != SET) {}
		/* Read ADC value */
		Chip_ADC_ReadValue(LPC_ADC0, ADC_CH1, &dataADC);
		/* Print ADC value */
		WriteInt(dataADC);
	}

	/* Disable burst mode, if any */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	ADC0 interrupt handler sub-routine
 * @return	Nothing
 */
void ADC0_IRQHandler(void)
{
	uint16_t dataADC;
	/* Interrupt mode: Call the stream interrupt handler */
	NVIC_DisableIRQ(ADC0_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, DISABLE);
	Chip_ADC_ReadValue(LPC_ADC0, ADC_CH1, &dataADC);
	ADC_Interrupt_Done_Flag = 1;
	WriteInt(dataADC);
	if (Leer_intUART() != 'x') {
		NVIC_EnableIRQ(ADC0_IRQn);
		Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	}
	else {Interrupt_Continue_Flag = 0; }
}

/**
 * @brief	ADC1 interrupt handler sub-routine
 * @return	Nothing
 */
void ADC1_IRQHandler(void)
{
	uint16_t dataADC;
	/* Interrupt mode: Call the stream interrupt handler */
	NVIC_DisableIRQ(ADC1_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC1, ADC_CH1, DISABLE);
	Chip_ADC_ReadValue(LPC_ADC1, ADC_CH1, &dataADC);
	ADC_Interrupt_Done_Flag = 1;
	WriteInt(dataADC);
	if (Leer_intUART() != 'x') {
		NVIC_EnableIRQ(ADC1_IRQn);
		Chip_ADC_Int_SetChannelCmd(LPC_ADC1, ADC_CH1, ENABLE);
	}
	else {Interrupt_Continue_Flag = 0; }
}

/**
 * @brief	DMA interrupt handler sub-routine
 * @return	Nothing
 */
void DMA_IRQHandler(void)
{
	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChannelNum) == SUCCESS) {
		channelTC++;
	}
	else {
		/* Process error here */
	}
	Led_Toggle(GREEN_LED);
}

/**
 * @brief	Main routine for ADC example
 * @return	Nothing
 */
int main(void)
{
	bool end_Flag = FALSE;
	uint32_t _bitRate = 100000;
	ADC_RESOLUTION_T _bitAccuracy = ADC_10BITS;
	uint8_t bufferUART;
	SystemCoreClockUpdate();
	Init_Leds();
	IniciarUart();

	//Board_Init();
	//Board_ADC_Init();
	/*ADC Init */
	Chip_ADC_Init(LPC_ADC0, &ADCSetup);
	Chip_ADC_EnableChannel(LPC_ADC0, ADC_CH1, ENABLE);

	while (!end_Flag) {

		while (!end_Flag) {
			bufferUART = 0xFF;
			bufferUART = uartReadByte();
			if (bufferUART == 'c') {

				bufferUART = 0xFF;
				while (bufferUART == 0xFF) {
					bufferUART = uartReadByte();
					if ((bufferUART != '1') && (bufferUART != '2') && (bufferUART != '3')) {
						bufferUART = 0xFF;
					}
				}
				switch (bufferUART) {
				case '1':		/* Polling Mode */
					App_Polling_Test();
					break;

				case '2':		/* Interrupt Mode */
					App_Interrupt_Test();
					break;

				case '3':		/* DMA mode */
					App_DMA_Test();

					break;
				}
				break;
			}
			else if (bufferUART == 'x') {
				end_Flag = TRUE;
				//DEBUGOUT("\r\nADC demo terminated!");
			}
			else if (bufferUART == 'o') {
				_bitRate -= _bitRate > 0 ? 1000 : 0;
				Chip_ADC_SetSampleRate(LPC_ADC0, &ADCSetup, _bitRate);
				//DEBUGOUT("Rate : %d Sample/s - Accuracy : %d bit \r\n", _bitRate, 10 - _bitAccuracy);
			}
			else if (bufferUART == 'p') {
				_bitRate += _bitRate < 400000 ? 1000 : 0;
				Chip_ADC_SetSampleRate(LPC_ADC0, &ADCSetup, _bitRate);
				//DEBUGOUT("Rate : %d Sample/s - Accuracy : %d bit \r\n", _bitRate, 10 - _bitAccuracy);
			}
			else if (bufferUART == 'k') {
				_bitAccuracy += _bitAccuracy < ADC_3BITS ? 1 : 0;
				Chip_ADC_SetResolution(LPC_ADC0, &ADCSetup, _bitAccuracy);
				//DEBUGOUT("Rate : %d Sample/s - Accuracy : %d bit \r\n", _bitRate, 10 - _bitAccuracy);
			}
			else if (bufferUART == 'l') {
				_bitAccuracy -= _bitAccuracy > 0 ? 1 : 0;
				Chip_ADC_SetResolution(LPC_ADC0, &ADCSetup, _bitAccuracy);
				//DEBUGOUT("Rate : %d Sample/s - Accuracy : %d bit \r\n", _bitRate, 10 - _bitAccuracy);
			}
			else if (bufferUART == 'b') {
				Burst_Mode_Flag = !Burst_Mode_Flag;
				ADCSetup.burstMode = Burst_Mode_Flag;
				Chip_ADC_SetSampleRate(LPC_ADC0, &ADCSetup, _bitRate);
				if (Burst_Mode_Flag) {
					//DEBUGOUT("Burst Mode ENABLED\r\n");
				}
				else {
					//DEBUGOUT("Burst Mode DISABLED\r\n");
				}
			}
		}
	}
	return 0;
}
