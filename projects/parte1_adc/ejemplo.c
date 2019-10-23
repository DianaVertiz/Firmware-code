/*
 * ejemplo.c
 *
 *  Created on: 15/12/2017
 *      Author: root
 */
/* Copyright Year, Author
 *
 * This file is part of a project.
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

/** @brief Brief for this file.
 **
 **/

/** \addtogroup groupName Group Name
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * VG           Valentin Giovagnoli
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20160808 v0.0.1   VG first version
 */

/*==================[inclusions]=============================================*/

#include "main.h"

/*==================[macros and definitions]=================================*/

#define DMA_BUFFER_SIZE 1024
#define EMPTY  0
#define FULL   1
#define PONG_BUFF 0
#define PING_BUFF 1
#define SAMPLE_RATE  44100
#define TICKRATE_HZ (200) /* 1ms */

volatile uint32_t ticks;

/*==================[internal data declaration]==============================*/

// This is where we put ADC results
uint32_t Ping_buffer[DMA_BUFFER_SIZE];
uint32_t Pong_buffer[DMA_BUFFER_SIZE];
uint32_t dataADC [DMA_BUFFER_SIZE*5];

static DMA_TransferDescriptor_t Ping;
static DMA_TransferDescriptor_t Pong;
static DMA_TransferDescriptor_t templli;

static volatile int dmaBlockCount = 0;
int rx_proc_buffer;
uint32_t BufferFull = EMPTY;

static volatile uint8_t dmaChannelNum;

static volatile uint8_t Burst_Mode_Flag = 0;


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/


static void setupHardware(void)
{
#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "Off"
    Board_LED_Set(0, 0);
    Board_LED_Set(1, 0);
#endif
#endif
}

void Analog_Init(void){

   ADC_CLOCK_SETUP_T ADCSetup;
   uint32_t Bit_Rate = SAMPLE_RATE;

   Chip_ADC_Init(LPC_ADC0, &ADCSetup);
   Chip_ADC_EnableChannel(LPC_ADC0, ADC_CH2, ENABLE);
   Chip_ADC_SetSampleRate(LPC_ADC0, &ADCSetup, Bit_Rate);


}

void DMA_Init(void){

   Chip_GPDMA_Init(LPC_GPDMA);

   LPC_GPDMA->SYNC = 0x00;

   NVIC_DisableIRQ(DMA_IRQn);
   NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
   NVIC_EnableIRQ(DMA_IRQn);

   Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC_CH2, ENABLE);

   dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_ADC_0);

   if (Burst_Mode_Flag) {
      Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
   }

   if (!Burst_Mode_Flag) {
      Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
   }


}



void DMA_Transfer(void)
{

   Chip_GPDMA_PrepareDescriptor(LPC_GPDMA,
                                 &Ping,
                                 GPDMA_CONN_ADC_0,
                                 (uint32_t) Ping_buffer,
                                 DMA_BUFFER_SIZE,
                                 GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
                                 &Pong);

   Chip_GPDMA_PrepareDescriptor(LPC_GPDMA,
                                  &Pong,
                                  GPDMA_CONN_ADC_0,
                                  (uint32_t) Pong_buffer,
                                  DMA_BUFFER_SIZE,
                                  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
                                  &Ping);


    // Creo un LLI temporal
    templli.ctrl=Ping.ctrl;
    templli.lli=(uint32_t)&Pong;
    templli.src=GPDMA_CONN_ADC_0;
    templli.dst=Ping.dst;

    Chip_GPDMA_SGTransfer(LPC_GPDMA,
                            dmaChannelNum,
                            &templli,
                            GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA);
}


/*==================[external functions definition]==========================*/

/**
 * @brief   DMA interrupt handler sub-routine
 * @return  Nothing
 */
void DMA_IRQHandler(void)
{
   if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChannelNum) == SUCCESS) {
      dmaBlockCount++;
   }
   else {
      /* Process error here */
   }
}

// Handler de interrupcion de Systick
// Se ejecuta cada 1 ms
void SysTick_Handler(void)
{

   ticks++;

}

int main(void)
{
   setupHardware();

   SysTick_Config( SystemCoreClock / TICKRATE_HZ); // Generate interrupt every 10 ms

   Analog_Init();

   DMA_Init();

   DMA_Transfer();

   while (1) {

   }

}

/*==================[end of file]============================================*/



