#include "http.h"

// obs³u¿ przychodz¹ce ¿¹dania do serwera
void http_handle_packet(unsigned char* data, unsigned int len) {

    // nie obs³uguj pustych ¿¹dañ
    if (len == 0) {
        return;
    }

    //rs_dump(data, len);

    //
    // parsuj zapytanie
    //
    // GET /foo HTTP/1.1
    // POST /do HTTP/1.1
    unsigned char* pos;             // zmienna pomocnicza
    unsigned char* method;          // metoda zapytania (GET/POST)
    unsigned char* query;           // zapytanie (URL)
    unsigned char* body;            // treœæ zapytania
    unsigned char* auth_header;     // nag³ówek z loginem i has³em (Authorization)

    // znajdŸ treœæ zapytania
    pos = (unsigned char*) strstr((char*)data, "\x0D\x0A\x0D\x0A");

    if (!pos) {
        return;
    }

    body = pos+4;
    data[len] = 0;

    // zamknij nag³ówek znakiem NULL
    *pos = 0;

    //
    // typ zapytania
    //
    method = data;
    pos = (unsigned char*) strchr((char*)data, ' ');

    // b³êdne zapytanie
    if (!pos) {
        return;
    }

    // postaw NULL za typem zapytania
    *(pos) = 0;
    
    //
    // zapytanie
    //
    query = pos+1;
    pos = (unsigned char*) strchr((char*)query, ' ');
    if (!pos) {
        return;
    }

    *(pos) = 0;

    //
    // nag³ówek z loginem i has³em
    //
    // Basic YWRtaW46YWRtaW4= (admin/admin)
    //
    pos = (unsigned char*) strstr_P((char*)pos+1, PSTR("Authorization: Basic"));

    if (pos) {
        auth_header = pos + 21;
        pos = (unsigned char*) strchr((char*)auth_header, 0x0D);
        *(pos) = 0;
    }
    else {
        auth_header = NULL;
    }

    // debug
    /**
    rs_text(method);        rs_newline();
    rs_text(query);         rs_newline();

    if (auth_header) {
        rs_text(auth_header);   rs_newline();
    }
    **/

    len = 0;

    // stan aplikacji
    //  0            -> zamknij po³¹czenie przy nastêpnym odpytywaniu przez stos
    //  inna wartoœæ -> doœlij dynamicznie generowan¹ zawartoœæ przy nastêpnym odpytywaniu przez stos
    uip_conn->appstate.request = 0;

    // autoryzacja
    if (!http_authenticate_user(auth_header)) {
        len += http_copy_from_pstr(data,     HTTP_401_HEADER);
        len += http_copy_from_pstr(data+len, HTTP_AUTHENTICATE);
        len += http_copy_from_pstr(data+len, HTTP_SERVER);
        len += http_copy_from_pstr(data+len, HTTP_401_BODY);
    }
    else {
        // rozdziel ¿¹dania GET i POST

        // POST
        if ( strcmp_P((char*)method, PSTR("POST")) == 0 ) {
            len += http_serve_post(data, query, body);
        }
        // GET
        else if ( strcmp_P((char*)method, PSTR("GET")) == 0 ) {

            // treœæ dynamiczna
            if ( HTTP_QUERY_IS("/") ) {
                uip_conn->appstate.request = HTTP_PAGE_INFO;
            }
            else if ( HTTP_QUERY_IS("/com") ) {
                uip_conn->appstate.request = HTTP_PAGE_COM_SETUP;
            }
            else if ( HTTP_QUERY_IS("/ip") ) {
                uip_conn->appstate.request = HTTP_PAGE_IP_SETUP;
            }
            else if ( HTTP_QUERY_IS("/mode") ) {
                uip_conn->appstate.request = HTTP_PAGE_MODE;
            }
            else if ( HTTP_QUERY_IS("/stats") ) {
                uip_conn->appstate.request = HTTP_PAGE_STATS;
            }
            else if ( HTTP_QUERY_IS("/admin") ) {
                uip_conn->appstate.request = HTTP_PAGE_ADMIN;
            }
            else if ( HTTP_QUERY_IS("/reset") ) {
                uip_conn->appstate.request = HTTP_PAGE_RESET;
            }
            else if ( HTTP_QUERY_IS("/help") ) {
                uip_conn->appstate.request = HTTP_PAGE_MANUAL;
            }
            // treœæ statyczna - JS/CSS/PNG
            else if( HTTP_QUERY_BEGINS("/s/") ) {
                len += http_serve_static(data, query+3);
            }

            // odczyt temperatury
            else if ( HTTP_QUERY_IS("/temp") ) {
                len += http_show_temperature(data, 0);
            }

            // wyœlij nag³ówki i pocz¹tek HTMLa (wraz z menu) dla treœci dynamicznej
            if (uip_conn->appstate.request > 0) {
                len += http_copy_from_pstr(data,      HTTP_200_HTML);
                len += http_copy_from_pstr(data+len,  HTTP_SERVER);
                len += http_copy_from_flash(data+len, HTTP_DATA_LAYOUT_START, HTTP_DATA_LAYOUT_START_LENGTH);
                len += http_serve_menu(data+len,      uip_conn->appstate.request);
            }
        }

        // nie znaleziono (zwróæ b³¹d 404)
        if (len == 0) {
            len += http_copy_from_pstr(data,     HTTP_404_HEADER);
            len += http_copy_from_pstr(data+len, HTTP_SERVER);
            len += http_copy_from_pstr(data+len, HTTP_404_BODY);
        }
    }
    
    uip_send(data, len);
    return;
}

