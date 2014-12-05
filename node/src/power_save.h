#ifndef POWER_SAVE_H
#define POWER_SAVE_H

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
// for WDIE
#include <avr/io.h>

void wdt_enable_intonly();
void low_power_mode();
void normal_mode();

#endif