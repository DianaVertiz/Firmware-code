
#ifndef UART_INT_H
#define UART_INT_H


/*==================[inclusions]=============================================*/
#include "stdint.h"
#include "string.h"
#include "sAPI_DataTypes.h"



//uint32_t BaudRate = 115200;

#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 32	/* Receive */
#define UART2_IRQn 26


void IniciarUart(void);
void IniciarInt(void);
void FinInt(void);
uint8_t Leer_intUART(void);
uint8_t uartReadByte(void);
bool_t ReadByte(uint8_t*);
void uartWriteByte(uint8_t);
void uartWriteString(char*);
uint16_t Escalado(uint16_t);
void EnviarMensaje(char*, uint8_t);
void Escribir_dato(uint8_t);
void WriteInt(uint16_t);
char* itoa(uint16_t, char*, uint16_t);
char* itoa2(uint32_t, uint8_t);




#endif /* PROJECTS_DRIVERS_BM_INC_TECLAS_H_ */
