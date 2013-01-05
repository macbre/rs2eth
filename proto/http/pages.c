#include "pages.h"

// informacja o koniecznoœci wykonania resetu po zmianie niektórych ustawieñ
const char INFO_RESET_NEEDED[] PROGMEM = "<p class=\"info\">Aby uaktywni\xc4\x87\
 zmienione ustawienia, nale\xc5\xbc\
y <a href=\"/reset\">ponownie uruchomi\xc4\x87\
 konwerter</a></p>\n";

// strona glowna
unsigned int http_page_info(unsigned char* data) {
    unsigned int len = 0;
    struct rs_current_setup_t *setup;
    config_t* conf = config_get();
    
    len += http_copy_from_pstr(data+len, PSTR("<h3>Stan urzÄ…dzenia</h3>\n<dl>"));

    //
    // ustawienia portu COM
    //
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Port COM</dt><dd>"));

    // predkosc
    setup = rs_get_setup();
    len += http_copy_from_val(data+len, setup->baud);
    data[len++] = ' ';

    // bity danych
    data[len++] = ('0' + setup->data_bits);

    // bity parzystosci
    switch (setup->parity_bits) {
        case 1: data[len++] = 'N'; break;
        case 2: data[len++] = 'O'; break;
        case 3: data[len++] = 'E'; break;
    }

    // bity stopu
    data[len++] = ('0' + setup->stop_bits);

    // ew. obs³uga RS485
    #ifdef RS_485
        len += http_copy_from_pstr(data+len, PSTR(" (RS485)"));
    #endif

    //
    // telnet
    //
    len += http_copy_from_pstr(data+len, PSTR("</dd>\n\t<dt>Telnet</dt><dd>"));

    if (conf->mode == MODE_CLIENT) {
            len += http_copy_from_pstr(data+len, PSTR("tryb klienta (zdalny serwer "));
            len += http_copy_from_ip(data+len, (uip_ipaddr_t*) &telnet_server_ip);
            data[len++] = ':';
            len += http_copy_from_val(data+len, telnet_server_port);
            data[len++] = ')';

            if (server_state & SERVER_STATE_CLIENT_MODE_CONN) {
                len += http_copy_from_pstr(data+len, PSTR(" - poÅ‚Ä…czony"));
            }
    }
    else {
        if (server_state & SERVER_STATE_TELNET) {
            len += http_copy_from_pstr(data+len, PSTR("podÅ‚Ä…czony klient ("));

            // wyswietl IP
            len += http_copy_from_ip(data+len, (uip_ipaddr_t*) telnet_get_client_ip());
        
            data[len++] = ')';
        }
        else {
            len += http_copy_from_pstr(data+len, PSTR("niepodÅ‚Ä…czony"));

            // telnetowy port nas³uchuj¹cy
            len += http_copy_from_pstr(data+len, PSTR(" (oczekiwanie na porcie "));
            len += http_copy_from_val(data+len, telnet_get_port());
            data[len++] = ')';
        }
    }

    //
    // IP
    //
    unsigned char* buf = data+len+100;
    uip_gethostaddr(buf);

    len += http_copy_from_pstr(data+len, PSTR("</dd>\n\t<dt>Adres IP</dt><dd>"));
    len += http_copy_from_ip(data+len, (uip_ipaddr_t*) buf);

    if (dhcp_done()) {
        len += http_copy_from_pstr(data+len, PSTR(" (z DHCP)"));
    }

    //
    // MAC
    //
    buf = nic_get_mac();

    len += http_copy_from_pstr(data+len, PSTR("</dd>\n\t<dt>Adres MAC</dt><dd><tt>"));

    // wyswietl MAC'a
    for (i=0; i<6; i++) {
        data[len++] = dec2hex(buf[i] >> 4);
        data[len++] = dec2hex(buf[i] & 0x0f);
        data[len++] = ':';
    }

    // usun ostatni dwukropek
    len--;

    len += http_copy_from_pstr(data+len, PSTR("</tt></dd>"));


    //
    // nazwa urz¹dzenia
    //
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Nazwa urzÄ…dzenia</dt><dd>"));

    strcpy((char*) (data+len), conf->name);
    len += strlen(conf->name);

    //
    // temperatura
    //
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Temperatura</dt><dd><span id=\"temp\">"));

    if (ds18b20_temp < 0) {
        data[len++] = '-';
    }
    len += http_copy_from_val(data+len, abs(ds18b20_temp)/10);
    data[len++] = '.';
    data[len++] = '0' + (abs(ds18b20_temp)%10);

    len += http_copy_from_pstr(data+len, PSTR("</span>&deg;C</dd>"));

    //
    // info z mikrosterowniki.pl
    //
    len += http_copy_from_pstr(data+len, PSTR("\n</dl>\n\n<h3>mikrosterowniki.pl - co nowego?</h3>\n<div id=\"news\" class=\"loading\">&nbsp;</div>\n\n"));

    // JS
    len += http_copy_from_pstr(data+len, PSTR("<script type=\"text/javascript\">\n\
function updateTemp() {\n\
\tLiteAjax('/temp', function(data) {\n\
\t\t$('temp').innerHTML = data.temp.toFixed(1);\n\
\t});\n\
\tsetTimeout('updateTemp()', 1000);\n\
}\n\
function fillNews(data) {\n\
\t$('news').className = '';\n\
\t$('news').innerHTML = data;\n\
}\n\
updateTemp();\n\
</script>\n\
<script type=\"text/javascript\" src=\"http://api.macbre.net/us/rs2eth.js?callback=fillNews\"></script>\n"));

    return len;
}

