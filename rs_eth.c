/*
    mikrosterowniki.pl (c) 2008-2009
    ===========================================================================================================

    RS2ETH jest konwerterem między magistralą szeregową RS232 (za pomocą odpowiedniej "nakładki" także RS485),
    a siecią Ethernet.
    
    Dostęp do portu szeregowego możliwy jest poprzez protokół Telnet (port 23, RFC 854) oraz VCOM (port 1123).
    
    Konwerter wspiera standard opisany w dokumencie RFC 2217 (COM port suboption) pozwalając aplikacji na zdalną
    konfigurację parametrów transmisji szeregowej.

    Tryb pracy jako klient pozwala połączyć dwa konwertery RS2ETH poprzez sieć lokalną/internet tworząc połączenie
    między dwoma portami szeregowymi omijające ograniczenia standardów RS232/485 dotyczące maksymalnych
    długości połączeń. Tryb też może być także wykorzystany jako konwerter prędkości dla dwóch portów szeregowych.


    URZĄDZENIE A         RS2ETH (serwer)                    RS2ETH (klient)       URZĄDZENIE B
                  <--->                  <-- (internet) -->                 <--->
       port RS                                           (łączy się z serwerem)     port RS


    RS2ETH posiada także interfejs konfiguracyjno-diagnostyczny dostępny z poziomu przeglądarki WWW.

    Konwerter wspiera również protokół MCHP Discovery (port 30303) pozwalający wykrywać urządzenia podłączone
    do lokalnej sieci.

    Domyślne ustawienia:
        - IP:               192.168.1.12
        - brama:            192.168.1.1
        - maska:            255.255.255.0
        - tryb pracy:       serwer
        - port telnet:      23
        - logowanie:        admin / admin
        - ustawienia RS:    115200 bps 8N1

    Wersja oprogramowania: $Id: rs_eth.c 377 2009-12-05 21:10:24Z macbre $
*/

#include "rs_eth.h"

// sygnaturka
const char RS2ETH_SIGNATURE[] PROGMEM =  "rs2eth $Rev: 377 $ (" __DATE__ ") ATmega644 [mikrosterowniki.pl]";

// stan systemu
unsigned char server_state;

// zmienna pomocnicza
unsigned char i;

// timer
volatile unsigned int timer;

// adres MAC
unsigned char mac[] = {0x00, 0x04, 0xA3, 0x01, 0x02, 0x03}; // 0004A3h - OUI dla Microchipa

// fuse bity (ATmega644)
/*
FUSES =
{
    .low = (SUT0),
    .high = (BOOTSZ0 & BOOTSZ1),
    .extended = EFUSE_DEFAULT,
};
*/

// inicjalizacja watchdog'a
void wdt_init(void) {
    // reset flagi
    MCUSR = 0;
    wdt_disable();
}

void rs_on_recv(unsigned char data) {
        LED_ON(LED_RX);

        // dodaj do bufora
        if (!com_buf_is_full(&com_buffer)) {
            com_buf_put(&com_buffer, data);
        }
        else {
            // przepełnienie bufora
        }
}

// czynności do wykonania po zainicjowaniu warstwy sieciowej
void on_network_ready() {
    // wyślij pakiet powitalny z konfiguracją
    udp_config_init_hello_packet();
}

void init_timer() {
    // inicjalizacja przerwania cyklicznie obsługującego stos
    TIMSK1 |= (1 << TOIE1);	// Timer Overflow Interrupt Enabled

	TCCR1B |= (1 << CS10) | (1 << CS12);	// preskaler CK/1024

	// Preskaler: 1MHz / 1024 ~ 976,56 Hz
	//
	// x taktow => 125 ms
	// 1 takt   => 0,00102 s ~ 1 ms

	// odlicz czas (1 ms)
    TCNT1   = 0xffff - 1*20; // 20 MHz
}

