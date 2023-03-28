
#include "CLOCK.h"
#include "GPIO.h"
#include "SYS_INIT.h"
#include "USART.h"
#include "stm32f4xx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum UART_DIRECTION{
	UART4_TO_UART5 = 0,
	UART5_TO_UART4 = 1,
};

static char input_buff[50],output_buff[50];
static uint32_t in_idx,out_idx;

static uint16_t runningNS = 1;
static uint16_t GREEN_NS = 8;
static uint16_t GREEN_EW = 6;
static uint16_t RED_EW = 5;
static uint16_t RED_NS = 9;
static uint16_t TRAFFIC_NS = 2;
static uint16_t TRAFFIC_EW = 1;
static uint16_t report_interval = 5000;


static uint16_t runningTime = 2000;
static uint16_t extraTime = 3;
static uint16_t g_delayNS = 5;
static uint16_t r_delayNS = 2;
static uint16_t y_delayNS = 1;

static uint16_t g_delayEW = 5;
static uint16_t r_delayEW = 2;
static uint16_t y_delayEW = 1;

static uint32_t global_time = 0;

void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void USART2_IRQHandler(void);
void USART2_GetString(void);
void transmit_data(uint32_t direction);

void TIM5Config(void);
void TIM2Config(void);
void delay_micro(uint16_t us);
void tim5_delay(uint16_t ms);

void getString(void);
void parseCommand(void);
void show_traffic_info(void);
void showTrafficConfig(uint32_t light_no);
void showReportIntervalConfig(void);
void setDelayTraffic(char ch,uint32_t del,uint32_t light_no);
void clearLEDs(void);

