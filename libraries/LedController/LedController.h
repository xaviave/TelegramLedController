#ifndef _LED_CONTROLLER_H_
#define _LED_CONTROLLER_H_

#include <vector>
#include <FastLED.h>

#define LED_PIN_EXT 16
#define LED_NUM_EXT 144
#define LED_NUM_INT (20 + 10)
#define LED_NUM_TOTAL (LED_NUM_EXT + LED_NUM_INT)
#define LED_HALF_NUM_EXT (int)(LED_NUM_EXT * 0.7)

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

	void init_display_vec();
	LedController(byte brightness);
	byte get_led_mode();
	// Setters
	void set_on(bool on);
	void set_led_mode(byte led_mode);
	void set_color(uint32_t color);
	void set_blend_mode(bool blend);
	void set_palette(byte palette);
	void set_brightness(byte brightness);
	// Color effects
	void start_mode();
	void solid_color();
	void random_solid();
	void random_gradient();
	void shine();
	void heartbeat();
	void rainbow();
	void moving_rainbow();
	void show();
};

void handle_queue_commands(QueueHandle_t g_xQueue);

#endif
