#ifndef _HTTP_H
#define _HTTP_H

#include "../rs_eth.h"
#include "http/data.h"
#include "http/pages.h"

// obs³u¿ przychodz¹ce ¿¹dania do serwera
void http_handle_packet(unsigned char*, unsigned int);

// serwuj treœæ dynamiczn¹
unsigned int http_serve_dynamic(unsigned char*, unsigned char);

// generuj menu
unsigned int http_serve_menu(unsigned char*, unsigned char);

// serwuj treœæ dynamiczn¹ generowan¹ jako odpowiedzi na ¿¹dania POST
unsigned int http_serve_post(unsigned char*, unsigned char*, unsigned char*);

// serwuj treœæ statyczn¹
unsigned int http_serve_static(unsigned char*, unsigned char*);

// sprawdŸ dane autoryzacyjne u¿ytkownika
unsigned char http_authenticate_user(unsigned char*);

// wykonuj czynnoœci okresowe: zamykanie otwartych po³¹czeñ, "dosy³anie" reszty danych (strony dynamiczne)
void http_periodic();

// kopiuje podany ³añcuch z pamiêci FLASH do pamiêci RAM oraz zwraca d³ugoœæ kopiowanego ³añcucha
unsigned int http_copy_from_pstr(unsigned char*, PGM_VOID_P);

// kopiuje podany obszar z pamiêci FLASH do pamiêci RAM oraz zwraca d³ugoœæ kopiowanego ³añcucha
unsigned int http_copy_from_flash(unsigned char*, PGM_VOID_P, unsigned int);

// wstawia podan¹ wartoœæ numeryczn¹ jako ³añcuch do pamiêci RAM oraz zwraca d³ugoœæ wstawionego ³añcucha
unsigned int http_copy_from_val(unsigned char*, unsigned long);

// wstawia podany adres IP jako ³añcuch do pamiêci RAM oraz zwraca d³ugoœæ wstawionego ³añcucha
unsigned int http_copy_from_ip(unsigned char*, uip_ipaddr_t*);

// kopiuje wartoœæ podanego parametru zapytania do bufora oraz zwraca jego d³ugoœæ
unsigned char http_get_query_param(unsigned char*, PGM_VOID_P, unsigned char*);

// parsuj podany ³añcuch z adresem IP
unsigned char http_parse_ip(unsigned char*, uip_ipaddr_t*);

// port serwera HTTP
#define HTTP_PORT 80

// typ stron dynamicznych (zak³adki menu)
#define HTTP_PAGE_INFO      1
#define HTTP_PAGE_COM_SETUP 2
#define HTTP_PAGE_IP_SETUP  3
#define HTTP_PAGE_MODE      4
#define HTTP_PAGE_STATS     5
#define HTTP_PAGE_ADMIN     6
#define HTTP_PAGE_RESET     7
#define HTTP_PAGE_MANUAL    8

// parsowanie zapytañ
#define HTTP_QUERY_IS(txt)      strcmp_P((char*)query, PSTR(txt)) == 0
#define HTTP_QUERY_BEGINS(txt)  strstr_P((char*)query, PSTR(txt)) == (char*) query

// nag³ówki odpowiedzi serwera
extern const char HTTP_SERVER[] PROGMEM;
extern const char HTTP_AUTHENTICATE[] PROGMEM;

extern const char HTTP_CACHE[] PROGMEM;
extern const char HTTP_DONT_CACHE[] PROGMEM;

extern const char HTTP_200_BODY[] PROGMEM;
extern const char HTTP_200_HTML[] PROGMEM;
extern const char HTTP_200_PNG[] PROGMEM;
extern const char HTTP_200_GIF[] PROGMEM;
extern const char HTTP_200_CSS[] PROGMEM;
extern const char HTTP_200_JS[] PROGMEM;
extern const char HTTP_200_TEXT[] PROGMEM;

extern const char HTTP_301_REDIRECT[] PROGMEM;

extern const char HTTP_401_HEADER[] PROGMEM;
extern const char HTTP_401_BODY[] PROGMEM;
extern const char HTTP_404_HEADER[] PROGMEM;
extern const char HTTP_404_BODY[] PROGMEM;

extern const char HTTP_AUTH_STRING[] PROGMEM;

#endif