void USART2_IRQHandler(void){
    USART2->CR1 &= ~(USART_CR1_RXNEIE);
    getString();
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


void getString(void){
    uint8_t ch,idx = 0;
    ch = UART_GetChar(USART2);
    while(ch != '!'){
        input_buff[idx++] = ch;
        ch = UART_GetChar(USART2);
        if(ch == '!')break;
    }      
    input_buff[idx] = '\0';

}


void setDelayTraffic(char ch,uint32_t del,uint32_t light_no){
	
	if(light_no == 1){
		if (ch == 'G'){
			g_delayNS = (uint16_t)del;
		}else  if (ch == 'Y'){
			y_delayNS = (uint16_t)del;
		}else if(ch == 'R'){
			r_delayNS = (uint16_t)del;
		}
	}else if(light_no == 2){
		if (ch == 'G'){
			g_delayEW = (uint16_t)del;
		}else  if (ch == 'Y'){
			y_delayEW = (uint16_t)del;
		}else if(ch == 'R'){
			r_delayEW = (uint16_t)del;
		}
	}
}

void showTrafficConfig(uint32_t light_no){
	char str[50];
	if (light_no == 1)
		sprintf(str,"\ntraffic light 1 G Y R %d %d %d %d\n",(uint32_t)g_delayNS,
		(uint32_t)y_delayNS,(uint32_t)r_delayNS,(uint32_t)extraTime);
	if (light_no == 2)
		sprintf(str,"\ntraffic light 2 G Y R %d %d %d %d\n",(uint32_t)g_delayEW,
		(uint32_t)y_delayEW,(uint32_t)r_delayEW,(uint32_t)extraTime);
	
//	UART_SendString(USART2,str);
	strcpy(input_buff,str);
	transmit_data(UART5_TO_UART4);
	UART_SendString(USART2,output_buff);
	strcpy(output_buff,"");
}

void showReportIntervalConfig(void){
	char str[50];
	sprintf(str,"\ntraffic monitor %d\n",(uint32_t)report_interval/1000);
	
//	UART_SendString(USART2,str);
	
	strcpy(input_buff,str);
	transmit_data(UART5_TO_UART4);
	UART_SendString(USART2,output_buff);
	strcpy(output_buff,"");
}
void parseCommand(void){
	/*
	transmit command from control center to traffic system
	UART4 = Control Center
	UART5 = Traffic System
	*/
	char command[50],c1,c2,c3;
	uint32_t light_no,del1,del2,del3,ext,rep_int;
	
	transmit_data(UART4_TO_UART5);
	strcpy(command,output_buff);
	
	if (command[0] == 'c'){
		if(command[15] == 'l'){
			sscanf(command,"config traffic light %d %c %c %c %d %d %d %d",
			&light_no,&c1,&c2,&c3,&del1,&del2,&del3,&ext);
			
			setDelayTraffic(c1,del1,light_no);
			setDelayTraffic(c3,del3,light_no);
			setDelayTraffic(c2,del2,light_no);
			
			extraTime = (uint16_t)ext;
		}
		else if(command[15] == 'm'){
			sscanf(command,"config traffic monitor %d",&rep_int);
			report_interval = (uint16_t)(rep_int * 1000);
		}
	}
	else if(command[0] == 'r'){
		if (strlen(command) == 4){
			showTrafficConfig(1);
			showTrafficConfig(2);
			showReportIntervalConfig();
		}
		else if (command[13] == 'l'){
			sscanf(command,"read traffic light %d",&light_no);
			showTrafficConfig(light_no);
		}
		else if(command[13] == 'm'){
			showReportIntervalConfig();
		}
	}
	strcpy(input_buff,"");
	strcpy(output_buff,"");
}

void show_traffic_info(void){
	char *G_NS_state = (GPIO_PIN_8 & GPIOA->IDR)?"ON":"OFF";
	char *R_NS_state = (GPIO_PIN_9 & GPIOA->IDR)?"ON":"OFF";
	char *G_EW_state = (GPIO_PIN_6 & GPIOA->IDR)?"ON":"OFF";
	char *R_EW_state = (GPIO_PIN_5 & GPIOA->IDR)?"ON":"OFF";
	char *Y_NS_state = (RED_NS == 0 && GREEN_NS == 0)? "ON" : "OFF";
	char *Y_EW_state = (RED_EW == 0 && GREEN_EW == 0)? "ON" : "OFF";
	
	char *NS_congestion = (GPIO_PIN_4 & GPIOB->IDR)?"heavy traffic":"light traffic";
	char *EW_congestion = (GPIO_PIN_5 & GPIOB->IDR)?"heavy traffic":"light traffic";
	
	char str[50];
	
	sprintf(str, "\n%d traffic light 1 %s %s %s\n", (uint32_t) global_time, G_NS_state, Y_NS_state, R_NS_state);
	strcpy(input_buff,str);
	transmit_data(UART5_TO_UART4);
	UART_SendString(USART2, output_buff);
	sprintf(str, "%d traffic light 2 %s %s %s\n", (uint32_t) global_time, G_EW_state, Y_EW_state, R_EW_state);
	strcpy(input_buff,str);
	transmit_data(UART5_TO_UART4);
	UART_SendString(USART2, output_buff);
	sprintf(str, "%d road north south %s \n", (uint32_t) global_time, NS_congestion);
	strcpy(input_buff,str);
	transmit_data(UART5_TO_UART4);
	UART_SendString(USART2, output_buff);
	sprintf(str, "%d road east west %s \n", (uint32_t) global_time, EW_congestion);
	strcpy(input_buff,str);
	transmit_data(UART5_TO_UART4);
	UART_SendString(USART2, output_buff);
}
void transmit_data(uint32_t direction)
{
    /*
    direction = 0 => transmit from UART4 -> UART5
    direction = 1 => transmit from UART5 -> UART4
     */
	
    uint32_t i = 0;
	USART_TypeDef* usart;
    in_idx = 0;
	out_idx = 0;
    if(direction == UART4_TO_UART5){
        usart = UART4;
    }
    else if (direction == UART5_TO_UART4){
        usart = UART5;
    }
    else{
        return;
    }
    
    for (i = 0;i < strlen(input_buff);i++){
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


void TIM5Config(void){
	RCC->APB1ENR |= (1<<3);
	
	TIM5->PSC = 45000 - 1; /* fck = 45 mhz, CK_CNT = fck / (psc[15:0] + 1)*/
	TIM5->ARR = 0xFFFF; /*maximum clock count*/
	
	TIM5->CR1 |= (1<<0);
	
	while(!(TIM5->SR & (1<<0)));
	
}
void tim5_delay(uint16_t ms){
	ms = (uint16_t)2 * ms;
	TIM5->CNT = 0;
//	char str[50];
//	sprintf(str,"in tim5delay tim2->cnt %d\n",TIM2->CNT);
	
//	UART_SendString(USART2,str);
	while(TIM5->CNT < ms){
        
        if(strlen(input_buff) != 0){
			parseCommand();
        }
		if(TIM2->CNT > report_interval*2){
			global_time += report_interval/1000;
			show_traffic_info();
			TIM2->CNT = 0;
		}
		if(strlen(input_buff) != 0){
			parseCommand();
		}
	}
}



void TIM2Config(void){
	RCC->APB1ENR |= (1<<0);
	
	TIM2->PSC = 45000 - 1; /* fck = 90 mhz, CK_CNT = fck / (psc[15:0] + 1)*/
	TIM2->ARR = 0xFFFF; /*maximum clock count*/
	
	TIM2->CR1 |= (1<<0);
	
	while(!(TIM2->SR & (1<<0)));
	
}




int main(void)
{   
    // uint32_t t_delay = 1000;
	// GPIO Config
    GPIO_InitTypeDef gpio_config;
	/*	Configuration */
	initClock();
	sysInit();
	UART2_Config();
	UART4_Config();
	UART5_Config();
	TIM5Config();
    TIM2Config();
	
    //set interrupt priority and enable IRQ
    NVIC_SetPriority(USART2_IRQn, 1);
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(UART4_IRQn, 1);
    NVIC_EnableIRQ(UART4_IRQn);
    NVIC_SetPriority(UART5_IRQn, 1);
    NVIC_EnableIRQ(UART5_IRQn);
    
    NVIC_SetPriority(SysTick_IRQn,2);
    NVIC_EnableIRQ(SysTick_IRQn);
    
    

    UART_SendString(USART2,"HELLO I'M IN\n");
    
    
    
    //config for output 
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_config.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_Init(GPIOA, &gpio_config);
    gpio_config.Pin = GPIO_PIN_1|GPIO_PIN_2;
	GPIO_Init(GPIOB, &gpio_config);
	
	//config for input 
	gpio_config.Mode = GPIO_MODE_INPUT;
	gpio_config.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_3;
    GPIO_Init(GPIOB, &gpio_config);
	
	//timer start
	TIM2->CNT = 0;
	strcpy(input_buff,"");
	strcpy(output_buff,"");
	
    while(1){
		runningTime = 0;
		clearLEDs();
		uint16_t trafficNS = (uint16_t) rand()%2;
		uint16_t trafficEW = (uint16_t) rand()%2;
		
		uint16_t balanced = trafficEW & trafficNS;
		
		if(balanced==0) runningTime += extraTime;
			
		 if (trafficNS) {
			GPIO_WritePin(GPIOB, TRAFFIC_NS, GPIO_PIN_SET);
			
		 }
		 if (trafficEW) {
			GPIO_WritePin(GPIOB, TRAFFIC_EW, GPIO_PIN_SET);
		 }
		 
		 if (runningNS) {
			GPIO_WritePin(GPIOA, GREEN_NS, GPIO_PIN_SET);
			GPIO_WritePin(GPIOA, RED_EW, GPIO_PIN_SET);
			runningTime += g_delayNS;
		 }
		 else {
			GPIO_WritePin(GPIOA, GREEN_EW, GPIO_PIN_SET);
			GPIO_WritePin(GPIOA, RED_NS, GPIO_PIN_SET);
			runningTime += g_delayEW;
		 }
//		 ms_delay(1000);
		 tim5_delay(runningTime*1000);
		 
		 if(runningNS){
			GPIO_WritePin(GPIOA, GREEN_NS, GPIO_PIN_RESET);
			tim5_delay(y_delayNS*1000);
		 }else {
			GPIO_WritePin(GPIOA, GREEN_EW, GPIO_PIN_RESET);
			tim5_delay(y_delayEW*1000);
		 }
		 
		 runningNS = (runningNS==1)? 0:1;
     
		
//        if(strlen(input_buff) != 0){
//			parseCommand();
//		}else{
//			UART_SendString(USART2,"\nINPUT BUFF: emp\n");
//		}
    }
}

void clearLEDs(void){
    for(uint16_t i = 0; i<13; i++){
        GPIO_WritePin(GPIOA, i, GPIO_PIN_RESET);
        GPIO_WritePin(GPIOB, i, GPIO_PIN_RESET);
    }
}




