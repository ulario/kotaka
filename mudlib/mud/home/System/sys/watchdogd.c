#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

int callout;

float min_dmem_ratio;		/* for fragmentation control */
float min_dmem_slack;		/* avoidance of thrashing */
float max_dmem_size;		/* cap memory usage */

float max_obj_ratio;		/* emergency shutdown trigger */
float max_co_ratio;		/* emergency shutdown trigger */
float max_swap_ratio;		/* emergency shutdown trigger */

float check_interval;		/* how frequently to check */
float max_lag;			/* max lag in watchdog check before we suspend callouts for 10 seconds */
float deadline;			/* when the next watchdog cycle is due */

float swap_warn_ratio;		/* how many cycles left until the next swap warning */
int swap_warn_ticks;		/* how many cycles left until the next swap warning */

float obj_warn_ratio;		/* how many cycles left until the next swap warning */
int obj_warn_ticks;		/* how many cycles left until the next swap warning */

float co_warn_ratio;		/* how many cycles left until the next swap warning */
int co_warn_ticks;		/* how many cycles left until the next swap warning */

private void schedule(float fraction);
private string percentage(mixed part, mixed total);
void freeze();
void thaw();

static void create()
{
	int i;

	min_dmem_ratio = 0.75;
	min_dmem_slack = 1048576.0 * 64.0;
	max_dmem_size = 1048576.0 * 512.0;
	check_interval = 10.0;

	max_swap_ratio = 0.75;
	max_obj_ratio = 0.95;
	max_co_ratio = 0.95;

	max_lag = 0.0;

	obj_warn_ratio = 0.005;
	co_warn_ratio = 0.75;
	swap_warn_ratio = 0.50;

	callout = 0;
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || LOCAL());

	LOGD->post_message("system", LOG_NOTICE, "Watchdog enabled");

	schedule(0.0);
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (callout) {
		remove_call_out(callout);
	}

	LOGD->post_message("system", LOG_NOTICE, "Watchdog disabled");

	callout = 0;
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());

	if (callout) {
		mixed *time;

		time = millitime();
		remove_call_out(callout);
		deadline = (float)time[0] + time[1];
		callout = call_out("check", 0);
	}
}

void set_config_item(string key, mixed value)
{
	ACCESS_CHECK(KADMIN());
	
	switch(key) {
	case "max_dmem_size":
		value = (float)value;

		max_dmem_size = value;

		break;
		
	case "min_dmem_ratio":
		value = (float)value;
		
		if (value < 0.0 || value > 1.0) {
			error("Invalid setting");
		}
		
		min_dmem_ratio = value;
		
		break;
	
	case "min_dmem_slack":
		value = (float)value;
		
		if (value < 1048576.0) {
			error("Invalid setting");
		}

		min_dmem_slack = value;

		break;
		
	case "max_swap_ratio":
		value = (float)value;
		
		if (value < 0.0 || value > 1.0) {
			error("Invalid setting");
		}
		
		max_swap_ratio = value;
		
		break;

	case "swap_warn_ratio":
		value = (float)value;
		
		if (value < 0.0 || value > 1.0) {
			error("Invalid setting");
		}

		swap_warn_ratio = value;

		break;

	case "obj_warn_ratio":
		value = (float)value;

		if (value < 0.0 || value > 1.0) {
			error("Invalid setting");
		}
		
		obj_warn_ratio = value;
		
		break;
		
	case "check_interval":
		value = (float)value;
		
		if (value < 0.0) {
			error("Invalid setting");
		}

		check_interval = value;

		schedule(0.0);

		break;

	case "max_lag":
		value = (float)value;

		if (value < -1.0) {
			error("Invalid setting");
		}

		max_lag = value;

		break;

	default:
		error("Invalid key");
	}
}

mixed query_config_item(string key)
{
	ACCESS_CHECK(KADMIN());
	
	switch(key) {
	case "max_dmem_size":
		return max_dmem_size;
		
	case "min_dmem_ratio":
		return min_dmem_ratio;
	
	case "min_dmem_slack":
		return min_dmem_slack;
		
	case "max_swap_ratio":
		return max_swap_ratio;
	
	case "swap_warn_ratio":
		return swap_warn_ratio;
		
	case "obj_warn_ratio":
		return obj_warn_ratio;
		
	case "check_interval":
		return check_interval;
	
	case "max_lag":
		return max_lag;

	default:
		error("Invalid key");
	}
}

mapping query_config()
{
	return ([
		"max_dmem_size": max_dmem_size,
		"min_dmem_ratio": min_dmem_ratio,
		"min_dmem_slack": min_dmem_slack,
		"max_swap_ratio": max_swap_ratio,
		"swap_warn_ratio": swap_warn_ratio,
		"obj_warn_ratio": obj_warn_ratio,
		"check_interval": check_interval,
		"max_lag": max_lag
	]);
}

private string ralign(mixed num, int width)
{
	string str;

	str = "                " + (string) num;
	return str[strlen(str) - width ..];
}

private string percentage(mixed part, mixed total)
{
	if (total == 0) {
		return "-1%";
	}

	return (int)floor(((float)part / (float)total) * 100.0) + "%";
}

private void schedule(float health)
{
	catch {
		float delay;
		mixed *timestamp;

		if (callout != -1) {
			remove_call_out(callout);
		}

		if (check_interval <= 0.0) {
			callout = 0;
			return;
		}

		timestamp = millitime();

		delay = check_interval * health;
		deadline = (float)timestamp[0] + timestamp[1] + delay;

		catch {
			callout = call_out("check", delay);
		} : {
			error("Callout delay of " + delay);
		}
	} : {
		dump_state(1);
		shutdown();

		LOGD->post_message("system", LOG_CRIT, "Watchdog unable to schedule callout\nFreezing and shutting down.");
	}
}

