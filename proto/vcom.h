#ifndef _VCOM_H
#define _VCOM_H

#include "../rs_eth.h"

// mo¿liwe stany po³¹czenia z portem COM
#define VCOM_STATE_NOT_CONNECTED    0
#define VCOM_STATE_CONNECTED        1

// obs³u¿ przychodz¹cy pakiet TCP
void vcom_handle_packet(unsigned char*, unsigned int);

// wyœlij pakiet z informacj¹ o b³êdzie
void vcom_send_error(unsigned char);

// wyœlij pakiet o podanym typie z podanymi danymi
void vcom_send_packet(unsigned char, unsigned char*, unsigned char);

// konfiguruj port COM wg podanych danych z ramki vCom
void vcom_setup_com(unsigned char*);

// okresowo sprawdzaj czy nie nadesz³y nowe dane na porcie COM
void vcom_periodic();

/*
* VCom Protocol Definitions
*
* All definitions in this file are part of the VCom protocol specification.
* They have to be included by the VCom Client as well as the VCom Server 
* (unless the Client_Error_Codes which are only used by the Client)
*/

#define VCOM_SERVER_PORT            1123

/** VCom_Frame_Encapsulation
* Start and End of a VCom Frame (first and last Byte of every VCom Frame)
*/
#define VCOM_SOF					0x01			//Start of Frame
#define VCOM_EOF					0xFF			//End of Frame

/** VCom_Frame_Types
* Second Byte in every VCom Frame: Frame ID
*/
#define VCOM_OPEN_FRAME				0x02			//Open virtual COM Frame	
#define VCOM_ISOPEN_FRAME			0x03			//Virtual COM is Open Frame
#define VCOM_DATA_FRAME				0x04			//Data Frame
#define VCOM_CLOSE_FRAME			0x05			//Close virtual COM Frame
#define VCOM_ISCLOSED_FRAME		    0x06			//Virtual COM is closed Frame
#define VCOM_ERROR_FRAME			0x07			//Error Frame

/** VCom_Properties
* Various values of the current VCom Protocol Specification
*/
#define VCOM_MINIMAL_FRAMELENGTH	4
#define VCOM_HEADERLENGTH			3
#define VCOM_FRAME_OVERHEAD			4	 // 3 Bytes Header + 1 Byte EOF
#define VCOM_OPEN_FRAMELENGTH		11   // BYTES long
#define VCOM_ISOPEN_FRAMELENGTH 	4
#define VCOM_CLOSE_FRAMELENGTH  	4
#define VCOM_ISCLOSED_FRAMELENGTH	4
#define VCOM_ERROR_FRAMELENGTH		5
#define VCOM_MAX_PAYLOAD_LENGTH		252  // = 256 - 4 (4 = SOF, ID, LENGTH, EOF)

/** Server_Error_Codes
* Fourth byte in a VCOM_ERROR_FRAME, sent by the server (embedded device)
*/
#define SERV_ERROR_UNKNOWN_FATAL						0x50
#define SERV_ERROR_NOT_YET_CONNECTED					0x51
#define SERV_ERROR_WRONG_STATE							0x52
#define SERV_ERROR_OPENING_SERIAL_COM					0x53
#define SERV_ERROR_CLOSING_SERIAL_COM					0x54
#define SERV_ERROR_INVALID_FRAME_ID_RECEIVED			0x55
#define SERV_ERROR_INVALID_FRAME_LENGTH_RECEIVED		0x56
#define SERV_ERROR_INVALID_FRAMETYPE_RECEIVED			0x57
#define SERV_ERROR_EOF_EXPECTED							0x58

#endif
