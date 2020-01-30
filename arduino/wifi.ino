/*
My ESP8266-01 FW:

2nd boot version : 1.5
  SPI Speed      : 40MHz
  SPI Mode       : DIO
  SPI Flash Size & Map: 8Mbit(512KB+512KB)
jump to run user1 @ 1000

AT version:1.2.0.0(Jul  1 2016 20:04:45)
SDK version:1.5.4.1(39cb9a32)
Ai-Thinker Technology Co. Ltd.
Dec  2 2016 14:21:16
*/

#define LOCAL_AP_SSID		"STEPAN3_"
#define LOCAL_AP_PASSWORD	"098765432"

#define EXT_AP_SSID		"SPOTTY"
#define EXT_AP_PASSWORD		"passwd0123"

#define SYSLOG_SERVER		"10.5.0.10"			// for ext AP

static String AP_MAC;						// for local AP
static String MY_IP;
static String SYSLOG_IP;


#ifdef WIFI_EXTERNAL_AP
void esp8266_connect_to_ext_ap()
{
	Serial.println("WIFI init (Station)");
	Serial3.flush();
	Serial3.print("+++");
	delay(100);
	esp8266_send_cmd("AT", "OK", 1);
	esp8266_send_cmd("AT+RST", "OK", 2);			// ESP Wifi module RESET
	esp8266_wait_for("ready", 6);
	esp8266_send_cmd("AT+GMR", "OK", 2);			// AT commands version
	esp8266_send_cmd("AT+CWMODE_CUR=1", "OK", 2);		// "station only" mode
	esp8266_send_cmd("AT+CWMODE_CUR?", "OK", 2);

	esp8266_send_cmd("AT+CWLAPOPT=1,127", "OK", 2);
	esp8266_send_cmd("AT+CWLAP", "OK", 5);			// prints available APs

	char cmd[50] = "AT+CWJAP_CUR=\"";			// connecting to external AP..
	strcat(cmd, EXT_AP_SSID);
	strcat(cmd, "\",\"");
	strcat(cmd, EXT_AP_PASSWORD);
	strcat(cmd, "\"");
	esp8266_send_cmd(cmd, "OK", 10);

	esp8266_send_cmd("AT+CWJAP_CUR?", "OK", 2);		// AP details
	Serial3.println("AT+CIPSTA_CUR?");			// prints my IP/mask/GW
	esp8266_parse_ip_addr();
	Serial.print(F("My ip-addr: ")); Serial.println(MY_IP);
	delay(200);
	SYSLOG_IP = SYSLOG_SERVER;
}
#endif


#ifdef WIFI_INTERNAL_AP
void esp8266_init_local_ap()
{
	Serial.println("WIFI init (AP)");
	Serial3.flush();
	Serial3.print("+++");
	delay(100);
	esp8266_send_cmd("AT", "OK", 1);
	esp8266_send_cmd("AT+RST", "OK", 2);			// ESP Wifi module RESET
	esp8266_wait_for("ready", 6);
	esp8266_send_cmd("AT+GMR", "OK", 2);			// AT commands version
	esp8266_send_cmd("AT+CWMODE_CUR=2", "OK", 2);		// Soft AP only mode
	esp8266_send_cmd("AT+CWMODE_CUR?", "OK", 2);		// Check the mode

	Serial3.println("AT+CIPSTAMAC_CUR?");			// Print out AP's MAC address
	esp8266_parse_mac_addr();
	Serial.print("MAC: "); Serial.println(AP_MAC);
	delay(200);
	esp8266_send_cmd("AT+CWSAP_CUR?", "OK", 3);		// Current AP's info

	char ssid[10] = LOCAL_AP_SSID;
	strcat(ssid, AP_MAC[10]);
	strcat(ssid, AP_MAC[11]);				// ssid = 'STEPAN3_XY'

	char cmd[50] = "AT+CWSAP_CUR=\"";			// Initing local AP
	strcat(cmd, ssid);
	strcat(cmd, "\",\"");
	strcat(cmd, LOCAL_AP_PASSWORD);
	strcat(cmd, "\",11,3");					// 11 - channel ID, 3 - WPA2-PSK
	esp8266_send_cmd(cmd, "OK", 10);

	Serial3.println("AT+CIPAP_CUR?");			// prints my IP
	esp8266_parse_ip_addr();
	Serial.print("My ip-addr: "); Serial.println(MY_IP);
	delay(200);
}


