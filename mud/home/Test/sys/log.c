#include <kotaka/paths/system.h>
#include <kotaka/paths/string.h>
#include <kotaka/log.h>
#include <status.h>

static void create()
{
	call_out("bomb", 0, 100, time());
}

static void bomb(int bombs, int time)
{
	int curtime;

	while (bombs && status(ST_TICKS) > 50000) {
		LOGD->post_message(
			"debug", LOG_DEBUG, STRINGD->spaces(random(4096))
		);
		bombs--;
		write_file("logcheck", ctime(time()) + ": " + bombs + " log bombs left\n");
	}

	if (bombs) {
		curtime = time();

		if (time < curtime) {
			time = curtime;

			LOGD->post_message("debug", LOG_DEBUG, bombs + " log bombs left to detonate");
		}

		call_out("bomb", 0, bombs, time);
	}
}
