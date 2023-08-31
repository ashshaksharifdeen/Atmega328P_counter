#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED1_PIN    PB0
#define LED2_PIN    PB1
#define LED3_PIN    PB2
#define LED4_PIN    PB3
#define BCD_PIN_1 PC0
#define BCD_PIN_2 PC1
#define BCD_PIN_3 PC2
#define BCD_PIN_4 PC3
#define BUTTON_PIN_1  PD2
#define BUTTON_PIN_2  PD3
#define SSD_Pin PD7
#define PWM_PIN PD6
#define PWM_PRESCALER 64
float PWM_TOP = 0;
#define BRIGHTNESS_LEVELS 10
#define Mode_Pin PD1
#define ocr_value = {0, 28, 57, 85, 113, 142, 170, 198, 227, 255};

volatile uint8_t mode = 0;

volatile uint8_t brightness = 0;

void init_PWM() {
	TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00); // Fast PWM, non-inverting mode
	TCCR0B |= (1 << CS01) | (1 << CS00); // Prescaler: 64 // Set initial PWM duty cycle to 0
}

void disable_PWM() {
	// Clear COM0A1 and WGM00 bits in TCCR0A to disable PWM output
	TCCR0A &= ~((1 << COM0A1) | (1 << WGM00));
	
	// Clear CS01 and CS00 bits in TCCR0B to stop the timer
	TCCR0B &= ~((1 << CS01) | (1 << CS00));
}

// Function to initialize Timer1 for debouncing buttons

volatile uint8_t count = 0;

void setup()
{
	DDRB |= (1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN) | (1 << LED4_PIN);
	DDRC |= (1 << BCD_PIN_1) | (1 << BCD_PIN_2) | (1 << BCD_PIN_3) | (1 << BCD_PIN_4);
	DDRD |= (1 << SSD_Pin);
	DDRD |= (1 << PWM_PIN);
	DDRD &= ~(1 << BUTTON_PIN_1);
	DDRD &= ~(1 << BUTTON_PIN_2);
	DDRD &= ~(1 << Mode_Pin);
	PORTD |= (1 << Mode_Pin); //enabling the pull up resistor
	PORTD |= (1 << BUTTON_PIN_1);
	PORTD |= (1 << BUTTON_PIN_2);
	//EIMSK |= (1 << INT0) | (1 << INT1); // enabling the INT0 and INT1 bit set high in register
	//EICRA |= (1 << ISC01) | (1 << ISC11); //The falling edge of INT0 generates an interrupt request.
	
	
	//sei();
}

/*ISR(INT0_vect)
{
	_delay_ms(80);
	count++;
	if (count > 9){
		count=0;
	}
}
ISR(INT1_vect)
{
	_delay_ms(60);
	count--;
	if (count == -1){
		count=9;
	}
	if (count > 9){
		count=0;
	}
}*/

uint8_t isSwitchPressed() {
	// Check the status of the SWITCH_PIN (bit 1 in PIND register)
	// 0 means the switch is pressed because of the pull-up resistor
	// 1 means the switch is not pressed (external pull-up resistor required)
	return (PIND & (1 << Mode_Pin));
}
uint8_t isSwitchincrease() {
	// Check the status of the SWITCH_PIN (bit 1 in PIND register)
	// 0 means the switch is pressed because of the pull-up resistor
	// 1 means the switch is not pressed (external pull-up resistor required)
	return (PIND & (1 << BUTTON_PIN_1));
}
uint8_t isSwitchdecrease() {
	// Check the status of the SWITCH_PIN (bit 1 in PIND register)
	// 0 means the switch is pressed because of the pull-up resistor
	// 1 means the switch is not pressed (external pull-up resistor required)
	return (PIND & (1 << BUTTON_PIN_2));
}
void clearBitInPortD(uint8_t bitPosition) {
	// Check if bitPosition is valid (0-7)
	if (bitPosition >= 0 && bitPosition <= 7) {
		// Create a mask with all bits set to 1 except for the bit at the given position
		//if it ssd common anode set to high not inverse it
		uint8_t mask = ~(1 << bitPosition);
		
		// Perform bitwise AND with the mask to clear the bit at the given position
		PORTD &= mask;
	}
}
int main(void)
{
	setup();

	// Initialize Timer1 for button debouncing
	while (1)
	{
		if ((isSwitchPressed())) {
			// Switch is pressed, do something
			// For example, you can toggle an LED connected to another pin
			// Assuming the LED is connected to PB0:
			mode ++;
			_delay_ms(150);
			// Add a small delay to avoid rapid toggling due to switch bouncing
		}
		if((isSwitchincrease()))
		{
			count++;
			_delay_ms(50);
			if (count > 9){
				count=0;
			}	
		}
		if((isSwitchdecrease()))
		{
			count--;
			_delay_ms(50);
			if (count == -1){
				count=9;
			}
			if (count > 9){
				count=0;
			}
		}
		
		if ((mode%3)==0){
			disable_PWM();
			PORTB = (count & 0x0F);
			PORTC = (0x00);
			PORTD|=(1<<SSD_Pin);
			PORTD &= ~(1 << PWM_PIN);
		}
		
		if ((mode%3)==1){
			
			disable_PWM();
			clearBitInPortD(7);
			PORTC = (count & 0xFF);
			PORTB = (0x00);
			PORTD &= ~(1 << PWM_PIN);
		}
		
		if ((mode%3)==2){
			if (count==0){
				disable_PWM();
				PORTD|=(1<<SSD_Pin);
				PORTD |= (1<<PWM_PIN);
				PORTB = (0x00);
				PORTC = (0x00);
				PORTD &= ~(1 << PWM_PIN);
			}
			else{
				init_PWM();
				PORTD|=(1<<SSD_Pin);
				PORTD |= (1<<PWM_PIN);
				OCR0A = (255/BRIGHTNESS_LEVELS)*count;
				PORTB = (0x00);
				PORTC = (0x00);
			}
		}
		
		//PORTD |= (1<<SSD_Pin);
		//PWM_TOP = ((F_CPU / (PWM_FREQ * PWM_PRESCALER)) - 1);
		
		//OCR0A = 50+(((PWM_TOP-50) / BRIGHTNESS_LEVELS) * count);
	}
}