static void check()
{
	float smemsize;
	float smemused;
	float dmemsize;
	float dmemused;
	float swapsize;
	float swapused;
	float objsize;
	float objused;
	float cosize;
	float coused;
	float worst;
	float fraction;
	float tardy;

	mixed *timestamp;

	int swap_warn_trigger;
	int obj_warn_trigger;
	int co_warn_trigger;

	int lag_trigger;
	int swap_trigger;
	int memory_trigger;
	int frag_trigger;
	int obj_trigger;
	int co_trigger;

	if (swap_warn_ticks > 0) {
		swap_warn_ticks--;
	}

	if (obj_warn_ticks > 0) {
		obj_warn_ticks--;
	}

	if (co_warn_ticks > 0) {
		co_warn_ticks--;
	}

	timestamp = millitime();
	tardy = ((float)timestamp[0] + timestamp[1]) - deadline;

	if (tardy < 0.0) {
		tardy = 0.0;
	}

	swapused = (float)status()[ST_SWAPUSED];
	swapsize = (float)status()[ST_SWAPSIZE];

	objused = (float)status()[ST_NOBJECTS];
	objsize = (float)status()[ST_OTABSIZE];

	coused = (float)(status()[ST_NCOSHORT] + status()[ST_NCOLONG]);
	cosize = (float)status()[ST_COTABSIZE];

	smemused = (float)status()[ST_SMEMUSED];
	smemsize = (float)status()[ST_SMEMSIZE];

	dmemused = (float)status()[ST_DMEMUSED];
	dmemsize = (float)status()[ST_DMEMSIZE];

	if (max_lag > 0.0 && tardy > max_lag) {
		lag_trigger = 1;
	}

	fraction = swapused / swapsize;
	worst = (1.0 - fraction);

	if (fraction >= swap_warn_ratio && swap_warn_ticks <= 0) {
		swap_warn_ticks = 60;
		swap_warn_trigger = 1;
	}

	if (fraction >= max_swap_ratio) {
		swap_trigger = 1;
	}

	fraction = objused / objsize;

	if (worst > 1.0 - fraction) {
		worst = 1.0 - fraction;
	}

	if (fraction >= obj_warn_ratio && obj_warn_ticks <= 0) {
		obj_warn_ticks = 60;
		obj_warn_trigger = 1;
	}

	if (fraction >= max_obj_ratio) {
		obj_trigger = 1;
	}

	fraction = coused / cosize;

	if (worst > 1.0 - fraction) {
		worst = 1.0 - fraction;
	}

	if (fraction >= co_warn_ratio && co_warn_ticks <= 0) {
		co_warn_ticks = 60;
		co_warn_trigger = 1;
	}

	if (fraction >= max_co_ratio) {
		co_trigger = 1;
	}

	fraction = dmemsize / max_dmem_size;

	if (worst > 1.0 - fraction) {
		worst = 1.0 - fraction;
	}

	if (dmemsize > max_dmem_size) {
		memory_trigger = 1;
	}

	if (dmemsize - dmemused > min_dmem_slack) {
		/* ignore fragmentation if there's not enough free dmem */
		fraction = dmemused / dmemsize;

		if (worst > fraction) {
			worst = fraction;
		}

		if (fraction < min_dmem_ratio) {
			frag_trigger = 1;
		}
	}

	if (swap_trigger) {
		LOGD->post_message("system", LOG_ALERT, "Swap usage exceeded quota");
	}

	if (obj_trigger) {
		LOGD->post_message("system", LOG_ALERT, "Object usage exceeded quota");
	}

	if (co_trigger) {
		LOGD->post_message("system", LOG_ALERT, "Callout usage exceeded quota");
	}

	if (lag_trigger) {
		LOGD->post_message("system", LOG_NOTICE, "System lag exceeded quota");
	}

	if (memory_trigger) {
		LOGD->post_message("system", LOG_NOTICE, "Memory usage exceeded quota");
	}

	if (frag_trigger) {
		LOGD->post_message("system", LOG_NOTICE, "Memory fragmentation exceeded quota");
	}

	if (swap_trigger || obj_trigger || co_trigger) {
		LOGD->post_message("system", LOG_ALERT, "Freezing and shutting down");
		freeze();
		dump_state(1);
		shutdown();
	} else {
		if (lag_trigger) {
			LOGD->post_message("system", LOG_NOTICE, "Suspending callouts for 10 seconds");
			CALLOUTD->hold_callouts(10.0);
		}

		if (memory_trigger || frag_trigger) {
			LOGD->post_message("system", LOG_NOTICE, "Swapping out");
			swapout();
		}

		if (swap_warn_trigger) {
			LOGD->post_message("system", LOG_INFO, "Swap sectors used: " + swapused + ", swap file size: " + swapsize);
			LOGD->post_message("system", LOG_INFO, "Swap usage at " + percentage(swapused, swapsize));
		}

		if (obj_warn_trigger) {
			LOGD->post_message("system", LOG_INFO, "Objects used: " + objused + ", obj size: " + objsize);
			LOGD->post_message("system", LOG_INFO, "Object usage at " + percentage(objused, objsize));
		}

		if (co_warn_trigger) {
			LOGD->post_message("system", LOG_INFO, "Callouts used: " + objused + ", co size: " + cosize);
			LOGD->post_message("system", LOG_INFO, "Callout usage at " + percentage(coused, cosize));
		}
	}

	if (worst < 0.0) {
		worst = 0.0;
	}

	schedule(worst);
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

	CALLOUTD->suspend_callouts();
	SYSTEM_USERD->block_connections();
}
