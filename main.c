
#include "CLOCK.h"
#include "GPIO.h"
#include "SYS_INIT.h"
#include "USART.h"
#include "stm32f4xx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(void)
{
	//uint8_t buffer;
	char *rcv_str;
	char send_str[] = "dumdum";
	char ch;
	uint32_t i = 0;
	
	
	/*	Configuration */
	initClock();
	sysInit();
	UART2_Config();
	UART4_Config();
	UART5_Config();

	
	UART_SendString(USART2,"\nHello World i work\n");
	
	
	rcv_str = (char *)malloc(5 * sizeof(char));
	
	for (i = 0;i < strlen(send_str);i++){
		UART_SendChar(UART4,send_str[i]);
		ch = UART_GetChar(UART5);
		rcv_str[i] = ch;
	}
	
//	UART_GetString(UART5,6,&buffer);
//	str = (char *)&buffer;
	
	UART_SendString(USART2,rcv_str);
}





