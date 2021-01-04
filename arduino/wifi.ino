/*
My ESP8266-01 FW:

2nd boot version : 1.7(5d6f877)
SPI Speed : 40MHz
SPI Mode : QIO
SPI Flash Size & Map: 8Mbit(512KB+512KB)
jump to run user1 @ 1000

AT version:1.7.4.0(May 11 2020 19:13:04)
SDK version:3.0.4(9532ceb)
compile time:May 27 2020 10:12:17
Bin version(Wroom 02):1.7.4
*/

#define AP_MODE_SSID_PREFIX	"STEPAN3_"
#define AP_MODE_PASSWORD	"098765432"
#define AP_MODE_CH_ID		11				// AP channel ID
#define AP_MODE_AUTH		3				// WPA2-PSK

#define STA_MODE_SSID		"SPOTTY"
#define STA_MODE_PASSWORD	"passwd0123"
#define STA_MODE_SYSLOG_IP	"10.5.0.7"

#define SYSLOG_UDP_PORT_DST	514
#define SYSLOG_UDP_PORT_SRC	1234
#define REMOTE_CTL_TCP_PORT	8080
#define TRANSMISSION_MODE	0
#define LINK_ID			0


static char cmd[50];
static char syslog_ip[16];


#ifdef WIFI_STA_MODE
void esp8266_connect_to_ap()
{
	Serial.println(F("WIFI init (STA mode)"));
	Serial3.flush();
        Serial3.print("+++");
        delay(100);
	esp8266_send_cmd("AT", "OK", 1000, ECHO);
	esp8266_send_cmd("AT+RST", "OK", 2000, ECHO);		// ESP Wifi module RESET
	esp8266_wait_for("ready", 6000, ECHO);
	esp8266_send_cmd("ATE0", "OK", 1000, ECHO);
	esp8266_send_cmd("AT+GMR", "OK", 2000, ECHO);		// "AT" firmware version
	esp8266_send_cmd("AT+CWMODE_CUR=1", "OK", 2000, ECHO);	// STA mode
	esp8266_send_cmd("AT+CWMODE_CUR?", "OK", 2000, ECHO);

	esp8266_send_cmd("AT+CWLAPOPT=1,127", "OK", 2000, ECHO);
	esp8266_send_cmd("AT+CWLAP", "OK", 5000, ECHO);		// prints out all available APs

	sprintf(cmd, "AT+CWJAP_CUR=\"%s\",\"%s\"", STA_MODE_SSID, STA_MODE_PASSWORD);
	esp8266_send_cmd(cmd, "OK", 15000, ECHO);
	esp8266_send_cmd("AT+CWJAP_CUR?", "OK", 2000, ECHO);	// AP details
	esp8266_send_cmd("AT+CIPSTA_CUR?", "OK", 2000, ECHO);	// shows my ip, gateway, netmask
#ifdef SYSLOG
	strcpy(syslog_ip, STA_MODE_SYSLOG_IP);
#endif
}
#endif


#ifdef WIFI_AP_MODE
void esp8266_init_local_ap()
{
	char ap_mac[20];
	char ssid[10];

	Serial.println(F("WIFI init (AP)"));
	Serial3.flush();
        Serial3.print("+++");
        delay(100);
	esp8266_send_cmd("AT", "OK", 1000, ECHO);
	esp8266_send_cmd("AT+RST", "OK", 2000, ECHO);		// ESP Wifi module RESET
	esp8266_wait_for("ready", 6000, ECHO);
	esp8266_send_cmd("ATE0", "OK", 1000, ECHO);
	esp8266_send_cmd("AT+GMR", "OK", 2000, ECHO);		// "AT" firmware version
	esp8266_send_cmd("AT+CWMODE_CUR=2", "OK", 2000, ECHO);	// SoftAP mode
	esp8266_send_cmd("AT+CWMODE_CUR?", "OK", 2000, ECHO);

	Serial3.println(F("AT+CIPSTAMAC_CUR?"));		// print out AP's MAC address
	esp8266_rx_buff_parser_wrapper(3000, ap_mac, __parse_mac_addr, NO_RET_ON_EMPTY_BUFF, ECHO);
	Serial.print(F("AP's MAC-addr: "));
	Serial.println(ap_mac);
	delay(200);
	esp8266_send_cmd("AT+CWSAP_CUR?", "OK", 3000, ECHO);	// Current AP's info

	strcpy(ssid, AP_MODE_SSID_PREFIX);
	strncat(ssid, &ap_mac[10], 1);
	strncat(ssid, &ap_mac[11], 1);				// ssid = 'STEPAN3_XY'

	sprintf(cmd, "AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d", ssid, AP_MODE_PASSWORD, AP_MODE_CH_ID, AP_MODE_AUTH);
	esp8266_send_cmd(cmd, "OK", 15000, ECHO);

	esp8266_send_cmd("AT+CIPSTA_CUR?", "OK", 2000, ECHO);	// show my ip,netmask,gw
}