// dostepne opcje ustawienia portu COM
//prog_uint32_t com_baud_rates[] = {150, 300, 600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 14400, 19200, 28800, 38400, 57600, 115200UL, 128000UL, 256000Ul}; /* 18 */
prog_uint32_t com_baud_rates[] = {600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 14400, 19200, 28800, 38400, 57600, 115200UL, 128000UL, 256000Ul}; /* 16 */
prog_char com_parity[] = {'N', 'O', 'E'};

#define COM_BAUD_RATES_COUNT 16

// ustawienia portu COM
unsigned int http_page_com_setup(unsigned char* data) {

    unsigned int len = 0;
    unsigned long baud;

    // aktualne ustawienia portu COM
    struct rs_current_setup_t *setup = rs_get_setup();

    // formularz wyboru ustawien portu
    len += http_copy_from_pstr(data+len, PSTR("<form method=\"post\" action=\"/com\"><dl>\n"));

    // predkosci
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>PrÄ™dko\xc5\x9b\xc4\x87</dt><dd><select name=\"baud\">"));

    for (i=0; i<COM_BAUD_RATES_COUNT; i++) {
        baud = pgm_read_dword(&com_baud_rates[i]);
        len += http_copy_from_pstr(data+len, PSTR("<option value=\""));
        len += http_copy_from_val(data+len, i);
        
        // wybierz aktualna predkosc
        if (baud == setup->baud) {
            len += http_copy_from_pstr(data+len, PSTR("\" selected=\"selected"));
        }

        data[len++] = '"';
        data[len++] = '>';

        len += http_copy_from_val(data+len, baud);
        len += http_copy_from_pstr(data+len, PSTR("</option>"));
    }

    len += http_copy_from_pstr(data+len, PSTR("</select></dd>\n"));

    // bity danych
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Bity danych</dt><dd><select name=\"data\">"));

    for (i=5; i<=8; i++) {
        len += http_copy_from_pstr(data+len, PSTR("<option value=\""));
        data[len++] = '0' + i;
        
        if (i == setup->data_bits) {
            len += http_copy_from_pstr(data+len, PSTR("\" selected=\"selected"));
        }

        data[len++] = '"';
        data[len++] = '>';

        data[len++] = '0' + i;
        len += http_copy_from_pstr(data+len, PSTR("</option>"));
    }

    len += http_copy_from_pstr(data+len, PSTR("</select></dd>\n"));

    // parzystosc
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Kontrola parzysto\xc5\x9b"));
    len += http_copy_from_pstr(data+len, PSTR("ci</dt><dd><select name=\"parity\">"));

    for (i=0; i<=2; i++) {
        len += http_copy_from_pstr(data+len, PSTR("<option value=\""));
        data[len++] = '1' + i;
        
        if ( (i+1) == setup->parity_bits ) {
            len += http_copy_from_pstr(data+len, PSTR("\" selected=\"selected"));
        }

        data[len++] = '"';
        data[len++] = '>';

        data[len++] = pgm_read_byte(&com_parity[i]);
        len += http_copy_from_pstr(data+len, PSTR("</option>"));
    }

    len += http_copy_from_pstr(data+len, PSTR("</select></dd>\n"));

    // bity stopu
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Bity stopu</dt><dd><select name=\"stop\">"));

    for (i=1; i<=2; i++) {
        len += http_copy_from_pstr(data+len, PSTR("<option value=\""));
        data[len++] = '0' + i;
        
        if ( i == setup->stop_bits ) {
            len += http_copy_from_pstr(data+len, PSTR("\" selected=\"selected"));
        }

        data[len++] = '"';
        data[len++] = '>';

        data[len++] = '0' + i;
        len += http_copy_from_pstr(data+len, PSTR("</option>"));
    }

    len += http_copy_from_pstr(data+len, PSTR("</select></dd>\n"));

    // zamknij formularz
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Zapisz\" class=\"submit\" /></dd></dl></form>\n"));

    //rs_int(len); rs_newline(); rs_dump(data, len);

    return len;
}

