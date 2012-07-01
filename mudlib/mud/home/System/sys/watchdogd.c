#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

int callout;
float frag_angst;

void freeze();
void thaw();

static void create()
{
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || LOCAL());

	if (callout) {
		error("Watchdog already enabled");
	}

	LOGD->post_message("system", LOG_NOTICE, "Watchdog enabled");

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
		LOGD->post_message("system", LOG_NOTICE, "Watchdog disabled");
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
	int mem_size;
	int mem_used;
	int mem_free;

	int obj_size;
	int obj_used;
	int obj_free;

	int swap_size;
	int swap_used;
	int swap_free;

	rlimits(0; -1) {
		callout = call_out("check", 1);
	}

	mem_used = status(ST_SMEMUSED) + status(ST_DMEMUSED);
	mem_size = status(ST_SMEMSIZE) + status(ST_DMEMSIZE);
	mem_free = mem_size - mem_used;

	obj_used = status(ST_NOBJECTS);
	obj_size = status(ST_OTABSIZE);
	obj_free = obj_size - obj_used;

	swap_used = status(ST_SWAPUSED);
	swap_size = status(ST_SWAPSIZE);
	swap_free = swap_size - swap_used;

	if ((float)swap_free / (float)swap_size < 0.10) {
		LOGD->post_message("watchdogd", LOG_NOTICE, "Swap file almost full, shutting down");
		freeze();
		dump_state(1);
		shutdown();
		return;
	}

	if ((float)obj_free / (float)obj_size < 0.10) {
		LOGD->post_message("watchdogd", LOG_NOTICE, "Object table almost full, shutting down");
		freeze();
		dump_state(1);
		shutdown();
		return;
	}

	if (mem_used >> 20 > 512) {
		LOGD->post_message("watchdogd", LOG_NOTICE, "Memory full, swapping out");
		frag_angst = 0.0;
		return;
	}

	frag_angst += ((float)mem_free / (float)mem_size) - 0.50;

	if (frag_angst < 0.0) {
		frag_angst = 0.0;
	}

	if (frag_angst > 5.0) {
		LOGD->post_message("watchdogd", LOG_NOTICE, "Memory fragmented, swapping out");
		frag_angst = 0.0;
		swapout();
	}
}

void freeze()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	CALLOUTD->suspend_callouts();
	SYSTEM_USERD->block_connections();
}

void thaw()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	SYSTEM_USERD->unblock_connections();
	CALLOUTD->release_callouts();
}
