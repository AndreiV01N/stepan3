#define LOCAL_AP_SSID		"STEPAN3_XX"
#define LOCAL_AP_PASSWORD	"098765432"

#define EXT_AP_SSID		"SPOTTY"
#define EXT_AP_PASSWORD		"passwd0123"

#define SYSLOG_SERVER		"192.168.100.2"			// for ext AP

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
	ESPsendCommand("AT", "OK", 1);
	ESPsendCommand("AT+RST", "OK", 2);			// ESP Wifi module RESET
	ESP_wait("ready", 6);
	ESPsendCommand("AT+GMR", "OK", 2);			// AT commands version
	ESPsendCommand("AT+CWMODE_CUR=1", "OK", 2);		// Station only mode
	ESPsendCommand("AT+CWMODE_CUR?", "OK", 2);		// Check the mode

	ESPsendCommand("AT+CWLAPOPT=1,127", "OK", 2);		// Configure view of list below
	ESPsendCommand("AT+CWLAP", "OK", 5);			// List available APs

	char cmd[50] = "AT+CWJAP_CUR=\"";			// Connecting to external AP..
	strcat(cmd, EXT_AP_SSID);
	strcat(cmd, "\",\"");
	strcat(cmd, EXT_AP_PASSWORD);
	strcat(cmd, "\"");
	ESPsendCommand(cmd, "OK", 10);

	ESPsendCommand("AT+CWJAP_CUR?", "OK", 2);		// AP details
	Serial3.println("AT+CIPSTA_CUR?");		// get my IP
	ESP_get_my_ip();
	Serial.print(F("MY IP (ext. ap): ")); Serial.println(MY_IP);
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
	ESPsendCommand("AT", "OK", 1);
	ESPsendCommand("AT+RST", "OK", 2);			// ESP Wifi module RESET
	ESP_wait("ready", 6);
	ESPsendCommand("AT+GMR", "OK", 2);			// AT commands version
	ESPsendCommand("AT+CWMODE_CUR=2", "OK", 2);		// Soft AP only mode
	ESPsendCommand("AT+CWMODE_CUR?", "OK", 2);		// Check the mode

	Serial3.println("AT+CIPSTAMAC_CUR?");			// Print out AP's MAC address
	ESP_get_ap_mac();
	Serial.print("MAC: "); Serial.println(AP_MAC);
	delay(200);
	ESPsendCommand("AT+CWSAP_CUR?", "OK", 3);		// Current AP's info

	char cmd[50] = "AT+CWSAP_CUR=\"";			// Initing local AP
	strcat(cmd, LOCAL_AP_SSID);
	strcat(cmd, "\",\"");
	strcat(cmd, LOCAL_AP_PASSWORD);
	strcat(cmd, "\",11,3");
	cmd[22] = AP_MAC[10];
	cmd[23] = AP_MAC[11];
	ESPsendCommand(cmd, "OK", 10);

	ESPsendCommand("AT+CIPUP_CUR?", "OK", 2);		// get my IP
	ESP_get_my_ip();
	Serial.print("MY IP(INT_AP): "); Serial.println(MY_IP);
}


bool ESP_get_ap_mac()
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
	Serial.println("!Timeout in ESP_get_ap_mac!");
	Serial3.flush();
	return false;						// timeout
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
		if ((millis() - timer) > 5000)		// Timeout 1s
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
		} else					// Serial3 is empty, didn't catch comma - node is offline
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

	ESPsendCommand("AT+CIPMUX=0", "OK", 3);			// Set single connection mode
	ESPsendCommand("AT+CIPMODE=1", "OK", 3);		// Transparent sending data (UART-WiFi passthrough)

	char cmd[48] = "AT+CIPSTART=\"UDP\",\"";
	strcat(cmd, SYSLOG_IP.c_str());
	strcat(cmd, "\",514");
	strcat(cmd, ",8080");
	strcat(cmd, ",0");
	ESPsendCommand(cmd, "OK", 3);
	ESPsendCommand("AT+CIPSEND", ">", 2);

	Serial3.print(F("ROBOT's IP: "));
	Serial3.println(MY_IP);
	return true;
}


bool ESPsendCommand(const char *command, String stopstr, int timeout_secs)
{
	Serial.print("Sending command: ");  Serial.println(command);
	Serial3.println(command);
	bool status = ESP_wait(stopstr, timeout_secs);
	return status;
}


bool ESP_wait(String stopstr, int timeout_secs)
{
	String response = "";
	bool found = false;
	char c;
	long timer_init;
	long timer;

	timer_init = millis();
	while (!found) {
		timer = millis();
		if (((timer - timer_init) / 1000) > timeout_secs) {	// Timeout?
			Serial.println("!Timeout in ESP_wait!");
			return false;					// timeout
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


bool ESP_get_my_ip()
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
	Serial.println("!Timeout in ESP_get_my_ip!");
	Serial3.flush();
	return false;						// timeout
}
#endif