// zapisz ustawienia portu COM
unsigned int http_post_com_setup(unsigned char* data, unsigned char* body) {
    unsigned int len = 0;
    unsigned char* buf = data+512;

    // aktualne ustawienia portu COM
    struct rs_current_setup_t *setup = rs_get_setup();

    //rs_text((char*)body); rs_newline();

    // parsuj predkosc
    if (http_get_query_param(body, PSTR("baud"), buf)) {
        len = abs(atoi((char*)buf));

        // ogranicz indeks
        len = (len > COM_BAUD_RATES_COUNT-1) ? (COM_BAUD_RATES_COUNT-1) : len;
        setup->baud = pgm_read_dword(&com_baud_rates[len]);
    }

    // parsuj bity danych
    if (http_get_query_param(body, PSTR("data"), buf)) {
        setup->data_bits = atoi((char*)buf);
    }

    // parsuj ustawienia parzystosci
    if (http_get_query_param(body, PSTR("parity"), buf)) {
        setup->parity_bits = atoi((char*)buf);
    }

    // parsuj bity stopu
    if (http_get_query_param(body, PSTR("stop"), buf)) {
        setup->stop_bits = atoi((char*)buf);
    }

    // zmieñ ustawienia portu RS
    rs_setup(setup->baud, setup->data_bits, setup->parity_bits, setup->stop_bits);

    // zapisz zmiany do pamiêci EEPROM
    config_t* conf = config_get();

    conf->rs_baud        = setup->baud;
    conf->rs_data_bits   = setup->data_bits;
    conf->rs_parity_bits = setup->parity_bits;
    conf->rs_stop_bits   = setup->stop_bits;

    config_save();

    // przekieruj na zakladke ustawien portu COM
    len = 0;
    len += http_copy_from_pstr(data,      HTTP_301_REDIRECT);
    len += http_copy_from_pstr(data+len,  PSTR("Location: /com"));
    len += http_copy_from_pstr(data+len,  HTTP_SERVER);

    return len;
}

