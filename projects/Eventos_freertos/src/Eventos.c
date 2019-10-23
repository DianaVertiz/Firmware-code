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
#include "Eventos.h"       /* <= own header */
#include "task.h"


/*==================[macros and definitions]=================================*/
#define COUNT_DELAY 3000000

SemaphoreHandle_t xMutex;
typedef struct{
	uint16_t delay;
	uint8_t led;
} blinking_t; /*No olvidar el ;*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*Elimino los lazos de delay*/


/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/



/*Defino la tarea*/
void Blinking(void * parametros){

	blinking_t * valores = (blinking_t *)parametros;
	while(1)
	    {
/*portMax_DELAY sirve para esperar indefinidamente por el mutex, sino configuro un timeout*/
			xSemaphoreTake(xMutex, portMAX_DELAY );
			{
			/*taskENTER_CRITICAL();*/
			//{
				Led_On(valores->led);
				vTaskDelay(valores->delay);
				Led_Off(valores->led);


			//}
	    	/*taskEXIT_CRITICAL();*/
			}
			xSemaphoreGive(xMutex);
			vTaskDelay(valores->delay);
		}

}

int main(void)
{
	/*Inicializaciones*/
		/*TaskHandle_t tarea;*/
		Init_Leds();
		xMutex = xSemaphoreCreateMutex();

	static const blinking_t valores[] ={
			{.delay = 500, .led = RGB_R_LED},
			{.delay =1000, .led = RGB_B_LED},
			{.delay = 500, .led = YELLOW_LED}
		};
	/*Creo la tarea NULL es el puntero a los parámetros, el segundo es la variable donde
	 * me devuelve el id */

	if( xMutex != NULL ){
	/*si utilizo espera indefinida portMAX_DELAY no es necesario preguntar si el mutex está liberado*/

	xTaskCreate(Blinking, "Rojo", configMINIMAL_STACK_SIZE, &valores[0], 1, NULL);
	xTaskCreate(Blinking, "Azul", configMINIMAL_STACK_SIZE, &valores[1], 1, NULL);
	xTaskCreate(Blinking, "Amarillo", configMINIMAL_STACK_SIZE, &valores[2], 1, NULL);
   /*arranco el sistema operativo */
    vTaskStartScheduler();
	}
    /*es un for infinito*/
    for(;;);
    
	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

