#include "nic.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

// timery
volatile unsigned char uip_timer;
volatile unsigned char uip_arptimer;

// inicjalizacja uk≥adu sieciowego
void nic_init(unsigned char* addr) {

    // inicjalizacja uk≥adu
    enc28_init();

    // struktura z adresem MAC dla stosu
    struct uip_eth_addr *mac = (struct uip_eth_addr *) addr;

    // ustaw MAC dla stosu TCP/IP
    uip_setethaddr((*mac));

    // inicjalizacja warstwy MAC+PHY
    enc28_net_init(addr);
}

// wysy≥a dane zawarte w uip_buf i uip_appdata
void nic_send(void) {
    if(uip_len<=40+UIP_LLH_LEN) {
		enc28_packet_send((unsigned char *)uip_buf, uip_len);
	}
	else{
		enc28_packet_send_double((unsigned char *)uip_buf, 54, (unsigned char *)uip_appdata, uip_len-40-UIP_LLH_LEN);
	}
}

#if UIP_BUFSIZE > 255
unsigned int nic_poll(void)
#else 
unsigned char nic_poll(void)
#endif /* UIP_BUFSIZE > 255 */
{
	unsigned int len;

    // brak pakietÛw do odebrania
    if (enc28_count_packets() == 0) {
        return 0;
    }

	// kopiuj pakiet do bufora odbiorczego stosu
	len = enc28_packet_recv( uip_buf, UIP_BUFSIZE );
		
#if UIP_BUFSIZE > 255
	return len;
#else 
	return (unsigned char)len;
#endif /* UIP_BUFSIZE > 255 */
}

// czeka na wys≥anie zakolejkowanego pakietu
void nic_wait_for_send(void) {
    for (i=0; i<10; i++)
        _delay_ms(5);
}

// pobierz adres MAC
unsigned char* nic_get_mac() {
    return (unsigned char*) &uip_ethaddr;
}

// obs≥uguje cykliczne czynnoúci wykonywane przez stos
void uip_check() {

    unsigned char i;

    // odbierz ew. pakiet
    if (enc28_count_packets() > 0) {

        uip_len = enc28_packet_recv(uip_buf, UIP_BUFSIZE);

        //rs_dump(uip_buf, uip_len);

        // pakiet IP - ICMP/TCP/UDP
        if(BUF->type == htons(UIP_ETHTYPE_IP))
        {
            //uip_log("IP packet"); rs_dump(uip_appdata, uip_datalen());
            // dodaj ürÛd≥o do ARP cache'a
            uip_arp_ipin();
            uip_input();

            // ew. wyúlij pakiet
            if(uip_len > 0)
            {
              uip_arp_out();
              nic_send();
            }
        }
        // pakiet ARP
        else if(BUF->type == htons(UIP_ETHTYPE_ARP))
        {
            //uip_log("ARP packet"); rs_dump(uip_appdata, uip_datalen());
            uip_arp_arpin();

            // ew. wyúlij pakiet
            if(uip_len > 0) {
              nic_send();
            }
        }
    }

    // obs≥uga cyklicznych operacji (co 3 ms)
    else if(uip_timer >= 3)
    {
        // zeruj timer
        uip_timer = 0;

        // sprawdü aktywne po≥πczenia (czy nie majπ nic do wys≥ania)
        for(i = 0; i < UIP_CONNS; i++)
        {
            uip_periodic(i);

            // ew. wyúlij pakiet
            if(uip_len > 0)
            {
                uip_arp_out();
                nic_send();
            }
        }

        // sprawdü aktywne po≥πczenia UDP (czy nie majπ nic do wys≥ania)
        for(i = 0; i < UIP_UDP_CONNS; i++)
        {
            uip_udp_periodic(i);

            // ew. wyúlij pakiet
            if(uip_len > 0)
            {
                uip_arp_out();
                nic_send();
            }
        }

        // obs≥uga ARP co 10 sekund
        if(uip_arptimer > 10)
        {
            uip_arp_timer();
            uip_arptimer = 0;
        }
    }
}
