/* Copyright 2014, Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
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

/** \brief Blinking_echo example source file
 **
 ** This is a mini example of the CIAA Firmware.
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Blinking Blinking_echo example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 * PR           Pablo Ridolfi
 * JuCe         Juan Cecconi
 * GMuro        Gustavo Muro
 * ErPe         Eric Pernia
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20150603 v0.0.3   ErPe change uint8 type by uint8_t
 *                        in line 172
 * 20141019 v0.0.2   JuCe add printf in each task,
 *                        remove trailing spaces
 * 20140731 v0.0.1   PR   first functional version
 */

/*==================[inclusions]=============================================*/
#include "Eje3.h"         /* <= own header */

#include "os.h"               /* <= operating system header */
#include "led.h"



/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
/*Alarma Activada = 1*/
uint8_t FlagAlarmBlue = 1;
uint8_t FlagAlarmRed = 1;
uint8_t FlagAlarmYellow = 1;
uint8_t FlagAlarmGreen = 1;
static uint8_t anterior = 0;
uint8_t tecla;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/** \brief File descriptor for digital output ports
 *
 * Device path /dev/dio/out/0
 */
//static int32_t fd_out;

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
int main(void)
{
   /* Starts the operating system in the Application Mode 1 */
   /* This example has only one Application Mode */
   StartOS(Normal);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */
   return 0;
}

/** \brief Error Hook function
 *
 * This fucntion is called from the os if an os interface (API) returns an
 * error. Is for debugging proposes. If called this function triggers a
 * ShutdownOs which ends in a while(1).
 *
 * The values:
 *    OSErrorGetServiceId
 *    OSErrorGetParam1
 *    OSErrorGetParam2
 *    OSErrorGetParam3
 *    OSErrorGetRet
 *
 * will provide you the interface, the input parameters and the returned value.
 * For more details see the OSEK specification:
 * http://portal.osek-vdx.org/files/pdf/specs/os223.pdf
 *
 */
void ErrorHook(void)
{
  Led_On(RED_LED);
  ShutdownOS(0);
}

/** \brief Initial task
 *
 * This task is started automatically in the application mode 1.
 */
/*Led rojo 200ms amarillo 500ms verde 800ms azul 1100ms*/
TASK(Configuracion)
{

	Init_Leds();

   SetRelAlarm(ActivarBalizaRed, 20, 200);
   SetRelAlarm(ActivarBalizaGreen, 40, 800);
   SetRelAlarm(ActivarBalizaYellow, 60, 500);
   SetRelAlarm(ActivarBalizaBlue, 80, 1100);
   SetRelAlarm(ActivarTeclado,100,500);
   /* terminate task */
   TerminateTask();
}

/** \brief Periodic Task
 *
 * This task is started automatically every time that the alarm
 * ActivatePeriodicTask expires.
 *
 */
TASK(BalizaRed)
{
   Led_Toggle(RED_LED);
   /* terminate task */
   TerminateTask();
}

TASK(BalizaGreen)
{
   Led_Toggle(GREEN_LED);
   /* terminate task */
   TerminateTask();
}

TASK(BalizaYellow)
{
   Led_Toggle(YELLOW_LED);
   /* terminate task */
   TerminateTask();
}

TASK(BalizaBlue)
{
   Led_Toggle(RGB_B_LED);
   /* terminate task */
   TerminateTask();
}

TASK(Teclado)
{
	Init_Switches();

		tecla= Read_Switches();
		if(tecla!= anterior){

			if(TEC1 == Read_Switches()){
				if(FlagAlarmBlue){
					CancelAlarm(ActivarBalizaBlue);
					Led_Off(RGB_B_LED);
					FlagAlarmBlue=0;
				}
			else {
				SetRelAlarm(ActivarBalizaBlue, 80, 1100);
				FlagAlarmBlue=1;
			}
		}

		else if(TEC2 == Read_Switches()){

			if(FlagAlarmRed){
				CancelAlarm(ActivarBalizaRed);
				Led_Off(RED_LED);
				FlagAlarmRed=0;
			}
			else {
				SetRelAlarm(ActivarBalizaRed, 20, 200);
				FlagAlarmRed=1;

			}

		}
		else if(TEC3 == Read_Switches()){

			if(FlagAlarmYellow){
				CancelAlarm(ActivarBalizaYellow);
				Led_Off(YELLOW_LED);
				FlagAlarmYellow=0;
			}
			else {
				SetRelAlarm(ActivarBalizaYellow, 60, 500);
				FlagAlarmYellow=1;

			}

		}
		else if(TEC4 == Read_Switches()){
			if(FlagAlarmGreen){
				CancelAlarm(ActivarBalizaGreen);
				Led_Off(GREEN_LED);
				FlagAlarmGreen=0;
			}
			else {
				SetRelAlarm(ActivarBalizaGreen, 40, 800);
				FlagAlarmGreen=1;

			}

		}
		anterior = tecla;
	}


	TerminateTask();
}


/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

