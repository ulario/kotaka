#include <kotaka/paths/system.h>
#include <kotaka/log.h>

inherit "~Text/lib/sub";

static void create()
{
	LOGD->post_message("debug", LOG_DEBUG,
		pinkfish2ansi("pinkfish%^YELLOW%^GREEN%^OCTARINE%^yellow%^RED")
	);
}
