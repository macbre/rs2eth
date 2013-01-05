#ifndef _RS_ETH_H
#define _RS_ETH_H

#define F_CPU 20000000UL      // 20 MHz

// zmienna pomocnicza
extern unsigned char i;

// timer - odliczanie okresów jednej sekundy
extern volatile unsigned int timer;
void init_timer();
void timer_tick();

// konfiguracja I/O wybranego uC
#include <avr/io.h>

// opóŸnienia
#include <util/delay.h>

// przerwania
#include <avr/interrupt.h>

// abs() / random() / itoa()
#include <stdlib.h>

// ctype
#include <ctype.h>

// sta³e w pamiêci FLASH
#include <avr/pgmspace.h>

// watchdog
#include <avr/wdt.h>

// EEPROM uC
#include <avr/eeprom.h>

// reset AVR'a
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

#define soft_reset()        \
do                          \
{                           \
    wdt_init();             \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)

// sygnaturka oprogramowania - wersja / data kompilacji
extern const char RS2ETH_SIGNATURE[] PROGMEM;

// stan serwera
extern unsigned char server_state;

#define SERVER_STATE_TELNET             1   // pod³¹czony klient telnetowy
#define SERVER_STATE_DHCP               2   // w trakcie pobierania adresu IP via DHCP (mruga dioda IP)
#define SERVER_STATE_DHCP_COMPLETED     4   // pobrano adres IP via DHCP
#define SERVER_STATE_DHCP_ERROR         8   // b³¹d w trakcie pobierania adresu IP via DHCP - przekroczony czas
#define SERVER_STATE_CLIENT_MODE        16  // w trybie klienta (³¹czenie z serwerem - mruga dioda TELNET - szybko)
#define SERVER_STATE_CLIENT_MODE_CONN   32  // w trybie klienta (po³¹czony z serwerem - mruga dioda TELNET - wolno)
#define SERVER_STATE_CONFIG_MODE        64  // w trybie konfiguracyjnym

// tryb pracy
#define MODE_SERVER     0
#define MODE_CLIENT     1

//
// ustawienia dla wyprowadzeñ AVR brd I
//
//      UWAGA!!!
//
//          Zakomentuj w przypadku korzystania z "gotowej" wersji p³ytki
//
//      UWAGA!!!
//
//#define RS2ETH_PROTOTYPE    1


//
// LCD
//

#ifndef RS2ETH_PROTOTYPE

//
// wersja "produkcyjna"
//

// RS (PA2)
#define LCD_RS_PORT PORTA
#define LCD_RS_BIT  2

// E (PA3)
#define LCD_E_PORT PORTA
#define LCD_E_BIT  3

// dane (PA4-PA7)
#define LCD_D4_PORT PORTA
#define LCD_D4_BIT  4
#define LCD_D5_PORT PORTA
#define LCD_D5_BIT  5
#define LCD_D6_PORT PORTA
#define LCD_D6_BIT  6
#define LCD_D7_PORT PORTA
#define LCD_D7_BIT  7

// 1wire (PB3)
#define OW_PORT PORTB
#define OW_PIN  3
#define DS_DEVICES_MAX 1

// SPI
#define SPI_PORT     PORTB
#define SPI_SS_PIN   4
#define SPI_MOSI_PIN 5
#define SPI_MISO_PIN 6
#define SPI_SCK_PIN  7

// SPI (CS dla ENC28) - PB4
#define ENC28_SS_PORT       PORTB
#define ENC28_SS_PIN        4

// RS485 - jako dodatkowa p³ytka
#define RS_485
#define RS_485_DIR_PORT PORTC
#define RS_485_DIR_PIN  0

#define RS_DUMP 1

#else

//
// wersja prototypowa
//

// RS
#define LCD_RS_PORT PORTC
#define LCD_RS_BIT  7

// E
#define LCD_E_PORT PORTC
#define LCD_E_BIT  6

// dane (D4-D7)
#define LCD_D4_PORT PORTA
#define LCD_D4_BIT  4
#define LCD_D5_PORT PORTA
#define LCD_D5_BIT  5
#define LCD_D6_PORT PORTA
#define LCD_D6_BIT  6
#define LCD_D7_PORT PORTA
#define LCD_D7_BIT  7

// 1wire (PD7)
#define OW_PORT PORTD
#define OW_PIN  7
#define DS_DEVICES_MAX 1

