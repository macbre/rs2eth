#include "ds18b20.h"

volatile signed int ds18b20_temp;

void ds18b20_init(unsigned char resolution) {
    
    // ustaw rozdzielczosc pomiaru temperatur i zazadaj pierwszego pomiaru
    //ds18b20_set_resolution(DS18B20_RESOLUTION_11_BITS); // 0.125C
    //ds18b20_set_resolution(DS18B20_RESOLUTION_12_BITS); // 0.0625C
    ds18b20_set_resolution(resolution);
    ds18b20_request_measure();
}

// wysy³a zadanie dokonania pomiaru temperatury i zapisu
// wyniku w pamieci Scrachpad
void ds18b20_request_measure()
{
    ow_reset();
    ow_write(0xcc); // skip ROM (zadanie do wszystkich)
    ow_write(0x44); // pomiar

    return;
}

// pobierz temperature z podanego czujnika
//
// np. temp. 25.4C odczytywana jest jako 254 (zakres odczytow (-550 ; +1250)
//
// odczyt 2000 oznacza b³¹d!
//
signed int ds18b20_get_temperature()
{
    unsigned int tmp;   // zmienna pomocnicza przy odczycie rejestrow ds18b20
    signed int temp;   // temperatura

    ow_reset();

    ow_write(0xcc); // skip ROM (zadanie do wszystkich)

    ow_write(0xbe); // read scratchpad

    // odczytaj temperature z rejestrow ds18b20
    tmp = ow_read() | ((unsigned int)ow_read() << 8);

    ow_reset(); // odczytalismy juz potrzebne dane (niech ds18b20 nie przesyla wiecej danych)

    // b³¹d na szynie
    if ( (tmp == 0xffff) || (tmp == 0x0000) ) {
        return 2000;
    }

    // test
    //tmp = 0xfc90;   // -55
    //tmp = 0xfe6f;   // -25.0625
    //tmp = 0xff5e;   // -10.125
    //tmp = 0xfff8;   // -0.5
    //tmp = 0x0008;   // 0.5
    //tmp = 0x00a2;   // 10.125
    //tmp = 0x0191;   // 25.0625
    //tmp = 0x07D0;   // 125

    //
    // przelicz temperature
    //

    // najpierw wartosci calkowita stopni
    if (tmp >> 12 == 0x0f) {
        temp = ((0xffff - tmp + 0x00001) >> 4); // temperatura ujemna -> przelicz
    }
    else {
        temp = (tmp >> 4);
    }

    temp *= 10;

    // czesc ulamkowa
    temp += (tmp & 0b1000) ? 5 : 0; // 0.5
    temp += (tmp & 0b0100) ? 3 : 0; // 0.25
    temp += (tmp & 0b0010) ? 2 : 0; // 0.125
    temp += (tmp & 0b0001) ? 1 : 0; // 0.0625


    if (tmp >> 12 == 0x0f) {
        temp *= -1; // temperatura ujemna -> przelicz
    }

    return temp;
}

// ustaw rozdzielczosc pomiaru temperatur
void ds18b20_set_resolution(unsigned char res)
{
    unsigned char conf = 0b00011111;

    conf |= res << 5;

    ow_reset();

	ow_write(0x4E); // konfiguracja DS18B20
	ow_write(0x00); // T_l
	ow_write(0x00); // T_h
	ow_write(conf); // bajt konfiguracyjny

    ow_reset();

    return;
}

// ustaw zakresy wyzwalania alarmow podanego czujnika
void ds18b20_set_triggers(signed char tl, signed char th)
{
    ow_reset();

    ow_write(0xcc); // skip ROM (zadanie do wszystkich)

	ow_write(0x4E); // konfiguracja DS18B20
	ow_write(0x00); // T_l
	ow_write(0x00); // T_h
    ow_reset();     // nie przesylamy juz bajtu konfiguracyjnego
}
