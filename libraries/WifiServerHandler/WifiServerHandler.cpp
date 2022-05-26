#include <WifiServerHandler.h>

WiFiServer connexion_server(80);

void set_creds_nvs(char *ssid, char *password)
{
	nvs_handle my_handle;
	esp_err_t err = nvs_open("WIFI", NVS_READWRITE, &my_handle);
	if (err == ESP_OK)
	{
		nvs_set_str(my_handle, "ssid", ssid);
		nvs_set_str(my_handle, "password", password);
		nvs_commit(my_handle);
		nvs_close(my_handle);
	}
}

bool get_creds_nvs(char *ssid, char *password)
{
	size_t l = SSID_LENGTH;

	nvs_handle my_handle;
	esp_err_t err = nvs_open("WIFI", NVS_READWRITE, &my_handle);
	if (err == ESP_OK)
	{
		err = nvs_get_str(my_handle, "ssid", ssid, &l);
		if (err != ESP_OK)
			return (false);
		err = nvs_get_str(my_handle, "password", password, &l);
		if (err != ESP_OK)
			return (false);
		nvs_close(my_handle);
		return (true);
	}
	return (false);
}

int wifi_connect(char *ssid, char *password)
{
	WiFi.begin(ssid, password);
	Serial.println(ssid);
	Serial.println(password);
	byte connect_count = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
		connect_count++;
		if (connect_count > 10)
		{
			Serial.println("Error connection to WiFi");
			return (false);
		}
	}
	Serial.println("WiFi connected");
	return (1);
}

bool get_creds_from_url(String get_url)
{
	// get_url from "http://192.168.4.1/set_over?ssid=SSID&password=PWD"
	int ssid_add = 0;
	int pwd_add = 0;
	int end_add = 0;
	String ssid = "";
	String password = "";

	pwd_add = get_url.indexOf('&');
	ssid = get_url.substring(get_url.indexOf('?') + 6, pwd_add);
	ssid.replace("+", " ");
	password = get_url.substring(pwd_add + 10, get_url.indexOf(' '));
	password.replace("+", " ");
	set_creds_nvs((char *)ssid.c_str(), (char *)password.c_str());
	return (wifi_connect((char *)ssid.c_str(), (char *)password.c_str()));
}

int wifi_config_server()
{
	WiFiClient client = connexion_server.available();

	if (client)
	{
		String currentLine = "";
		while (client.connected())
		{
			if (client.available())
			{
				char c = client.read();
				if (c == '\n')
				{
					if (currentLine.length() == 0)
					{
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println();
						client.println(HOME_HTML);
						break;
					}
					else
						currentLine = "";
				}
				else if (c != '\r')
					currentLine += c;
				if (currentLine.endsWith("GET /set_over"))
				{
					String get_request = "";
					while (1)
					{
						char c_get = client.read();
						Serial.write(c_get);
						if (c_get == '\n')
							break;
						else
							get_request += c_get;
					}
					if (get_creds_from_url(get_request))
					{
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println();
						client.println(SUCCESS_HTML);
						client.stop();
						return (true);
					}
					else
					{
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println();
						client.println(FAIL_HTML);
						currentLine = "";
						break;
					}
				}
			}
		}
		client.stop();
	}
	return (false);
}

esp_err_t init_nvs()
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
	return (err);
}

void ap_init()
{
	// WiFi.softAP(ssid, password);
	WiFi.softAP("GMX_LED_LAMP_WIFI");
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("GMX_LED_LAMP_WIFI IP address: ");
	Serial.println(myIP);
	connexion_server.begin();
}

void wifi_handler()
{
	char ssid[SSID_LENGTH];
	char password[SSID_LENGTH];
	esp_err_t init_nvs();

	Serial.println("Connecting to wifi....");
	if (!get_creds_nvs(ssid, password) || !wifi_connect(ssid, password))
	{
		ap_init();
		while (!wifi_config_server())
			;
	}
	connexion_server.end();
	Serial.println("Connect WIFI SUCCESS");
}
