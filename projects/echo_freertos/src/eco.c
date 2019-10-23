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
#include "eco.h"       /* <= own header */
#include "task.h"
#include "event_groups.h"
#include "string.h"

/*==================[macros and definitions]=================================*/

#define BIT_EVENTO1	( 1 << 0 )
#define BIT_EVENTO2	( 1 << 1 )
#define BIT_EVENTO3 ( 1 << 2 )
#define DELAY 100

typedef struct{
	uint16_t delay;
	uint8_t led;
} blinking_t; /*No olvidar el ;*/

/*debo definir 2 colas para recibir y transmitir al mismo tiempo rx y tx
 * recibir texto debe recibir el tamaño habilita la int
 * como tengo la misma recepcion tengo que saber cuando estoy transmitiendo y cuando
 * recibo*/
/*Debo definir recibir_caracter y recibir_texto
 * recibir caracter debe iniciar la interrupcion y recibir texto debe fijarse */

typedef struct{
	const char * datos;
	uint8_t cantidad;
	uint8_t enviados;
}cola_t;

typedef struct{
	const char * datos;
	uint8_t cantidad;
	uint8_t enviados;
}cola_r;


EventGroupHandle_t xEventGroup;


/*==================[internal data declaration]==============================*/
cola_t cola;
/*==================[internal functions declaration]=========================*/

/*Elimino los lazos de delay*/


/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */

void ISR_Evento(void){

	Led_On(RGB_B_LED);
	if(EnviarCaracter()){

		xEventGroupSetBits(xEventGroup, BIT_EVENTO2);

	}


}

bool EnviarCaracter(void){

	/*Devuelve cuando el envío esta completo*/
	uint8_t eventos;
	bool completo = FALSE;

		eventos = Chip_UART_ReadLineStatus(USB_UART);

		if(eventos & UART_LSR_THRE){
			Chip_UART_SendByte(USB_UART, cola.datos[cola.enviados]);
			cola.enviados++;

			if(cola.enviados == cola.cantidad){
				Chip_UART_IntDisable(USB_UART, UART_IER_THREINT);
				completo = TRUE;
			}
		}
		return (completo);

}

bool RecibirCaracter(void){

	/*Devuelve cuando el envío esta completo*/
	uint8_t eventos;
	bool completo = FALSE;

		eventos = Chip_UART_ReadLineStatus(USB_UART);

		if(eventos & UART_LSR_THRE){
			Chip_UART_SendByte(USB_UART, cola.datos[cola.enviados]);
			cola.enviados++;

			if(cola.enviados == cola.cantidad){
				Chip_UART_IntDisable(USB_UART, UART_IER_THREINT);
				completo = TRUE;
			}
		}
		return (completo);

}


bool EnviarTexto(const char * cadena){

	/*Devuelve si queda texto por enviar*/
	bool pendiente = FALSE;

		cola.datos = cadena;
		cola.cantidad = strlen(cadena);
		cola.enviados = 0;

		if(cola.cantidad){
			Chip_UART_SendByte(USB_UART, cola.datos[cola.enviados]);
			cola.enviados++;

			if(cola.enviados < cola.cantidad){
				/*interrupcion de transmision cuando vacia el buffer*/
				Chip_UART_IntEnable(USB_UART, UART_IER_THREINT);
				pendiente = TRUE;
			}
		}
		return (pendiente);


}

bool RecibirTexto(const char * cadena, uint8_t cantidad){

	/*Devuelve si queda texto por enviar*/
	bool pendiente = FALSE;

		cola.datos = cadena;
		cola.cantidad = strlen(cadena);
		cola.enviados = 0;

		if(cola.cantidad){
			Chip_UART_SendByte(USB_UART, cola.datos[cola.enviados]);
			cola.enviados++;

			if(cola.enviados < cola.cantidad){
				/*interrupcion de transmision cuando vacia el buffer*/
				Chip_UART_IntEnable(USB_UART, UART_IER_THREINT);
				pendiente = TRUE;
			}
		}
		return (pendiente);


}


void Enviar(void * parametro)
{
	static const char cadena1[] ="Hola\r\n";
	static const char cadena2[] ="Mundo\r\n";

	const EventBits_t xBitsToWaitFor = (BIT_EVENTO1 | BIT_EVENTO2);
	EventBits_t xEventGroupValue;

	while(1){

	/* Block to wait for event bits to become set within the event group. */
	xEventGroupValue = xEventGroupWaitBits( /* The event group to read. */
												xEventGroup,

												/* Bits to test. */
												xBitsToWaitFor,

												/* Clear bits on exit if the
												unblock condition is met. */
												pdTRUE,

												/* Don't wait for all bits. */
												pdFALSE,

												/* Don't time out. */
												portMAX_DELAY );

		if( ( xEventGroupValue & BIT_EVENTO1 ) != 0 )
		{
			Led_Toggle(YELLOW_LED);
			if(EnviarTexto(cadena1)){
				xEventGroupValue = xEventGroupWaitBits(xEventGroup,xBitsToWaitFor,pdTRUE,pdFALSE,portMAX_DELAY );

				if(( xEventGroupValue & BIT_EVENTO2 ) != 0 ){
					EnviarTexto(cadena2);
				}
			}

		}

	}


}

void Recibir(void * parametro)
{
	char cadena[20];

	const EventBits_t xBitsToWaitFor = (BIT_EVENTO1 | BIT_EVENTO2);
	EventBits_t xEventGroupValue;

	while(1){

	/* Block to wait for event bits to become set within the event group. */
	xEventGroupValue = xEventGroupWaitBits( /* The event group to read. */
												xEventGroup,

												/* Bits to test. */
												xBitsToWaitFor,

												/* Clear bits on exit if the
												unblock condition is met. */
												pdTRUE,

												/* Don't wait for all bits. */
												pdFALSE,

												/* Don't time out. */
												portMAX_DELAY );

		if( ( xEventGroupValue & BIT_EVENTO1 ) != 0 )
		{
			Led_Toggle(YELLOW_LED);
			if(EnviarTexto(cadena1)){
				xEventGroupValue = xEventGroupWaitBits(xEventGroup,xBitsToWaitFor,pdTRUE,pdFALSE,portMAX_DELAY );

				if(( xEventGroupValue & BIT_EVENTO2 ) != 0 ){
					EnviarTexto(cadena2);
				}
			}

		}

	}


}



void Teclado(void * parametro){

	static uint8_t anterior = 0;
	uint8_t tecla;


	while(1){
		tecla = Read_Switches();
		if(tecla != anterior){

			switch (tecla){
			case TEC1:
				xEventGroupSetBits(xEventGroup, BIT_EVENTO1);
				break;
			case TEC2:
				break;
			case TEC3:
				break;
			case TEC4:
				break;
			default:
				break;

			}
			anterior = tecla;
		}
		vTaskDelay(DELAY);
	}
}

int main(void)
{
	/*Inicializaciones*/
		TaskHandle_t tarea;
		Init_Leds();
		Init_Switches();
		Init_Uart_Ftdi();

		NVIC_EnableIRQ(26);

		xEventGroup = xEventGroupCreate();

	/*const para que ocupe flash(rom) y no ram*/
	/* */

	static const blinking_t valores[] ={
			{.delay = 500, .led = RED_LED},
			{.delay =1000, .led = YELLOW_LED}
	};


	/*Creo la tarea NULL es el puntero a los parámetros, el segundo es la variable donde
	 * me devuelve el id */

	xTaskCreate(Teclado, "Tecla", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(Enviar, "Enviar", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    /*es un for infinito*/
    for(;;);
    
	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

