/* Copyright 2017, XXXX
 *
 *  * All rights reserved.
 *
 * This file is part of CIAA Firmware.
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

/** \brief Blinking Bare Metal example source file
 **
 ** This is a mini example of the CIAA Firmware.
 **
 * @brief    ADC DMA example
 * @author   TDII - UTN FRH
 * @version  1.0
 * @date     June 2016
 * @details  Trigger an interrupt after completing N_SAMPLES A/D conversions
 *           on ADC0_CH1 analog pin using the DMA controller
 */
/*==================[inclusions]=============================================*/
#include "parte1_adc.h"       /* <= own header */
//#include "board.h"

/*==================[macros and definitions]=================================*/
#define COUNT_DELAY 3000000

#define N_SAMPLES   256

static volatile uint8_t dma_ch_adc;  /* There are 8 DMA channels available */
static volatile uint8_t Burst_Mode_Flag = 0;
static volatile channelTC;  /* TC: terminal count*/
static uint32_t dma_buffer;
static uint16_t adc_buffer[N_SAMPLES];
ADC_CLOCK_SETUP_T adc_setup;

 void initHardware(void)
{


    //Board_Init();
    /* ADC configuration */
    // Setup ADC0: 10-bit, 100kHz
    Chip_ADC_Init(LPC_ADC0, &adc_setup);
    //Chip_ADC_SetSampleRate(LPC_ADC0, &adc_setup, 100);
    Chip_ADC_EnableChannel(LPC_ADC0, ADC_CH1, ENABLE);
    //Chip_ADC_Channel_Enable_Cmd(LPC_ADC0, ADC_CH1, ENABLE);
	/* DMA controller configuration */
	Chip_GPDMA_Init(LPC_GPDMA);

	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	//NVIC_ClearPendingIRQ(DMA_IRQn);
	NVIC_EnableIRQ(DMA_IRQn);

    // Get a free channel for a ADC->Memory DMA transfer
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH1, ENABLE);
	dma_ch_adc = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_ADC_0);
	Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
}




/* In this example there is only one ADC->Memory DMA transfer */
void startDMATransfer(void)
{
	static uint16_t j = 0;
	channelTC = 0;

	while (Leer_intUART() != 'x') {


		Chip_GPDMA_Transfer(LPC_GPDMA, dma_ch_adc,
							  GPDMA_CONN_ADC_0,
							  (uint32_t) &dma_buffer,
							  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
							  1);

		/* Waiting for reading ADC value completed */
		while (channelTC == 0) {}

		/* Get the ADC value from Data register*/
		adc_buffer[j] = ADC_DR_RESULT(dma_buffer);
		WriteInt(adc_buffer[j]);
		uartWriteByte('\n');
		j++;
		if(j==N_SAMPLES){ j=0;}
	}

}

/* DMA interrupt triggered after N_SAMPLES conversions of ADC0_1 */
void DMA_IRQHandler(void)
{
	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dma_ch_adc) == SUCCESS) {
			channelTC++;
		}
		else {
			/* Process error here */
		}
		Led_Toggle(GREEN_LED);

}

void DisableInterrupt(void){
	Chip_GPDMA_Stop(LPC_GPDMA, dma_ch_adc);
	NVIC_DisableIRQ(DMA_IRQn);
}

void HabilitarInt(void){
	NVIC_DisableIRQ(ADC0_IRQn);
	/*Enable interrupt for ADC channel */

	Chip_ADC_Int_SetChannelCmd(LPC_ADC0,ADC_CH2,ENABLE);
	/*Chip_ADC_Int_SetChannelCmd(LPC_ADC0,ADC_CH2,ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0,ADC_CH3,ENABLE);*/

	NVIC_EnableIRQ(ADC0_IRQn);
	//Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);

  }

void Adc_Irq(void){
	uint16_t valueRead = 0 ;
	NVIC_DisableIRQ(ADC0_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, DISABLE);
	Chip_ADC_ReadValue(LPC_ADC0,ADC_CH2, &valueRead);
	NVIC_EnableIRQ(ADC0_IRQn);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, ENABLE);

}

int main(void)
{
   /* inicializaciones */
	//uint16_t datos;

	//Init_Leds();
	Init_Adc();
	//IniciarUart();
   /*acá va mi programa principal */
	//Start_Adc();
   /*
    while(1)
    {
    	/*datos = Read_Adc_Value_Pooling();

    	WriteInt(datos);
    	uartWriteByte('\n');*/
	//}

	//return 0;
	uint16_t datos;
	uint8_t bufferUART = 0;
	SystemCoreClockUpdate();
	//initHardware();
	Init_Leds();
	IniciarUart();
	HabilitarInt();

	while (1) {

		/*bufferUART = uartReadByte();
		if (bufferUART == 'c') {
			bufferUART=0;
			startDMATransfer();


		}*/
	datos= Read_Adc_Value_Pooling();

	}

	return 0;

}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

