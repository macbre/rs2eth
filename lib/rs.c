#include "rs.h"

// inicjalizacja UART z okreskona predkoscia
void rs_init(unsigned long rate)
{
	// ustaw prêdkoœæ transmisji
	//UBRRH = (unsigned char) (USART_BAUDRATE(rate) >> 8);
	//UBRRL = (unsigned char) USART_BAUDRATE(rate);

	// USART setup (p. 151)
	//UCSR0A = (1<<U2X0);                // wlacz podwojna predkosc U2x
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);   // wlacz nadajnik i odbiornik

    rs_setup(rate, 8, 0, 1);

	//UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);  // transmisja asynchroniczna, 8N1

    // RS485: pin DIR jako wyjœcie i przejdŸ w tryb odbioru
    #ifdef RS_485
        sbi(DDR(RS_485_DIR_PORT), RS_485_DIR_PIN);
        rs_enable_transmit(0);
    #endif

	return; 
}

// ustawienia portu COM (wartoœci wg RFC 2217)
void rs_setup(unsigned long baud, unsigned char data_bits, unsigned char parity_bits, unsigned char stop_bits) {

    unsigned char reg = 0;

    // ustaw prêdkoœæ transmisji
    UBRR0H = (unsigned char) (USART_BAUDRATE(baud) >> 8);
	UBRR0L = (unsigned char) USART_BAUDRATE(baud);

    // bity danych
    switch(data_bits) {
        case 5:
            break;

        case 6:
            reg |= (1<<UCSZ00); break;

        case 7:
            reg |= (1<<UCSZ01); break;

        default:
        case 8:
            reg |= (1<<UCSZ01)|(1<<UCSZ00);
            data_bits = 8;
            break;
    }

    // bity parzystoœci       
    switch(parity_bits) {
        case 2: // odd
            reg |= (1<<UPM01)|(1<<UPM00); break;

        case 3: // even
            reg |= (1<<UPM01); break;

        default: // mark / space nieobs³ugiwane przez USART AVR'owy
        case 1: // brak
            parity_bits = 1;
            break;
    }

    // bity stopu
    switch(stop_bits) {
        case 1: // 1
            break;
        
        default: // 1.5 bitu stopy nieobs³ugiwane przez USART AVR'owy
        case 2: // 2
            reg |= (1<<USBS0);
            stop_bits = 2;
            break;
    }

    // zapisz do rejestru
    UCSR0C = /*(1<<URSEL0) |*/ reg;

    // zapisz do struktury info o aktualnej konfiguracji
    rs_current_setup.baud        = baud;
    rs_current_setup.data_bits   = data_bits;
    rs_current_setup.parity_bits = parity_bits;
    rs_current_setup.stop_bits   = stop_bits;
}

// pobierz ustawienia portu COM (wartoœci wg RFC 2217)
struct rs_current_setup_t *rs_get_setup() {
    return &rs_current_setup;
}

// czyœci bufor odbiorczy
void rs_flush()
{
    unsigned char dummy;
    while ( UCSR0A & (1<<RXC0) ) dummy = UDR0;
}

// w³¹cza obs³ugê przerwania na przychodz¹ce dane z USART
void rs_int_on_recv() {
    // Receive Complete Interrupt Enable
    UCSR0B |= (1<<RXCIE0);
}

// obs³uga przerwania
ISR(SIG_USART_RECV) {

    #ifdef RS_STATS
    rs_stats.recv++;
    #endif

    rs_on_recv(UDR0);
}

//void rs_on_recv(unsigned char data) {}

// odbiera dane z bufora (procedura blokujaca!)
unsigned char rs_recv()
{
	// czekaj na dane
	while ( !(UCSR0A & (1<<RXC0)) );

    #ifdef RS_STATS
    rs_stats.recv++;
    #endif

	// zwroc zawartosc bufora
	return UDR0;
}

// wysyla dane do bufora (procedura blokujaca!)
void rs_send(unsigned char data)
{
    // obs³uga RS485
    #ifdef RS_485
        rs_enable_transmit(1);
    #endif

	// czekaj na zwolnienie bufora nadawczego
	while ( !( UCSR0A & (1<<UDRE0)) );

    #ifdef RS_STATS
    rs_stats.sent++;
    #endif

	// wyslij dane
	UDR0 = data;

    // czekaj na koniec transmisji
    while( !(UCSR0A & (1<<TXC0)) );

    // wyczyœc flagê TXC poprzez wpisanie '1'
    sbi(UCSR0A, TXC0);

    // obs³uga RS485
    #ifdef RS_485
        rs_enable_transmit(0);
    #endif
}

// sprawdza czy w buforze znajduja sie nieodczytane odebrane dane (USART Receive Complete)
unsigned char rs_has_recv()
{
	return (UCSR0A & (1<<RXC0));
}

// sprawdza czy dane w buforze zostaly wyslane (USART Transmit Complete)
unsigned char rs_has_send()
{
	return (UCSR0A & (1<<UDRE0));
}

// wysyla lancuch tekstowy (procedura blokujaca!)
void rs_text(char txt[])
{
	unsigned char c;

	while( (c = *(txt++)) != 0x00 )
		rs_send(c);
}

// wysyla lancuch tekstowy z pamieci programu
void rs_text_P(PGM_P txt)
{
	unsigned char c;
  	while ((c = pgm_read_byte(txt++)))
    	rs_send(c);
}

void rs_int(int value)
{
	char buf[8];

    rs_text( itoa(value, buf, 10) );
}

void rs_long(unsigned long value)
{
	char buf[8];

    rs_text( ltoa(value, buf, 10) );
}

void rs_int2(unsigned char value)
{
	rs_send( (value / 10) + 48);
	rs_send( (value % 10) + 48);
}

void rs_hex(unsigned char val)
{
    rs_send(dec2hex(val >> 4));
    rs_send(dec2hex(val & 0x0f));
}

#ifdef RS_DUMP
void rs_dump(unsigned char* data, unsigned int len)
{
    unsigned int i, j;

    rs_newline();

    for (i=0; i < len; i++) {
        rs_hex(data[i]);

        if ( (i % 16) == 15) {
            rs_send('|');
            for (j=i-15;j<=i; j++) {
                rs_send( data[j] > 0x20 ? data[j] : '.');
            }
        }
        else {
            rs_send(' ');
        }
    }

    rs_newline();
}
#endif

void rs_enable_transmit(unsigned char enable) {
    if (enable) {
        sbi(RS_485_DIR_PORT, RS_485_DIR_PIN);
    }
    else {
        cbi(RS_485_DIR_PORT, RS_485_DIR_PIN);
    }
}
