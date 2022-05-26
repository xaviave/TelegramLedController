#include <common.h>
#include <LedController.h>
#include <TelegramHandler.h>
#include <WifiServerHandler.h>

QueueHandle_t g_xQueue;
TaskHandle_t g_LedTask;
TaskHandle_t g_TelegramTask;

void command_task(void *p)
{
	Serial.print("command_task running on core: ");
	Serial.println(xPortGetCoreID());
	handle_new_messages(g_xQueue);
	vTaskDelete(NULL);
}

void led_task(void *p)
{
	Serial.print("led_task running on core: ");
	Serial.println(xPortGetCoreID());
	handle_queue_commands(g_xQueue);
	vTaskDelete(NULL);
}

void setup()
{
	Serial.begin(115200);
	wifi_handler();

	// Launch tasks
	g_xQueue = xQueueCreate(5, sizeof(struct LedState));
	if (g_xQueue == NULL)
		Serial.println("Failed to create queue");

	// FastLED.addLeds<NEOPIXEL, LED_PIN_EXT>(g_ext_leds, LED_NUM_TOTAL);
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
}

void loop()
{
	// we got tasks, no need loop
}
