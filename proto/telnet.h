#ifndef __TELNET_H
#define __TELNET_H

#include "../rs_eth.h"

// port lokalnego serwera telnet
extern unsigned int telnet_port;

// IP:port zdalnego serwera telnet
extern uip_ipaddr_t telnet_server_ip;
extern unsigned int telnet_server_port;

// po³¹czenie ze zdalnym serwerem telnet
extern struct uip_conn* telnet_server_conn;

// obs³u¿ przychodz¹ce dane na port telnetowy
void telnet_handle_packet(unsigned char*, unsigned int);

// obs³u¿ pakiet negocjacji opcji portu COM (RFC 2217)
unsigned char telnet_handle_com_negotiation(unsigned char*, unsigned int);

// okresowo sprawdzaj czy nie nadesz³y nowe dane na porcie COM
void telnet_periodic();

// adres IP klienta pod³¹czonego po telnecie
unsigned char* telnet_client_ip;

// zapisz adres IP pod³¹czonego klienta
void           telnet_set_client_ip(unsigned char*);

// pobierz zapisany adres IP pod³¹czonego klienta
unsigned char* telnet_get_client_ip();

// ustaw nas³uchuj¹cy port telnetowy
void telnet_set_port(unsigned int);

// pobierz nas³uchuj¹cy port telnetowy
unsigned int telnet_get_port();

// ustaw IP:port zdalnego serwera telnet
void telnet_set_remote(uip_ipaddr_t*, unsigned int);

// po³¹cz ze zdalnym serwerem
unsigned char telnet_connect();

// komendy telnetowe
#define TELNET_IAC      0xFF    // Is A Command
#define TELNET_AYT      0xF6    // Are You There?
#define TELNET_SB       0xFA    // Suboption
#define TELNET_COM_SB   0x2C    // COM port suboption (RFC 2217)

// ustawienia telnetowe dla portu COM (RFC 2217)
#define TELNET_COM_SET_BAUDRATE     1
#define TELNET_SET_DATASIZE         2
#define TELNET_SET_PARITY           3
#define TELNET_SET_STOPSIZE         4
#define TELNET_SET_CONTROL          5
#define TELNET_NOTIFY_LINESTATE     6
#define TELNET_NOTIFY_MODEMSTATE    7
#define TELNET_FLOWCONTROL_SUSPEND  8
#define TELNET_FLOWCONTROL_RESUME   9
#define TELNET_SET_LINESTATE_MASK   10
#define TELNET_SET_MODEMSTATE_MASK  11
#define TELNET_PURGE_DATA           12

// przesuniêcie kodu ustawienia dla portu COM dla odpowiedzi od serwera
#define TELNET_COM_SB_SERVER_CODE   0x64

#endif
