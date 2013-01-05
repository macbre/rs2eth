#ifndef _DS18B20_H
#define _DS18B20_H

#include "../rs_eth.h"

extern volatile signed int ds18b20_temp;

// inicjalizacja czujnika DS18B20 z zadana rozdzielczoscia pomiarow
void ds18b20_init(unsigned char);

// wysy³a zadanie dokonania pomiaru temperatury i zapisu
// wyniku w pamieci Scrachpad
void ds18b20_request_measure();

// pobierz temperature z czujnika
signed int ds18b20_get_temperature();

// ustaw rozdzielczosc pomiaru temperatur
void ds18b20_set_resolution(unsigned char);

// ustaw zakresy wyzwalania alarmow czujnika
void ds18b20_set_triggers(signed char, signed char);

// rozdzielczoœci                           // precyzja     maksymalny czas pomiaru
#define DS18B20_RESOLUTION_9_BITS    0b00   // 0.5C         93.75 ms
#define DS18B20_RESOLUTION_10_BITS   0b01   // 0.25C        187.5 ms
#define DS18B20_RESOLUTION_11_BITS   0b10   // 0.125C       375 ms
#define DS18B20_RESOLUTION_12_BITS   0b11   // 0.0625C      750 ms

#endif
