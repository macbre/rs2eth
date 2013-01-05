#ifndef _DHCP_H
#define _DHCP_H

#include "../rs_eth.h"

// pakiet DHCP
typedef struct
{
    uint8_t  op;
    uint8_t  htype;
    uint8_t  hlen;
    uint8_t  hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t  client_addr[4];
    uint8_t  client_given_addr[4];
    uint8_t  server_addr[4];
    uint8_t  gate_addr[4];
    uint8_t  client_haddr[8];
    uint8_t  fill[8];           
    uint8_t  server_name[64];
    uint8_t  boot_file[128];
    uint32_t cookie;
    uint8_t  options[];
} dhcp_packet; /* 240 (bez opcji) */

// inicjalizacja klienta DHCP
void dhcp_init();

// obs³uga ¿¹dañ DHCP
void dhcp_handle_packet(unsigned char*, unsigned int);

// przygotuj podany typ pakietu DHCP
unsigned int dhcp_create_packet(dhcp_packet*, unsigned char);

// pobiera wartoœæ podanej opcji
unsigned char dhcp_get_option(dhcp_packet*, unsigned char, unsigned char, unsigned char*);

// czy zakoñczono pobieranie konfiguracji
unsigned char dhcp_done();

// przerwij pobieranie konfiguracji
void dhcp_stop();

// porty UDP serwera/klienta
#define DHCP_SERVER_PORT  67
#define DHCP_CLIENT_PORT  68

extern volatile unsigned char dhcp_timer;

// stan klienta DHCP
extern volatile unsigned char dhcp_state;
#define DHCP_STATE_INITIAL         0
#define DHCP_STATE_SENDING         1
#define DHCP_STATE_OFFER_RECEIVED  2
#define DHCP_STATE_CONFIG_RECEIVED 3 
#define DHCP_STATE_STOPED          4

// sta³e
#define DHCP_REQUEST	        1
#define DHCP_REPLY		        2

#define DHCP_HTYPE_ETHERNET	    1
#define DHCP_HLEN_ETHERNET	    6

#define DHCP_OPTION_PAD         0
#define DHCP_OPTION_NETMASK     1
#define DHCP_OPTION_ROUTER      3
#define DHCP_OPTION_HOSTNAME    12
#define DHCP_OPTION_DOMAINNAME  15
#define DHCP_OPTION_REQUESTEDIP 50
#define DHCP_OPTION_LEASETIME   51
#define DHCP_OPTION_DHCPMSGTYPE	53
#define DHCP_OPTION_END     	255

#define DHCP_MSG_DHCPDISCOVER	1
#define DHCP_MSG_DHCPOFFER  	2
#define DHCP_MSG_DHCPREQUEST	3
#define DHCP_MSG_DHCPACK		5

#define DHCP_COOKIE             0x63538263

#endif
