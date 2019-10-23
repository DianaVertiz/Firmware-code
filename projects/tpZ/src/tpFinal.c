

/*==================[inclusions]=============================================*/
#include "tpFinal.h"
#include "task.h"
#include "event_groups.h"
#include "stdint.h"
#include <string.h>
#include "switch.h"
#include "uart.h"
#include "led.h"
#include "adc.h"
#include "semphr.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
/*Corresponden a la numeracion de * The channels on one ADC peripheral* en adc_18xx_43xx.h*/
typedef enum CANALES {CANAL0 = 0, CANAL1, CANAL2, CANAL3};
typedef enum PERIFERICOS {PER1 = 1, PER2};

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

#define N_SAMPLES   10
#define N_SAMPLES3   10

#define BIT_EVENTO_TECLADO ( 1 << 0 )
#define BIT_EVENTO_BUFFER_LLENO (2 << 0)
#define BIT_EVENTO_CONVERTIDO (3 << 0)

EventGroupHandle_t xEventTecladoGroup;
EventGroupHandle_t xEventBufferGroup;
//EventGroupHandle_t xEventBufferConversion;
SemaphoreHandle_t  xMutex;

static volatile uint8_t Burst_Mode_Flag = 0;
static volatile uint8_t dma_ch_adc;
static volatile uint8_t activoCH1=0;
static volatile uint8_t activoCH2=0;
static volatile uint8_t activoCH3=0;
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
static uint16_t flag_primer_convertido = 0;

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

void Conversion(ADCData ADC){

	ADC_CLOCK_SETUP_T configADC;
//if(flag_conversion == 0){
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
	/*Empiezo la conversión*/

	if(ADC.canal == CANAL1){activoCH1=1;Led_On(RED_LED);}
	if(ADC.canal == CANAL2){activoCH2=1;Led_On(YELLOW_LED);}
	if(ADC.canal == CANAL3){activoCH3=1;Led_On(GREEN_LED);}


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
	//}

}

bool llenarBuffer(uint16_t * buffer, uint16_t tamanio, uint16_t * indice){

	bool completo = FALSE;

	buffer[*indice] = ADC_DR_RESULT(DMAbuffer);

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
	uint16_t menor = 1023;
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

	if(activoCH1){
		if(llenarBuffer(ValoresADC[0].datos ,ValoresADC[0].tamanio, &indice_ch1)){
			/*procesar y vaciar el buffer*/
			resultado = Procesamiento(ValoresADC[0].datos, ValoresADC[0].tamanio);
			activoCH1 = 0;
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

/*==================[external functions definition]==========================*/

void RecibirADC(void * parametros){

	ADCData * ADC = (ADCData *) parametros;
	const EventBits_t xBitsToWaitForTeclado = BIT_EVENTO_TECLADO | BIT_EVENTO_CONVERTIDO;

	EventBits_t xEventGroupTecladoValue;
	//EventBits_t xEventGroupBufferValue;
	while(1){
		/*Espero evento de teclado para iniciar la conversión AD*/
		xEventGroupTecladoValue = xEventGroupWaitBits( xEventTecladoGroup,
												xBitsToWaitForTeclado,
												pdTRUE,
												pdFALSE,
												portMAX_DELAY );


		if(((xEventGroupTecladoValue & BIT_EVENTO_TECLADO) != 0) |((xEventGroupTecladoValue & BIT_EVENTO_CONVERTIDO) != 0)){
			/*Empiezo la conversión*/

			//vTaskDelay(500);
			Conversion(*(ADC));


		}
	}
}



void DMA_IRQHandler(void){


	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dma_ch_adc) == SUCCESS) {
		/*Dato convertido!*/
		NVIC_DisableIRQ(DMA_IRQn);
		GuardarDatos();

		xEventGroupSetBits(xEventTecladoGroup, BIT_EVENTO_CONVERTIDO);
	}

}



void Teclado(void * parametros) {
   uint8_t tecla;
   uint8_t anterior = 0;

   while(1) {
      tecla = Read_Switches();
      if (tecla != anterior) {
         switch(tecla) {
            case TEC1:
            	xEventGroupSetBits(xEventTecladoGroup, BIT_EVENTO_TECLADO);
               break;
            case TEC2:
            	xEventGroupClearBits(xEventTecladoGroup, BIT_EVENTO_TECLADO);
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
	//InitADC_DMA(ValoresADC[0]);
	//InitADC_DMA(ValoresADC[2]);
	xEventTecladoGroup = xEventGroupCreate();
	xEventBufferGroup = xEventGroupCreate();
	//xEventBufferConversion = xEventGroupCreate();
	//ADC.adc= LPC_ADC0;
	xMutex = xSemaphoreCreateMutex();

	if( xMutex != NULL ){
	/*Creo la tarea NULL es el puntero a los parámetros, el segundo es la variable donde
	 * me devuelve el id */

		xTaskCreate(Teclado, "pulsadores", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		//xTaskCreate(RecibirADC, "ADC", configMINIMAL_STACK_SIZE, &ValoresADC[0], 1, NULL);
		xTaskCreate(RecibirADC, "ADC2", configMINIMAL_STACK_SIZE, &ValoresADC[2], 1, NULL);
		//xTaskCreate(LLenar, "llenar", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

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

