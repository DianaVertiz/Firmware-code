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
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */

/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Baremetal Bare Metal example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * yyyymmdd v0.0.1 initials initial version
 */

/*==================[inclusions]=============================================*/
#include "serial.h"       /* <= own header */
#include "task.h"
#include "event_groups.h"
#include "stdint.h"
#include "led.h"
#include "switch.h"
#include "uart.h"
#include <string.h>

/*==================[macros and definitions]=================================*/

#define EVENTO_INICIAR        ( 1 << 0)
#define EVENTO_TRANSMITIDO    ( 1 << 1)
#define EVENTO_RECIBIDO       ( 1 << 2)

static const char *WelcomeMenu = "\r\nHello \r\n"
							"ADC DEMO \r\n"
							"Press \'i\' to initiate or \'x\' to stop\r\n"
							"Press \'s\' to set Sample rate\r\n"
							"Press \'n\' to select number of Channels\r\n"
							"Press \'p\' to select processing type\r\n";

static const char *SelectMenuRate = "\r\nPress number 1-4 to set ADC sample rate:\r\n"
						   " 1: 100Hz \r\n"
						   " 2: 250Hz \r\n"
						   " 3: 500Hz \r\n"
						   " 4: 1KHz \r\n";

static const char *SelectMenuProcess = "\r\nPress number 1-3 to select processing type:\r\n"
						   " 1: Average \r\n"
						   " 2: Maximum \r\n"
						   " 3: Minimum \r\n";

/*Frecuencia de muestreo 100Hz -> 10ms
 * 						 250Hz -> 4ms
 * 						 500Hz -> 2ms
 * 						1000Hz -> 1ms*/


typedef struct {
   struct {
      const char * datos;
      uint8_t cantidad;
      uint8_t enviados;
   } tx;
   struct {
      char * datos;
      uint8_t cantidad;
      uint8_t recibidos;
   } rx;
} cola_t;

typedef struct {
   char * comando;
   uint8_t led;
} comando_t;

/*flags de comando activo*/
static volatile uint8_t empiezaAdquisicion = 0;
static volatile uint8_t terminaAdquisicion = 0;
static volatile uint8_t frecMuestreo = 0;
static volatile uint8_t numCanales = 0;
static volatile uint8_t f_procesamiento = 0;
static volatile uint8_t opcion = 0;
uint8_t SampleRate = 10; /*por defecto frecuencia de muestreo 100Hz*/
volatile uint8_t procesamiento = 1;
/*==================[internal data declaration]==============================*/

cola_t cola;

EventGroupHandle_t eventos;

/*==================[internal functions declaration]=========================*/

bool EnviarCaracter(void);

bool EnviarTexto(const char * cadena);

bool RecibirCaracter(void);

bool RecibirTexto(char * cadena, uint8_t espacio);

void Blinking(void * parametros);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

uint8_t ModificarSampleRate(uint8_t opcion){

	switch(opcion){
		case 1:frecMuestreo = 0; return 10; break;
		case 2:frecMuestreo = 0; return 4; break;
		case 3:frecMuestreo = 0; return 2; break;
		case 4:frecMuestreo = 0; return 1; break;

	}

}
void menu(char * cadena){

static const comando_t comandos[] = {
	 { .comando = "i", .led = RGB_R_LED },
	 { .comando = "x", .led = RGB_G_LED },
	 { .comando = "s", .led = RED_LED },
	 { .comando = "n", .led = YELLOW_LED },
	 { .comando = "p", .led = GREEN_LED },
};

 uint8_t indice;
 uint8_t opcion;
 opcion = (uint8_t)cadena[0] - 48;
/*si la adquisición empezó y pulso otra tecla distinta de x, no debe hacer nada*/
      if(empiezaAdquisicion == 0){

      	  if (strcmp(cadena, comandos[0].comando) == 0){
      		EnviarTexto("\r\nADC acquisition started :) ");
      		empiezaAdquisicion = 1;
      		terminaAdquisicion = 0;
      	   }
      	  if (strcmp(cadena, comandos[2].comando) == 0){
      		EnviarTexto(SelectMenuRate);
      		frecMuestreo = 1;
      	  }
      	  if (strcmp(cadena, comandos[3].comando) == 0){
      		EnviarTexto("\r\nPress number 1-3 to set number of activated ADC channels:\r\n");
      		numCanales = 1;
      	  }
      	  if (strcmp(cadena, comandos[4].comando) == 0){
        	EnviarTexto(SelectMenuProcess);
        	f_procesamiento = 1;
      	  }
      }
      else{
    	  if (strcmp(cadena, comandos[1].comando) == 0){
    	     EnviarTexto("\r\nADC acquisition terminated :( ");
    	     terminaAdquisicion = 1;
    	     empiezaAdquisicion = 0;
    	   }
      }

      switch(opcion){

      case 1: if(frecMuestreo == 1){
    	  	  	  SampleRate =ModificarSampleRate(1);
      	  	  }
      	  	  if(numCanales == 1){
      	  		Led_On(RGB_R_LED);
      	  		numCanales = 0;
      	  	  }
      	  	  if(f_procesamiento == 1){
      	  		  procesamiento = 1;
      	  		  f_procesamiento = 0;
      	  	  }
      	  	  break;

      case 2: if(frecMuestreo == 1){
    	  	  	  SampleRate =ModificarSampleRate(2);
      	  	  }
      	  	  if(numCanales == 1){
      	  		Led_On(RED_LED);
      	  		numCanales = 0;
      	  	  }
      	  	  if(f_procesamiento == 1){
      	  		  procesamiento = 2;
      	  		  f_procesamiento = 0;
      	  	  }
      	  	  break;

      case 3: if(frecMuestreo == 1){
    	  	  	  SampleRate =ModificarSampleRate(3);
      	  	  }
      	  	  if(numCanales == 1){
      	  		Led_On(YELLOW_LED);
      	  		numCanales = 0;
      	  	  }
      	  	  if(f_procesamiento == 1){
      	  		procesamiento = 3;
      	  		f_procesamiento = 0;
      	  	  }
      	  	  break;

      case 4: if(frecMuestreo == 1){
    	  	  	  SampleRate =ModificarSampleRate(4);
      	  	  }
      	  	  break;


      }

}