// przerwanie od timera (co 1 ms)
ISR(SIG_OVERFLOW1)
{
    // odlicz czas (1 ms) - kolejny okres
    TCNT1   = 0xffff - 1*20; // 20 MHz

    uip_timer++;
    timer++;

    // licz odstęp między odbieranymi znakami po RS - detekcja końca ramki
    #ifdef RS_485
        rs_rx_tx_timer++;
    #endif

    // co 100 ms
    if (timer % 100 == 0) {

        // mrugaj diodą telnet w momencie nawiązywania połączenia ze zdalnym serwerem
        if (server_state & SERVER_STATE_CLIENT_MODE) {
            LED_TOGGLE(LED_TELNET);
        }

        // identyfikacja
        if (udp_identify_timer > 0) {
            udp_identify_timer--;

            if (udp_identify_timer%2) {
                LED_ON(LED_RX); LED_OFF(LED_TX);
            }
            else {
                LED_OFF(LED_RX); LED_ON(LED_TX);
            }
        }
        else {
            // gaś diody statusowe transmisji RS
            LED_OFF(LED_RX);
            LED_OFF(LED_TX);
        }
    }

    // co sekundę
    if (timer >= 1000) {
        timer = 0;
        uip_arptimer++; // timer ARP (stos uIP)
        dhcp_timer++;   // timer dla DHCP
        timer_tick();

        //
        // mrugaj diodami
        //

        // DHCP
        if (server_state & SERVER_STATE_DHCP) {
            LED_TOGGLE(LED_IP);

            if (dhcp_done()) {
                LED_ON(LED_IP);

                // ustaw flagę zakończenie pobierania konfiguracji sieciowej
                server_state &= ~SERVER_STATE_DHCP;
                server_state |= SERVER_STATE_DHCP_COMPLETED;

                // wyślij pakiet powitalny z konfiguracją
                on_network_ready();
            }
            // timeout dla DHCP - 5 sekund
            else if (dhcp_timer > 5) {
                LED_ON(LED_IP);

                // ustaw flagę błędu
                server_state &= ~SERVER_STATE_DHCP;
                server_state |= SERVER_STATE_DHCP_ERROR;

                // domyślne ustawienia: IP / brama / maska
                config_t* conf = config_get();
                uip_sethostaddr(conf->ip);
                uip_setdraddr(conf->gate);
                uip_setnetmask(conf->mask);

                // wyślij pakiet powitalny z konfiguracją
                on_network_ready();
            }
        }

        // tryb pracy
        if (server_state & SERVER_STATE_CLIENT_MODE) {
            // połącz ze zdalnym serwerem
            if (telnet_connect()) {
                server_state &= ~SERVER_STATE_CLIENT_MODE;
                server_state |= SERVER_STATE_CLIENT_MODE_CONN;

                LED_OFF(LED_TELNET);
            }
        }

        // mrugaj co sekundę w trakcie aktywnego połączenia ze zdalnym serwerem
        if (server_state & SERVER_STATE_CLIENT_MODE_CONN) {
            LED_TOGGLE(LED_TELNET);
        }
    }
}

void timer_tick() {
/*
    // odczyt temperatury z czujnika
    ds18b20_temp = ds18b20_get_temperature();

    // żądanie kolejnego pomiaru
    ds18b20_request_measure();

    // wyślij temperaturę po RS'ie
    //rs_int(abs(ds18b20_temp)); rs_newline();
*/
}

//
// stos uIP
//