// serwuj treœæ dynamiczn¹
unsigned int http_serve_dynamic(unsigned char* data, unsigned char type) {

    unsigned int len = 0;

    //
    // wype³nij treœæ strony
    //
    switch(type) {
        case HTTP_PAGE_INFO:
            len += http_page_info(data+len);
            break;

        case HTTP_PAGE_COM_SETUP:
            len += http_page_com_setup(data+len);
            break;

        case HTTP_PAGE_IP_SETUP:
            len += http_page_ip_setup(data+len);
            break;

        case HTTP_PAGE_MODE:
            len += http_page_mode(data+len);
            break;

        case HTTP_PAGE_STATS:
            len += http_page_stats(data+len);
            break;

        case HTTP_PAGE_ADMIN:
            len += http_page_admin(data+len);
            break;

        case HTTP_PAGE_RESET:
            len += http_page_reset(data+len);
            break;

        case HTTP_PAGE_MANUAL:
            len += http_page_manual(data+len);
            break;
    }


    //
    // zamknij HTMLa
    //
    len += http_copy_from_pstr(data+len, PSTR("\n\n<address>"));
    len += http_copy_from_pstr(data+len, RS2ETH_SIGNATURE);
    len -= 21; // obetnij [mikrosterowniki.pl] z sygnaturki
    len += http_copy_from_pstr(data+len, PSTR("</address></div></body></html>"));

    //rs_int(type); rs_send(' '); rs_int(len); rs_newline(); //rs_dump(data, len);

    return len;
}

// generuj menu
unsigned int http_serve_menu(unsigned char* data, unsigned char type) {

    unsigned int len = 0;

    for (unsigned char item=1; item <= HTTP_MENU_ITEMS_COUNT; item++) {

        // oznacz aktywn¹ pozycjê
        if (item == type) {
            len += http_copy_from_pstr(data+len, PSTR("\n\t<li class=\"selected\""));
        }
        else {
            len += http_copy_from_pstr(data+len, PSTR("\n\t<li"));
        }

        // dodaj górny margines dla dokumentacji - ostatnia pozycja menu
        if (item == HTTP_MENU_ITEMS_COUNT) {
            len += http_copy_from_pstr(data+len, PSTR(" style=\"margin-top: 50px\""));
        }

        // odnoœnik
        len += http_copy_from_pstr(data+len, PSTR("><a href=\""));
        len += http_copy_from_pstr(data+len, HTTP_MENU_LINKS[item-1]);
        len += http_copy_from_pstr(data+len, PSTR("\">"));

        // pozycja menu
        len += http_copy_from_pstr(data+len, HTTP_MENU_ITEMS[item-1]);

        // zamknij
        len += http_copy_from_pstr(data+len, PSTR("</a></li>"));
    }

    // zamknij menu
    len += http_copy_from_pstr(data+len, PSTR("\n</ul>\n\n<div id=\"content\">\n"));

    // nag³ówek
    len += http_copy_from_pstr(data+len, PSTR("<h1>"));
    len += http_copy_from_pstr(data+len, HTTP_MENU_ITEMS[type-1]);
    len += http_copy_from_pstr(data+len, PSTR("</h1>\n\n"));

    return len;
}