// konfiguracja sieciowa
unsigned int http_page_ip_setup(unsigned char* data) {
    unsigned int len = 0;
    config_t* conf = config_get();

     // JS
    len += http_copy_from_pstr(data+len, PSTR("<script type=\"text/javascript\">\n\
function switchFields(checkbox) {\n\
\tvar fields = $('form').getElementsByTagName('input');\n\
\tfor(f=1; f<=3; f++) {\n\
\t\tfields[f].disabled = checkbox.checked;\n\
\t}\n\
}\n\
window.onload = function() {switchFields($('dhcp'));}\n\
</script>\n\n"));

    len += http_copy_from_pstr(data+len, INFO_RESET_NEEDED);

    len += http_copy_from_pstr(data+len, PSTR("<form method=\"post\" action=\"/ip\" id=\"form\"><dl>\n"));

    // DHCP
    len += http_copy_from_pstr(data+len, PSTR("\t<dt><label for=\"dhcp\">Autokonfiguracja (DHCP)</label></dt><dd><input type=\"checkbox\" name=\"dhcp\" id=\"dhcp\""));
    if (conf->use_dhcp) {
        len += http_copy_from_pstr(data+len, PSTR(" checked=\"checked\""));
    }
    len += http_copy_from_pstr(data+len, PSTR("\" style=\"width:auto;border:none\" onclick=\"this.blur()\" onchange=\"switchFields(this)\" /></dd>\n"));

    // IP
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Adres IP</dt><dd><input type=\"text\" name=\"ip\" size=\"15\" value=\""));
    len += http_copy_from_ip(data+len, (uip_ipaddr_t*) (conf->ip) );
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    // maska
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Maska</dt><dd><input type=\"text\" name=\"mask\" size=\"15\" value=\""));
    len += http_copy_from_ip(data+len, (uip_ipaddr_t*) (conf->mask) );
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    // brama
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Brama</dt><dd><input type=\"text\" name=\"gate\" size=\"15\" value=\""));
    len += http_copy_from_ip(data+len, (uip_ipaddr_t*) (conf->gate) );
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    // zapisz
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Zapisz\" class=\"submit\" /></dd>\n</dl></form>\n\n"));
    
    return len;
}

// zapisz konfiguracjê sieciow¹
unsigned int http_post_ip_setup(unsigned char* data, unsigned char* body) {
    unsigned int len = 0;
    unsigned char* val  = data+1300;
    unsigned char* addr = data+1400;

    // pobierz aktualne ustawienia
    config_t* conf = config_get();

    // DHCP
    conf->use_dhcp = 0;

    if (http_get_query_param(body, PSTR("dhcp"), val)) {
        // $_POST['dhcp'] == 'on'
        if ( (val[0] == 'o') && (val[1] == 'n') ) {
            conf->use_dhcp = 1;
        }
    }

    // IP
    if (http_get_query_param(body, PSTR("ip"), val)) {
        if ( http_parse_ip(val, (uip_ipaddr_t*) addr) ) {
            memcpy(conf->ip, addr, 4);
        }
    }

    // brama
    if (http_get_query_param(body, PSTR("gate"), val)) {
        if ( http_parse_ip(val, (uip_ipaddr_t*) addr) ) {
            memcpy(conf->gate, addr, 4);
        }
    }

    // maska
    // TODO: walidacja
    if (http_get_query_param(body, PSTR("mask"), val)) {
        if ( http_parse_ip(val, (uip_ipaddr_t*) addr) ) {
            memcpy(conf->mask, addr, 4);
        }
    }

    // zapisz zmiany do pamiêci EEPROM
    config_save();

    // przekieruj na zakladke ustawien sieciowych
    len = 0;
    len += http_copy_from_pstr(data,      HTTP_301_REDIRECT);
    len += http_copy_from_pstr(data+len,  PSTR("Location: /ip"));
    len += http_copy_from_pstr(data+len,  HTTP_SERVER);

    return len;
}

