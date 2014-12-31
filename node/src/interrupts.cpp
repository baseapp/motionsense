#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
// for WDIE
#include <avr/io.h>

#include <Arduino.h>
#include "config.h"

void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void enable_button_pci()
{
  cli();
  *digitalPinToPCMSK(BUTTON_INT_PIN_AN) |= bit (digitalPinToPCMSKbit(BUTTON_INT_PIN_AN));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(BUTTON_INT_PIN_AN)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(BUTTON_INT_PIN_AN)); // enable interrupt for the group
  sei();
}

void disable_button_pci()
{
  cli();
  *digitalPinToPCMSK(BUTTON_INT_PIN_AN) &= ~ bit (digitalPinToPCMSKbit(BUTTON_INT_PIN_AN));  // enable pin
  PCIFR  &= ~ bit (digitalPinToPCICRbit(BUTTON_INT_PIN_AN)); // clear any outstanding interrupt
  PCICR  &= ~ bit (digitalPinToPCICRbit(BUTTON_INT_PIN_AN)); // enable interrupt for the group
  sei();
}