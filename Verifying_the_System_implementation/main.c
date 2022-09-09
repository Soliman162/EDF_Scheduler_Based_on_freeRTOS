/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"



/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
static void Init_voidRTOS(void);
/*-----------------------------------------------------------*/
/*
								{PORT_0, PIN0, OUTPUT},	Button_1_Monitor			16
								{PORT_0, PIN1, OUTPUT}, Button_2_Monitor			17
								{PORT_0, PIN2, OUTPUT}, Periodic_Transmitter	18
								{PORT_0, PIN3, OUTPUT}, Uart_Receiver					19
								{PORT_0, PIN4, OUTPUT}, Load_1_Simulation			20
								{PORT_0, PIN5, OUTPUT}, Load_2_Simulation			21

								{PORT_0, PIN6, OUTPUT},	tickhook							22

								{PORT_0, PIN7, INPUT},												23
								{PORT_0, PIN8, INPUT},												24

								{PORT_0, PIN9, OUTPUT}, Idle_Task							25
*/
/***************************************************************************************/
#define  Button_1_GPIO_PIN  											PIN0		//16		50	
#define  Button_2_GPIO_PIN  											PIN1		//17		50	
#define  Periodic_Transmitter_GPIO_PIN  					PIN2		//18		100	
#define  Uart_Receiver_GPIO_PIN 								 	PIN3		//19		20	
#define  Load_1_Simulation_GPIO_PIN  							PIN4		//20		10	
#define  Load_2_Simulation_GPIO_PIN  							PIN5		//21		100	
#define  IDLE_TASK_GPIO_PIN												PIN9		//25		200
#define  TICK_GPIO_PIN														PIN6
/******************************************Period*********************************************/
#define  Button_1_Period  											50
#define  Button_2_Period  											50
#define  Periodic_Transmitter_Period  					100
#define  Uart_Receiver_Period 								 	20
#define  Load_1_Simulation_Period  							10
#define  Load_2_Simulation_Period  							100
/***************************************************************************************/
#define NUMBER_OF_TASKS													7
/***************************************************************************************/
typedef struct{
	
	unsigned long int Time_IN;
	unsigned long int Time_OUT;
	unsigned long int Total_Time;
	
}Task_info;

unsigned short int CPU_LOAD = 0;
Task_info Task[NUMBER_OF_TASKS];
/***************************************************************************************/

/**************************************Handlers**********************************/
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;

TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;

TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL;

QueueHandle_t UART_Queue_Handler = NULL;

