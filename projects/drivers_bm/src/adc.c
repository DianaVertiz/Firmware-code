/* Copyright 2016, 
 * Leandro D. Medus
 * lmedus@bioingenieria.edu.ar
 * Eduardo Filomena
 * efilomena@bioingenieria.edu.ar
 * Juan Manuel Reta
 * jmrera@bioingenieria.edu.ar
 * Facultad de Ingeniería
 * Universidad Nacional de Entre Ríos
 * Argentina
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief Bare Metal driver for adc in the EDU-CIAA board.
 **
 **/

/*
 * Initials     Name
 * ---------------------------
 *	LM			Leandro Medus
 *  EF			Eduardo Filomena
 *  JMR			JuanManuel Reta
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20160422 v0.1 initials initial version leo
 * 20160807 v0.2 modifications and improvements made by Eduardo Filomena
 * 20160808 v0.3 modifications and improvements made by Juan Manuel Reta
 */

/*==================[inclusions]=============================================*/
#include "adc.h"


/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
void (*pIsrADC0)();

volatile uint8_t ADC_Interrupt_Done_Flag, channelTC, dmaChannelNum;
uint32_t DMAbuffer;
volatile uint8_t Burst_Mode_Flag = 0;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
 void ISR_ADC0(){

	pIsrADC0();
}
/*==================[external functions definition]==========================*/
/** \brief ADC Initialization method  */
uint8_t Init_Adc(void)
{

	/** \details
	 * This function initialize the ADC peripheral in the EDU-CIAA board,
	 * with the correct parameters with LPCOpen library. It uses CH1
	 *
	 * \param none
	 *
	 * \return uint8_t: TBD (to support errors in the init function)
	 * */
	static ADC_CLOCK_SETUP_T configADC;

	configADC.adcRate=1000;		/** max 409 KHz*/
	configADC.burstMode=DISABLE;
	configADC.bitsAccuracy=ADC_10BITS;

	Chip_ADC_Init(LPC_ADC0,&configADC);

	//Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH1,ENABLE);
	Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH2,ENABLE);
	//Chip_ADC_EnableChannel(LPC_ADC0,ADC_CH3,ENABLE);

	//Chip_ADC_SetSampleRate(LPC_ADC0, &configADC,ADC_MAX_SAMPLE_RATE);

	return TRUE;
}

/** \brief ADC Ch1 Acquisition method by pooling */
uint16_t Read_Adc_Value_Pooling(void)
{
	/** \details
	 * This function initialize the DAC peripheral in the EDU-CIAA board,
	 * with the correct parameters with LPCOpen methods.
	 *
	 * \param none
	 *
	 * \return uint8_t: TBD (to support errors in the init function)
	 * */
	uint16_t valueRead = 0 ;

	/** Start Acquisition */
	Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
	/** The pooling magic! */
	/*while (Chip_ADC_ReadStatus(LPC_ADC0, ADC_CH1, ADC_DR_DONE_STAT) != SET)
	{
		/** pooooliiinnggg maaagggicccc plif plif pluf pluf */
	//}
	/** Conversion complete, and value reading */
	//Chip_ADC_ReadValue(LPC_ADC0,ADC_CH1, &valueRead);

	while (Chip_ADC_ReadStatus(LPC_ADC0, ADC_CH2, ADC_DR_DONE_STAT) != SET)
		{
			/** pooooliiinnggg maaagggicccc plif plif pluf pluf */
		}

		/** Conversion complete, and value reading */
	Chip_ADC_ReadValue(LPC_ADC0,ADC_CH2, &valueRead);

	/*while (Chip_ADC_ReadStatus(LPC_ADC0, ADC_CH3, ADC_DR_DONE_STAT) != SET)
		{
			/** pooooliiinnggg maaagggicccc plif plif pluf pluf */
		//}
		/** Conversion complete, and value reading */
	/*Chip_ADC_ReadValue(LPC_ADC0,ADC_CH3, &valueRead);*/

	return valueRead;
}

/** Start Acquisition */
void Start_Adc(void){
  Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
	
}
uint16_t Read_Adc_Value(void){
  uint16_t data;
  //Chip_ADC_ReadValue(LPC_ADC0,ADC_CH1, &data);
  Chip_ADC_ReadValue(LPC_ADC0,ADC_CH2, &data);
  //Chip_ADC_ReadValue(LPC_ADC0,ADC_CH3, &data);
  return data;

  }
  
void Enable_Adc_Irq(void *pfunc){
	pIsrADC0=pfunc;
	/*Enable interrupt for ADC channel */

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0,ADC_CH1,ENABLE);
	/*Chip_ADC_Int_SetChannelCmd(LPC_ADC0,ADC_CH2,ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0,ADC_CH3,ENABLE);*/

	NVIC_EnableIRQ(ADC0_IRQn);
  }




/*
static void App_DMA_Test(void)
{
	uint16_t dataADC;


	/* Initialize GPDMA controller */
	/*Chip_GPDMA_Init(LPC_GPDMA);
	/* Setting GPDMA interrupt */
	/*NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, ENABLE);
	/* Get the free channel for DMA transfer */
	/*dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_ADC_0);
	/* Enable burst mode if any, the AD converter does repeated conversions
	   at the rate selected by the CLKS field in burst mode automatically */
	/*if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	}
	/* Get  adc value until get 'x' character */
	/*while (DEBUGIN() != 'x') {
		/* Start A/D conversion if not using burst mode */
		/*if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		channelTC = 0;
		Chip_GPDMA_Transfer(LPC_GPDMA, dmaChannelNum,GPDMA_CONN_ADC_0, (uint32_t) &DMAbuffer,
						  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
						  1);

		/* Waiting for reading ADC value completed */
/*		while (channelTC == 0) {}

		/* Get the ADC value fron Data register*/
		/*dataADC = ADC_DR_RESULT(DMAbuffer);
		App_print_ADC_value(dataADC);
	}
	/* Disable interrupts, release DMA channel */
	/*Chip_GPDMA_Stop(LPC_GPDMA, dmaChannelNum);
	NVIC_DisableIRQ(DMA_IRQn);
	/* Disable burst mode if any */
	/*if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(LPC_ADC0, DISABLE);
	}
}*/




/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
