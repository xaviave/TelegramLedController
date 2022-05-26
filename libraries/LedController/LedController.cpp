#include <common.h>
#include <LedController.h>

CRGB g_ext_leds[LED_NUM_TOTAL];

LedController::LedController(byte brightness)
{
	Serial.println("LedController init");
	FastLED.addLeds<NEOPIXEL, LED_PIN_EXT>(g_ext_leds, LED_NUM_TOTAL);
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

void LedController::init_display_vec()
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

byte LedController::get_led_mode()
{
	return (this->led_mode);
}

// Setters
void LedController::set_on(bool on)
{
	this->on = on;
}

void LedController::set_led_mode(byte led_mode)
{
	this->led_mode = led_mode;
}

void LedController::set_color(uint32_t color)
{
	this->color = CRGB(color);
}

void LedController::set_blend_mode(bool blend)
{
	this->blend_mode = blend ? LINEARBLEND : NOBLEND;
}

void LedController::set_palette(byte palette)
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

void LedController::set_brightness(byte brightness)
{
	this->brightness = brightness;
	FastLED.setBrightness(brightness);
}

// Color effects
void LedController::start_mode()
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

void LedController::solid_color()
{
	fill_solid(g_ext_leds, LED_NUM_TOTAL, this->color);
}

void LedController::random_solid()
{
	vTaskDelay(pdMS_TO_TICKS(this->fps * 100));
	this->rainbow_index++;
	fill_solid(g_ext_leds, LED_NUM_TOTAL, ColorFromPalette(this->palette, inoise8(millis(), this->rainbow_index, xPortGetFreeHeapSize()) + this->rainbow_index, this->brightness, this->blend_mode));
}

void LedController::random_gradient()
{
	vTaskDelay(pdMS_TO_TICKS(this->fps * 100));
	CHSV c = CHSV(random8(), 255, 255);
	CHSV c1 = CHSV(random8(), 255, 255);
	fill_gradient(g_ext_leds, LED_NUM_EXT, c, c1, c);
	fill_solid(&g_ext_leds[LED_NUM_EXT], LED_NUM_INT, c1);
}

void LedController::shine()
{
	this->set_brightness(inoise16(millis(), this->rainbow_index, xPortGetFreeHeapSize()));
	this->solid_color();
}

void LedController::heartbeat()
{
	this->set_brightness(beat8(88));
	this->solid_color();
}

void LedController::rainbow()
{
	for (int i = 0; i < LED_NUM_EXT; i++)
		g_ext_leds[i] = ColorFromPalette(this->palette, i, this->brightness, this->blend_mode);
	fill_solid(&g_ext_leds[LED_NUM_EXT], LED_NUM_INT, g_ext_leds[LED_NUM_EXT / 2]);
}

void LedController::moving_rainbow()
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

void LedController::show()
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

void handle_queue_commands(QueueHandle_t g_xQueue)
{
	BaseType_t queue_signal;
	struct LedState led_state;
	LedController ext_led_controller(100);

	while (1)
	{
		// Receive update from `command_task` with pointer to cled_state
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
}
