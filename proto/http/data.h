#ifndef _HTTP_DATA_H
#define _HTTP_DATA_H

#include "../../rs_eth.h"

// PNG
extern prog_char    HTTP_DATA_FAVICON_PNG[];
#define             HTTP_DATA_FAVICON_PNG_LENGTH    874

extern prog_char    HTTP_DATA_BG_PNG[];
#define             HTTP_DATA_BG_PNG_LENGTH         443

extern prog_char    HTTP_DATA_INFO_PNG[];
#define             HTTP_DATA_INFO_PNG_LENGTH       918

extern prog_char    HTTP_DATA_HEADER_BG_PNG[];
#define             HTTP_DATA_HEADER_BG_PNG_LENGTH  174

extern prog_char    HTTP_DATA_LOADING_GIF[];
#define             HTTP_DATA_LOADING_GIF_LENGTH    847

// CSS
extern prog_char    HTTP_DATA_CSS_CSS[];
#define             HTTP_DATA_CSS_CSS_LENGTH        53

extern prog_char    HTTP_DATA_LAYOUT_CSS[];
#define             HTTP_DATA_LAYOUT_CSS_LENGTH     877

extern prog_char    HTTP_DATA_CONTENT_CSS[];
#define             HTTP_DATA_CONTENT_CSS_LENGTH    1048

// JS
extern prog_char    HTTP_DATA_JS_JS[];
#define             HTTP_DATA_JS_JS_LENGTH          626

// HTML
extern prog_char    HTTP_DATA_LAYOUT_START[];
#define             HTTP_DATA_LAYOUT_START_LENGTH   692

// elementy menu
#define             HTTP_MENU_ITEMS_COUNT           8
extern char         HTTP_MENU_ITEMS[HTTP_MENU_ITEMS_COUNT][30] PROGMEM;
extern char         HTTP_MENU_LINKS[HTTP_MENU_ITEMS_COUNT][8] PROGMEM;

#endif
