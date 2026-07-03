/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Demonstrate how to trigger ADC conversions via Timer0 hardware.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
#define CONV_TOTAL 40	/* Total number of conversions for Burst Mode */

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag = 0;   								/* ADC conversion complete interrupt flag */
volatile uint32_t g_u32COVNUMFlag = 0;   								/* Accumulator for single conversions */
volatile uint32_t u32ConvCount;          								/* Conversion counter for Burst Mode */
volatile uint32_t i32ConversionData0 = 0; 							/* ADC data for single conversion */
volatile uint32_t i32ConversionData1[CONV_TOTAL] = {0}; /* Data buffer for Burst Mode */

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2);

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable ADC module clock */
    CLK_EnableModuleClock(ADC_MODULE);

    /* ADC clock source is PCLK1, set divider to 1 */
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL2_ADCSEL_PCLK1, CLK_CLKDIV0_ADC(1));

    /* Enable Timer 0 module clock */
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Select Timer 0 module clock source as HIRC */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*----------------------------------------------------------------------*/
    /* Init I/O Multi-function                                              */
    /*----------------------------------------------------------------------*/
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);
										
		/* Configure Test Pins: PA0 (for ADC conversion time) / PA1 (for Burst Mode duration) */
		GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);
		GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT);
		PA0 = 0;
		PA1 = 0;

    /* Set PB.0 - PB.15 to input mode */
    GPIO_SetMode(PB, BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT10|BIT11|BIT12|BIT13|BIT14|BIT15, GPIO_MODE_INPUT);
										 
		/* Configure the PB.0 - PB.7 ADC analog input pins */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk |
                                       SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk |
                                       SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk |
                                       SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB7MFP_Msk)) |
																			(SYS_GPB_MFPL_PB0MFP_ADC0_CH0 | SYS_GPB_MFPL_PB1MFP_ADC0_CH1 |
																			 SYS_GPB_MFPL_PB2MFP_ADC0_CH2 | SYS_GPB_MFPL_PB3MFP_ADC0_CH3 |
																			 SYS_GPB_MFPL_PB4MFP_ADC0_CH4 | SYS_GPB_MFPL_PB5MFP_ADC0_CH5 |
																		   SYS_GPB_MFPL_PB6MFP_ADC0_CH6 | SYS_GPB_MFPL_PB7MFP_ADC0_CH7);

    /* Configure the PB.8 - PB.15 ADC analog input pins */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk  | SYS_GPB_MFPH_PB9MFP_Msk  |
                                       SYS_GPB_MFPH_PB10MFP_Msk | SYS_GPB_MFPH_PB11MFP_Msk |
                                       //SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk |
                                       SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk)) |
																		  (SYS_GPB_MFPH_PB8MFP_ADC0_CH8   | SYS_GPB_MFPH_PB9MFP_ADC0_CH9   |
																			 SYS_GPB_MFPH_PB10MFP_ADC0_CH10 | SYS_GPB_MFPH_PB11MFP_ADC0_CH11 |
																	     //SYS_GPB_MFPH_PB12MFP_ADC0_CH12 | SYS_GPB_MFPH_PB13MFP_ADC0_CH13 |
																			 SYS_GPB_MFPH_PB14MFP_ADC0_CH14 | SYS_GPB_MFPH_PB15MFP_ADC0_CH15);
										
    /* Disable the PB.0 - PB.15 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT10|BIT11|BIT12|BIT13|BIT14|BIT15);

    /* Lock protected registers */
    SYS_LockReg();
}

void TIMER0_Init()
{
    /* Set timer0 periodic time-out frequency is 20KHz */
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 20000);

    /* Enable timer interrupt trigger ADC */
    TIMER_SetTriggerSource(TIMER0, TIMER_TRGSRC_TIMEOUT_EVENT);
    TIMER_SetTriggerTarget(TIMER0, TIMER_TRG_TO_ADC);
	
		/* Enable Timer0 interrupt and configure NVIC core interrupt */
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
}