// tryb pracy
unsigned int http_page_mode(unsigned char* data) {
    unsigned int len = 0;
    config_t* conf = config_get();

     // JS
    len += http_copy_from_pstr(data+len, PSTR("<script type=\"text/javascript\">\n\
function switchFields(checkbox) {\n\
\tvar fields = $('form').getElementsByTagName('input');\n\
\tfor(f=1; f<=2; f++) {\n\
\t\tfields[f].disabled = checkbox.checked;\n\
\t}\n\
}\n\
window.onload = function() {switchFields($('server'));}\n\
</script>\n\n"));

    len += http_copy_from_pstr(data+len, INFO_RESET_NEEDED);

    len += http_copy_from_pstr(data+len, PSTR("<form method=\"post\" action=\"/mode\" id=\"form\"><dl>\n"));

    // tryb pracy
    len += http_copy_from_pstr(data+len, PSTR("\t<dt><label for=\"server\">Pracuj w trybie serwera</label></dt><dd><input type=\"checkbox\" name=\"server\" id=\"server\""));
    if (conf->mode == MODE_SERVER) {
        len += http_copy_from_pstr(data+len, PSTR(" checked=\"checked\""));
    }
    len += http_copy_from_pstr(data+len, PSTR("\" style=\"width:auto;border:none\" onclick=\"this.blur()\" onchange=\"switchFields(this)\" /></dd>\n"));

    // IP serwera
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Adres IP zdalnego serwera</dt><dd><input type=\"text\" name=\"ip\" size=\"15\" value=\""));
    len += http_copy_from_ip(data+len, (uip_ipaddr_t*) (conf->server_ip) );
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    // port serwera
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Port serwera</dt><dd><input type=\"text\" name=\"port\" size=\"5\" value=\""));
    len += http_copy_from_val(data+len, conf->server_port);
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    // zapisz
    len += http_copy_from_pstr(data+len, PSTR("\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Zapisz\" class=\"submit\" /></dd>\n</dl></form>\n\n"));
    
    return len;
    
    return len;
}

// zapisz konfiguracjê trybu pracy
unsigned int http_post_mode(unsigned char* data, unsigned char* body) {
    unsigned int len = 0;
    unsigned char* val  = data+1300;
    unsigned char* addr = data+1400;

    // pobierz aktualne ustawienia
    config_t* conf = config_get();

    // tryb pracy
    conf->mode = MODE_CLIENT;

    if (http_get_query_param(body, PSTR("server"), val)) {
        // $_POST['server'] == 'on'
        if ( (val[0] == 'o') && (val[1] == 'n') ) {
            conf->mode = MODE_SERVER;
        }
    }

    // IP
    if (http_get_query_param(body, PSTR("ip"), val)) {
        if ( http_parse_ip(val, (uip_ipaddr_t*) addr) ) {
            memcpy(conf->server_ip, addr, 4);
        }
    }

    // port
    if (http_get_query_param(body, PSTR("port"), val)) {
        len = abs(atoi((char*)val));

        if (len > 0 && len < 65535) {
            conf->server_port = len;
        }
    }

    // zapisz zmiany do pamiêci EEPROM
    config_save();

    // przekieruj na zakladke ustawien sieciowych
    len = 0;
    len += http_copy_from_pstr(data,      HTTP_301_REDIRECT);
    len += http_copy_from_pstr(data+len,  PSTR("Location: /mode"));
    len += http_copy_from_pstr(data+len,  HTTP_SERVER);

    return len;
}


