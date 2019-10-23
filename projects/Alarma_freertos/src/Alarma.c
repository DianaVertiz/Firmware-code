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
#include "Alarma.h"       /* <= own header */
#include "task.h"
#include "event_groups.h"
#include "portmacro.h"
#include "timers.h"

/*==================[macros and definitions]=================================*/
/*Bits de desarme*/
#define BIT_EVENTO_SENSOR	( 1 << 0 )
#define BIT_EVENTO_CLAVE	( 1 << 1 )
#define BIT_EVENTO_TECLADO ( 1 << 2 )
#define BIT_CLAVE_ERROR ( 1 << 3 )
#define INTENTOS 3
#define DELAY 100

uint8_t PressedTEC1 = 0;/*TEC1 no esta pulsada*/
uint8_t PressedTEC2 = 0;
uint8_t PressedTEC3 = 0;
uint8_t PressedTEC4 = 0;

bool clave_correcta=FALSE;

TaskHandle_t sensor;
TaskHandle_t teclado;
TaskHandle_t ingreso;
EventGroupHandle_t xEventGroup;
EventGroupHandle_t xEventGroup2;
EventGroupHandle_t xEventGroup3;


typedef struct{
	uint16_t delay;
	uint8_t led;
} blinking_t; /*No olvidar el ;*/


void Ingreso(void * parametros){
/*Debe verificar si el código ingresado es correcto */
/*Debe recibir 3 valores y confirma en cada pulsado */
/*recibe una señal desde teclado cada vez que se ingresa una tecla de la clave*/
/*Código: TEC3-TEC1-TEC2*/
static uint8_t ingresos=0;
static uint8_t primera=0;
static uint8_t segunda=0;
static uint8_t tercera=0;

const EventBits_t xBitsToWaitFor2 = BIT_EVENTO_TECLADO;
EventBits_t xEventGroupValue2;

	while(1){

		xEventGroupValue2 = xEventGroupWaitBits( xEventGroup2,
												xBitsToWaitFor2,
												pdTRUE,
												pdFALSE,
												portMAX_DELAY );
		/*si recibe una señal de ingreso desde el teclado*/
		if( ( xEventGroupValue2 & BIT_EVENTO_TECLADO ) != 0 ){

			ingresos++;

			if(ingresos==1){
				Led_Off(RGB_B_LED);
				vTaskDelay(DELAY);
				Led_On(RGB_B_LED);
				primera = TeclaPulsada();
			}
			if(ingresos==2){
				Led_Off(RGB_B_LED);
				vTaskDelay(DELAY);
				Led_On(RGB_B_LED);
				segunda = TeclaPulsada();
			}
			if(ingresos==3){

				Led_Off(RGB_B_LED);
				vTaskDelay(DELAY);
				Led_On(RGB_B_LED);
				tercera = TeclaPulsada();

				if( primera == TEC3 && segunda == TEC1 && tercera == TEC2 ){
					/*clave correcta!!!*/
					ingresos = 0;
					/*Debe desarmar alarma*/
					clave_correcta=TRUE;
				}
				else{
					ingresos = 0;
					clave_correcta=FALSE;
				}

				/*envío señal de ingreso de clave (3 teclas)*/
				xEventGroupSetBits(xEventGroup2, BIT_EVENTO_CLAVE);
			}

		}

	}


}

uint8_t TeclaPulsada(){

	uint8_t tecla;

	if(PressedTEC1){
		tecla=TEC1;
		PressedTEC1 = 0;
	}
	if(PressedTEC2){
		tecla=TEC2;
		PressedTEC2 = 0;
	}
	if(PressedTEC3){
		tecla=TEC3;
		PressedTEC3 = 0;
	}

	return tecla;
}


void Teclado(void * parametro){
/*Lee el teclado correspondiente a la clave: TEC1-TEC3 */
/*Ante cada tecla pulsada debe enviar una señal a Ingreso para verificar el código*/

static uint8_t anterior = 0;
uint8_t tecla;

	while(1){

		if(PressedTEC4 ==1){
			tecla = Read_Switches();
			if(tecla != anterior){

				if ((tecla==TEC1) | (tecla==TEC2) | (tecla==TEC3)){

					if(tecla==TEC1){
						PressedTEC1 = 1;
					}
					if(tecla==TEC2){
						PressedTEC2 = 1;
					}
					if(tecla==TEC3){
						PressedTEC3 = 1;
					}

					/*envía señal de ingreso de tecla*/
					xEventGroupSetBits(xEventGroup2, BIT_EVENTO_TECLADO);
				}

				anterior = tecla;
			}

			vTaskDelay(DELAY);
		}

	}

}