// aplikacja -> wypełnianie pakietów danymi
void uip_appcall() {

    // okresowe odpytywanie aktywnych połączeń
    if (uip_poll()) {

        // TCP
        if ( (HTONS(uip_conn->lport) == telnet_get_port()) || (HTONS(uip_conn->rport) == telnet_server_port) ) {
            // telnet (lokalnie / zdalny serwer)
            telnet_periodic();
        }
        else switch (HTONS(uip_conn->lport)) {
            // HTTP
            case HTTP_PORT:
                http_periodic();
                break;

            // vCom
            case VCOM_SERVER_PORT:
                vcom_periodic();
                break;
        };

        // UDP
        switch(HTONS(uip_udp_conn->lport)) {
            // port konfiguracyjny - hello
            case UDP_CONFIG_HELLO_PORT:
                udp_config_send_hello_packet((unsigned char*)uip_appdata);
                break;

            // DHCP
            case DHCP_CLIENT_PORT:
                dhcp_handle_packet((unsigned char*)uip_appdata, 0);
                break;

            // 
        };
        return;
    }

    // pakiety UDP
    if (uip_udpconnection()) {
        switch(HTONS(uip_udp_conn->lport)) { 
            // port konfiguracyjny
            case UDP_CONFIG_PORT:
                udp_config_handle_packet((unsigned char*)uip_appdata, uip_datalen());
                break;

            // DHCP
            case DHCP_CLIENT_PORT:
                dhcp_handle_packet((unsigned char*)uip_appdata, uip_datalen());
                break;

            // MCHP discovery
            case ANNOUNCE_PORT:
                announce_handle_packet((unsigned char*)uip_appdata, uip_datalen());
                break;
        };
        return;
    }

    // sesja telnetowa
    // sprawdź zajętość serwera + ustaw flagę w rejestrze stanu serwera + zapal/zgaś diodę LED_TELNET
    if (HTONS(uip_conn->lport) == telnet_get_port()) {
        // próba połączenia z serwerem
        if (uip_connected()) {
            if ( !(server_state & SERVER_STATE_TELNET) ) {
                // brak aktywnej sesji telnetowej
                LED_ON(LED_TELNET);
                server_state |= SERVER_STATE_TELNET;

                // zapisz adres IP klienta
                telnet_set_client_ip((unsigned char*) &(uip_conn->ripaddr));
            }
            else {
                // jest już aktywna sesja telnetowa - rozłącz
                uip_abort();
            }
        }
        // rozłączenie się klienta
        else if (uip_closed() || uip_aborted() || uip_timedout()) {
            LED_OFF(LED_TELNET);
            server_state &= ~SERVER_STATE_TELNET;

            // "usuń" adres IP klienta
            telnet_set_client_ip(NULL);

            uip_close();
        }
    }

    // sprawdź czy strona zdalna nas nie rozłączyła
    if (HTONS(uip_conn->rport) == telnet_server_port) {
        if (uip_closed() || uip_aborted() || uip_timedout()) {
            uip_close();

            server_state &= ~SERVER_STATE_CLIENT_MODE_CONN;
            server_state |= SERVER_STATE_CLIENT_MODE;
        }
    }

    // nowe dane lub konieczność retransmisji
    if( uip_newdata() /* || uip_rexmit() */ ) {

        //rs_dump(uip_appdata, uip_datalen()); rs_int(HTONS(uip_conn->lport));

        // rozróżnij protokoły wg nr portów
        if ( (HTONS(uip_conn->lport) == telnet_get_port()) || (HTONS(uip_conn->rport) == telnet_server_port) ) {
            // telnet (lokalnie / zdalny serwer)
            telnet_handle_packet((unsigned char*)uip_appdata, uip_datalen());
        }
        else switch (HTONS(uip_conn->lport)) {
            // HTTP
            case HTTP_PORT:
                http_handle_packet((unsigned char*)uip_appdata, uip_datalen());
                break;

            // vCom
            case VCOM_SERVER_PORT:
                vcom_handle_packet((unsigned char*)uip_appdata, uip_datalen());
                break;
        }
    }
}

// debug dla stosu
void uip_log(char *msg) {
    rs_text(msg);
    rs_newline();
}