// statystyki
unsigned int http_page_stats(unsigned char* data) {
    unsigned int len = 0;

    //
    // ENC28
    //
    len += http_copy_from_pstr(data+len, PSTR("<h3>Sterownik sieciowy</h3>\n<dl>"));

    // wersja ENC28
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Sterownik ETH</dt><dd>enc28j60 rev. B"));
    data[len++] = '0' + enc28_read_rev_id();

    // Duplex?
    len += http_copy_from_pstr(data+len, PSTR("</dd>\n\t<dt>Full-Duplex</dt><dd>"));
    len += http_copy_from_pstr(data+len, (enc28_is_full_duplex() ? PSTR("tak</dd>") : PSTR("nie</dd>")) );

    // poprawna polaryzacja skretki?
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Polaryzacja TPIN</dt><dd>"));
    len += http_copy_from_pstr(data+len, (enc28_is_correct_polarity() ? PSTR("poprawna</dd>") : PSTR("odwrotna</dd>")) );

    //
    // stos TCP/IP
    //
    #ifdef UIP_STATISTICS
    len += http_copy_from_pstr(data+len, PSTR("\n</dl>\n\n<h3>Warstwa sieciowa</h3>\n<dl>"));
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Stos TCP/IP</dt><dd>uIP 1.0</dd>"));
    /*
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>MSS</dt><dd><tt>"));
    len += http_copy_from_val(data+len, UIP_TCP_MSS);
    len += http_copy_from_pstr(data+len, PSTR("</tt></dd>"));
    */

    // IP
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Pakiety IP</dt><dd><tt>Tx: "));
    len += http_copy_from_val(data+len, uip_stat.ip.sent);
    len += http_copy_from_pstr(data+len, PSTR(" / Rx: "));
    len += http_copy_from_val(data+len, uip_stat.ip.recv);
    len += http_copy_from_pstr(data+len, PSTR(" / Drop: "));
    len += http_copy_from_val(data+len, uip_stat.ip.drop);
    len += http_copy_from_pstr(data+len, PSTR("</tt></dd>"));

    // ICMP
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Pakiety ICMP</dt><dd><tt>Tx: "));
    len += http_copy_from_val(data+len, uip_stat.icmp.sent);
    len += http_copy_from_pstr(data+len, PSTR(" / Rx: "));
    len += http_copy_from_val(data+len, uip_stat.icmp.recv);
    len += http_copy_from_pstr(data+len, PSTR(" / Drop: "));
    len += http_copy_from_val(data+len, uip_stat.icmp.drop);
    len += http_copy_from_pstr(data+len, PSTR("</tt></dd>"));

    // TCP
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Pakiety TCP</dt><dd><tt>Tx: "));
    len += http_copy_from_val(data+len, uip_stat.tcp.sent);
    len += http_copy_from_pstr(data+len, PSTR(" / Rx: "));
    len += http_copy_from_val(data+len, uip_stat.tcp.recv);
    len += http_copy_from_pstr(data+len, PSTR(" / Drop: "));
    len += http_copy_from_val(data+len, uip_stat.tcp.drop);
    len += http_copy_from_pstr(data+len, PSTR("</tt></dd>"));

    // UDP
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Pakiety UDP</dt><dd><tt>Tx: "));
    len += http_copy_from_val(data+len, uip_stat.udp.sent);
    len += http_copy_from_pstr(data+len, PSTR(" / Rx: "));
    len += http_copy_from_val(data+len, uip_stat.udp.recv);
    len += http_copy_from_pstr(data+len, PSTR(" / Drop: "));
    len += http_copy_from_val(data+len, uip_stat.udp.drop);
    len += http_copy_from_pstr(data+len, PSTR("</tt></dd>"));

    // tablica ARP
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Wpisy w tablicy ARP</dt><dd>"));
    len += http_copy_from_val(data+len, uip_arp_entries());
    data[len++] = ' ';
    data[len++] = '(';
    len += http_copy_from_val(data+len, UIP_ARPTAB_SIZE);
    len += http_copy_from_pstr(data+len, PSTR(" miejsc)</dd>"));
    #endif

    //
    // port COM
    //
    len += http_copy_from_pstr(data+len, PSTR("\n</dl>\n\n<h3>Port COM</h3>\n<dl>"));

    // rozmiar bufora
    len += http_copy_from_pstr(data+len, PSTR("\n\t<dt>Rozmiar bufora odbiorczego</dt><dd>"));
    len += http_copy_from_val(data+len, COM_BUF_SIZE);
    len += http_copy_from_pstr(data+len, PSTR(" bajt\xc3\xb3\
w"));

    // bajty odebrane/wys³ane
    #ifdef RS_STATS
    len += http_copy_from_pstr(data+len, PSTR("</dd>\n\t<dt>Odebrane bajty</dt><dd>"));
    len += http_copy_from_val(data+len, rs_stats.recv);
    len += http_copy_from_pstr(data+len, PSTR("</dd>\n\t<dt>WysÅ‚ane bajty</dt><dd>"));
    len += http_copy_from_val(data+len, rs_stats.sent);
    #endif

    // koniec listy <dl>
    len += http_copy_from_pstr(data+len, PSTR("</dd>\n</dl>"));

    return len;
}

