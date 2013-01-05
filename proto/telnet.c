#include "telnet.h"

// port lokalnego serwera telnet
unsigned int telnet_port;

// IP:port zdalnego serwera telnet
uip_ipaddr_t telnet_server_ip;
unsigned int telnet_server_port;

// po³¹czenie ze zdalnym serwerem telnet
struct uip_conn* telnet_server_conn;

// obs³u¿ przychodz¹ce dane na port telnetowy
void telnet_handle_packet(unsigned char* data, unsigned int len) {

    // komenda telnetowa - IAC?
    if (data[0] == TELNET_IAC) {

        // typ komendy
        switch(data[1]) {

            // Are You There?
            case TELNET_AYT:
                len = 0;

                // dodaj nazwê urz¹dzenia nadan¹ przez u¿ytkownika
                strcpy((char*)data, config_get()->name);
                len += strlen(config_get()->name);

                data[len++] = ' ';
                data[len++] = '|';
                data[len++] = ' ';

                // sygnatura urz¹dzenia                
                memcpy_P(data+len, RS2ETH_SIGNATURE, strlen_P(RS2ETH_SIGNATURE));
                len += strlen_P(RS2ETH_SIGNATURE);
                data[len++] = 0x0D; // \r
                data[len++] = 0x0A; // \n
                uip_send(data, len);
                break;

            // negocjacja podopcji
            case TELNET_SB:
                // podopcja dla portu COM
                if (data[2] == TELNET_COM_SB) {
                    if (telnet_handle_com_negotiation((data+3), len-3)) {
                        uip_send(data, len);
                    }
                }
                break;

            // przeœlij znak 0xFF
            case TELNET_IAC:
                rs_send(0xFF);
                break;
        }
    }
    // dane -> przeœlij na port COM
    else {
        if (len > 0) {
            LED_ON(LED_TX);
        }

        for (; len > 0; len--) {
            rs_send(*(data++));
        }
    }
}

// obs³u¿ pakiet negocjacji opcji portu COM (RFC 2217)
unsigned char telnet_handle_com_negotiation(unsigned char* data, unsigned int len) {

    // pobierz aktualne ustawienia portu COM
    struct rs_current_setup_t *setup = rs_get_setup();

    unsigned long *val = (unsigned long*) (data+1);

    // typ ustawienia
    //
    // ustaw wartoœæ, gdy pole wartoœci != 0
    // zwróæ wartoœæ, gdy pole wartoœci == 0
    switch(data[0]) {
        case TELNET_COM_SET_BAUDRATE:
            if (*val == 0) {
                *val = htonl(setup->baud);
            }
            else {
                setup->baud = htonl(*val);
            }
            break;

        case TELNET_SET_DATASIZE:
            if (data[1] == 0) {
                data[1] = setup->data_bits;
            }
            else {
                setup->data_bits = data[1];
            }
            break;

        case TELNET_SET_PARITY:
            if (data[1] == 0) {
                data[1] = setup->parity_bits;
            }
            else {
                setup->parity_bits = data[1];
            }
            break;

        case TELNET_SET_STOPSIZE:
            if (data[1] == 0) {
                data[1] = setup->stop_bits;
            }
            else {
                setup->stop_bits = data[1];
            }
            break;

        case TELNET_SET_CONTROL:
            if (data[1] == 0) {
                data[1] = 1;
            }
            break;

        default:
            return 0;
    }

    // zmieñ ustawienia portu COM
    rs_setup(setup->baud, setup->data_bits, setup->parity_bits, setup->stop_bits);

    // zmieñ typ ustawienia na odpowiedŸ od serwera
    data[0] += TELNET_COM_SB_SERVER_CODE;

    return 1;
}

// okresowo sprawdzaj czy nie nadesz³y nowe dane na porcie COM
void telnet_periodic() {
    
    unsigned char* buf = uip_appdata;
    unsigned int len = 0;
    unsigned char ch;

    // wyœlij dane z bufora odbiorczego
    if (!com_buf_is_empty(&com_buffer)) {

        // dla trybu RS485 poczekaj na przerwê miêdzy ramkami (RS_RX_TX_DELAY [ms]), aby wys³aæ ca³¹ ramkê Modbusa w jednym pakiecie
        #ifdef RS_485
            if (rs_rx_tx_timer < RS_RX_TX_DELAY) {
                return;
            }
            else {
                rs_rx_tx_timer = 0;
            }
        #endif

        while ( !com_buf_is_empty(&com_buffer) && (len < 1470) ) {
            ch = com_buf_get(&com_buffer);

            buf[len++] = ch;

            // w³aœciwie koduj transfer binarny
            // @see RFC 854
            if (ch == TELNET_IAC) {
                buf[len++] = ch;
            }
        }

        uip_send(buf, len);
    }
}

// zapisz adres IP w³aœnie pod³¹czonego klienta
void telnet_set_client_ip(unsigned char* ip) {
    telnet_client_ip = ip;

    // wyczyœæ bufory COM
    com_buf_init(&com_buffer);
}

// pobierz zapisany adres IP pod³¹czonego klienta
unsigned char* telnet_get_client_ip() {
    return telnet_client_ip;
}

// ustaw nas³uchuj¹cy port telnetowy
void telnet_set_port(unsigned int port) {
    telnet_port = port;
}

// pobierz nas³uchuj¹cy port telnetowy
unsigned int telnet_get_port() {
    return telnet_port;
}

// ustaw IP:port zdalnego serwera telnet
void telnet_set_remote(uip_ipaddr_t *ip, unsigned int port) {
    memcpy( (void*)telnet_server_ip, (void*) ip, 4);
    telnet_server_port = port;
}

// po³¹cz ze zdalnym serwerem
unsigned char telnet_connect() {
    telnet_server_conn = uip_connect(&telnet_server_ip, HTONS(telnet_server_port));
    return (!telnet_server_conn) ? 0 : 1;
}
