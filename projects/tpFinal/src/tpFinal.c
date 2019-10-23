

/*==================[inclusions]=============================================*/
#include "tpFinal.h"

#include "task.h"
#include "event_groups.h"
#include "stdint.h"
#include "switch.h"
#include "uart.h"
#include "led.h"
#include "adc.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
/*Corresponden a la numeracion de * The channels on one ADC peripheral* en adc_18xx_43xx.h*/
typedef enum CANALES {CANAL0 = 0, CANAL1, CANAL2, CANAL3};
typedef enum PERIFERICOS {PER1 = 1, PER2};


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

typedef struct {
	 int16_t * datos;
	 uint8_t tamanio;
	 uint8_t recibidos;
}buffer;


typedef struct {
	uint8_t canal;
	uint8_t PERIFERICO;
	uint32_t PeripheralConnection_ID;
    int16_t * datos;
	uint16_t tamanio;
	uint16_t indice;
}ADCData;

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

cola_t cola;

#define N_SAMPLES   10
#define N_SAMPLES3   10

#define BIT_EVENTO_TECLADO ( 1 << 0 )
#define BIT_INICIO_CONVERSION (3 << 0)
#define BIT_EVENTO_CONVERTIDO (4 << 0)
#define EVENTO_INICIAR (2 << 0)
#define EVENTO_TRANSMITIDO (5 << 0)
#define EVENTO_RECIBIDO (6 << 0)

EventGroupHandle_t xEventTecladoGroup;
EventGroupHandle_t xEventBufferGroup;
EventGroupHandle_t eventos;
EventGroupHandle_t eventoSerial;
//EventGroupHandle_t xEventBufferConversion;
TaskHandle_t teclado;
TaskHandle_t recibir;
TaskHandle_t recibirADC;
TaskHandle_t transmitir;
SemaphoreHandle_t  xMutex;

static volatile uint8_t Burst_Mode_Flag = 0;
static volatile uint8_t dma_ch_adc;
static volatile uint8_t activoCH1=0;
static volatile uint8_t activoCH2=0;
static volatile uint8_t activoCH3=0;
static volatile uint8_t canal_activo=1;
static volatile uint8_t buffer_lleno=0;
//static ADCData ADC;
uint32_t DMAbuffer;
static uint16_t adc_buffer_ch1[N_SAMPLES];
static uint16_t adc_buffer_ch2[N_SAMPLES];
static uint16_t adc_buffer_ch3[N_SAMPLES3];
volatile uint8_t procesamiento = 2; /*1 promedio por defecto*/
volatile uint32_t resultado;
static uint16_t indice_ch1 = 0;
static uint16_t indice_ch2 = 0;
static uint16_t indice_ch3 = 0;

/*flags de comando activo*/
static volatile uint8_t empiezaAdquisicion = 0;
static volatile uint8_t terminaAdquisicion = 0;
static volatile uint8_t frecMuestreo = 0;
static volatile uint8_t numCanales = 0;
static volatile uint8_t f_procesamiento = 0;
static volatile uint8_t opcion = 0;
static volatile uint32_t SampleRate = 1000;
static char cadena[30];

//volatile uint8_t procesamiento = 1;


static const ADCData ValoresADC[] = {
		{.canal = CANAL1, .PERIFERICO = PER1, .PeripheralConnection_ID = GPDMA_CONN_ADC_0, .datos = adc_buffer_ch1, .tamanio = N_SAMPLES},
		{.canal = CANAL2, .PERIFERICO = PER1, .PeripheralConnection_ID = GPDMA_CONN_ADC_0, .datos = adc_buffer_ch2, .tamanio = N_SAMPLES},
		{.canal = CANAL3, .PERIFERICO = PER2, .PeripheralConnection_ID = GPDMA_CONN_ADC_1, .datos = adc_buffer_ch3, .tamanio = N_SAMPLES}
};


/*==================[internal functions declaration]=========================*/
bool EnviarCaracter(void);

bool EnviarTexto(const char *);

void Conversion(ADCData);

bool llenarBuffer(uint16_t *, uint16_t, uint16_t *);

