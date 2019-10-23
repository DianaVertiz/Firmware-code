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
#include "Eje4.h"       /* <= own header */
#include "task.h"

/*==================[macros and definitions]=================================*/
#define COUNT_DELAY 3000000
#define xTicksBlue 1100
#define delayTecla 250

/*FlagColor=1 Tarea de Led activa*/
uint8_t FlagBlue = 1;
uint8_t FlagRed = 0;
uint8_t FlagYellow = 0;
uint8_t FlagGreen = 0;
uint8_t primer_pulsado = 0;
uint8_t activo_elegir = 0;
/*uint8_t FlagTEC1 = 0;*/
uint8_t LedActual = 2;
uint8_t LedAnterior = 2;
/* Aumenta freq =1*/
uint8_t FlagFreq = 1;
/*Derecha =1*/
uint8_t Der_Izq = 1;

uint16_t PasoAnteriorBlue = 1000;
uint16_t PasoAnteriorRed = 1000;
uint16_t PasoAnteriorYellow = 1000;
uint16_t PasoAnteriorGreen = 1000;

uint16_t PasoActualBlue = 1000;
uint16_t PasoActualRed = 1000;
uint16_t PasoActualYellow = 1000;
uint16_t PasoActualGreen = 1000;
uint8_t paso = 250;

TaskHandle_t tareaAzul;
TaskHandle_t tareaRojo;
TaskHandle_t tareaAmarillo;
TaskHandle_t tareaVerde;
TaskHandle_t tareaElegir;
TaskHandle_t tareaCambiar;

static uint8_t anterior = 0;
uint8_t tecla=0;

typedef struct{
	uint16_t delay;
	uint8_t led;
} blinking_t; /*No olvidar el ;*/

static const blinking_t valores[] ={
				{.delay = 1100, .led = RGB_B_LED},
				{.delay = 200, .led = RED_LED},
				{.delay =500, .led = YELLOW_LED},
				{.delay = 800, .led = GREEN_LED}

};



void Blinking(void * parametros){

	blinking_t * valores = (blinking_t *)parametros;
	while(1) {
	    	Led_Toggle(valores->led);
	    	vTaskDelay(valores->delay);
		}

}


void ElegirLed(void * parametros){

	while(1){

		LedAnterior = LedActual;

		if(activo_elegir==1){

			if (Der_Izq == 1){
				if(LedActual >= GREEN_LED){
					LedActual = RGB_B_LED;
				}
				else{
			   LedActual = LedActual + 1;
				}

			}
			else if(Der_Izq == 0){
				if(LedActual <= RGB_B_LED){
					LedActual =GREEN_LED;
				}
				else{
					LedActual = LedActual - 1;
				}
			}


			switch(LedActual){

				case(RED_LED):
					activo_elegir=0;
					if(FlagRed == 1){
						if(Der_Izq ==1){
							vTaskSuspend(tareaAzul);
							vTaskResume(tareaRojo);
							Led_Off(LedAnterior);
						}
						if(Der_Izq == 0){
							vTaskSuspend(tareaAmarillo);
							vTaskResume(tareaRojo);
							Led_Off(LedAnterior);
						}

					}
					if(FlagRed == 0){
						FlagRed = 1;
						if(Der_Izq == 1){
						vTaskSuspend(tareaAzul);
						}
						if(Der_Izq == 0){
						vTaskSuspend(tareaAmarillo);
						}
						Led_Off(LedAnterior);
						xTaskCreate(Blinking, "Rojo", configMINIMAL_STACK_SIZE, &valores[1], 1, &tareaRojo);
					}

					break;
				case(RGB_B_LED):
					activo_elegir=0;
					if(FlagBlue == 1){
						if(Der_Izq ==1){
							vTaskSuspend(tareaVerde);
							vTaskResume(tareaAzul);
							Led_Off(LedAnterior);
						}
						if(Der_Izq == 0){
							vTaskSuspend(tareaRojo);
							vTaskResume(tareaAzul);
							Led_Off(LedAnterior);
						}
					}

					break;

				case(YELLOW_LED):
					activo_elegir=0;
					if(FlagYellow == 1){
						if(Der_Izq ==1){
							vTaskSuspend(tareaRojo);
							vTaskResume(tareaAmarillo);
							Led_Off(LedAnterior);
						}
						if(Der_Izq == 0){
							vTaskSuspend(tareaVerde);
							vTaskResume(tareaAmarillo);
							Led_Off(LedAnterior);
						}
					}
					if(FlagYellow == 0){
						FlagYellow = 1;
						if(Der_Izq == 1){
						vTaskSuspend(tareaRojo);
						}
						if(Der_Izq == 0){
						vTaskSuspend(tareaVerde);
						}
						Led_Off(LedAnterior);
						xTaskCreate(Blinking, "Amarillo", configMINIMAL_STACK_SIZE, &valores[2], 1, &tareaAmarillo);
					}
					break;

				case(GREEN_LED):
					activo_elegir=0;
					if(FlagGreen == 1){
						if(Der_Izq ==1){
							vTaskSuspend(tareaAmarillo);
							vTaskResume(tareaVerde);
							Led_Off(LedAnterior);
						}
						if(Der_Izq == 0){
							vTaskSuspend(tareaAzul);
							vTaskResume(tareaVerde);
							Led_Off(LedAnterior);
						}
					}
					if(FlagGreen == 0){
						FlagGreen = 1;
						if(Der_Izq == 1){
						vTaskSuspend(tareaAmarillo);
						}
						if(Der_Izq == 0){
						vTaskSuspend(tareaAzul);
						}
						Led_Off(LedAnterior);
						xTaskCreate(Blinking, "Verde", configMINIMAL_STACK_SIZE, &valores[3], 1, &tareaVerde);
					}
					break;
			}

		}
	}

}

//void CambiarFrecuencia(void * parametros){



//}

void Teclado(void * parametros){



	while(1){

	tecla= Read_Switches();
	if(tecla!= anterior){


		if(TEC1 == Read_Switches()){

			Der_Izq = 0;
			activo_elegir = 1;

			if(primer_pulsado == 0){
				primer_pulsado = 1;
				xTaskCreate(ElegirLed, "Elegir", configMINIMAL_STACK_SIZE, NULL, 1, &tareaElegir);
			}

		}
		else if(TEC2 == Read_Switches()){

			Der_Izq = 1;
			activo_elegir = 1;

			if(primer_pulsado == 0){
				primer_pulsado = 1;
				xTaskCreate(ElegirLed, "Elegir", configMINIMAL_STACK_SIZE, NULL, 1, &tareaElegir);
			}


		}
		else if(TEC3 == Read_Switches()){

			FlagFreq = 0;
		}
		else if(TEC4 == Read_Switches()){

			FlagFreq = 1;
		}

		anterior = tecla;
	}

	vTaskDelay(delayTecla);
	}
}


int main(void)
{
	/*Inicializaciones*/
		Init_Leds();
		Init_Switches();


	xTaskCreate(Blinking, "Azul", configMINIMAL_STACK_SIZE, &valores[0], 1, &tareaAzul);
	xTaskCreate(Teclado, "Teclado", configMINIMAL_STACK_SIZE, NULL, 1, NULL);


	/*arranco el sistema operativo */
    vTaskStartScheduler();

    /*es un for infinito*/
    for(;;);
    
	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

