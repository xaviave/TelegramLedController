#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <vector>

// WIFI Credentials
#define SSID "XXXX"
#define PASSWORD "XXXX"
// Bot Token
#define BOT_TOKEN "XXXX"
#define CHAT_ID "XXXX"
// Telegram usage
#define WELCOME "Use the following commands to control your light.\nSend /on or /off to start\nSend /brightness [0-255] to brightness\nGet custom color with /color RGB HEX code\nGet preset color:\n/red | /light_red | /green | /lime | /blue | /cyan | /yellow | /orange | /purple | /pink | /magenta | /white | /warm_white\nGet different light mode:\n /solid_color | /random_solid | /random_gradient | /shine | /heartbeat | /rainbow | /moving_rainbow\nGet different rainbow mode:\n /rainbow_color | /rainbow_color_stripe | /rainbow_color_random"
#define ERROR_BRIGHTNESS "Error: Brightness must be between 0 and 255"
// LED Configuration
#define LED_PIN_EXT 16
#define LED_NUM_EXT 144
#define LED_NUM_INT (20 + 10)
#define LED_NUM_TOTAL (LED_NUM_EXT + LED_NUM_INT)
#define LED_HALF_NUM_EXT (int)(LED_NUM_EXT * 0.7)

CRGB g_ext_leds[LED_NUM_TOTAL];
QueueHandle_t g_xQueue;
TaskHandle_t g_LedTask;
TaskHandle_t g_TelegramTask;
WiFiClientSecure g_WIFIClient;
UniversalTelegramBot g_bot(BOT_TOKEN, g_WIFIClient);

struct LedState
{
	bool on;
	bool blend;
	byte mode;
	byte brightness;
	byte palette;
	uint32_t color;
};

class LedController
{
	typedef void (LedController::*pfunc_t)();
	typedef std::vector<pfunc_t> f_display_vec_t;

private:
	bool on;
	byte led_mode;
	byte fps;
	byte brightness;
	int rainbow_index;
	CRGB color = CRGB::Red;
	TBlendType blend_mode;
	CRGBPalette16 palette;

public:
	f_display_vec_t dv;

	void init_display_vec()
	{
		this->dv.push_back(&LedController::start_mode);
		this->dv.push_back(&LedController::solid_color);
		this->dv.push_back(&LedController::random_solid);
		this->dv.push_back(&LedController::shine);
		this->dv.push_back(&LedController::heartbeat);
		this->dv.push_back(&LedController::rainbow);
		this->dv.push_back(&LedController::moving_rainbow);
		this->dv.push_back(&LedController::random_gradient);
	}

	LedController(byte brightness)
	{
		Serial.println("LedController init");
		this->init_display_vec();
		this->on = true;
		this->led_mode = 0;
		this->brightness = brightness;
		this->palette = RainbowColors_p;
		this->blend_mode = LINEARBLEND;
		this->rainbow_index = 0;
		this->fps = 17;
		this->show();
	}

	byte get_led_mode()
	{
		return (this->led_mode);
	}

	// Setters
	void set_on(bool on)
	{
		this->on = on;
	}

	void set_led_mode(byte led_mode)
	{
		this->led_mode = led_mode;
	}

	void set_color(uint32_t color)
	{
		this->color = CRGB(color);
	}

	void set_blend_mode(bool blend)
	{
		this->blend_mode = blend ? LINEARBLEND : NOBLEND;
	}

	void set_palette(byte palette)
	{
		if (!palette)
			this->palette = RainbowColors_p;
		else if (palette == 1)
			this->palette = RainbowStripeColors_p;
		else if (palette == 2)
		{
			for (int i = 0; i < 16; i++)
				this->palette[i] = CHSV(random8(), 255, random8());
		}
	}

	void set_brightness(byte brightness)
	{
		this->brightness = brightness;
		FastLED.setBrightness(brightness);
	}

	// Color effects
	void start_mode()
	{
		CHSV c;
		byte h = LED_NUM_EXT / 2;
		FastLED.setBrightness(255);
		for (byte i = 0; i < h; i++)
		{
			c = CHSV(i * (255 / h), 255, 255);
			g_ext_leds[h - i] = c;
			g_ext_leds[h + i] = c;
			fill_solid(&g_ext_leds[LED_NUM_EXT], LED_NUM_INT, c);
			FastLED.show();
			vTaskDelay(pdMS_TO_TICKS(this->fps * 6));
		}
		vTaskDelay(pdMS_TO_TICKS(2000));
		FastLED.setBrightness(this->brightness);
		this->led_mode = 1;
	}