bool EnviarCaracter(void) {
   uint8_t eventos;
   uint8_t habilitados;
   bool completo = FALSE;

   eventos = Chip_UART_ReadLineStatus(USB_UART);
   habilitados = Chip_UART_GetIntsEnabled(USB_UART);
/*el Tx está vacío y está habilitada la interrupción de Tx*/
   if ((eventos & UART_LSR_THRE) && (habilitados & UART_IER_THREINT)) {
      Chip_UART_SendByte(USB_UART, cola.tx.datos[cola.tx.enviados]);
      cola.tx.enviados++;

      if (cola.tx.enviados == cola.tx.cantidad) {
         Chip_UART_IntDisable(USB_UART, UART_IER_THREINT);
         completo = TRUE;
      }
   }
   return (completo);
}

bool EnviarTexto(const char * cadena) {
   bool pendiente = FALSE;

   cola.tx.datos = cadena;
   cola.tx.cantidad = strlen(cadena);
   cola.tx.enviados = 0;

   if (cola.tx.cantidad) {
      Chip_UART_SendByte(USB_UART, cola.tx.datos[cola.tx.enviados]);
      cola.tx.enviados++;

      if (cola.tx.enviados < cola.tx.cantidad) {
         Chip_UART_IntEnable(USB_UART, UART_IER_THREINT);
         pendiente = TRUE;
      }
   }
   return (pendiente);
}

bool RecibirCaracter(void) {
   uint8_t eventos;
   uint8_t habilitados;
   char caracter;
   bool completo = FALSE;

   eventos = Chip_UART_ReadLineStatus(USB_UART);
   habilitados = Chip_UART_GetIntsEnabled(USB_UART);

   if ((eventos & UART_LSR_RDR) && (habilitados & UART_LSR_RDR)) {
      caracter = Chip_UART_ReadByte(USB_UART);
      if ((caracter != 13) && (caracter != 10)) {
         cola.rx.datos[cola.rx.recibidos] = caracter;
         cola.rx.recibidos++;
         completo = (cola.rx.recibidos == cola.rx.cantidad);
      } else {
         cola.rx.datos[cola.rx.recibidos] = 0;
         cola.rx.recibidos++;
         completo = TRUE;
      }

      if (completo) {
         Chip_UART_IntDisable(USB_UART, UART_LSR_RDR);         
      }
   }
   return (completo);
}

bool RecibirTexto(char * cadena, uint8_t espacio) {
   bool pendiente = TRUE;

   cola.rx.datos = cadena;
   cola.rx.cantidad = espacio;
   cola.rx.recibidos = 0;

   Chip_UART_IntEnable(USB_UART, UART_LSR_RDR);

   return (pendiente);
}

/*---------------------------------------------------------------------------*/
void Teclado(void * parametros) {
   uint8_t tecla;
   uint8_t anterior = 0;

   while(1) {
      tecla = Read_Switches();
      if (tecla != anterior) {
         switch(tecla) {
            case TEC1:
                  xEventGroupSetBits(eventos, EVENTO_INICIAR);
               break;
            case TEC2:
               break;
            case TEC3:
               break;
            case TEC4:
               break;
         }
         anterior = tecla;
      }
      vTaskDelay(100);
      //Led_Toggle(GREEN_LED);
   }
}

void Transmitir(void * parametros) {
   static const char cadena[] = "Canal";
   
   while(1) {

      while(xEventGroupWaitBits(eventos, EVENTO_INICIAR, 
         TRUE, FALSE, portMAX_DELAY) == 0);

      //Led_On(YELLOW_LED);
      if (EnviarTexto(cadena)) {
         while(xEventGroupWaitBits(eventos, EVENTO_TRANSMITIDO, 
            TRUE, FALSE, portMAX_DELAY) == 0);
      }
      //Led_Off(YELLOW_LED);
   }
}

void Recibir(void * parametros) {

   char cadena[16];

   while(1) {

      if (RecibirTexto(cadena, sizeof(cadena))) {
         xEventGroupWaitBits(eventos, EVENTO_RECIBIDO, TRUE, FALSE, portMAX_DELAY);
      }

      menu(cadena);

   }
}
/*==================[external functions definition]==========================*/

void EventoSerial(void) {
   if (EnviarCaracter()) {
      xEventGroupSetBits(eventos, EVENTO_TRANSMITIDO);
   };
   if (RecibirCaracter()) {
      xEventGroupSetBits(eventos, EVENTO_RECIBIDO);
   };
}

/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */

int main(void) {

   /* inicializaciones */
   Init_Leds();
   Init_Switches();
   Init_Uart_Ftdi();
   NVIC_EnableIRQ(26);
   eventos = xEventGroupCreate();
   EnviarTexto(WelcomeMenu);

   if (eventos != NULL) {
      xTaskCreate(Teclado, "Teclado", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
      //xTaskCreate(Transmitir, "Transmitir", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
      xTaskCreate(Recibir, "Recibir", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
      vTaskStartScheduler();
   }

   for(;;);
	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

