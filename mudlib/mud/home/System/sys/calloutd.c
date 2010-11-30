#include <status.h>

#include <kernel/rsrc.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

# undef CO_HANDLE

# define CO_OBJ		0	/* callout object */
# define CO_HANDLE	1	/* handle in object */
# define CO_PREV	2	/* previous callout */
# define CO_NEXT	3	/* next callout */

inherit SECOND_AUTO;

int suspend;			/* callouts suspended */
int last_notify;		/* co count at which we last reported */
object queue;

int releases;			/* current number of release callouts */
int max_releases;		/* maximum number of release callouts */

static void create()
{
	queue = load_object("callout_queue");
	max_releases = 500;

	catch {
		RSRCD->set_suspension_manager(this_object());
	}
}

private void statcheck()
{
	int callouts;

	callouts = queue->callouts();

	if (callouts % 1000 == 0 && last_notify != callouts) {
		last_notify = callouts;
		LOGD->post_message("system", LOG_INFO, callouts / 1000 + "k callouts suspended");
	}
}

static void resume()
{
	RSRCD->release_callouts();
}

atomic void hold_callouts(float delay)
{
	RSRCD->suspend_callouts();

	call_out("resume", delay);
}

void suspend_callouts()
{
	if (previous_program() == RSRCD) {
		LOGD->post_message("system", LOG_INFO, "Suspending callouts");

		suspend = -1;
	} else {
		RSRCD->suspend_callouts();
	}
}

void set_max_releases(int new_max)
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (new_max < 1) {
		error("Invalid argument");
	}

	max_releases = new_max;
}

private void charge_release()
{
	int callouts;

	callouts = queue->callouts();

	while(releases < max_releases && releases < callouts) {
		call_out("release", 0);
		releases++;
	}
}

void release_callouts()
{
	if (previous_program() == RSRCD) {
		LOGD->post_message("system", LOG_INFO, "Releasing callouts");

		if (queue->empty()) {
			suspend = 0;
			LOGD->post_message("system", LOG_INFO, "Released callouts");
		} else {
			suspend = 1;

			charge_release();
		}
	} else {
		RSRCD->release_callouts();
	}
}

private int bypass(object obj)
{
	if (obj == this_object()) {
		return 1;
	}

	if (sscanf(object_name(obj), USR_DIR + "/Game/obj/ustate/status#%*d")) {
		return 1;
	}

	return DRIVER->creator(object_name(obj)) == "System";
}

void suspend(object obj, int handle)
{
	mixed *callout;

	ACCESS_CHECK(previous_program() == RSRCD);

	if (bypass(obj)) {
		RSRCD->release_callout(obj, handle);
		return;
	}

	queue->suspend(obj, handle);

	statcheck();
}

int remove_callout(object obj, int handle)
{
	int suspended;
	mapping callouts;
	mixed *callout;

	ACCESS_CHECK(previous_program() == RSRCD);

	suspended = queue->remove_callout(obj, handle);

	statcheck();

	return suspended;
}

void remove_callouts(object obj)
{
	ACCESS_CHECK(previous_program() == RSRCD);

	queue->remove_callouts(obj);

	statcheck();
}

static void release()
{
	mixed *callout;
	object obj;
	int handle;
	int callouts;

	releases--;
	callout = queue->release();

	if (callout) {
		obj = callout[CO_OBJ];
		handle = callout[CO_HANDLE];

		if (obj) {
			catch {
				RSRCD->release_callout(obj, handle);
			}
		}
		statcheck();
	} else {
		suspend = 0;
	}

	switch(suspend) {
	case -1:
		break;
	case 0:
		RSRCD->release_callout(nil, 0);
		LOGD->post_message("system", LOG_INFO, "Released callouts");

		if (callouts = queue->callouts() != 0) {
			error(callouts + " callouts unaccounted for");
		}

		break;
	case 1:
		charge_release();
		break;
	}
}