	void solid_color()
	{
		fill_solid(g_ext_leds, LED_NUM_TOTAL, this->color);
	}

	void random_solid()
	{
		vTaskDelay(pdMS_TO_TICKS(this->fps * 100));
		this->rainbow_index++;
		fill_solid(g_ext_leds, LED_NUM_TOTAL, ColorFromPalette(this->palette, inoise8(millis(), this->rainbow_index, xPortGetFreeHeapSize()) + this->rainbow_index, this->brightness, this->blend_mode));
	}

	void random_gradient()
	{
		vTaskDelay(pdMS_TO_TICKS(this->fps * 100));
		CHSV c = CHSV(random8(), 255, 255);
		CHSV c1 = CHSV(random8(), 255, 255);
		fill_gradient(g_ext_leds, LED_NUM_EXT, c, c1, c);
		fill_solid(&g_ext_leds[LED_NUM_EXT], LED_NUM_INT, c1);
	}

	void shine()
	{
		this->set_brightness(inoise16(millis(), this->rainbow_index, xPortGetFreeHeapSize()));
		this->solid_color();
	}

	void heartbeat()
	{
		this->set_brightness(beat8(88));
		this->solid_color();
	}

	void rainbow()
	{
		for (int i = 0; i < LED_NUM_EXT; i++)
			g_ext_leds[i] = ColorFromPalette(this->palette, i, this->brightness, this->blend_mode);
		fill_solid(&g_ext_leds[LED_NUM_EXT], LED_NUM_INT, g_ext_leds[LED_NUM_EXT / 2]);
	}

	void moving_rainbow()
	{
		this->rainbow_index++;
		unsigned int r = this->rainbow_index;
		for (int i = 0; i < LED_NUM_EXT; i++)
		{
			g_ext_leds[i] = ColorFromPalette(this->palette, r++, this->brightness, this->blend_mode);
			r += 2;
		}
		r = this->rainbow_index;
		for (int i = 0; i < LED_NUM_INT; i++)
		{
			g_ext_leds[LED_NUM_EXT + i] = ColorFromPalette(this->palette, r, this->brightness, this->blend_mode);
			r += 2;
		}
	}

	void show()
	{
		if (this->on)
		{
			(this->*dv[led_mode])();
			FastLED.show();
			vTaskDelay(pdMS_TO_TICKS(this->fps));
		}
		else
		{
			FastLED.clear();
			FastLED.show();
		}
	}
};

bool handle_commands(String user_text, LedState *cled_state)
{
	bool c = true;

	if (user_text == "/on")
		cled_state->on = true;
	else if (user_text == "/off")
		cled_state->on = false;
	else if (user_text == "/red")
		cled_state->color = 0xFF0000;
	else if (user_text == "/light_red")
		cled_state->color = 0xCD5C5C;
	else if (user_text == "/green")
		cled_state->color = 0x008000;
	else if (user_text == "/lime")
		cled_state->color = 0x32CD32;
	else if (user_text == "/blue")
		cled_state->color = 0x0000FF;
	else if (user_text == "/cyan")
		cled_state->color = 0x00FFFF;
	else if (user_text == "/yellow")
		cled_state->color = 0xFFFF00;
	else if (user_text == "/orange")
		cled_state->color = 0xFF4500;
	else if (user_text == "/purple")
		cled_state->color = 0x800080;
	else if (user_text == "/pink")
		cled_state->color = 0xFF1493;
	else if (user_text == "/magenta")
		cled_state->color = 0xFF69B4;
	else if (user_text == "/white")
		cled_state->color = 0xFFFFFF;
	else if (user_text == "/warm_white")
		cled_state->color = 0xFAFAD2;
	else if (user_text.startsWith("/color", 0))
	{
		String s = user_text.substring(6);
		Serial.print("Get custom color: ");
		//		Serial.println(stoi(s, 0, 16));
		if (s.length() > 0)
			cled_state->color = s.toInt();
		else
			g_bot.sendMessage(CHAT_ID, "Please enter a RGB color value)");
	}
	else if (user_text == "/solid_color")
		cled_state->mode = 1;
	else if (user_text == "/random_solid")
		cled_state->mode = 2;
	else if (user_text == "/shine")
		cled_state->mode = 3;
	else if (user_text == "/heartbeat")
		cled_state->mode = 4;
	else if (user_text == "/rainbow")
		cled_state->mode = 5;
	else if (user_text == "/moving_rainbow")
		cled_state->mode = 6;
	else if (user_text == "/random_gradient")
		cled_state->mode = 7;
	else if (user_text == "/rainbow_color")
	{
		cled_state->palette = 0;
		cled_state->blend = true;
	}
	else if (user_text == "/rainbow_color_stripe")
	{
		cled_state->palette = 1;
		cled_state->blend = false;
	}
	else if (user_text == "/rainbow_color_random")
	{
		cled_state->palette = 2;
		cled_state->blend = true;
	}
	else if (user_text.startsWith("/brightness", 0))
	{
		String b = user_text.substring(11);
		if (b.length() > 0)
		{
			int b_int = b.toInt();
			if (b_int > 0 && b_int <= 255)
				cled_state->brightness = b_int;
			else
				g_bot.sendMessage(CHAT_ID, ERROR_BRIGHTNESS);
		}
		else
			g_bot.sendMessage(CHAT_ID, ERROR_BRIGHTNESS);
	}
	else
		c = false;
	return (c);
}