// program
int main() {
    // inicjalizacja timera
    //
    init_timer();

    // LEDy informacyjne
    //
    LED_SETUP();
    LED_ON(LED_RX);
    LED_ON(LED_TX);
    LED_ON(LED_IP);
    LED_ON(LED_TELNET);

    // LCD
    //
    #ifdef RS2ETH_PROTOTYPE
    lcd_init();
    lcd_xy(0,0);
    lcd_text("rs2eth");
    #endif

    // odczyt ustawień
    //
    config_read();

    // pobierz ustawienia
    config_t* conf = config_get();

    // RS232
    //
    rs_init(conf->rs_baud);

    // włącz obsługę przerwania w momencie otrzymania danych na RS'a
    rs_int_on_recv();

    // inicjalizacja bufora cyklicznego na dane
    com_buf_init(&com_buffer);

    // DS18B20
    //
    ds18b20_init(DS18B20_RESOLUTION_12_BITS);

    // ID urządzenia na 1wire -> serial number płytki i MAC interfejsu ethernetowego
    unsigned char serial[8];
    ow_reset();
    ow_read_rom_code(serial);

    // wypisz id na LCD
    #ifdef RS2ETH_PROTOTYPE
    lcd_xy(0,1);
    lcd_char('i');
    lcd_char('d');
    lcd_char(' ');

    // ID: family code + 6 bajtów ID + CRC
    for (i=1; i<4; i++) {
        lcd_hex(serial[i]);
    }
    #endif

    // SPI
    //
    spi_master_init();

    //
    // stos uIP
    //

    // ustaw unikalny MAC na podstawie ID urządzenia 1wire
    mac[3] = serial[1];
    mac[4] = serial[2];
    mac[5] = serial[3];

    // inicjalizacja ENC28 i stosu
    nic_init(mac);

    // koniec inicjalizacji
    _delay_ms(10);
    
    LED_OFF(LED_RX);
    LED_OFF(LED_TX);
    LED_OFF(LED_IP);
    LED_OFF(LED_TELNET);

    // czekaj na aktywność połączenia
    while( !enc28_is_link_up() ) {
        LED_TOGGLE(LED_IP);

        // czekaj 100ms
        for (i=0; i<10; i++) {
            _delay_ms(10);
        }
    }

    // inicjalizacja stosu TCP/IP
    uip_init();
    uip_arp_init();

    // zeruj stan serwera
    server_state = 0;

    // ustaw port dla serwera telnet
    telnet_set_port(conf->telnet_port);
    
    // tryby pracy
    if (conf->mode == MODE_SERVER) {
        uip_listen(HTONS(telnet_get_port()));       // telnet (zgodność z RFC 2217)
        uip_listen(HTONS(VCOM_SERVER_PORT));        // vCom (ponticulus)
    }
    else {
         telnet_set_remote((uip_ipaddr_t*)conf->server_ip, conf->server_port);
         server_state |= SERVER_STATE_CLIENT_MODE;
    }

    // nasłuchuj (TCP/UDP)
    uip_listen(HTONS(HTTP_PORT));               // serwer WWW
    uip_udp_listen(HTONS(UDP_CONFIG_PORT));     // pakiety UDP: konfiguracyjne i discovery
    uip_udp_listen(HTONS(DHCP_CLIENT_PORT));    // DHCP

    // DHCP?
    if (conf->use_dhcp) {
        server_state |= SERVER_STATE_DHCP;
        LED_OFF(LED_IP);

        // inicjalizacja klienta DHCP
        dhcp_init();
    }
    else {
        // zapal diodę 
        LED_ON(LED_IP);

        // IP / brama / maska
        uip_sethostaddr(conf->ip);
        uip_setdraddr(conf->gate);
        uip_setnetmask(conf->mask);

        // wyślij pakiet powitalny z konfiguracją
        on_network_ready();
    }

    // włącz przerwania
    sei();

    #ifdef RS2ETH_PROTOTYPE
    lcd_xy(14,1);   lcd_char('o'); lcd_char('k');
    #endif

    // sprawdzaj bufor odbiorczy i obsługuj przychodzące pakiety
    for(;;) {
        uip_check();
    }

    return 0;
}
