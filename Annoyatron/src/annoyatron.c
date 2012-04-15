// Sergio Campama 2011
// http://kaipi.me
//
// Made for the TI MSP430 LaunchPad
// http://ti.com/launchpadwiki
//
// Heavily based on http://www.43oh.com/forum/viewtopic.php?f=10&t=1159

#include <msp430.h>
#include <inttypes.h>

//In uniarch there is no more signal.h to sugar coat the interrupts definition, so we do it here
#define interrupt(x) void __attribute__((interrupt (x)))

//Defines to help understand what is attached to each port
#define LED_DIR P1DIR
#define LED_OUT P1OUT

//P1.0 is the red LED
#define LED_RED BIT0

//Piezo defines
//Here it is defined as BIT6 to see the results on the LED, but should be changed to the bit where it is
//connected the piezo
#define PIEZO BIT6
#define PIEZO_DIR P1DIR
#define PIEZO_SEL P1SEL
#define PIEZO_OUT P1OUT

interrupt(TIMERA1_VECTOR) serviceTimerA(void);

void timer_init()
{
	//Set DCO to 1 MHz
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

        /* Initialise Timer_A */
        TACTL = TACLR;
        TACTL |= TASSEL_2; // SMCLK clock source
        TACTL |= ID_0; // Divide input clock by 1
        TACTL |= MC_1; // Timer up to CCR0 mode
        TACCR0 = 980; // Tuned to interrupt at 1ms intervals for 1Mhz timer
        TACTL |= TAIE; // Enable interrupt

}

void led_init()
{
	//Set the direction as output
	LED_DIR |= LED_RED;
}

void init()
{
        WDTCTL = WDTPW + WDTHOLD;	//Stop the watchdog

        led_init();
        timer_init();

        TACTL |= MC_1;  //Sets the TimerA count mode in UP, and thus starting the Timer

        _BIS_SR(GIE);//Enable global interrupts
}

//This function delivers a uniform random variable between 20 and 899
uint64_t random_seed = 65738265;
uint16_t random_uniform()
{
	random_seed = (16807*random_seed) % (2147483647LLU);
	return random_seed % 880 + 20;
}

int main()
{
	init();

        uint16_t delay = 0;
        uint16_t waittime = 0;
        uint16_t playtime = 0;

        enum state {playing, waiting};
        enum state state = playing;

        while (1) {
                switch (state) {
                case (waiting):
                        TACCR0 = 49147; // 50ms
                        if (++waittime >= delay) {
                                state = playing;
                                playtime = 0;
                        }
                        break;
                case (playing):
                        if (playtime++ > 1000) { // Tone length (periods)
                                TACCR0 = 49147; // 50ms
                                delay = 20*random_uniform(); // Decide when to next play
                                state = waiting;
                                waittime = 0;
                        } else {
                                TACCR0 = 97; // Tone frequency (not in HZ)
                                LED_OUT ^= LED_RED;
                        }
                        break;
                }
                LPM1; // Put the device into sleep mode 1
        }
	return 0;
}

interrupt(TIMERA1_VECTOR) serviceTimerA(void)
{
        // Clear TimerA interrupt flag
        TACTL &= ~TAIFG;
        // Exit low power mode
        LPM1_EXIT;
}