void handle_new_messages(int nu_new, LedState *cled_state)
{
	for (int i = 0; i < nu_new; i++)
	{
		String chat_id = String(g_bot.messages[i].chat_id);
		if (chat_id == CHAT_ID)
		{
			String user_text = g_bot.messages[i].text;
			Serial.println(user_text);
			if (user_text == "/start" || user_text == "/help" || user_text == "/commands")
				g_bot.sendMessage(chat_id, WELCOME, "");
			else
			{
				if (handle_commands(user_text, cled_state))
					// Send cled_state->update to Task2 if changed
					xQueueSend(g_xQueue,
							   (void *)cled_state,
							   (TickType_t)10);
			}
		}
	}
}

void command_task(void *p)
{
	Serial.print("command_task running on core: ");
	Serial.println(xPortGetCoreID());
	unsigned long last_time_bot = 0;
	struct LedState cled_state;

	// Init cled_state use in queue
	cled_state.on = true;
	cled_state.mode = 0;
	cled_state.brightness = 50;
	cled_state.color = 0x303030;
	cled_state.blend = true;
	cled_state.palette = 0;
	for (;;)
	{
		if (millis() > last_time_bot + 1000)
		{
			int nu_new = g_bot.getUpdates(-1);
			while (nu_new)
			{
				handle_new_messages(nu_new, &cled_state);
				nu_new = g_bot.getUpdates(g_bot.last_message_received + 1);
			}
			last_time_bot = millis();
		}
	}
	vTaskDelete(NULL);
}

void led_task(void *p)
{
	Serial.print("led_task running on core: ");
	Serial.println(xPortGetCoreID());

	BaseType_t queue_signal;
	struct LedState led_state;
	LedController ext_led_controller(100);
	for (;;)
	{
		// Receive update from `command_task` with pointer to cled_state->
		if (g_xQueue != NULL)
		{
			queue_signal = xQueueReceive(g_xQueue,
										 &led_state,
										 (TickType_t)0);
			if (queue_signal == pdPASS)
			{
				ext_led_controller.set_on(led_state.on);
				ext_led_controller.set_led_mode(led_state.mode);
				ext_led_controller.set_color(led_state.color);
				ext_led_controller.set_brightness(led_state.brightness);
				ext_led_controller.set_palette(led_state.palette);
				ext_led_controller.set_blend_mode(led_state.blend);
			}
			// static mode `solid_static || rainbow` no need all this refresh
			if (queue_signal == pdPASS || ext_led_controller.get_led_mode() != 1 || ext_led_controller.get_led_mode() != 7)
				ext_led_controller.show();
		}
	}
	vTaskDelete(NULL);
}

void connect_to_wifi()
{
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID, PASSWORD);
	g_WIFIClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
	while (WiFi.status() != WL_CONNECTED)
		delay(1000);
	Serial.println("Connected to WiFi..");
}

void setup()
{
	Serial.begin(115200);
	connect_to_wifi();

	g_xQueue = xQueueCreate(5, sizeof(struct LedState));
	if (g_xQueue == NULL)
		Serial.println("Failed to create queue");

	FastLED.addLeds<NEOPIXEL, LED_PIN_EXT>(g_ext_leds, LED_NUM_TOTAL);
	// Create and launched tasks
	xTaskCreate(
		command_task,
		"command_task",
		5000,
		NULL,
		2,
		&g_TelegramTask);
	xTaskCreate(
		led_task,
		"led_task",
		50000,
		NULL,
		3,
		&g_LedTask);
	Serial.println("Setup done");
	//	g_bot.sendMessage(CHAT_ID, "Connected to WiFi ;)\n" + WELCOME, "");
}

void loop()
{
}
