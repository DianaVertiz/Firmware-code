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
#include "Eje4_2.h"         /* <= own header */
#include "os.h"               /* <= operating system header */
#include "led.h"



/*==================[macros and definitions]=================================*/
#define MAXVALUE 2000
#define MINVALUE 100
/*==================[internal data declaration]==============================*/
/*Alarma Activada = 1*/
uint8_t FlagAlarmBlue = 1;
uint8_t FlagAlarmRed = 0;
uint8_t FlagAlarmYellow = 0;
uint8_t FlagAlarmGreen = 0;

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

TASK(Configuracion)
{

	Init_Leds();

   /*SetRelAlarm(ActivarBalizaRed, 20, 200);
   SetRelAlarm(ActivarBalizaGreen, 40, 800);
   SetRelAlarm(ActivarBalizaYellow, 60, 1000);*/
   SetRelAlarm(ActivarBalizaBlue, 80, PasoActualBlue);
   SetRelAlarm(ActivarTeclado,100,500);
   /* terminate task */
   TerminateTask();
}

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

TASK(CambiaFreq)
{
	uint8_t LedAux= LedActual;

	switch(LedAux){

	case(RED_LED):
		PasoAnteriorRed = PasoActualRed;

		if(FlagAlarmRed ==1){
			CancelAlarm(ActivarBalizaRed);
			FlagAlarmRed =0;
		}
		/*Aumenta frecuencia*/
		if(FlagFreq == 1){

			if(PasoActualRed - paso <= MINVALUE){
				PasoActualRed = MAXVALUE;
			}

			else {
				PasoActualRed = PasoActualRed - paso;
			}
		}
		else{
		/*Disminuye frecuencia*/
			if(PasoActualRed + paso >= MAXVALUE){
				PasoActualRed = MINVALUE;
			}

			else {
				PasoActualRed = PasoActualRed + paso;
			}
		}

		SetRelAlarm(ActivarBalizaRed, 20, PasoActualRed);
		FlagAlarmRed =1;
		break;

	case(RGB_B_LED):
		PasoAnteriorBlue = PasoActualBlue;

		if(FlagAlarmBlue ==1){
			CancelAlarm(ActivarBalizaBlue);
			FlagAlarmBlue =0;
		}

		if(FlagFreq == 1){

			if(PasoActualBlue - paso <= MINVALUE){
				PasoActualBlue = MAXVALUE;
			}

			else {
				PasoActualBlue = PasoActualBlue - paso;
			}
		}
		else{

			if(PasoActualBlue + paso >= MAXVALUE){
				PasoActualBlue = MINVALUE;
			}

			else {
				PasoActualBlue = PasoActualBlue + paso;
			}
		}

		SetRelAlarm(ActivarBalizaBlue, 20, PasoActualBlue);
		FlagAlarmBlue=1;
		break;

	case(YELLOW_LED):
		PasoAnteriorYellow = PasoActualYellow;

		if(FlagAlarmYellow ==1){
			CancelAlarm(ActivarBalizaYellow);
			FlagAlarmYellow = 0;
		}

		if(FlagFreq ==1){

			if(PasoActualYellow - paso <= MINVALUE){
				PasoActualYellow = MAXVALUE;
			}

			else {
				PasoActualYellow = PasoActualYellow - paso;
			}
		}
		else{

			if(PasoActualYellow + paso >= MAXVALUE){
				PasoActualYellow = MINVALUE;
			}

			else {
				PasoActualYellow = PasoActualYellow + paso;
			}

		}

		SetRelAlarm(ActivarBalizaYellow, 20, PasoActualYellow);
		FlagAlarmYellow =1;
		break;

	case(GREEN_LED):
		PasoAnteriorGreen = PasoActualGreen;

		if(FlagAlarmGreen ==1){
			CancelAlarm(ActivarBalizaGreen);
			FlagAlarmGreen =0;
		}

		if(FlagFreq == 1){

			if(PasoActualGreen - paso <= MINVALUE){
				PasoActualGreen = MAXVALUE;
			}

			else {
				PasoActualGreen = PasoActualGreen - paso;
			}
		}
		else{

			if(PasoActualGreen + paso >= MAXVALUE){
				PasoActualGreen = MINVALUE;
			}

			else {
				PasoActualGreen = PasoActualGreen + paso;
			}
		}

		SetRelAlarm(ActivarBalizaGreen, 20, PasoActualGreen);
		FlagAlarmGreen =1;
		break;

	}


   TerminateTask();
}


TASK(ElegirLed)
{
	LedAnterior = LedActual;

	switch(LedAnterior){

	case(RED_LED):
			CancelAlarm(ActivarBalizaRed);
			Led_Off(RED_LED);
			FlagAlarmRed =0;
			break;
	case(RGB_B_LED):
			CancelAlarm(ActivarBalizaBlue);
			Led_Off(RGB_B_LED);
			FlagAlarmBlue =0;
			break;
	case(YELLOW_LED):
			CancelAlarm(ActivarBalizaYellow);
			Led_Off(YELLOW_LED);
			FlagAlarmYellow =0;
			break;
	case(GREEN_LED):
			CancelAlarm(ActivarBalizaGreen);
			Led_Off(GREEN_LED);
			FlagAlarmGreen =0;
			break;
	}

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
		if(FlagAlarmRed == 0){
			SetRelAlarm(ActivarBalizaRed, 20, PasoActualRed);
			FlagAlarmRed =1;
			}
   		break;

   	case(RGB_B_LED):
		if(FlagAlarmBlue == 0){
			SetRelAlarm(ActivarBalizaBlue, 20, PasoActualBlue);
			FlagAlarmBlue =1;
		}
   		break;

   	case(YELLOW_LED):

		if(FlagAlarmYellow == 0){
			SetRelAlarm(ActivarBalizaYellow, 20, PasoActualYellow);
			FlagAlarmYellow =1;
		}
   		break;

   	case(GREEN_LED):

		if(FlagAlarmGreen == 0){
			SetRelAlarm(ActivarBalizaGreen, 20, PasoActualGreen);
			FlagAlarmGreen =1;
		}
		break;
   	}

   TerminateTask();
}

TASK(Teclado)
{
	Init_Switches();

	if(TEC1 == Read_Switches()){

		Der_Izq = 0;
		ActivateTask(ElegirLed);
	}
	else if(TEC2 == Read_Switches()){

		Der_Izq = 1;
		ActivateTask(ElegirLed);

	}
	else if(TEC3 == Read_Switches()){

		FlagFreq = 0;
		ActivateTask(CambiaFreq);
	}
	else if(TEC4 == Read_Switches()){

		FlagFreq = 1;
		ActivateTask(CambiaFreq);
	}

	TerminateTask();
}


/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

