#ifndef __ANNOUNCE_H
#define __ANNOUNCE_H

#include "../rs_eth.h"

// port UDP pakietów MCHP discovery
#define ANNOUNCE_PORT   30303

// obs³u¿ pakiet MCHP discovery
void announce_handle_packet(unsigned char*, unsigned int);


#endif
