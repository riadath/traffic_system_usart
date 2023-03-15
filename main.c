
#include "CLOCK.h"
#include "GPIO.h"
#include "SYS_INIT.h"
#include "USART.h"
#include "stm32f4xx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char input_buff = '#';
static char output_buff = '#';

void UART4_IRQHandler(void);
void UART5_IRQHandler(void);

void UART4_IRQHandler(void)
{
//    UART_SendString(USART2,"\n4->IRQ Handler\n");
    
    if (UART4->SR & USART_SR_RXNE){
        output_buff = (uint8_t) UART4->DR;
//        while(!(UART4->SR & (1<<5)));
        UART4->SR &= ~(USART_SR_RXNE);
    }
    
    if (UART4->SR & USART_SR_TXE){
        //handle queue here
        UART4->DR = input_buff;
//        while(!(UART4->SR & (1<<7)));
        
        UART4->SR &= ~(USART_SR_TXE);
        UART4->CR1 &= ~USART_CR1_TXEIE;
    }
    
}
void UART5_IRQHandler(void){
    
//    UART_SendString(USART2,"\n5->IRQ Handler\n");
    
    if (UART5->SR & USART_SR_RXNE){
        
        output_buff = (uint8_t) UART5->DR;
//        while(!(UART5->SR & (1<<5)));
        
//        UART_SendString(USART2,"OUTPUT BUFF:");
//        UART_SendChar(USART2,output_buff);
//        UART_SendChar(USART2,'\n');
        
        UART5->SR &= ~(USART_SR_RXNE);
    }
    if (UART5->SR & USART_SR_TXE){
        //handle queue here
        UART5->DR = input_buff;
//        while(!(UART5->SR & (1<<7)));
        
        UART5->SR &= ~(USART_SR_TXE);
        UART5->CR1 &= ~USART_CR1_TXEIE;
    }
}
int main(void)
{
	
	/*	Configuration */
	initClock();
	sysInit();
	UART2_Config();
	UART4_Config();
	UART5_Config();
    NVIC_SetPriority(UART4_IRQn, 1);
    NVIC_EnableIRQ(UART4_IRQn);
    
    NVIC_SetPriority(UART5_IRQn, 1);
    NVIC_EnableIRQ(UART5_IRQn);
    
    
    input_buff = 'L';
    UART4->CR1 |= USART_CR1_TXEIE;
    
    UART_SendString(USART2,"(main): ");
    UART_SendChar(USART2,output_buff);
}





