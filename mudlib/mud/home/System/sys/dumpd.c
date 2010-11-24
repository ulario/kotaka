#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

static void create()
{
}

void schedule_dump(int delay)
{
	call_out("dump", delay);
}

static void dump()
{
	dump_state(1);
}
