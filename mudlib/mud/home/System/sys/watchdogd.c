#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

int callout;
int frag_angst;

static void create()
{
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || LOCAL());

	if (callout) {
		error("Watchdog already enabled");
	}

	rlimits (0; -1) {
		callout = call_out("check", 1);
	}
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (!callout) {
		error("Watchdog not enabled");
	}

	rlimits (0; -1) {
		remove_call_out(callout);
		callout = 0;
	}
}

int enabled()
{
	return !!callout;
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

static void check()
{
	float mem_size;
	float mem_used;
	float mem_free;

	int obj_size;
	int obj_used;
	int obj_free;

	int swap_size;
	int swap_used;
	int swap_free;

	int freeze;

	rlimits(0; -1) {
		callout = call_out("check", 1);
	}

	mem_used = (float)status(ST_DMEMUSED);
	mem_size = (float)status(ST_DMEMSIZE);
	mem_free = mem_size - mem_used;

	obj_used = status(ST_NOBJECTS);
	obj_size = status(ST_OTABSIZE);
	obj_free = obj_size - obj_used;

	swap_used = status(ST_SWAPUSED);
	swap_size = status(ST_SWAPSIZE);
	swap_free = swap_size - swap_used;

#if 0
	if ((float)obj_free / (float)obj_size < 0.25) {
		if (CALLOUTD->query_suspend() != -1) {
			CALLOUTD->suspend_callouts();
		}
	}

	if ((float)swap_free / (float)swap_size < 0.25) {
		if (CALLOUTD->query_suspend() != -1) {
			CALLOUTD->suspend_callouts();
		}
	}
#endif

	if (mem_used > ldexp(1.0, 31)) {
		LOGD->post_message("watchdog", LOG_NOTICE, "Memory full, swapping out");
		swapout();
		return;
	}

	if (mem_free > (float)(64 << 20) && (mem_free / mem_size) > 0.75) {
		if (!frag_angst) {
			LOGD->post_message("watchdog", LOG_NOTICE, "Memory fragmented");
		}
		++frag_angst;
		if (frag_angst >= 60) {
			frag_angst = 0;
			LOGD->post_message("watchdog", LOG_NOTICE, "Defragmenting");
//			swapout();
		}
	} else {
		if (frag_angst) {
			if (!--frag_angst) {
				LOGD->post_message("watchdog", LOG_NOTICE, "Memory no longer fragmented");
			}
		}
	}

}
