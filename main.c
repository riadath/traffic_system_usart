
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
    uint8_t ch,idx = 0;
    ch = UART_GetChar(USART2);
    while(ch != '!'){
        input_buff[idx++] = ch;
        ch = UART_GetChar(USART2);
        if(ch == '!')break;
    }      
    input_buff[idx] = '\0';
    
//    for(int i = 0;i < strlen(input_buff);i++){
//        UART_SendChar(USART2,input_buff[i]);
//    }
}

void USART2_IRQHandler(void){
    USART2->CR1 &= ~(USART_CR1_RXNEIE);
    parseInputString();
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


void transmit_data(uint32_t direction)
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
    
    for (i = 0;i < strlen(input_buff);i++){
        //Enable Interrupt
        usart->CR1 |= USART_CR1_TXEIE;
        while((usart->CR1 & USART_CR1_TXEIE));
        ms_delay(2);
        in_idx++;
        out_idx++;
    }
    output_buff[out_idx++] = '\0';
    usart = NULL;
}




int main(void)
{   
    uint32_t t_delay = 500;
	
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
    
    NVIC_SetPriority(SysTick_IRQn,0);
    NVIC_EnableIRQ(SysTick_IRQn);
    
    UART_SendString(USART2,"HELLO I'M IN\n");
    
    
    // GPIO Config
    GPIO_InitTypeDef gpio_config;
    gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_config.Pin = GPIO_PIN_5;
    
    
    GPIO_Init(GPIOA,&gpio_config);
    
    
    while(1){
        
        if(strlen(input_buff) != 0){
//            UART_SendString(USART2,input_buff);
//            UART_SendString(USART2,"\n");
            transmit_data(1);
            strcpy(input_buff,"");
        }    
        GPIO_WritePin(GPIOA,5,GPIO_PIN_SET);
        ms_delay(t_delay);
        GPIO_WritePin(GPIOA,5,GPIO_PIN_RESET);
        ms_delay(t_delay);
        
        if (strlen(output_buff) != 0){
            if(!strcmp(output_buff,"inc") == 1){
                t_delay = 1000;
            }else if(!strcmp(output_buff,"dec")){
                t_delay = 100;
            }
            
            UART_SendString(USART2,"(main): ");
            UART_SendString(USART2,output_buff);
            UART_SendString(USART2,"\n");
            strcpy(output_buff,"");
        }
        
        
    }
}





