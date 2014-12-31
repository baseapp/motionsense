#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
// for WDIE
#include <avr/io.h>

#include <Arduino.h>
#include "power_save.h"
#include "config.h"

void wdt_enable_intonly()
{
	cli(); //disable global interrupts
	MCUSR = 0x00;  //clear all reset flags
	//set WD_ChangeEnable and WD_resetEnable to alter the register
	WDTCSR |= (1<<WDCE) | (1<<WDE);   // this is a timed sequence to protect WDTCSR
	WDTCSR = (1<<WDP3) | (1<<WDP0) | (1<<WDIE); //8s
	WDTCSR &= ~(1<<WDE);
	sei(); //enable global interrupts
}

void wdt_enable_reset()
{
	cli(); //disable global interrupts
	MCUSR = 0x00;  //clear all reset flags
	//set WD_ChangeEnable and WD_resetEnable to alter the register
	WDTCSR |= (1<<WDCE) | (1<<WDE);   // this is a timed sequence to protect WDTCSR
	WDTCSR = (1<<WDP3) | (1<<WDP0) | (1<<WDIE) | (1<<WDE); //8s int and reset
	// WDTCSR &= ~(1<<WDE);
	sei(); //enable global interrupts
}

void wdt_dis()
{
	cli(); //disable global interrupts
	MCUSR = 0x00;  //clear all reset flags
	//set WD_ChangeEnable and WD_resetEnable to alter the register
	WDTCSR |= (1<<WDCE) | (1<<WDE);   // this is a timed sequence to protect WDTCSR
	WDTCSR = (1<<WDP3) | (1<<WDP0) | (1<<WDIE); //8s int and reset
	WDTCSR &= ~(1<<WDE);
	sei(); //enable global interrupts	
}

// wdt_disable() is same as in libc

// wdt_reset() pet wdt

void low_power_mode()
{
#ifdef DEBUG
	Serial.println("low power mode");
#endif
	
	// Serial.flush();
	// pinMode(0, OUTPUT);
	// pinMode(1, OUTPUT);
	// digitalWrite(0, LOW);
	// digitalWrite(1, LOW);
	// power_usart0_disable();	

	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);

	pinMode(MISO, OUTPUT);
	pinMode(MOSI, OUTPUT);
	pinMode(SCK, OUTPUT);
	digitalWrite(MISO,LOW);
	digitalWrite(MOSI,LOW);
	digitalWrite(SCK,LOW);

	ADCSRA &= ~(1 << ADEN);
	power_adc_disable();

	ADCSRB &= ~(1<<6);     

	power_all_disable();
	
	power_usart0_enable();
}

void normal_mode()
{
	// digitalWrite(BOOST_REG_PIN, HIGH);

	power_timer0_enable();

	// pinMode(0, INPUT);
	// pinMode(1, OUTPUT);
	power_usart0_enable();	

	power_spi_enable();
	pinMode(MISO, INPUT);
	pinMode(MOSI, OUTPUT);
	pinMode(SCK, OUTPUT);

#ifdef DEBUG
	Serial.println("normal mode");
#endif
}
