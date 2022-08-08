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
#include "lpc21xx.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

#define NULL_PTR                             (void*)0

/* period for each task */
#define BUTTON_1_MONITOR_TASK_PERIOD         50
#define BUTTON_2_MONITOR_TASK_PERIOD         50
#define PERIODIC_TRANSMITTER_TASK_PERIOD     100
#define UART_RECEIVER_TASK_PERIOD            20
#define LOAD_1_SIMULATION_TASK_PERIOD        10
#define LOAD_2_SIMULATION_TASK_PERIOD        100

/* handler for each task */
TaskHandle_t Button1Monitor_TaskHandler      = NULL;
TaskHandle_t Button2Monitor_TaskHandler      = NULL;
TaskHandle_t PeriodicTransmitter_TaskHandler = NULL;
TaskHandle_t UartReceiver_TaskHandler        = NULL;
TaskHandle_t Load1Simulation_TaskHandler     = NULL;
TaskHandle_t Load2Simulation_TaskHandler     = NULL;

QueueHandle_t xQueue;


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/* Task to be created. */

/* Description:
   this task monitor button 1 (PIN1) state every 50 ms
	 if a rising or falling edge happens on PIN1 a message will be sent to the Queue
	 the maximum number of messages to be sent is 3, until a recieve happens
 */
void Button_1_Monitor( void * pvParameters )
{
	pinState_t button1_OldState = PIN_IS_HIGH;  /* the initial value of PIN1 is high */
	pinState_t button1_NewState;                /* variable to store the new PIN1 value each task call */
	char* button1_message = NULL_PTR;           /* pointer to hold the message string */

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();


	for( ;; )
	{
		button1_NewState = GPIO_read(PORT_0, PIN1);

		/* check if an edge switch occured on PIN1 or not */
		if ( button1_NewState != button1_OldState )
		{
			/* rising edge case */
			if ( button1_NewState == PIN_IS_HIGH )
			{
				/* check if the Queue have a place or not if yes store the message in it
							     and dont't wait any ticks for a place to be empty in the Queue */
				if ( xQueue != 0 )
				{
					button1_message = "BUTTON_1_RISING_EDGE\n";
					xQueueSend( xQueue, ( void * ) &button1_message, ( TickType_t ) 0 );
				}
			}
			/* falling edge case */
			else if ( button1_NewState == PIN_IS_LOW )
			{
				/* check if the Queue have a place or not if yes store the message in it
							     and dont't wait any ticks for a place to be empty in the Queue */
				if ( xQueue != 0 )
				{
					button1_message = "BUTTON_1_FALLING_EDGE\n";
					xQueueSend( xQueue, ( void * ) &button1_message, ( TickType_t ) 0 );
				}
			}
		}
	}
	/* update the old state of the button */
	button1_OldState = button1_NewState;

	vTaskDelayUntil( &xLastWakeTime, BUTTON_1_MONITOR_TASK_PERIOD );
}

/* Description:
   this task monitor button 2 (PIN2) state every 50 ms
	 if a rising or falling edge happens on PIN2 a message will be sent to the Queue
   the maximum number of messages to be sent is 3, until a recieve happens
 */
void Button_2_Monitor( void * pvParameters )
{
	pinState_t button2_OldState = PIN_IS_HIGH;  /* the initial value of PIN1 is high */
	pinState_t button2_NewState;                /* variable to store the new PIN1 value each task call */
	char* button2_message = NULL_PTR;           /* pointer to hold the message string */

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		button2_NewState = GPIO_read(PORT_0, PIN2);

		/* check if an edge switch occured on PIN1 or not */
		if ( button2_NewState != button2_OldState )
		{
			/* rising edge case */
			if ( button2_NewState == PIN_IS_HIGH )
			{
				/* check if the Queue have a place or not if yes store the message in it
							     and dont't wait any ticks for a place to be empty in the Queue */
				if ( xQueue != 0 )
				{
					button2_message = "BUTTON_2_RISING_EDGE\n";
					xQueueSend( xQueue, ( void * ) &button2_message, ( TickType_t ) 0 );
				}
			}
			/* falling edge case */
			else if ( button2_NewState == PIN_IS_LOW )
			{
				/* check if the Queue have a place or not if yes store the message in it
							     and dont't wait any ticks for a place to be empty in the Queue */
				if ( xQueue != 0 )
				{
					button2_message = "BUTTON_2_FALLING_EDGE\n";
					xQueueSend( xQueue, ( void * ) &button2_message, ( TickType_t ) 0 );
				}
			}
		}
	}
	/* update the old state of the button */
	button2_OldState = button2_NewState;

	vTaskDelayUntil( &xLastWakeTime, BUTTON_2_MONITOR_TASK_PERIOD );
}

/* Description:
   this task sent a message to the Queue every 100 ms
   the maximum number of messages to be sent is 3, until a recieve happens
 */
