
#include "CLOCK.h"
#include "GPIO.h"
#include "SYS_INIT.h"
#include "USART.h"
#include "stm32f4xx.h"
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	uint8_t buffer;
	char *str;
	char ch = 'l';
	uint32_t i = 0;
	/*	Configuration */
	initClock();
	sysInit();
	
	//UART Configuration 
	UART2_Config();
	UART4_Config();
	UART5_Config();
		
	UART_SendString(USART2,"\nHello World i work\n");
	
	for (i = 0;i < 6;i++){
		UART_SendChar(UART4,'d');
	}
	
	UART_GetString(UART5,6,&buffer);
	str = (char *)&buffer;
	
	UART_SendString(USART2,str);
}