// serwuj treœæ dynamiczn¹ generowan¹ jako odpowiedzi na ¿¹dania POST
unsigned int http_serve_post(unsigned char* data, unsigned char* query, unsigned char* body) {

    unsigned int len = 0;

    //rs_text(query); rs_dump(body, strlen(body));

    //
    // rozdziel zapytania POST
    //

    // konfiguracja portu COM
    if ( HTTP_QUERY_IS("/com") ) {
        len += http_post_com_setup(data, body);
    }
    // konfiguracja sieciowa
    if ( HTTP_QUERY_IS("/ip") ) {
        len += http_post_ip_setup(data, body);
    }
    // wykonanie resetu
    else if ( HTTP_QUERY_IS("/reset") ) {
        len += http_post_reset(data);
    }
    // wykonanie powrotu do domyœlnych ustawieñ
    else if ( HTTP_QUERY_IS("/default") ) {
        len += http_post_default(data);
    }
    // zmiana nazwy urz¹dzenia
    else if ( HTTP_QUERY_IS("/admin/name") ) {
        len += http_post_admin_name(data, body);
    }
    // zmiana numerów portów
    else if ( HTTP_QUERY_IS("/admin/ports") ) {
        len += http_post_admin_ports(data, body);
    }
    // zmiana loginu/has³a do panelu WWW
    else if ( HTTP_QUERY_IS("/admin/pass") ) {
        len += http_post_admin_pass(data, body);
    }
    // tryb pracy
    else if ( HTTP_QUERY_IS("/mode") ) {
        len += http_post_mode(data, body);
    }
    // temperatura via AJAX
    else if ( HTTP_QUERY_IS("/temp") ) {
        len += http_show_temperature(data, 1);
    }

    return len;
}

// serwuj treœæ statyczn¹
unsigned int http_serve_static(unsigned char* data, unsigned char* query) {

    unsigned int len = 0;

    //
    // PNG
    //
    if ( HTTP_QUERY_IS("favicon.png") ) {
        len += http_copy_from_pstr(data,      HTTP_200_PNG);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_FAVICON_PNG, HTTP_DATA_FAVICON_PNG_LENGTH);
    }
    else if ( HTTP_QUERY_IS("bg.png") ) {
        len += http_copy_from_pstr(data,      HTTP_200_PNG);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_BG_PNG, HTTP_DATA_BG_PNG_LENGTH);
    }
    else if ( HTTP_QUERY_IS("header_bg.png") ) {
        len += http_copy_from_pstr(data,      HTTP_200_PNG);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_HEADER_BG_PNG, HTTP_DATA_HEADER_BG_PNG_LENGTH);
    }
    else if ( HTTP_QUERY_IS("info.png") ) {
        len += http_copy_from_pstr(data,      HTTP_200_PNG);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_INFO_PNG, HTTP_DATA_INFO_PNG_LENGTH);
    }
    else if ( HTTP_QUERY_IS("loading.gif") ) {
        len += http_copy_from_pstr(data,      HTTP_200_GIF);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_LOADING_GIF, HTTP_DATA_LOADING_GIF_LENGTH);
    }
    //
    // CSS
    //
    else if ( HTTP_QUERY_IS("css.css") ) {
        len += http_copy_from_pstr(data,      HTTP_200_CSS);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_CSS_CSS, HTTP_DATA_CSS_CSS_LENGTH);
    }
    else if ( HTTP_QUERY_IS("layout.css") ) {
        len += http_copy_from_pstr(data,      HTTP_200_CSS);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_LAYOUT_CSS, HTTP_DATA_LAYOUT_CSS_LENGTH);
    }
    else if ( HTTP_QUERY_IS("content.css") ) {
        len += http_copy_from_pstr(data,      HTTP_200_CSS);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_CONTENT_CSS, HTTP_DATA_CONTENT_CSS_LENGTH);
    }
    //
    // JS
    //
    else if ( HTTP_QUERY_IS("js.js") ) {
        len += http_copy_from_pstr(data,      HTTP_200_JS);
        len += http_copy_from_pstr(data+len,  HTTP_SERVER);
        len += http_copy_from_flash(data+len, HTTP_DATA_JS_JS, HTTP_DATA_JS_JS_LENGTH);
    }


    return len;
}

