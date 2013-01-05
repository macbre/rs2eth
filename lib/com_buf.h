#ifndef _COM_BUF_H
#define _COM_BUF_H

#include "../rs_eth.h"

#define COM_BUF_SIZE 1700

typedef struct {
    unsigned int read_ptr;
    unsigned int write_ptr;
    unsigned char data[COM_BUF_SIZE];
    unsigned char flags;
} com_buf_t;

extern volatile com_buf_t com_buffer;

// wsparcie dla ramek Modbus - wysy³aj ramki w jednym pakiecie
#ifdef RS_485
    extern volatile unsigned char rs_rx_tx_timer;

    // czekaj 5 ms po odebraniu ostatniego znaku po RS'ie zanim nast¹pi komunikacja po sieci
    // zgodne z Modbusem dla prêdkoœci >= 9600 bps
    // #define RS_RX_TX_DELAY  5

    // oblicz okres miêdzy ramkami wg aktualnej prêdkoœci transmisji (ms)
    #define RS_RX_TX_DELAY com_buf_get_frame_delay()

    unsigned char com_buf_get_frame_delay();
#endif

// inicjalizacja bufora
void com_buf_init(volatile com_buf_t*);

// bufor przepe³niony?
unsigned char com_buf_is_full(volatile com_buf_t*);

// bufor pusty?
unsigned char com_buf_is_empty(volatile com_buf_t*);

// zapisz do bufora
unsigned char com_buf_put(volatile com_buf_t*, unsigned char);

// odczytaj z bufora
unsigned char com_buf_get(volatile com_buf_t*);

#define COM_BUF_PTR_INC(ptr)    if (ptr < COM_BUF_SIZE-1) {ptr++;} else {ptr = 0;}

#endif