void GuardarDatos(void);
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/**
 * C++ version 0.4 char* style "itoa":
 * Written by Luk�s Chmela
 * Released under GPLv3.
 */
char* itoa(uint16_t value, char* result, uint16_t base) {
   // check that the base is valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   uint16_t tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

uint8_t ModificarSampleRate(uint8_t opcion){

	switch(opcion){
		case 1:frecMuestreo = 0; return 10; break;
		case 2:frecMuestreo = 0; return 4; break;
		case 3:frecMuestreo = 0; return 2; break;
		case 4:frecMuestreo = 0; return 1; break;

	}

}

void Transmision(void) {

	static char aux1[1];
	static char aux2[1];
	static char aux3[1];
	static char proceso[]= "";

	if(procesamiento == 1){
		strcat(proceso,"Promedio: ");
	}
	else{
		if(procesamiento == 2){
			strcat(proceso,"Maximo: ");
		}
		else{
			strcat(proceso, "Minimo: ");
		}
	}
		itoa(canal_activo, aux1, 10);
		itoa((resultado/10), aux2, 10);
		itoa((resultado%10), aux3, 10);
		strcat(cadena,"Canal ");
		strcat(cadena,aux1);
		strcat(cadena, ": ");
		strcat(cadena, proceso);
		strcat(cadena, aux2);
		strcat(cadena, ",");
		strcat(cadena, aux3);
		strcat(cadena, "\r\n");
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
      		xEventGroupSetBits(xEventTecladoGroup, BIT_EVENTO_TECLADO);
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
    	     xEventGroupClearBits(xEventTecladoGroup, BIT_EVENTO_TECLADO);
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

void Conversion(ADCData ADC){

	/*Empiezo la conversión*/
	if (!Burst_Mode_Flag) {
		if(ADC.PERIFERICO == PER1){
			Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		else{
			Chip_ADC_SetStartMode(LPC_ADC1, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}

	}
	/* Get the free channel for DMA transfer */
	dma_ch_adc = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, ADC.PeripheralConnection_ID);


	Chip_GPDMA_Transfer(LPC_GPDMA, dma_ch_adc, ADC.PeripheralConnection_ID,
						 (uint32_t) &DMAbuffer,
						 GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,1);
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

void InitADC(ADCData ADC){

	ADC_CLOCK_SETUP_T configADC;

	if(ADC.PERIFERICO == PER1){
		Chip_ADC_Init(LPC_ADC0,&configADC);
		Chip_ADC_EnableChannel(LPC_ADC0,ADC.canal,ENABLE);
		Chip_ADC_SetSampleRate(LPC_ADC0, &configADC, 20000);
	}
	else{
		Chip_ADC_Init(LPC_ADC1,&configADC);
		Chip_ADC_EnableChannel(LPC_ADC1,ADC.canal,ENABLE);
		Chip_ADC_SetSampleRate(LPC_ADC1, &configADC, 20000);
	}
	/* Initialize GPDMA controller */
	Chip_GPDMA_Init(LPC_GPDMA);

	/* Setting GPDMA interrupt */
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);

	if(ADC.PERIFERICO == PER1){
		Chip_ADC_Int_SetChannelCmd(LPC_ADC0, ADC.canal, ENABLE);
	}
	else{
		Chip_ADC_Int_SetChannelCmd(LPC_ADC1, ADC.canal, ENABLE);
	}

	if(ADC.canal == CANAL1){activoCH1=1;Led_On(RED_LED); canal_activo = 1;}
	if(ADC.canal == CANAL2){activoCH2=1;Led_On(YELLOW_LED); canal_activo = 2;}
	if(ADC.canal == CANAL3){activoCH3=1;Led_On(GREEN_LED); canal_activo = 3;}

}

bool llenarBuffer(uint16_t * buffer, uint16_t tamanio, uint16_t * indice){

	bool completo = FALSE;

	buffer[*indice] = (uint16_t) ((ADC_DR_RESULT(DMAbuffer))*33)/1023;

	if(*indice == tamanio-1){
		completo = TRUE;
		*indice = 0;
	}
	else {
		(*indice)++;
	}
	return (completo);
}


uint16_t Procesamiento(uint16_t * buffer, uint16_t tamanio){

	uint8_t i;
	uint16_t mayor = 0;
	uint16_t menor = 33;
	uint32_t aux = 0;
	uint32_t promedio = 0;

	for(i=0; i<tamanio; i++){

		aux = buffer[i] + aux;
		if(buffer[i] > mayor){
			mayor = buffer[i];
		}
		if(buffer[i] < menor){
			menor = buffer[i];
		}
		buffer[i] = 0;
	}

	switch(procesamiento){

	case 1: promedio = aux/tamanio;
			return promedio;
			break;

	case 2: return mayor;
			break;

	case 3:	return menor;
			break;

	}

}

void GuardarDatos(void){

	buffer_lleno=0;
	if(activoCH1){
		if(llenarBuffer(ValoresADC[0].datos ,ValoresADC[0].tamanio, &indice_ch1)){
			buffer_lleno=1;
			/*procesar y vaciar el buffer*/
			resultado = Procesamiento(ValoresADC[0].datos, ValoresADC[0].tamanio);
			activoCH1 = 0;
			Transmision();
			Led_Off(RED_LED);

		}
	}

	else{

		if(activoCH2){
			if(llenarBuffer(ValoresADC[1].datos ,ValoresADC[1].tamanio, &indice_ch2)){
				/*procesar y vaciar el buffer*/
				resultado = Procesamiento(ValoresADC[1].datos, ValoresADC[1].tamanio);
				activoCH2 = 0;
				Led_Off(YELLOW_LED);
			}
		}
		else{
			if(llenarBuffer(ValoresADC[2].datos ,ValoresADC[2].tamanio, &indice_ch3)){
				/*procesar y vaciar el buffer*/
				resultado = Procesamiento(ValoresADC[2].datos, ValoresADC[2].tamanio);
				activoCH3 = 0;
				Led_Off(GREEN_LED);
			}
		}
	}

}

void RitTimer(uint32_t time_ms){
	Chip_RIT_Init(LPC_RITIMER);
	Chip_RIT_SetTimerInterval(LPC_RITIMER,time_ms);
	NVIC_EnableIRQ(RITIMER_IRQn);
}

/*==================[external functions definition]==========================*/

void Recibir(void * parametros) {

   char cadena[16];

   while(1) {

      if (RecibirTexto(cadena, sizeof(cadena))) {
         xEventGroupWaitBits(eventoSerial, EVENTO_RECIBIDO, TRUE, FALSE, portMAX_DELAY);
      }

      menu(cadena);

   }
}

void Transmitir(void * parametros) {
   //static const char cadena[] = "Hola Mundo\r\n";

   while(1) {

      while(xEventGroupWaitBits(eventoSerial, EVENTO_INICIAR,
         TRUE, FALSE, portMAX_DELAY) == 0);

      Led_On(YELLOW_LED);
      if (EnviarTexto(cadena)) {
         while(xEventGroupWaitBits(eventoSerial, EVENTO_TRANSMITIDO,
            TRUE, FALSE, portMAX_DELAY) == 0);
      }
      Led_Off(YELLOW_LED);

   }
}

void RecibirADC(void * parametros){

	ADCData * ADC = (ADCData *) parametros;
	const EventBits_t xBitsToWaitForTeclado = BIT_EVENTO_TECLADO;

	EventBits_t xEventGroupTecladoValue;
	//EventBits_t xEventGroupBufferValue;
	while(1){
		/*Espero evento de teclado para iniciar la conversión AD*/
		xEventGroupTecladoValue = xEventGroupWaitBits( xEventTecladoGroup,
												xBitsToWaitForTeclado,
												pdFALSE,
												pdFALSE,
												portMAX_DELAY );


		if(((xEventGroupTecladoValue & BIT_EVENTO_TECLADO) != 0)){

			RitTimer(SampleRate);
			while(xEventGroupWaitBits(eventos, BIT_INICIO_CONVERSION,pdTRUE, pdFALSE, portMAX_DELAY) == 0);
			InitADC(*(ADC));
			Conversion(*(ADC));
			while(xEventGroupWaitBits(eventos, BIT_EVENTO_CONVERTIDO,pdTRUE, pdFALSE, portMAX_DELAY) == 0);
			GuardarDatos();
			if(buffer_lleno == 1){
				vTaskPrioritySet( transmitir, ( tskIDLE_PRIORITY + 2) );
				xEventGroupSetBits(eventoSerial, EVENTO_INICIAR);
			}


			//RitTimer(1000);
		}

	}
}


void EventoSerial(void) {
   if (EnviarCaracter()) {
      xEventGroupSetBits(eventoSerial, EVENTO_TRANSMITIDO);
      vTaskPrioritySet( transmitir, ( tskIDLE_PRIORITY + 1 ) );
   };
   if (RecibirCaracter()) {
      xEventGroupSetBits(eventoSerial, EVENTO_RECIBIDO);
   };
}


void DMA_IRQHandler(void){

	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dma_ch_adc) == SUCCESS) {
		/*Dato convertido!*/
		NVIC_DisableIRQ(DMA_IRQn);

		xEventGroupSetBits(eventos, BIT_EVENTO_CONVERTIDO);

	}

}

void RIT_IRQHandler(void){
	Chip_RIT_ClearInt(LPC_RITIMER);

	NVIC_DisableIRQ(RITIMER_IRQn);
	xEventGroupSetBits(eventos, BIT_INICIO_CONVERSION);

}




void Teclado(void * parametros) {
   uint8_t tecla;
   uint8_t anterior = 0;

   while(1) {
      tecla = Read_Switches();
      if (tecla != anterior) {
         switch(tecla) {
            case TEC1:
            	vTaskPrioritySet( teclado, ( tskIDLE_PRIORITY + 1) );
            	//vTaskPrioritySet( recibir, ( tskIDLE_PRIORITY + 1));
            	xTaskCreate(RecibirADC, "ADC", configMINIMAL_STACK_SIZE, &ValoresADC[0], tskIDLE_PRIORITY + 2, recibirADC);
            	xEventGroupSetBits(xEventTecladoGroup, BIT_EVENTO_TECLADO);

               break;
            case TEC2:
            	xEventGroupClearBits(xEventTecladoGroup, BIT_EVENTO_TECLADO);
            	vTaskSuspend(recibirADC);
               break;
            case TEC3:

               break;
            case TEC4:
               break;
         }
         anterior = tecla;
      }
      /*vTaskDelay(100);
      Led_Toggle(GREEN_LED);*/
   }
}



int main(void)
{


	/*Inicializaciones*/
	Init_Leds();
	Init_Switches();
	Init_Uart_Ftdi();
	NVIC_EnableIRQ(26);
	//InitADC_DMA(ValoresADC[0]);
	//InitADC_DMA(ValoresADC[2]);
	xEventTecladoGroup = xEventGroupCreate();
	xEventBufferGroup = xEventGroupCreate();
	eventos = xEventGroupCreate();
	eventoSerial = xEventGroupCreate();
	//xEventBufferConversion = xEventGroupCreate();
	//ADC.adc= LPC_ADC0;
	xMutex = xSemaphoreCreateMutex();
	EnviarTexto(WelcomeMenu);

	if( xMutex != NULL ){
	/*Creo la tarea NULL es el puntero a los parámetros, el segundo es la variable donde
	 * me devuelve el id */
		/*tskIDLE_PRIORITY = zero*/

		xTaskCreate(Teclado, "pulsadores", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, teclado);
		//xTaskCreate(RecibirADC, "ADC", configMINIMAL_STACK_SIZE, &ValoresADC[0], tskIDLE_PRIORITY + 4, NULL);
		//xTaskCreate(RecibirADC, "ADC2", configMINIMAL_STACK_SIZE, &ValoresADC[2], 1, NULL);
		//xTaskCreate(Recibir, "Recibir", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2 , recibir);
		xTaskCreate(Transmitir, "Transmitir", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, transmitir);
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