void ADC_Burst_CH2(void)
{		
		/* Disable hardware trigger */ 
		ADC_DisableHWTrigger(ADC);

		/* Disable ADC interrupt to avoid ISR stealing ADF flag during burst polling */
		NVIC_DisableIRQ(ADC_IRQn);
		ADC_DISABLE_INT(ADC, ADC_ADF_INT);
	
		/* Set input mode as single-end, burst mode, and select channel 2 */
		ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_BURST, BIT2);

		/* Clear the A/D interrupt flag for safe */
		ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

		/* Reset the ADC interrupt indicator and trigger sample module to start A/D conversion */
		u32ConvCount = 0;
		ADC_START_CONV(ADC);

		while(1)
		{
				/* Wait ADC conversion completed */
				while (ADC_GET_INT_FLAG(ADC, ADC_ADF_INT) == 0);
				ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* clear ADF interrupt flag */

				/* Get the conversion result until VALIDF turns to 0 */
				while(ADC->ADSR0 & ADC_ADSR0_VALIDF_Msk)
				{
						/* Fetch conversion data from Channel 2 always and store into array */
						i32ConversionData1[u32ConvCount++] = ADC_GET_CONVERSION_DATA(ADC, 0);
						if(u32ConvCount == CONV_TOTAL)
								break;
				}

				/* Break out of outer loop if the target conversion total (40) is reached */
				if(u32ConvCount == CONV_TOTAL)
						break;
		}

		/* Stop A/D conversion */
		ADC_STOP_CONV(ADC);

		/* Show the conversion result */
		/*
		for(u32ConvCount = 0; u32ConvCount < CONV_TOTAL; u32ConvCount++)
		{
				printf("%d\n", i32ConversionData1[u32ConvCount]);
		}		
		*/

		/* Re-enable ADC interrupt for the timer-triggered single mode */
		ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
		ADC_ENABLE_INT(ADC, ADC_ADF_INT);
		NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_FunctionTest()
{
    uint8_t  u8Ch;

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                     ADC trigger by Timer test                        |\n");
    printf("+----------------------------------------------------------------------+\n");

    /* Enable ADC converter */
    ADC_POWER_ON(ADC);
		
		/* Enable TIMER0 */
		TIMER_Start(TIMER0);

    while(1)
    {
				/* Sequentially scan and sample 0 to 15 (all 16 ADC channels) */
				for(u8Ch = 0; u8Ch < 16; u8Ch++)
				{					
						/* Configure ADC: Single-ended input, Single Mode, and enable corresponding channel */
						ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_SINGLE, (1 << u8Ch));

						/* Enable hardware trigger source from Timer with delay clock set to 0 */
						ADC_EnableHWTrigger(ADC, ADC_ADCR_TRGS_TIMER, 0);

						/* Clear the A/D interrupt flag for safe */
						ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

						/* Enable the sample module interrupt */
						ADC_ENABLE_INT(ADC, ADC_ADF_INT);
						NVIC_EnableIRQ(ADC_IRQn);

						/* Reset the ADC indicator and enable Timer0 counter */
						g_u32AdcIntFlag = 0;
					
						/* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
						while(g_u32AdcIntFlag == 0);

						/* Get the conversion result of ADC channel */
						i32ConversionData0 = ADC_GET_CONVERSION_DATA(ADC, u8Ch);
						//printf("ADC[%02d] = %d\n",u8Ch, i32ConversionData0);
						
						/* Once 20000 single conversions have accumulated, run a Burst Mode test routine */
						if(g_u32COVNUMFlag >= 20000)	// 20KHz
						{
								/* [PA1 HIGH - START] Entering Burst Mode test */
								PA1 = 1;
							
								//printf("---\n");
								ADC_Burst_CH2();
							
								/* [PA1 LOW - END] Leaving Burst Mode test */
								PA1 = 0;
							
								/* Clear counter to restart accumulation loop */
								g_u32COVNUMFlag = 0;
						}
				}
				//printf("-----------------\n");
    }
}

/*----------------------------------------------------------------------*/
/* ADC Interrupt Service Routine (ISR)                                  */
/*----------------------------------------------------------------------*/
void ADC_IRQHandler(void)
{
		/* [PA0 LOW - END] Conversion finished, pull PA0 low immediately */
    PA0 = 0;
	
    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* Clear the A/D interrupt flag */
    g_u32AdcIntFlag = 1;                /* Notify main loop that conversion is ready */
    g_u32COVNUMFlag++;                  /* Accumulate conversion trigger counter */
}

/*----------------------------------------------------------------------*/
/* Timer 0 Interrupt Service Routine (ISR)                              */
/*----------------------------------------------------------------------*/
void TMR0_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER0) == 1)
    {
        /* Clear Timer0 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER0);
        
        /* [PA0 HIGH - START] Timer timeout occurs, ADC triggered simultaneously; pull PA0 high */
        PA0 = 1; 
    }
}

/*----------------------------------------------------------------------*/
/* Init UART0                                                           */
/*----------------------------------------------------------------------*/
void UART0_Init(void)
{
    /* Reset UART0 */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

int32_t main(void)
{
    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Init UART0 for printf */
    UART0_Init();

    /* Init TIMER0 for ADC */
    TIMER0_Init();

    printf("\nSystem clock rate: %d Hz", SystemCoreClock);

		/* Execute ADC sampling function test */
    ADC_FunctionTest();

    while(1);
}
