#include <common.h>
#include <TelegramHandler.h>

WiFiClientSecure g_WIFIClient;
UniversalTelegramBot g_bot(BOT_TOKEN, g_WIFIClient);

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

void process_new_messages(int nu_new, LedState *cled_state, QueueHandle_t g_xQueue)
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

void handle_new_messages(QueueHandle_t g_xQueue)
{
	struct LedState cled_state;
	unsigned long last_time_bot = 0;

	// Init cled_state use in queue
	cled_state.on = true;
	cled_state.mode = 0;
	cled_state.brightness = 50;
	cled_state.color = 0x303030;
	cled_state.blend = true;
	cled_state.palette = 0;
	g_WIFIClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);

	while (1)
	{
		if (millis() > last_time_bot + 1000)
		{
			int nu_new = g_bot.getUpdates(-1);
			while (nu_new)
			{
				process_new_messages(nu_new, &cled_state, g_xQueue);
				nu_new = g_bot.getUpdates(g_bot.last_message_received + 1);
			}
			last_time_bot = millis();
		}
	}
}
