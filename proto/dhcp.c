#include "dhcp.h"

volatile unsigned char dhcp_state;
volatile unsigned char dhcp_timer;

// inicjalizacja klienta DHCP
void dhcp_init() {

    // ustaw stan klienta
    dhcp_state = DHCP_STATE_INITIAL;

    // zeruj timer
    dhcp_timer = 0;

    // broadcast
    uip_ipaddr_t addr;
    uip_ipaddr(addr, 255, 255, 255, 255);

    // utwórz "po³¹czenie" UDP z serwerem DHCP
    struct uip_udp_conn* conn;

    conn = uip_udp_new(&addr, HTONS(DHCP_SERVER_PORT));

    if(conn != NULL) {
        uip_udp_bind(conn, HTONS(DHCP_CLIENT_PORT));
    }
}

// obs³uga ¿¹dañ DHCP
void dhcp_handle_packet(unsigned char* data, unsigned int len) {
    if (dhcp_state == DHCP_STATE_STOPED) {
        return;
    }

    dhcp_packet* dhcp = (dhcp_packet*) data;

    // pooling -> wyœlij pakiet DISCOVERY
    if ( uip_poll() && (dhcp_state == DHCP_STATE_INITIAL) ) {
        len = dhcp_create_packet(dhcp, DHCP_MSG_DHCPDISCOVER);
        uip_send(data, len);
        nic_wait_for_send();
        dhcp_state = DHCP_STATE_SENDING;
    }
    // odpowiedŸ od serwera DHCP
    else if ( !uip_poll() ) {
        // ignoruj b³êdne pakiety
        if (dhcp->op != DHCP_REPLY) {
            return;
        }

        // pobierz typ pakietu
        if (!dhcp_get_option(dhcp, DHCP_OPTION_DHCPMSGTYPE, 1, &(data[0])) ) {
            return;
        }

        // odpowiedz na pakiet od serwera
        switch (data[0]) {
            case DHCP_MSG_DHCPOFFER:

                // oferta od DHCP z adresami
                uip_sethostaddr(dhcp->client_given_addr);

                // wyœlij proœbê o dalsze dane
                len = dhcp_create_packet(dhcp, DHCP_MSG_DHCPREQUEST);
                uip_send(data, len);

                dhcp_state = DHCP_STATE_OFFER_RECEIVED;

                break;

            case DHCP_MSG_DHCPACK:

                // przydzielone IP
                uip_sethostaddr(dhcp->client_given_addr);

                // brama
                if (dhcp_get_option(dhcp, DHCP_OPTION_ROUTER, 4, data)) {
                    uip_setdraddr(data);
                }
                else {
                    return;
                }

                // maska sieciowa
                if (dhcp_get_option(dhcp, DHCP_OPTION_NETMASK, 4, data)) {
                    uip_setnetmask(data);
                }
                else {
                    return;
                }

                // koniec
                dhcp_state = DHCP_STATE_CONFIG_RECEIVED;

                break;

        } // \switch
    } // \if
}

// przygotuj podany typ pakietu DHCP
unsigned int dhcp_create_packet(dhcp_packet* dhcp, unsigned char type) {

    unsigned int len = 0;

    dhcp->op          = DHCP_REQUEST;
    dhcp->htype       = DHCP_HTYPE_ETHERNET;
    dhcp->hlen        = DHCP_HLEN_ETHERNET;

    // wpisujemy puste adresy
    // stosowne dane dopiero otrzymamy w odpowiedzi od serwera DHCP
    memset(dhcp->client_addr, 0, 4);
    memset(dhcp->client_given_addr, 0, 4);
    memset(dhcp->server_addr, 0, 4);
    memset(dhcp->gate_addr, 0, 4);

    // adres MAC
    memcpy(dhcp->client_haddr, nic_get_mac(), 6);

    // fill
    memset(dhcp->fill, 0, 8);

    // puste stringi
    dhcp->server_name[0] = 0x00;
    dhcp->boot_file[0]   = 0x00;

    // cookie
    dhcp->cookie = DHCP_COOKIE;

    // xid
    memcpy((unsigned char*) &(dhcp->xid), (nic_get_mac() + 3), 3);

    dhcp->secs  = 0;
    dhcp->flags = HTONS(1); // flaga UNICAST
    dhcp->hops  = 0;

    // wpisz opcje "msg_type" (discovery / request)
    dhcp->options[len++] = DHCP_OPTION_DHCPMSGTYPE;
    dhcp->options[len++] = 1;
    dhcp->options[len++] = type;

    // wyslij potwierdzenie przyjecia proponowanego IP (REQUEST)
    if (type == DHCP_MSG_DHCPREQUEST) {
        dhcp->options[len++] = DHCP_OPTION_REQUESTEDIP;
        dhcp->options[len++] = 4;

        uip_gethostaddr(&(dhcp->options[len]));
        len += 4;
    }
    
    // zakoncz opcje
    dhcp->options[len++] = DHCP_OPTION_END;

    return sizeof(dhcp_packet) + len;
}

// pobiera wartoœæ podanej opcji
unsigned char dhcp_get_option(dhcp_packet* dhcp, unsigned char optcode, unsigned char optlen, unsigned char* optvalptr) {

    unsigned char* options = dhcp->options;

	// parsuj do skutku
	for (;;)
	{
		// przeskocz wype³nienia
		if(*options == DHCP_OPTION_PAD) {
			options++;
        }
		// koniec opcji
		else if(*options == DHCP_OPTION_END) {
			break;
        }
		// znaleziono
		else if(*options == optcode)
		{
            optlen = (optlen < *(options+1) ) ? optlen : *(options+1) ;

            memcpy(optvalptr, options + 2, optlen);

			// zwróæ d³ugoœæ opcji
			return optlen;
		}
		else
		{
			// przeskocz do nastêpnej opcji
			options++;
			options+=*options;
			options++;
		}
	}

	// nie znaleziono
	return 0;
}

// czy zakoñczono pobieranie konfiguracji
unsigned char dhcp_done() {
    return (dhcp_state & DHCP_STATE_CONFIG_RECEIVED) ? 1 : 0;
}

// przerwij pobieranie konfiguracji
void dhcp_stop() {
    dhcp_state = DHCP_STATE_STOPED;
}
