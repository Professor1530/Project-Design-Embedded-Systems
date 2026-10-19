/*
 * Experiment 1 - LCD Display with ADC
 * ATmega328P - LM35 Temperature Sensor and Tank Level Monitor
 * PROTEUS SIMULATION VERSION
 * 
 * Hardware Connections (from your schematic):
 * LCD (4-bit mode) - LM016L:
 *   - VSS (Pin 1) -> GND
 *   - VDD (Pin 2) -> +5V
 *   - V0  (Pin 3) -> Middle of 10K pot (ends to +5V and GND) for contrast
 *   - RS  (Pin 4) -> PB5 (Arduino D13)
 *   - RW  (Pin 5) -> GND (IMPORTANT!)
 *   - EN  (Pin 6) -> PB4 (Arduino D12)
 *   - D0-D3 -> Not connected (4-bit mode)
 *   - D4  (Pin 11) -> PB0 (Arduino D8)
 *   - D5  (Pin 12) -> PB1 (Arduino D9)
 *   - D6  (Pin 13) -> PB2 (Arduino D10)
 *   - D7  (Pin 14) -> PB3 (Arduino D11)
 *   - A   (Pin 15) -> +5V (backlight anode)
 *   - K   (Pin 16) -> GND (backlight cathode)
 * 
 * LM35 Temperature Sensor:
 *   - VOUT -> PC0 (ADC0/Pin 23)
 *   - VCC  -> +5V
 *   - GND  -> GND
 * 
 * Variable Resistor (Tank Level):
 *   - Wiper -> PC1 (ADC1/Pin 24)
 *   - One end -> +5V
 *   - Other end -> GND
 * 
 * ATmega328P:
 *   - Pin 7 (VCC) -> +5V
 *   - Pin 8 (GND) -> GND
 *   - Pin 20 (AVCC) -> +5V 
 *   - Pin 21 (AREF) -> 100nF capacitor to GND
 *   - Pin 22 (GND) -> GND
 
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// LCD Pin Definitions - All on Port B
#define LCD_RS PB5
#define LCD_EN PB4
#define LCD_D4 PB0
#define LCD_D5 PB1
#define LCD_D6 PB2
#define LCD_D7 PB3

// Function Prototypes
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_String(const char *str);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_Enable_Pulse(void);
void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);
void Display_Temperature(uint16_t adc_value);
void Display_TankLevel(uint16_t adc_value);

int main(void)
{
	
    uint16_t temp_adc, tank_adc;
    
    // Small delay for power stabilization
    _delay_ms(100);
    
    // Initialize LCD
    LCD_Init();
    
    // Initialize ADC
    ADC_Init();
    
    _delay_ms(100);
    
    // Display static text
    LCD_SetCursor(0, 0);
    LCD_String("Temp =");
    
    LCD_SetCursor(1, 0);
    LCD_String("T_Litres =");
    
    while(1)
    {
        // Read temperature from ADC0 (using 1.1V reference)
        ADMUX = (1 << REFS1) | (1 << REFS0) | 0; // Internal 1.1V, ADC0
        _delay_ms(20); // Allow reference to stabilize
        temp_adc = ADC_Read(0);
        Display_Temperature(temp_adc);
        
        // Read tank level from ADC1 (using AVCC reference)
        ADMUX = (1 << REFS0) | 1; // AVCC reference, ADC1
        _delay_ms(20); // Allow reference to stabilize
		ADC_Read(1);
        tank_adc = ADC_Read(1);
        Display_TankLevel(tank_adc);
        
        _delay_ms(200); // Update rate - slower for better visibility
    }
    
    return 0;
}

void LCD_Init(void)
{
    // Set LCD pins as output (All on Port B)
    DDRB |= (1 << LCD_RS) | (1 << LCD_EN) | (1 << LCD_D4) | 
            (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);
    
    // Ensure all pins start LOW
    PORTB &= ~((1 << LCD_RS) | (1 << LCD_EN) | (1 << LCD_D4) | 
               (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));
    
    _delay_ms(50); // Wait for LCD to power up (important for Proteus)
    
    // LCD Initialization Sequence for 4-bit mode
    // Send 0x30 three times
    PORTB = (PORTB & 0xF0) | 0x03; // 0x30 upper nibble = 0x03
    LCD_Enable_Pulse();
    _delay_ms(5);
    
    PORTB = (PORTB & 0xF0) | 0x03;
    LCD_Enable_Pulse();
    _delay_us(150);
    
    PORTB = (PORTB & 0xF0) | 0x03;
    LCD_Enable_Pulse();
    _delay_us(150);
    
    // Set to 4-bit mode
    PORTB = (PORTB & 0xF0) | 0x02; // 0x20 upper nibble = 0x02
    LCD_Enable_Pulse();
    _delay_us(150);
    
    // Now use LCD_Command for the rest
    LCD_Command(0x28); // 4-bit mode, 2 lines, 5x8 font
    LCD_Command(0x0C); // Display ON, cursor OFF, blink OFF
    LCD_Command(0x01); // Clear display
    _delay_ms(2);
    LCD_Command(0x06); // Entry mode: increment cursor, no shift
    _delay_ms(2);
}
void LCD_Command(unsigned char cmd)
{
	PORTB &= ~(1 << LCD_RS); // RS = 0 for command

	// Send upper nibble (bits 7–4) ? PB0–PB3
	PORTB = (PORTB & 0xF0) | ((cmd >> 4) & 0x0F);
	LCD_Enable_Pulse();

	// Send lower nibble (bits 3–0) ? PB0–PB3
	PORTB = (PORTB & 0xF0) | (cmd & 0x0F);
	LCD_Enable_Pulse();

	_delay_us(50);
}

void LCD_Char(unsigned char data)
{
	PORTB |= (1 << LCD_RS); // RS = 1 for data

	// Send upper nibble
	PORTB = (PORTB & 0xF0) | ((data >> 4) & 0x0F);
	LCD_Enable_Pulse();

	// Send lower nibble
	PORTB = (PORTB & 0xF0) | (data & 0x0F);
	LCD_Enable_Pulse();

	_delay_us(50);
}

void LCD_String(const char *str)
{
    while(*str)
    {
        LCD_Char(*str++);
    }
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char position;
    
    if(row == 0)
        position = 0x80 + col; // First line starts at 0x80
    else
        position = 0xC0 + col; // Second line starts at 0xC0
    
    LCD_Command(position);
}

void LCD_Enable_Pulse(void)
{
    PORTB |= (1 << LCD_EN);   // EN = 1
    _delay_us(1);             // Enable pulse width
    PORTB &= ~(1 << LCD_EN);  // EN = 0
    _delay_us(50);            // Command execution time
}

void ADC_Init(void)
{
    // ADC Enable, prescaler = 128 for 16MHz (125kHz ADC clock)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // Disable digital input on ADC pins to reduce noise
    DIDR0 = (1 << ADC0D) | (1 << ADC1D);
    
    // Perform a dummy conversion to initialize ADC
    ADMUX = (1 << REFS0); // AVCC reference
    _delay_ms(10);
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
}

uint16_t ADC_Read(uint8_t channel)
{
	//TESTING CODE
	// Select channel (0–7)
	//ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADMUX = (ADMUX & 0xF0) | (channel & 0x07);
	
	_delay_ms(10);

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete
	while(ADCSRA & (1 << ADSC));

    return ADC;
}

void Display_Temperature(uint16_t adc_value)
{
    // LM35 with 1.1V reference:
    // LM35 outputs 10mV per degree Celsius
    // With 1.1V reference: Temperature = (ADC * 1.1 * 100) / 1024
    // Simplified: temp = (ADC * 110) / 1024
    
   /* uint32_t temp_calc = ((uint32_t)adc_value * 1100UL) / 1024UL;
    uint16_t temp_whole = temp_calc / 10;
    uint16_t temp_decimal = temp_calc % 10;
    
    char buffer[10];
    // %c with 0xDF prints the degree symbol
    sprintf(buffer, "%2d.%d%cC ", temp_whole, temp_decimal, 0xDF);
    
    LCD_SetCursor(0, 7); // Start printing right after "Temp ="
    LCD_String(buffer);*/
   
   uint32_t temp_calc = ((uint32_t)adc_value * 1100UL) / 1024UL;
   
   LCD_SetCursor(0, 7);
   char buf[10];
   // Manual formatting is safer than sprintf if the linker is acting up
   int whole = temp_calc / 10;
   int decimal = temp_calc % 10;
   
   LCD_Char((whole / 10) + '0'); // Tens
   LCD_Char((whole % 10) + '0'); // Units
   LCD_Char('.');
   LCD_Char(decimal + '0');      // Tenths
   LCD_Char(0xDF);               // Degree symbol
   LCD_Char('C');
   LCD_String("  ");             // Clear trailing characters
}

void Display_TankLevel(uint16_t adc_value)
{
    // Tank level: 0-5V represents 0-3000 litres
    // With 10-bit ADC: Litres = (ADC * 3000) / 1023
    
   // Calculation for 0-3000 litres
  /* uint32_t litres = ((uint32_t)adc_value * 3000UL) / 1023UL;
   
   char buffer[10];
   // Adding 'L' after the number and spaces to clear old digits
   sprintf(buffer, "%4luL   ", (unsigned long)litres);
   
   LCD_SetCursor(1, 10); // Start printing right after "T_Litres ="
   LCD_String(buffer);*/
  
  uint32_t litres = ((uint32_t)adc_value * 3000UL) / 1023UL;
  
  LCD_SetCursor(1, 11);
  char buf[10];
  sprintf(buf, "%luL    ", litres); // If sprintf works, use it; if not, use manual
  LCD_String(buf);
}

