
#include "CLOCK.h"
#include "GPIO.h"
#include "SYS_INIT.h"
#include "USART.h"
#include "stm32f4xx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char input_buff[50],output_buff[50];
static uint32_t in_idx,out_idx;

void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void USRT2_IRQHandler(void);
void USART2_GetString(void);
void parseInputString(void);

void parseInputString(void){
//    uint8_t ch,idx = 0;
//    ch = UART_GetChar(USART2);
//    input_buff[idx] = 0;
}
void USART2_GetString(void){
    USART2->CR1 &= ~USART_CR1_RXNEIE;
    parseInputString();
    USART2->CR1 |= USART_CR1_RXNEIE;
}

void USRT2_IRQHandler(void){
    USART2->CR1 &= ~(USART_CR1_RXNEIE);
    USART2_GetString();
    USART2->CR1 |= (USART_CR1_RXNEIE);
}

void UART4_IRQHandler(void)
{   
    if (UART4->SR & USART_SR_RXNE){
        while(!(UART4->SR & USART_SR_RXNE));
        
        output_buff[out_idx] = (uint8_t) UART4->DR;
        
        UART4->SR &= ~(USART_SR_RXNE);
    }
    
    if (UART4->SR & USART_SR_TXE){
        //handle queue here
        UART4->DR = input_buff[in_idx];
        while(!(UART4->SR & USART_SR_TXE));
        
        UART4->SR &= ~(USART_SR_TXE);
        UART4->CR1 &= ~(USART_CR1_TXEIE);
    }
    
}
void UART5_IRQHandler(void){
    
    if (UART5->SR & USART_SR_RXNE){   
        while(!(UART5->SR & USART_SR_RXNE));
        
        output_buff[out_idx] = (uint8_t) UART5->DR; 
        
        UART5->SR &= ~(USART_SR_RXNE);
        
    }
    if (UART5->SR & USART_SR_TXE){
        //handle queue here
        UART5->DR = input_buff[in_idx];  
        while(!(UART5->SR & USART_SR_TXE));        
        
        UART5->SR &= ~(USART_SR_TXE);
        UART5->CR1 &= ~USART_CR1_TXEIE;
    }
}


void transmit_data(char to_send[],uint32_t direction)
{
    /*
    direction = 0 => transmit from UART4 -> UART5
    direction = 1 => transmit from UART5 -> UART4
     */
    uint32_t i = 0;
    in_idx = 0,out_idx = 0;
    USART_TypeDef* usart;
    if(direction == 0){
        usart = UART4;
    }
    else if (direction == 1){
        usart = UART5;
    }
    else{
        return;
    }
    
    for (i = 0;i < strlen(to_send);i++){
        input_buff[in_idx] = to_send[i];
        
        //Enable Interrupt
        usart->CR1 |= USART_CR1_TXEIE;
        while((usart->CR1 & USART_CR1_TXEIE));
        ms_delay(1);
        in_idx++;
        out_idx++;
    }
    output_buff[out_idx++] = '\0';
    usart = NULL;
}




int main(void)
{
	
	/*	Configuration */
	initClock();
	sysInit();
	UART2_Config();
	UART4_Config();
	UART5_Config();
    
    //set interrupt priority and enable IRQ
    NVIC_SetPriority(USART2_IRQn, 1);
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(UART4_IRQn, 1);
    NVIC_EnableIRQ(UART4_IRQn);
    
    NVIC_SetPriority(UART5_IRQn, 1);
    NVIC_EnableIRQ(UART5_IRQn);
    UART_SendString(USART2,"  HELLO I'M IN   ");
    while(1){
        uint8_t *buffer;
        UART_GetString(USART2,6,&buffer);
        char *to_send = (char *)&buffer;
        
        transmit_data(to_send,1);
        
        UART_SendString(USART2,"(main): ");
        
        if (strlen(output_buff) != 0){
            UART_SendString(USART2,output_buff);
            
        }else{
            UART_SendString(USART2,"OUTPUT EMPTY");
        }
        UART_SendString(USART2,"\n");
        strcpy(output_buff,"");
    }
}





