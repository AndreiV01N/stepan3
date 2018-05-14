#ifndef _WIFI_H
#define _WIFI_H

void esp8266_connect_to_ext_ap();
void esp8266_init_local_ap();
bool esp8266_check_node();
bool esp8266_ipservices_init();
bool ESPsendCommand(const char *command, String stopstr, int timeout_secs);

#endif