// sprawdŸ dane autoryzacyjne u¿ytkownika
unsigned char http_authenticate_user(unsigned char* auth_key) {
    return (auth_key && strcmp_P((char*)auth_key, HTTP_AUTH_STRING) == 0) ? 1 : 0;
}

// wykonuj czynnoœci okresowe: zamykanie otwartych po³¹czeñ, "dosy³anie" reszty danych
void http_periodic() {

    // zamknij po³¹czenie z klientem
    if (uip_conn->appstate.request == 0) {
        uip_close();
    }
    // doœlij statyczn¹ zawartoœæ
    else {
        unsigned int len = http_serve_dynamic((unsigned char*)uip_appdata, uip_conn->appstate.request);

        uip_send(uip_appdata, len);

        // zamknij po³¹czenie przy nastêpnym odpytywaniu
        uip_conn->appstate.request = 0;
    }
}

// kopiuje podany ³añcuch z pamiêci FLASH do pamiêci RAM oraz zwraca d³ugoœæ kopiowanego ³añcucha
unsigned int http_copy_from_pstr(unsigned char* buf, PGM_VOID_P src) {
    unsigned int len = strlen_P(src);
    memcpy_P(buf, src, len);
    return len;
}

// kopiuje podany obszar z pamiêci FLASH do pamiêci RAM oraz zwraca d³ugoœæ kopiowanego ³añcucha
unsigned int http_copy_from_flash(unsigned char* buf, PGM_VOID_P src, unsigned int len) {
    memcpy_P(buf, src, len);
    return len;
}

// wstawia podan¹ wartoœæ numeryczn¹ jako ³añcuch do pamiêci RAM oraz zwraca d³ugoœæ wstawionego ³añcucha
unsigned int http_copy_from_val(unsigned char* buf, unsigned long val) {
    ltoa(val, (char*) buf, 10);
    return strlen((char*)buf);
}

// wstawia podany adres IP jako ³añcuch do pamiêci RAM oraz zwraca d³ugoœæ wstawionego ³añcucha
unsigned int http_copy_from_ip(unsigned char* buf, uip_ipaddr_t* ip) {
    unsigned int len = 0;
    unsigned char* tab = (unsigned char*)ip;

    for (i=0; i<4; i++) {
        len += http_copy_from_val(buf+len, tab[i]);
        buf[len++] = '.';
    }

    // obetnij kropkê
    return len-1;
}

// zwraca wskaŸnik do wartoœci podanego parametru zapytania
// foo=bar&ble=boo
unsigned char http_get_query_param(unsigned char* query, PGM_VOID_P param, unsigned char* buf) {

    unsigned len = 0;
    char* ptr;
    char* end;
    
    // znajdŸ nazwê parametru
    ptr = strstr_P((char*)query, param);

    // nie znaleziono
    if (!ptr) {
        return 0;
    }

    // sprawdŸ obecnoœæ znaku = po parametrze
    ptr += strlen_P(param);

    if (*ptr != '=') {
        return 0;
    }

    ptr++;

    // znajdŸ znak & lub koniec ³añcucha
    end = strchr(ptr, '&');

    if (!end) {
        end = ptr + strlen(ptr);
    }

    // dekoduj wartoœæ parametru
    while( (ptr < end) && (len < 128) ) {
        switch(*ptr) {
            case '+':
                buf[len++] = ' ';
                break;

            // %26 -> &
            case '%':
                ptr++;
                buf[len] = *ptr;
                ptr++;
                buf[len] = (hex2dec(buf[len]) << 4) | hex2dec(*ptr);
                len++;
                break;

            default:
                buf[len++] = *ptr;
        }

        ptr++;
    }

    // zamknij ³añcuch
    buf[len] = 0;

    return len;
}