void Periodic_Transmitter( void * pvParameters )
{
	char* periodic_message = NULL_PTR;   /* pointer to hold the message string */
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* check if the Queue have a place or not if yes store the message in it
					 and dont't wait any ticks for a place to be empty in the Queue */
		if ( xQueue != 0 )
		{
			periodic_message = "PERIODIC_TRANSIMITTER\n";
			xQueueSend( xQueue, ( void * ) &periodic_message, ( TickType_t ) 0 );
		}
	}
	vTaskDelayUntil( &xLastWakeTime, PERIODIC_TRANSMITTER_TASK_PERIOD );
}

/* Description:
   this task receives messages every 20ms and send it via Uart
 */
void Uart_Receiver( void * pvParameters )
{   
	char* receive_message = NULL_PTR;   /* pointer to hold the message string */
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* check if there is an exisitng item in the Queue if yes send it via Uart
			     and wait no ticks for an item to be in the Queue */
		if( xQueueReceive( xQueue,&(receive_message ),( TickType_t ) 0 ) == pdPASS )
		{
			vSerialPutString((const signed char *)(receive_message), 22);
		}
	}
	vTaskDelayUntil( &xLastWakeTime, UART_RECEIVER_TASK_PERIOD );
}

/* Description:
   this task runs every 10ms for 5ms to simulate a load
 */
void Load_1_Simulation( void * pvParameters )
{
	uint16_t Load1Tick;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* do nothing for 5ms */
		for ( Load1Tick=0; Load1Tick < 30000; Load1Tick++ );

	}
	vTaskDelayUntil( &xLastWakeTime, LOAD_1_SIMULATION_TASK_PERIOD );
}

/* Description:
   this task runs every 100ms for 12ms to simulate a load
 */
void Load_2_Simulation( void * pvParameters )
{
	uint32_t Load2Tick;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* do nothing for 12ms */
		for ( Load2Tick = 0; Load2Tick < 90500; Load2Tick++);

	}
	vTaskDelayUntil( &xLastWakeTime, LOAD_2_SIMULATION_TASK_PERIOD );
}

/* tick hook callback function */
void vApplicationTickHook( void )
{
	GPIO_write (PORT_0, PIN0, PIN_IS_HIGH);
	GPIO_write (PORT_0, PIN0, PIN_IS_LOW);
}

/* idle task callback function */
void vApplicationIdleHook( void )
{
	GPIO_write (PORT_0, PIN9, PIN_IS_HIGH);
}

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	xQueue = xQueueCreate( 3, sizeof( char* ) );

	/* Create Tasks here */

	/* Create the task, storing the handle. */
	xTaskPeriodicCreate(
			Button_1_Monitor,                  /* Function that implements the task. */
			"BUTTON 1 MONITOR",                /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Button1Monitor_TaskHandler,       /* Used to pass out the created task's handle. */
			BUTTON_1_MONITOR_TASK_PERIOD);     /* Period for the task */

	/* Create the task, storing the handle. */
	xTaskPeriodicCreate(
			Button_2_Monitor,                  /* Function that implements the task. */
			"BUTTON 2 MONITOR",                /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Button2Monitor_TaskHandler,       /* Used to pass out the created task's handle. */
			BUTTON_2_MONITOR_TASK_PERIOD);     /* Period for the task */

	/* Create the task, storing the handle. */
	xTaskPeriodicCreate(
			Periodic_Transmitter,               /* Function that implements the task. */
			"PERIODIC TRANSMITTER",             /* Text name for the task. */
			100,                                /* Stack size in womain.crds, not bytes. */
			( void * ) 0,                       /* Parameter passed into the task. */
			1,                                  /* Priority at which the task is created. */
			&PeriodicTransmitter_TaskHandler,   /* Used to pass out the created task's handle. */
			PERIODIC_TRANSMITTER_TASK_PERIOD);  /* Period for the task */

	/* Create the task, storing the handle. */
	xTaskPeriodicCreate(
			Uart_Receiver,                      /* Function that implements the task. */
			"UART RECEIVER",                    /* Text name for the task. */
			100,                                /* Stack size in words, not bytes. */
			( void * ) 0,                       /* Parameter passed into the task. */
			1,                                  /* Priority at which the task is created. */
			&UartReceiver_TaskHandler,          /* Used to pass out the created task's handle. */
			UART_RECEIVER_TASK_PERIOD);         /* Period for the task */

	/* Create the task, storing the handle. */
	xTaskPeriodicCreate(
			Load_1_Simulation,                 /* Function that implements the task. */
			"LOAD 1 SIMULATION",               /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Load1Simulation_TaskHandler,      /* Used to pass out the created task's handle. */
			LOAD_1_SIMULATION_TASK_PERIOD);	   /* Period for the task */

	/* Create the task, storing the handle. */
	xTaskPeriodicCreate(
			Load_2_Simulation,                 /* Function that implements the task. */
			"LOAD 1 SIMULATION",               /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Load2Simulation_TaskHandler,      /* Used to pass out the created task's handle. */
			LOAD_2_SIMULATION_TASK_PERIOD); 	 /* Period for the task */

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

void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

static void ConfigTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Configure trace timer 1 and read TITC to get current tick */
	ConfigTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