bool esp8266_is_any_sta_online()
{
	char sta_ip[16];

	Serial3.println(F("AT+CWLIF"));				// List connected stations IPs (<ip>,<mac>)
	if (esp8266_rx_buff_parser_wrapper(50, sta_ip, __parse_any_sta_ip, RET_ON_EMPTY_BUFF, NO_ECHO) == 0) {
		Serial.println();
		Serial.print(F("Online STA: "));
		Serial.println(sta_ip);
#ifdef SYSLOG
		strcpy(syslog_ip, sta_ip);
#endif
		return true;
	} else
		return false;
}
#endif		// WIFI_AP_MODE


#if defined WIFI_STA_MODE || defined WIFI_AP_MODE
void esp8266_ipservices_init()
{
	esp8266_send_cmd("AT+CIPMODE=0", "OK", 3000, ECHO);		// 0 - 'normal', 1 - 'passthrough'
	esp8266_send_cmd("AT+CIPMUX=1", "OK", 3000, ECHO);

	sprintf(cmd, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d,%d",
			LINK_ID, syslog_ip, SYSLOG_UDP_PORT_DST, SYSLOG_UDP_PORT_SRC, TRANSMISSION_MODE);
	esp8266_send_cmd(cmd, "OK", 3000, ECHO);
#ifdef SYSLOG
	sprintf(syslog_msg, "INIT: Self-balancing ROBOT is online.\n");
	logger_common(syslog_msg);
#endif
}


bool esp8266_send_cmd(const char *at_command, const char *expected_response, uint16_t timeout_ms, bool echo)
{
	if (echo) {
		Serial.print(F("Sending command: "));
		Serial.println(at_command);
	}

	Serial3.println(at_command);
	return esp8266_wait_for(expected_response, timeout_ms, echo);
}


bool esp8266_wait_for(const char *stop_str, uint16_t timeout_ms, bool echo)
{
	if (esp8266_rx_buff_parser_wrapper(timeout_ms, (char *)stop_str, __parse_any_pattern, NO_RET_ON_EMPTY_BUFF, echo) == 0)
		return true;
	else
		return false;
}


#ifdef SYSLOG
void logger_common(char *msg)
{
	sprintf(cmd, "AT+CIPSEND=%d,%d", LINK_ID, strlen(msg));

	if (esp8266_send_cmd(cmd, "> ", 1000, NO_ECHO))
		esp8266_send_cmd(msg, "SEND OK", 1000, NO_ECHO);

	strcpy(msg, "");
}


void logger_telemetry(char *msg)			// is called once per system loop
{
	static char msg_backup[255];			// unsent msg is saved here (in case of fail)
	static bool is_msg_sent = true;

	if (is_msg_sent) {
		sprintf(cmd, "AT+CIPSEND=%d,%d", LINK_ID, strlen(msg));
		Serial3.println(cmd);

		if (esp8266_rx_buff_parser_wrapper(8, syslog_ip, __parse_loop_data, NO_RET_ON_EMPTY_BUFF, NO_ECHO) == 0) {
			Serial3.print(msg);		// send a message to syslog as soon as '> ' prompt appears
			is_msg_sent = true;
		} else {				// did not get a prompt '> ' in time, the msg is lost
			strcpy(msg_backup, msg);
			msg_backup[0] = '*';
			is_msg_sent = false;
		}
	} else {					// last time we failed to send a msg, try to send it now
		Serial3.print(msg_backup);
		is_msg_sent = true;
	}

	strcpy(msg, "");
}
#endif