void Sensor(void * parametros){

/*cuando pulso TEC4 envía señal a Desarmar
 * inicia la tarea Teclado*/

static uint8_t anterior = 0;
uint8_t tecla;


	while(1){
		tecla = Read_Switches();
		if(tecla != anterior){

			if (tecla==TEC4){
				PressedTEC4 = 1;
				xEventGroupSetBits(xEventGroup3, BIT_EVENTO_SENSOR);
				xTaskCreate(Teclado, "teclado", configMINIMAL_STACK_SIZE, NULL, 1, &teclado);
			}
			anterior = tecla;
		}

		vTaskDelay(DELAY);
	}

}




void Desarmar(void * parametros){

/*Cuando recibe la señal del sensor inicia la secuencia de desarme*/
/*Recibe tambien señal de ingreso de clave desde la función ingreso*/

const EventBits_t xBitsToWaitForever = BIT_EVENTO_SENSOR|BIT_CLAVE_ERROR ;
EventBits_t xEventGroupValue3;

const EventBits_t xBitsToWaitFor =BIT_EVENTO_CLAVE ;
EventBits_t xEventGroupValue2;

/*tiempo de espera de 30 segundos*/
const TickType_t TimeToWait = 30000;

/*intentos de ingresar clave*/
uint8_t intentos = INTENTOS;

	while(1){

		/* Espera el evento proveniente del sensor */
		xEventGroupValue3 = xEventGroupWaitBits( xEventGroup3,
												xBitsToWaitForever,
												pdTRUE,
												pdFALSE,
												portMAX_DELAY );

		if( ( (xEventGroupValue3 & BIT_EVENTO_SENSOR ) != 0) | ( ( xEventGroupValue3 & BIT_CLAVE_ERROR ) != 0 ) )
		{
			/*Alerta ingreso detectado*/
			Led_On(RGB_B_LED);
			vTaskSuspend(sensor);

			/*Espera durante 30 segundos confirmación de ingreso de una clave sea correcta o no*/
			xEventGroupValue2 = xEventGroupWaitBits(xEventGroup2,xBitsToWaitFor,pdTRUE,pdFALSE,TimeToWait );

			if( ( xEventGroupValue2 & BIT_EVENTO_CLAVE ) != 0 ){
				/*si recibe señal de ingreso de una clave*/
				/*si la clave es correcta*/
				if(clave_correcta){
					Led_Off(RGB_B_LED);
					vTaskResume(sensor);
					PressedTEC4 =0;
					/*puedo reiniciar la alarma*/
					intentos = INTENTOS+1;
				}
				/*Si la clave es incorrecta*/
				else{
					Led_Off(RGB_B_LED);
					Led_On(RED_LED);
					vTaskDelay(DELAY);
					Led_Off(RED_LED);
					Led_On(RGB_B_LED);

					if (intentos == 1){
						Led_Off(RGB_B_LED);
						Led_On(RED_LED);
						vTaskSuspend(teclado);
						vTaskSuspend(ingreso);
					}
					else{
						xEventGroupSetBits(xEventGroup3, BIT_CLAVE_ERROR);
					}

				}
				intentos--;
			}

			/*se te acabó el tiempo de ingreso de clave*/
			else{
					Led_Off(RGB_B_LED);
					Led_On(RED_LED);
					vTaskSuspend(teclado);
					vTaskSuspend(ingreso);
			}

		}

	}

}

int main(void)
{
	/*Inicializaciones*/

	Init_Leds();
	Init_Switches();

	xEventGroup = xEventGroupCreate();
	xEventGroup2 = xEventGroupCreate();
	xEventGroup3 = xEventGroupCreate();

	xTaskCreate(Sensor, "sensor", configMINIMAL_STACK_SIZE, NULL, 1, &sensor);
	xTaskCreate(Desarmar, "desarma", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(Ingreso, "ingreso", configMINIMAL_STACK_SIZE, NULL, 1, &ingreso);

	/*arranco el sistema operativo */
 	 vTaskStartScheduler();

    for(;;);
    
	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

