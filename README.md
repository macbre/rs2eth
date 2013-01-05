rs2eth
======

RS2ETH jest konwerterem między magistralą szeregową RS232 (za pomocą odpowiedniej "nakładki" także RS485),
a siecią Ethernet.

## Protokoły

Dostęp do portu szeregowego możliwy jest poprzez protokoły:
* Telnet (port 23, [RFC 854](https://tools.ietf.org/html/rfc854))
* VCOM (port 1123)
 
Konwerter wspiera standard opisany w dokumencie [RFC 2217](https://tools.ietf.org/html/rfc2217) (COM port suboption) pozwalając aplikacji na zdalną
konfigurację parametrów transmisji szeregowej.

Obsługiwany jest również protokół [MCHP Discovery](http://www.microchip.com/stellent/idcplg?IdcService=SS_GET_PAGE&nodeId=2505&param=en535724) (port 30303) pozwalający wykrywać urządzenia podłączone
do lokalnej sieci.

## Aplikacje pomocnicze

* [Ponticulus](http://sourceforge.net/projects/ponticulus/) - wirtualny port szeregowy z obslugą zdalnych połączeń via telnet
* [Microchip Ethernet Discoverer](http://code.google.com/p/pic-moodlight/source/browse/branches/MCHPStack402/Microchip/TCPIP+Stack/Microchip+Ethernet+Discoverer/bin/Release/) - wykrywanie rs2eth w lokalnej sieci

## RS232 over Internet

Tryb pracy jako klient pozwala połączyć dwa konwertery RS2ETH poprzez sieć lokalną/internet tworząc połączenie
między dwoma portami szeregowymi omijające ograniczenia standardów RS232/485 dotyczące maksymalnych
długości połączeń

Tryb też może być także wykorzystany jako konwerter prędkości dla dwóch portów szeregowych.

```
URZĄDZENIE A         RS2ETH (serwer)                    RS2ETH (klient)        URZĄDZENIE B
  		       <--->               <-- (internet) -->                   <--->
   port RS                                          (łączy się z serwerem)       port RS
```

## Diagnostyka

RS2ETH posiada także interfejs konfiguracyjno-diagnostyczny dostępny z poziomu przeglądarki WWW.
