#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <time.h>
#include <stdlib.h>

#define MCLK_FREQUENCY 3000000
#define PWM_PERIOD (MCLK_FREQUENCY/5000)

volatile unsigned int S1buttonDebounce = 0; // Deboounce state for button S1
volatile unsigned int S2buttonDebounce = 0; // Deboounce state for button S1

volatile int oldTick = 0;            // SysTick value of previous tap
volatile int newTick = 0;            // SysTick value of current tap

volatile int counting = 0;           // Whether currently counting taps and interval between taps
volatile int count = 0;              // Counts # of seconds elapsed since last tap
volatile int sysTickCount = 0;       // Counts # of SysTick interrupts since last tap
volatile int taps = 0;               // Counts # of taps from last reset

/* Current color of the blinking RGB LED
 * 4 possible states: ALL OFF, R, G, B, ALL ON */
volatile unsigned int currentColor = 0;

#define RED     255
#define GREEN   255
#define BLUE    255

/* TimerA UpMode Configuration Parameter */
const Timer_A_UpModeConfig upConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // SMCLK/1 = 3MHz
        45000,                                  // 15ms debounce period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

/* Port mapper configuration register */
const uint8_t port_mapping[] =
{
    //Port P2:
    PM_TA0CCR1A, PM_TA0CCR2A, PM_TA0CCR3A, PM_NONE, PM_TA1CCR1A, PM_NONE, PM_NONE, PM_NONE
};

/*
 * Main function
 */
int main(void)
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    /* Seed the pseudo random num generator */
    srand(TLV->RANDOM_NUM_1);

    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initialize main clock to 3MHz */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_3);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );

    /* GPIO Setup for Pins 2.0-2.2 */
    MAP_PMAP_configurePorts((const uint8_t *) port_mapping, PMAP_P2MAP, 1,
        PMAP_DISABLE_RECONFIGURATION);

    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,
        GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Confinguring P1.1 & P1.4 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4, GPIO_HIGH_TO_LOW_TRANSITION);

    /* Configure TimerA0 without Driverlib (CMSIS style register access) */
    TIMER_A0->CCR[0] = PWM_PERIOD;
    TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;                      // CCR1 reset/set
    TIMER_A0->CCTL[2] = TIMER_A_CCTLN_OUTMOD_7;                      // CCR2 reset/set
    TIMER_A0->CCTL[3] = TIMER_A_CCTLN_OUTMOD_7;                      // CCR3 reset/set
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP | TIMER_A_CTL_CLR;  // SMCLK, up mode, clear TAR

    /* Configuring TimerA1 and TimerA2 for Up Mode  using Driverlib*/
    MAP_Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Configure and enable SysTick */
    MAP_SysTick_setPeriod(3000000);

    /* Main while loop */
    while(1)
    {
        MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
        MAP_Interrupt_enableInterrupt(INT_PORT1);
        MAP_Interrupt_enableInterrupt(INT_TA1_0);
        MAP_Interrupt_enableMaster();
        MAP_PCM_gotoLPM0();
    }
}

/*
 * Port 1 interrupt handler. This handler is called whenever switches attached
 * to P1.1 (S1) and P1.4 (S2) are pressed.
 */
void PORT1_IRQHandler(void)
{
    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    /* Handles S2 button press */
    if (status & GPIO_PIN4)
    {
        if (S2buttonDebounce == 0)
        {
            S2buttonDebounce = 1;

            MAP_Interrupt_disableInterrupt(INT_PORT1);

            /* Cycle through R, G, B, random color */
            if (!(P2->IN & (BIT0 | BIT1 | BIT2))) {
                if (currentColor < 4)
                    currentColor++;
                else
                    currentColor = 0;
            }

            switch(currentColor)
            {
                case 0:  // All off
                    TIMER_A0->CCR[1] = 0;      // CCR1 PWM duty cycle
                    TIMER_A0->CCR[2] = 0;      // CCR2 PWM duty cycle
                    TIMER_A0->CCR[3] = 0;      // CCR3 PWM duty cycle
                    break;
                case 1:  // Red
                    TIMER_A0->CCR[1] = PWM_PERIOD * (RED/255);              // CCR1 PWM duty cycle
                    TIMER_A0->CCR[2] = PWM_PERIOD * (0/255);                // CCR2 PWM duty cycle
                    TIMER_A0->CCR[3] = PWM_PERIOD * (0/255);                // CCR3 PWM duty cycle
                    break;
                case 2:  // Green
                    TIMER_A0->CCR[1] = PWM_PERIOD * (0/255);                // CCR1 PWM duty cycle
                    TIMER_A0->CCR[2] = PWM_PERIOD * (GREEN/255);            // CCR2 PWM duty cycle
                    TIMER_A0->CCR[3] = PWM_PERIOD * (0/255);                // CCR3 PWM duty cycle
                    break;
                case 3:  // Blue
                    TIMER_A0->CCR[1] = PWM_PERIOD * (0/255);                // CCR1 PWM duty cycle
                    TIMER_A0->CCR[2] = PWM_PERIOD * (0/255);                // CCR2 PWM duty cycle
                    TIMER_A0->CCR[3] = PWM_PERIOD * (BLUE/255);             // CCR3 PWM duty cycle
                    break;
                case 4:  // All on
                    TIMER_A0->CCR[1] = PWM_PERIOD * (RED/255);      // CCR1 PWM duty cycle
                    TIMER_A0->CCR[2] = PWM_PERIOD * (GREEN/255);      // CCR2 PWM duty cycle
                    TIMER_A0->CCR[3] = PWM_PERIOD * (BLUE/255);      // CCR3 PWM duty cycle
                    break;
                default:
                    break;
            }

            MAP_SysTick_enableModule();
            MAP_SysTick_enableInterrupt();

            /* Start button debounce timer */
            MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
        }
    }
}

/*
 * Timer A1 interrupt handler. This handler determines whether to reset button
 * debounce after debounce timer expires.
 */
void TA1_0_IRQHandler(void)
{
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    if (P1IN & GPIO_PIN1)
    {
        S1buttonDebounce = 0;
    }
    if (P1IN & GPIO_PIN4)
    {
        S2buttonDebounce = 0;
    }

    if ((P1IN & GPIO_PIN1) && (P1IN & GPIO_PIN4))
    {
        MAP_Timer_A_stopTimer(TIMER_A1_BASE);
    }
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
                TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

/*
 * SysTick interrupt handler. This handler toggles RGB LED on/off.
 */
void SysTick_Handler(void)
{
    /* Toggle RGB LED OFF */
    if (TIMER_A0->CCTL[1] & TIMER_A_CCTLN_OUTMOD_7){
        MAP_SysTick_disableModule();
        TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_0;
        TIMER_A0->CCTL[2] = TIMER_A_CCTLN_OUTMOD_0;
        TIMER_A0->CCTL[3] = TIMER_A_CCTLN_OUTMOD_0;
    }
    /* Toggle RGB LED ON */
    else if (!(TIMER_A0->CCTL[1] & TIMER_A_CCTLN_OUTMOD_0)){
        TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;
        TIMER_A0->CCTL[2] = TIMER_A_CCTLN_OUTMOD_7;
        TIMER_A0->CCTL[3] = TIMER_A_CCTLN_OUTMOD_7;
    }
}