// SPI
#define SPI_PORT     PORTB
#define SPI_SS_PIN   4
#define SPI_MOSI_PIN 5
#define SPI_MISO_PIN 6
#define SPI_SCK_PIN  7

// SPI (CS dla ENC28)
#define ENC28_SS_PORT       PORTB
#define ENC28_SS_PIN        3

// RS485 - jako dodatkowa p³ytka
#define RS_485
#define RS_485_DIR_PORT PORTC
#define RS_485_DIR_PIN  0

#endif

// MAC
extern unsigned char mac[6];

//
// biblioteki obs³ugi peryferiów
//

// LCD 2x16 alfanumeryczny
#include "lib/lcd.h"

// RS232
#include "lib/rs.h"
#include "lib/com_buf.h"

// konfiguracja zapisana w pamiêci EEPROM uC
#include "lib/config.h"

// DS...
#include "lib/1wire.h"
#include "lib/ds18b20.h"

// SPI + FLASH/EEPROM + ENC28J60
#include "lib/spi.h"
#include "lib/enc28.h"
#include "lib/nic.h" // sterownik uk³adu ENC28J60

//
// stos uIP
//

// Ÿród³a
#include "uip/uip.h"
#include "uip/uip_arp.h"

// protoko³y
#include "proto/vcom.h"         // "tunelowanie" COM po TCP/IP (ponticulus)
#include "proto/telnet.h"       // wirtualny COM via telnet (com2tcp, HW Virtual Serial Port, ...)
#include "proto/http.h"         // serwer WWW (panel konfiguracyjny)
#include "proto/dhcp.h"         // DHCP (autokonfiguracja warstwy sieciowej - IP, maska, brama)
#include "proto/udp_config.h"   // pakiety broadcast UDP wykrywaj¹ce obecnoœæ / konfiguruj¹ce urz¹dzenia w lokalnej sieci
#include "proto/announce.h"     // pakiety broadcast UDP wykrywaj¹ce urz¹dzenia w lokalnej sieci (Microchip Ethernet Device Discovery)

//
// przydatne makra
//
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~(1<<(bit)))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= (1<<(bit)))

#define DDR(x) (_SFR_IO8(_SFR_IO_ADDR(x)-1))
#define PIN(x) (_SFR_IO8(_SFR_IO_ADDR(x)-2))

#define nop() __asm__ __volatile__ ("nop")

#define dec2hex(val)  ( (val) < 10 ? '0' + (val) : 'A'-10 + (val) )
#define hex2dec(val)  ( ((val) >= '0' && (val) <= '9') ? (val - '0') : (tolower(val) - 'a' + 10) )

//
// LEDy statusowe
//

// inicjalizacja pinów (sterowanie "odwrotn¹ logik¹" - 0: zapalony LED, 1: zgaszony LED)
// wyjœcia w stanie wysokim

// ID LED'ów
#define LED_RX      0
#define LED_TX      1
#define LED_IP      2
#define LED_TELNET  3

#ifndef RS2ETH_PROTOTYPE

//
// wersja "produkcyjna"
//

#define LED_SETUP()  sbi(DDR(PORTD), 4); \
                     sbi(DDR(PORTD), 5); \
                     sbi(DDR(PORTD), 6); \
                     sbi(DDR(PORTD), 7); \
                     sbi(PORTD, 4); \
                     sbi(PORTD, 5); \
                     sbi(PORTD, 6); \
                     sbi(PORTD, 7);

// w³¹cz / wy³¹cz
#define LED_ON(id)      sbi(PORTD, (id+4));
#define LED_OFF(id)     cbi(PORTD, (id+4));
#define LED_TOGGLE(id)  PORTD ^= (1<<(id+4));

#else

//
// wersja prototypowa
//

#define LED_SETUP()  sbi(DDR(PORTC), 0); \
                     sbi(DDR(PORTC), 1); \
                     sbi(DDR(PORTC), 2); \
                     sbi(DDR(PORTC), 3); \
                     sbi(PORTC, 0); \
                     sbi(PORTC, 1); \
                     sbi(PORTC, 2); \
                     sbi(PORTC, 3);

// w³¹cz / wy³¹cz
#define LED_ON(id)      cbi(PORTC, id);
#define LED_OFF(id)     sbi(PORTC, id);
#define LED_TOGGLE(id)  PORTC ^= (1<<(id));

#endif


// eof
#endif