/*-----------------------------------------------------------*/
void Button_1_Monitor(void * pvParameters)
{
	char * B1;
	TickType_t Button_1_LastWakeTime ;
	vTaskSetApplicationTaskTag( NULL,(void *)Button_1_GPIO_PIN  );
	
	Button_1_LastWakeTime = xTaskGetTickCount();
	while(1)
	{
		if(GPIO_read(PORT_0, PIN7) == PIN_IS_HIGH)
		{
			B1 = (char *)"B1_HIGH";
		}else
		{
			B1 = (char *)"B1_LOW";
		}
		xQueueSend(UART_Queue_Handler, (void *) &B1,(TickType_t)0 );
		vTaskDelayUntil( &Button_1_LastWakeTime, Button_1_Period );
	}
}
/*-----------------------------------------------------------*/
void Button_2_Monitor(void * pvParameters)
{
	char * B2;
	TickType_t Button_2_LastWakeTime ;
	vTaskSetApplicationTaskTag( NULL,(void *) Button_2_GPIO_PIN  );
	
	Button_2_LastWakeTime = xTaskGetTickCount();
	while(1)
	{
		if(GPIO_read(PORT_0, PIN8) == PIN_IS_HIGH )
		{
			B2 = (char *)"B2_HIGH";
		}else
		{
			B2 = (char *)"B2_LOW";
		}
		xQueueSend(UART_Queue_Handler, (void *) &B2,(TickType_t)0 );
		vTaskDelayUntil( &Button_2_LastWakeTime, Button_2_Period );
	}

}
/*-----------------------------------------------------------*/
void Periodic_Transmitter(void * pvParameters)
{
	char * PT;
	TickType_t Periodic__Transmitter_LastWakeTime ;
	vTaskSetApplicationTaskTag( NULL,(void *) Periodic_Transmitter_GPIO_PIN  );
	
	Periodic__Transmitter_LastWakeTime = xTaskGetTickCount();
	while(1)
	{
		
		PT = (char *)"TRANSMIITER";
		xQueueSend(UART_Queue_Handler, (void *) &PT,(TickType_t)0 );
		vTaskDelayUntil( &Periodic__Transmitter_LastWakeTime, Periodic_Transmitter_Period );
	}
}
/*-----------------------------------------------------------*/
void Uart_Receiver(void * pvParameters)
{
	char * Uart_Rec;
	TickType_t Uart_Receiver_LastWakeTime ;
	vTaskSetApplicationTaskTag( NULL,(void *) Uart_Receiver_GPIO_PIN  );
	
	Uart_Receiver_LastWakeTime = xTaskGetTickCount();
	while(1)
	{
		xQueueReceive(UART_Queue_Handler,&(Uart_Rec),(TickType_t) 0 );
		xSerialPutChar((signed char )'\n');
		vSerialPutString(Uart_Rec, 11);	
		xSerialPutChar((signed char )'\n');
		vTaskDelayUntil( &Uart_Receiver_LastWakeTime, Uart_Receiver_Period );
	}

}
/*-----------------------------------------------------------*/
void Load_1_Simulation( void * pvParameters)
{
	TickType_t Load_1_Simulation_LastWakeTime ; 
	vTaskSetApplicationTaskTag( NULL,(void *) Load_1_Simulation_GPIO_PIN  );
	
	Load_1_Simulation_LastWakeTime = xTaskGetTickCount();
	while(1)
	{
		int load_1_Iterator = 0;
		vTaskDelayUntil( &Load_1_Simulation_LastWakeTime, Load_1_Simulation_Period );
		
		for(load_1_Iterator=0;load_1_Iterator<37500;load_1_Iterator++)
		{
			
		}
	}
}
/*-----------------------------------------------------------*/
void Load_2_Simulation( void * pvParameters)
{
	TickType_t Load_2_Simulation_LastWakeTime ;
	vTaskSetApplicationTaskTag( NULL,(void *) Load_2_Simulation_GPIO_PIN  );
	
	Load_2_Simulation_LastWakeTime = xTaskGetTickCount();
	while(1)
	{
		int load_2_Iterator = 0;
		vTaskDelayUntil( &Load_2_Simulation_LastWakeTime, Load_2_Simulation_Period );
		
		for(load_2_Iterator=0;load_2_Iterator<89500;load_2_Iterator++)
		{
			
		}
	}
}
//
/*-----------------------------------------------------------*/
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	 prvSetupHardware();
	
	 Init_voidRTOS();

    /* Create Tasks here */

		xTaskPeriodicCreate(  
													Button_1_Monitor,
													"Button 1 Monitor", 
													50,
													(void *)0,
													1,
													&Button_1_Monitor_Handler,
													(TickType_t)Button_1_Period
													);
												
		xTaskPeriodicCreate(  
													Button_2_Monitor,
													"Button 2 Monitor", 
													50,
													(void *)0,
													1,
													&Button_2_Monitor_Handler,
													(TickType_t)Button_2_Period
													);
												
		xTaskPeriodicCreate(  
													Periodic_Transmitter,
													"Periodic Transmitter", 
													50,
													(void *)0,
													1,
													&Periodic_Transmitter_Handler,
													(TickType_t)Periodic_Transmitter_Period
													);
													
		xTaskPeriodicCreate(  
													Uart_Receiver,
													"Uart Receiver", 
													50,
													(void *)0,
													1,
													&Uart_Receiver_Handler,
													(TickType_t)Uart_Receiver_Period
													);
	
		
		xTaskPeriodicCreate(  
													Load_1_Simulation,   
													"Load 1 Simulation",    
													50,   
													(void *)0,   
													1,
													&Load_1_Simulation_Handler,   
													(TickType_t)Load_1_Simulation_Period   
													);
   
		xTaskPeriodicCreate(  
													Load_2_Simulation,
													"Load 2 Simulation", 
													50,
													(void *)0,
													1,
													&Load_2_Simulation_Handler,
													(TickType_t)Load_2_Simulation_Period
													);
												
		
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/
/*
void vApplicationIdleHook(void)
{
	GPIO_write(PORT_0, IDLE_TASK_GPIO_PIN, PIN_IS_HIGH);
	GPIO_write(PORT_0, IDLE_TASK_GPIO_PIN, PIN_IS_LOW);
}
*/
/*-----------------------------------------------------------*/
void vApplicationTickHook(void)
{
	GPIO_write(PORT_0, TICK_GPIO_PIN, PIN_IS_HIGH);
	GPIO_write(PORT_0, TICK_GPIO_PIN, PIN_IS_LOW);
}
/*-----------------------------------------------------------*/
static void CONFIG_voidTIMER1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}
/*-----------------------------------------------------------*/
static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	/* Configure Timer1 */
	CONFIG_voidTIMER1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/
static void Init_voidRTOS(void)
{
	unsigned short int Iterator = 0;
	 UART_Queue_Handler = xQueueCreate( 3, sizeof( signed char[12] ) );
	 
	 for(Iterator = 0;Iterator<NUMBER_OF_TASKS;Iterator++)
	 {
			Task[Iterator].Time_IN = 0;
			Task[Iterator].Time_OUT = 0;
		  Task[Iterator].Total_Time = 0;
	 }
}
/*-----------------------------------------------------------*/
void SET_voidTASK_TIME_IN(int Copy_TaskTag, int Copy_TimeIN)
{
	Task[Copy_TaskTag-PIN0].Time_IN = Copy_TimeIN; 
}
/*-----------------------------------------------------------*/
void SET_voidTASK_TIME_OUT(int Copy_TaskTag, int Copy_Time_OUT)
{
	Task[Copy_TaskTag-PIN0].Time_OUT = Copy_Time_OUT;
}
/*-----------------------------------------------------------*/
void SET_voidTASK_TOTAL_TIME(int Copy_TaskTag)
{
	Task[Copy_TaskTag-PIN0].Total_Time += (Task[Copy_TaskTag-PIN0].Time_OUT - Task[Copy_TaskTag-PIN0].Time_IN);
}
/*-----------------------------------------------------------*/
double GET_u64TOTAL_TIME_TASKS(void)
{
	unsigned short int Iterator = 0;
	int long long		total_time = 0;
		for(Iterator = 0;Iterator<NUMBER_OF_TASKS;Iterator++)
	 {
		  total_time += Task[Iterator].Total_Time;
	 }
	 return total_time;
}
/*-----------------------------------------------------------*/
