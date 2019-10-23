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
      Led_Toggle(GREEN_LED);
   }
}

void Transmitir(void * parametros) {
   static const char cadena[] = "Hola Mundo\r\n";
   
   while(1) {

      while(xEventGroupWaitBits(eventos, EVENTO_INICIAR, 
         TRUE, FALSE, portMAX_DELAY) == 0);

      Led_On(YELLOW_LED);
      if (EnviarTexto(cadena)) {
         while(xEventGroupWaitBits(eventos, EVENTO_TRANSMITIDO, 
            TRUE, FALSE, portMAX_DELAY) == 0);
      }
      Led_Off(YELLOW_LED);
   }
}

void Recibir(void * parametros) {
   static const comando_t comandos[] = {
      { .comando = "rojo", .led = RGB_R_LED },
      { .comando = "azul", .led = RGB_B_LED },
      { .comando = "verde", .led = RGB_G_LED },
      { .comando = "amarillo", .led = YELLOW_LED },

   };

   char cadena[16];
   uint8_t indice;
   
   while(1) {
      if (RecibirTexto(cadena, sizeof(cadena))) {
         xEventGroupWaitBits(eventos, EVENTO_RECIBIDO, TRUE, FALSE, portMAX_DELAY);
      }
      Led_On(YELLOW_LED);
      for(indice = 0; indice < sizeof(comandos) / sizeof(comando_t); indice++) {
         if (strcmp(cadena, comandos[indice].comando) == 0) {
            Led_Toggle(comandos[indice].led);
         }
      }
      Led_Off(YELLOW_LED);
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
   if (eventos != NULL) {
      xTaskCreate(Teclado, "Teclado", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
      xTaskCreate(Transmitir, "Transmitir", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
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