// administracja
unsigned int http_page_admin(unsigned char* data) {
    unsigned int len = 0;
    config_t* conf = config_get();

    // nazwa urz¹dzenia
    len += http_copy_from_pstr(data+len, PSTR("<h3>Nazwa urzÄ…dzenia</h3>\n"));
    len += http_copy_from_pstr(data+len, PSTR("<form method=\"post\" action=\"/admin/name\"><dl>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Nazwa urzÄ…dzenia</dt><dd><input type=\"text\" name=\"name\" size=\"15\" value=\""));
    strcpy((char*) (data+len), conf->name);
    len += strlen(conf->name);
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Zmie\xc5\x84\
\" class=\"submit\" /></dd>\n</dl></form>\n\n"));

    // przypisania portów
    len += http_copy_from_pstr(data+len, PSTR("<h3>Przypisane porty</h3>\n"));

    len += http_copy_from_pstr(data+len, INFO_RESET_NEEDED);
    len += http_copy_from_pstr(data+len, PSTR("<form method=\"post\" action=\"/admin/ports\"><dl>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Port TELNET</dt><dd><input type=\"text\" name=\"telnet\" size=\"5\" value=\""));
    len += http_copy_from_val(data+len, conf->telnet_port);
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Zmie\xc5\x84\
\" class=\"submit\" /></dd>\n</dl></form>\n\n"));

    // has³o
    len += http_copy_from_pstr(data+len, PSTR("<h3>Zmie\xc5\x84\
 login / hasÅ‚o</h3>\n"));
 
    len += http_copy_from_pstr(data+len, INFO_RESET_NEEDED);
    len += http_copy_from_pstr(data+len, PSTR("<form method=\"post\" action=\"/admin/pass\"><dl>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Login</dt><dd><input type=\"text\" name=\"login\" size=\"15\" value=\""));
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Nowe hasÅ‚o</dt><dd><input type=\"password\" name=\"pass1\" size=\"15\" value=\""));
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>Nowe hasÅ‚o (powt\xc3\xb3\
rz)</dt><dd><input type=\"password\" name=\"pass2\" size=\"15\" value=\""));
    len += http_copy_from_pstr(data+len, PSTR("\" /></dd>\n"));

    len += http_copy_from_pstr(data+len, PSTR("\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Zmie\xc5\x84\
\" class=\"submit\" /></dd>\n</dl></form>\n\n"));
    
    return len;
}

// zmiana nazwy urz¹dzenia
unsigned int http_post_admin_name(unsigned char* data, unsigned char* body) {
    unsigned int len = 0;
    unsigned char* buf = data+512;
    config_t* conf = config_get();

    // parsuj nazwê
    if (http_get_query_param(body, PSTR("name"), buf)) {

        // wyczyœæ pamiêæ
        memset(conf->name, 0, 16);

        for(i=0; (i<strlen((char*)buf)) && (i<15); i++) {
            // dozwolone tylko znaki alfanumeryczne, spacjê i myœlnik
            if (isalnum(buf[i]) || (buf[i] == '-') || (buf[i] == ' ')) {
                conf->name[len++] = buf[i];
            }
        }
        // zakoñcz NULLem
        conf->name[len++] = 0;

        // zapisz do EEPROMu
        config_save();
    }

    // przekieruj na zakladke ustawien portu COM
    len = 0;
    len += http_copy_from_pstr(data,      HTTP_301_REDIRECT);
    len += http_copy_from_pstr(data+len,  PSTR("Location: /admin"));
    len += http_copy_from_pstr(data+len,  HTTP_SERVER);

    return len;
}

// zmiana numeru portów
unsigned int http_post_admin_ports(unsigned char* data, unsigned char* body) {
    unsigned int len = 0;
    config_t* conf = config_get();
    unsigned char* buf = data+512;

    // parsuj port
    if (http_get_query_param(body, PSTR("telnet"), buf)) {
        
        len = abs(atoi((char*)buf));

        if (len > 0 && len < 65535) {
            conf->telnet_port = len;

            // zapisz do EEPROMu
            config_save();
        }
    }

    // przekieruj na zakladke ustawien portu COM
    len = 0;
    len += http_copy_from_pstr(data,      HTTP_301_REDIRECT);
    len += http_copy_from_pstr(data+len,  PSTR("Location: /admin"));
    len += http_copy_from_pstr(data+len,  HTTP_SERVER);

    return len;
}

