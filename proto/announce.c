#include "announce.h"

// obs³u¿ pakiet
void announce_handle_packet(unsigned char* data, unsigned int len) {
    // wype³nij pakiet (nazwa urz¹dzenia, adres MAC, sygnaturka)
    len = 0;

    // skonfigurowana nazwa urz¹dzenia
    config_t* config_get();
    
    strcpy(config_ram.name, (char*)data+len);
    len += strlen((char*)data);

    // ³amanie wiersza
    memcpy_P(PSTR("\r\n"), data+len, 2);
    len += 2;

    // adres MAC (w formacie "human-readable")
    unsigned char *mac = nic_get_mac();
    for (unsigned char i=0; i<6; i++) {
        data[len++] = dec2hex(mac[i] >> 4);
        data[len++] = dec2hex(mac[i] & 0x0f);

        if (i<5)
            data[len++] = '-';
    }

    // ³amanie wiersza
    memcpy_P(PSTR("\r\n"), data+len, 2);
    len += 2;
    
    // sygnaturka
    strcpy_P(RS2ETH_SIGNATURE, (char*)data+len);
    len += strlen_P(RS2ETH_SIGNATURE);

    rs_dump(data, len);

    // utwórz "po³¹czenie" UDP (odpowiedŸ)
    struct uip_udp_conn* conn;
    conn = uip_udp_new(&uip_udp_conn->ripaddr, uip_udp_conn->rport);

    if (!conn) {
        return;
    }

    // wyœlij z portu konfiguracyjnego
    uip_udp_bind(conn, HTONS(ANNOUNCE_PORT));

    // wyœlij
    uip_udp_send(len + 10);

    // czekaj na wys³anie
    nic_wait_for_send();

    // zamknij po³¹czenia UDP (przychodz¹cy broadcast i wychodz¹cy unicast)
    uip_udp_remove(conn);
    uip_udp_remove(uip_udp_conn);
}
