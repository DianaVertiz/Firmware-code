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
#include "contexto.h"       /* <= own header */

/*==================[macros and definitions]=================================*/
#define COUNT_DELAY 3000000

#define STACK_SIZE 256

 /*!número de tareas*/
#define TASK_COUNT 2

typedef uint8_t stack_t[STACK_SIZE];
/*==================[internal data declaration]==============================*/
/*!stack total de tareas*/

static stack_t stack[TASK_COUNT];

static uint32_t context[TASK_COUNT + 1];

/*==================[internal functions declaration]=========================*/

void Delay(void)
{
	/** \details Basic delay function to obtain a delay between flashing lights */
	uint32_t i;

	for(i=COUNT_DELAY; i!=0; i--)
	{
		   asm  ("nop");
	}
}


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
 *
 */
void CambioContexto(void){
	/*!variable global*/
	static int activa = TASK_COUNT;
	asm("push {r0-r6,r8-r12}");
	asm("str r13, %0": "=m" (context[activa]));
	asm("ldr r13, %0": : "m"(context[TASK_COUNT]));

	/*!Pongo activa la tarea*/
	activa = (activa +1) % TASK_COUNT;

	/*!El SO cambia el estado del LED VERDE*/
	Led_Toggle(GREEN_LED);

	/*!Carga contexto de la tarea activa*/
	asm("str r13, %0": "=m" (context[TASK_COUNT]));
	asm("ldr r13, %0": : "m" (context[activa]));
	asm("pop {r0-r6,r8-r12}");
}


void Tarea1(void){

	while(1) {
	Led_Toggle(RED_LED);
	Delay();
	CambioContexto();
	}
}

void Tarea2(){

	while(1){
		if(TEC2 == Read_Switches()){

			Led_On(YELLOW_LED);
			Delay();
			Led_Off(YELLOW_LED);
		}
		CambioContexto();
	}
}


int main(void)
{
	/*!Estructura para definir el contenido de la pila para iniciar la tarea*/
	struct{
		struct{
			uint32_t r0;
			uint32_t r1;
			uint32_t r2;
			uint32_t r3;
			uint32_t r4;
			uint32_t r5;
			uint32_t r6;
			uint32_t r8;
			uint32_t r9;
			uint32_t r10;
			uint32_t r11;
			uint32_t ip;
		}context;
		struct{
			uint32_t r7;
			uint32_t lr;
		}subroutine;
	} frame_call;

	/*!Puntero a la pila*/
	void * pointer = stack;

	memset(stack, 0, sizeof(stack));
	memset(&frame_call, 0, sizeof(frame_call));

	pointer += (STACK_SIZE);
	frame_call.subroutine.r7 = (uint32_t) (pointer);
	frame_call.subroutine.lr = (uint32_t) Tarea1;
	memcpy(pointer - sizeof(frame_call), &frame_call, sizeof(frame_call));
	context[0] = (uint32_t) (pointer - sizeof(frame_call));

	pointer += (STACK_SIZE);
	frame_call.subroutine.r7 = (uint32_t) (pointer);
	frame_call.subroutine.lr = (uint32_t) Tarea2;
	memcpy(pointer - sizeof(frame_call), &frame_call, sizeof(frame_call));
	context[1] = (uint32_t) (pointer - sizeof(frame_call));

	Init_Leds();
	Init_Switches();
	CambioContexto();

	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