uint8_t esp8266_rx_buff_parser_wrapper(uint16_t timeout_ms, char *aux_str_buff, bool (*parser)(char, char*), bool ret_on_empty, bool echo)
{
	bool is_timeout = false;
	char c;
	uint32_t timer;

	timer = millis();
	while (!is_timeout) {
		if (Serial3.available()) {
			c = Serial3.read();

			if (echo)
				Serial.print(c);

			if (parser(c, aux_str_buff)) {
				if (echo)
					Serial.println();
				return 0;
			}
		} else if (ret_on_empty)
			return 1;

		if ((millis() - timer) > timeout_ms)
			is_timeout = true;
	}
	if (echo)
		Serial.println();
	Serial.println(F("!Timeout in esp8266_rx_buff_parser_wrapper()"));
	return 2;
}


/*
parses remote cmds {x=y} and/or a prompt '> ' depending on MODE settings
*/
bool __parse_loop_data(const char c, char *stub)
{
	static uint8_t stage = 0;

	switch (stage) {
		case 0:
#ifdef REMOTE_CTL
			if (c == '{') {		// remote command format: {remote_cmd=remote_cmd_val}, e.g.: {a0=-0.12}
				strcpy(remote_cmd, "");
				stage = 2;
			}
#endif
#ifdef SYSLOG
			if (c == '>')
				stage = 1;
#endif
			break;
#ifdef SYSLOG
		case 1:
			if (c == ' ') {
				stage = 0;
				return true;	// got '> '
			} else
				stage = 0;
			break;
#endif
#ifdef REMOTE_CTL
		case 2:
			if (c == '=' || strlen(remote_cmd) == MAX_LEN_REMOTE_CMD) {
				strcpy(remote_cmd_val, "");
				stage = 3;
			} else
				strncat(remote_cmd, &c, 1);
			break;
		case 3:
			if (c == '}' || strlen(remote_cmd_val) == MAX_LEN_REMOTE_CMD_VAL) {
				apply_remote_cmd();
				stage = 0;
			} else
				strncat(remote_cmd_val, &c, 1);
			break;
#endif
	}
	return false;
}


bool __parse_any_pattern(const char c, char *pattern)
{
	static char buff[16];
	uint8_t pattern_len = strlen(pattern);

	if (pattern_len > 15) {
		Serial.println(F("ERROR: the pattern has exceeded 15 chars in __parse_any_pattern"));
		return false;
	}

	buff[pattern_len] = 0x00;

	for (uint8_t i = 0; i < (pattern_len - 1); i++)
		buff[i] = buff[i + 1];

	buff[pattern_len - 1] = c;

	if (strcmp(buff, pattern) == 0)
		return true;
	else
		return false;
}


bool __parse_mac_addr(const char c, char *mac_str)
{
	static uint8_t stage = 0;

	switch (stage) {
		case 0:
			if (c == ':')
				stage = 1;
			break;
		case 1:
			if (c == '\r')
				stage = 2;
			else if ((c != '"') && (c != ':'))
				strncat(mac_str, &c, 1);
			break;
		case 2:
			if (esp8266_wait_for("OK", 1000, ECHO)) {
				stage = 0;
				return true;
			} else
				Serial.println(F("ERROR: Did not get 'OK' response in __parse_mac_addr"));
			break;
	}
	return false;
}


bool __parse_any_sta_ip(const char c, char *sta_ip)
{
	static uint8_t stage = 0;

	switch (stage) {
		case 0:
			if (c == '1') {			// [1]0.x.x.x [1]72.x.x.x [1]92.168.x.x
				sprintf(sta_ip, "%c", c);
				stage = 1;
			}
			break;
		case 1:
			if (c == ',') {
				stage = 2;
			} else
				strncat(sta_ip, &c, 1);
			break;
		case 2:
			if (esp8266_wait_for("OK", 1000, NO_ECHO)) {
				stage = 0;
				return true;
			} else
				Serial.println(F("ERROR: Did not get 'OK' response in __parse_any_sta_ip"));
			break;
	}
	return false;
}
#endif		// (WIFI_STA_MODE) || (WIFI_AP_MODE)