bool esp8266_parse_mac_addr()
{
	char c1, c2;
	bool timeout = false;
	uint32_t timer;
	uint8_t state = 0;

	timer = millis();
	while (!timeout) {
		if ((millis() - timer) > 5000)
			timeout = true;

		if (Serial3.available()) {
			c2 = c1;
			c1 = Serial3.read();
			switch (state) {
				case 0:
					if (c1 == ':')
						state = 1;
					break;
				case 1:
					if (c1 == '\r')
						state = 2;
					else if ((c1 != '"') && (c1 != ':'))
						AP_MAC += c1;
					break;
				case 2:
					if ((c2 == 'O') && (c1 == 'K')) {
						Serial.println();
						Serial3.flush();
						return true;
					}
					break;
			}
		}
	}
	Serial.println("!Timeout in esp8266_parse_mac_addr!");
	Serial3.flush();
	return false;						// timed out
}


bool esp8266_check_node()
{
	Serial3.println("AT+CWLIF");				// List connected stations IPs (<ip>,<mac>)
	if (ESP_get_node_ip()) {
		Serial3.flush();
		return true;
	}  else {
		Serial3.flush();
		return false;
	}
}


bool ESP_get_node_ip()
{
	char ch;
	bool timeout = false;
	uint32_t timer;
	uint8_t state = 0;

	timer = millis();
	while (!timeout) {
		if ((millis() - timer) > 5000)
			timeout = true;

		if (Serial3.available()) {
			ch = Serial3.read();
			switch (state) {
				case 0:
					if (ch == '1') {		// 10.x.x.x 172.x.x.x 192.168.x.x
						SYSLOG_IP += ch;
						state = 1;
					}
					break;
				case 1:
					if (ch == ',') {
						Serial.println();
						Serial.print("Node IP address: ");
						Serial.println(SYSLOG_IP);
						return true;
					} else
						SYSLOG_IP += ch;
					break;
			}
		} else							// Serial3 is empty, didn't catch comma - node is offline
			return false;
	}
	Serial.println("!Timeout in ESP_get_node_ip!");
	return false;
}
#endif


#if defined(WIFI_EXTERNAL_AP) || defined(WIFI_INTERNAL_AP)
bool esp8266_ipservices_init()
{
	Serial.println("Setup IP-services..");

	esp8266_send_cmd("AT+CIPMUX=0", "OK", 3);			// Set single connection mode
	esp8266_send_cmd("AT+CIPMODE=1", "OK", 3);			// Transparent sending data (UART-WiFi passthrough)

	char cmd[48] = "AT+CIPSTART=\"UDP\",\"";
	strcat(cmd, SYSLOG_IP.c_str());
	strcat(cmd, "\",514");
	strcat(cmd, ",8080");
	strcat(cmd, ",0");
	esp8266_send_cmd(cmd, "OK", 3);
	esp8266_send_cmd("AT+CIPSEND", ">", 2);

	Serial3.print(F("ROBOT's IP: "));
	Serial3.println(MY_IP);
	return true;
}


bool esp8266_send_cmd(const char *command, String stopstr, uint8_t timeout_secs)
{
	Serial.print("Sending command: ");
	Serial.println(command);
	Serial3.println(command);
	bool status = esp8266_wait_for(stopstr, timeout_secs);
	return status;
}


bool esp8266_wait_for(String stopstr, uint8_t timeout_secs)
{
	String response = "";
	bool found = false;
	char c;
	long timer_init;
	long timer;

	timer_init = millis();
	while (!found) {
		timer = millis();
		if (((timer - timer_init) / 1000) > timeout_secs) {
			Serial.println("!Timeout in esp8266_wait_for!");
			return false;					// timed out
		}
		if (Serial3.available()) {
			c = Serial3.read();
			Serial.print(c);
			response += c;
			if (response.endsWith(stopstr)) {
				found = true;
				delay(10);
				Serial3.flush();
				Serial.println();
			}
		}
	}
	return true;
}


bool esp8266_parse_ip_addr()
{
	char c1, c2;
	bool timeout = false;
	uint32_t timer;
	uint8_t state = 0;

	timer = millis();
	while (!timeout) {
		if ((millis() - timer) > 5000)			// Timeout 5s
			timeout = true;

		if (Serial3.available()) {
			c2 = c1;
			c1 = Serial3.read();
			switch (state) {
				case 0:
					if (c1 == '"')
						state = 1;
					break;
				case 1:
					if (c1 == '"') {
						Serial.println();
						Serial3.flush();
						return true;
					} else
						MY_IP += c1;
					break;
			}
		}
	}
	Serial.println("!Timeout in esp8266_parse_ip_addr!");
	Serial3.flush();
	return false;						// timed out
}
#endif