// parsuj podany ³añcuch z adresem IP
unsigned char http_parse_ip(unsigned char* str, uip_ipaddr_t* ip) {
    
    // bufor na przeparsowany adres IP
    unsigned char *buf = (unsigned char*) ip;
    unsigned char *ptr = str;
    unsigned char byte = 0;

    // parsuj kolejne bajty adresu IP
    // 192.168.1.12
    while ( (ptr) && (byte < 4) ) {
        // znajdŸ nastêpn¹ kropkê
        ptr = (unsigned char*) strchr((char*)str, '.');

        if (ptr) {
            *ptr = 0;
        }

        // sprawdŸ rozmiar fragmentu adresu IP
        if ( (strlen((char*)str) == 0) || (strlen((char*)str) > 3) ) {
            return 0;
        }

        // przeparsuj ³añcuch do wartoœci liczbowej
        buf[byte++] = atoi((char*)str);

        // przesuñ wskaŸnik
        str = ptr+1;
    }

    if (byte != 4) {
        return 0;
    }
    
    return 1;
}

//
// sta³e przechowywane w pamiêci FLASH
//

// nag³ówek Server
const char HTTP_SERVER[] PROGMEM       = "\nServer: rs2eth ($Rev: 206 $) ATmega644 [mikrosterowniki.pl]\nX-Powered-By: AVR + uIP 1.0\n\n";
const char HTTP_AUTHENTICATE[] PROGMEM = "\nWWW-Authenticate: Basic realm=\"rs2eth\"";

// OK
const char HTTP_200_HTML[] PROGMEM = "HTTP/1.0 200 OK\nContent-Type: text/html; charset=utf-8\nPragma: no-cache";
const char HTTP_200_BODY[] PROGMEM   = "<b>rs2eth</b> | hello";

// HTTP 200 + typ odpowiedzi
const char HTTP_200_PNG[]  PROGMEM = "HTTP/1.0 200 OK\nContent-Type: image/png\nExpires: Fri, 18 Jan 2019 21:12:31 GMT";
const char HTTP_200_GIF[]  PROGMEM = "HTTP/1.0 200 OK\nContent-Type: image/gif\nExpires: Fri, 18 Jan 2019 21:12:31 GMT";
const char HTTP_200_CSS[]  PROGMEM = "HTTP/1.0 200 OK\nContent-Type: text/css\nExpires: Fri, 18 Jan 2019 21:12:31 GMT";
const char HTTP_200_JS[]   PROGMEM = "HTTP/1.0 200 OK\nContent-Type: application/x-javascript\nExpires: Fri, 18 Jan 2019 21:12:31 GMT";
const char HTTP_200_TEXT[] PROGMEM = "HTTP/1.0 200 OK\nContent-Type: text/plain; charset=utf-8\nPragma: no-cache";

// przekierowanie
const char HTTP_301_REDIRECT[] PROGMEM = "HTTP/1.0 301 Moved Permanently\n";

// informacje o b³êdach HTTP
const char HTTP_401_HEADER[] PROGMEM = "HTTP/1.0 401 Unauthorized\nContent-Type: text/html";
const char HTTP_401_BODY[] PROGMEM   = "<b>rs2eth</b> | 401 - nieupowa¿niony dostêp";
const char HTTP_404_HEADER[] PROGMEM = "HTTP/1.0 404 Not Found\nContent-Type: text/html";
const char HTTP_404_BODY[] PROGMEM   = "<b>rs2eth</b> | 404 - nie znaleziono";

// ci¹g autoryzuj¹cy -> generowany przy zmianie has³a (base encode64)
const char HTTP_AUTH_STRING[] PROGMEM = "YWRtaW46YWRtaW4="; // admin/admin