// zmiana loginu/has³a do panelu WWW
unsigned int http_post_admin_pass(unsigned char* data, unsigned char* body) {
    return 0;
}



// uruchom ponownie
unsigned int http_page_reset(unsigned char* data) {
    unsigned int len = 0;

    // JS
    len += http_copy_from_pstr(data+len, PSTR("<script type=\"text/javascript\">\n\
function action(url, button) {\n\
\tbutton.disabled = true;\n\tbutton.parentNode.className = 'loading';\n\tLiteAjax(url);\n\tsetTimeout('document.location.href=\"/\"', 3000);\n\
}\n\
function reset(button) {\n\
\tif (confirm('Czy na pewno chcesz ponownie uruchomi\xc4\x87\
 konwerter?')) {\n\
\t\taction('/reset', button);\n\
\t}\n\
}\n\
function set_default(button) {\n\
\tif (confirm('Czy na pewno chcesz ponownie wprowadzi\xc4\x87\
 domy\xc5\x9b\
lne ustawienia?')) {\n\
\t\taction('/default', button);\n\
\t}\n\
}\n\
</script>\n\n"));

    // info
    len += http_copy_from_pstr(data+len, PSTR("<p class=\"info\">Aby uaktywni\xc4\x87\
 zmienione ustawienia, nale\xc5\xbc\
y ponownie uruchomi\xc4\x87\
 konwerter</p>\n"));
    
    // formularz - uruchom ponownie
    len += http_copy_from_pstr(data+len, PSTR("<dl>\n\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Uruchom ponownie\" class=\"submit\" onclick=\"reset(this)\"/></dd>\n</dl>\n"));

    // formularz - domyœlne ustawienia
    len += http_copy_from_pstr(data+len, PSTR("<dl>\n\t<dt>&nbsp;</dt><dd><input type=\"submit\" value=\"Domy\xc5\x9b\
lne ustawienia\" class=\"submit\" onclick=\"set_default(this)\"/></dd>\n</dl>\n"));
    
    return len;
}

// wykonanie ponownego uruchomienia
unsigned int http_post_reset(unsigned char* data) {
    
    // wykonaj reset...
    soft_reset();

    return 0;
}

// wykonanie powrotu do domyœlnych ustawieñ
unsigned int http_post_default(unsigned char* data) {
    
    // pobierz domyœlne ustawienia i zapisz je do pamiêci EEPROM
    config_set_default();
    config_save();

    // wykonaj reset...
    soft_reset();

    return 0;
}

// dokumentacja
unsigned int http_page_manual(unsigned char* data) {
    unsigned int len = 0;

    len += http_copy_from_pstr(data+len, PSTR("<p class=\"info\">Dokumentacja dostÄ™pna przy aktywnym poÅ‚Ä…czeniu z internetem</p>"));
    len += http_copy_from_pstr(data+len, PSTR("<iframe src=\"http://mikrosterowniki.pl\" width=\"100%\" style=\"height:600px;_width:800px\"></iframe>\n"));

    return len;
}


unsigned int http_show_temperature(unsigned char* data, unsigned char jsonify) {
    unsigned int len = 0;

    len += http_copy_from_pstr(data,      HTTP_200_TEXT);
    len += http_copy_from_pstr(data+len,  HTTP_SERVER);

    if (jsonify) {
        len += http_copy_from_pstr(data+len,  PSTR("{temp: "));
    }

    if (ds18b20_temp < 0) {
        data[len++] = '-';
    }
    len += http_copy_from_val(data+len, abs(ds18b20_temp)/10);
    data[len++] = '.';
    data[len++] = '0' + (abs(ds18b20_temp)%10);

    if (jsonify) {
        data[len++] = '}';
    }

    return len;
}
