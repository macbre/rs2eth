// sterownik uk³adu sieciowego ENC28J60 dla stosu uIP

#ifndef _NIC_H
#define _NIC_H

#include "../rs_eth.h"

// inicjalizacja uk³adu sieciowego oraz cyklicznego sprawdzania odebranych pakietów
void nic_init(unsigned char*);

// pobierz adres MAC
unsigned char* nic_get_mac();

// wysy³a dane zawarte w uip_buf i uip_appdata
void nic_send(void);

// czeka na wys³anie zakolejkowanego pakietu
void nic_wait_for_send(void);

// zwraca rozmiar odebranego pakietu lub zero, jeœli brak pakietu
#if UIP_BUFSIZE > 255
unsigned int nic_poll(void);
#else 
unsigned char nic_poll(void);
#endif /* UIP_BUFSIZE > 255 */

// sprawdza bufor ENC28 i obs³uguje przychodz¹ce pakiety
void uip_check();

// timery
extern volatile unsigned char uip_timer;
extern volatile unsigned char uip_arptimer;

#endif
