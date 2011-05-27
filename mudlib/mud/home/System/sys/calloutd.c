#include <status.h>

#include <kernel/rsrc.h>

#include <kotaka/assert.h>
#include <kotaka/bigstruct.h>
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

int lrc;			/* last reported callout count */
int lrh;			/* last reported hole count */

object queue;
object releasers;

int releases;		/* current number of release callouts */
int max_releases;	/* maximum number of release callouts */
int unordered;	/* allow current callouts to skip the suspension queue */

private int bypass(object obj);

/* private */

static void create()
{
	queue = load_object("callout_queue");
	releasers = new_object(BIGSTRUCT_DEQUE_LWO);

	max_releases = 2000000;

	catch {
		RSRCD->set_suspension_manager(this_object());
	}
}

private void wipe_releasers()
{
	while (!releasers->empty()) {
		object releaser;

		releaser = releasers->get_front();
		releasers->pop_front();

		if (releaser) {
			destruct_object(releaser);
		}
	}

	releases = 0;
}

void set_unordered(int new_unordered)
{
	if (unordered == new_unordered) {
		return;
	}

	unordered = new_unordered;
}

int query_unordered()
{
	return unordered;
}

private void charge_releasers()
{
	int needed;

	needed = queue->callouts() + queue->holes() + 1;

	if (needed > max_releases) {
		needed = max_releases;
	}

	needed -= releases;

	if (needed <= 0) {
		return;
	}

	if (needed < 100) {
		needed = 100;
	}

	while (needed > 0) {
		int charges;
		object releaser;

		charges = needed;

		if (charges > 500) {
			charges = 500;
		}

		releaser = clone_object("~/obj/co_releaser");
		releasers->push_back(releaser);

		releaser->charge(charges);
		releases += charges;
		needed -= charges;
	}
}

/* public */

private int changed_enough(int new, int old)
{
	if (new == old)
		return 0;

	if (old < new)
		old = new;

	if (new == 0)
		return 1;

	if (new % (int)sqrt((float)old) == 0)
		return 1;
}

private void statcheck()
{
	int callouts;
	int holes;

	callouts = queue->callouts();

	if (changed_enough(callouts, lrc)) {
		lrc = callouts;
		LOGD->post_message("system", LOG_INFO, "callouts: " + callouts + " suspended");
	}

	holes = queue->holes();

	if (changed_enough(holes, lrh)) {
		lrh = holes;
		LOGD->post_message("system", LOG_INFO, "callouts: " + holes + " holes");
	}
}

atomic void hold_callouts(mixed delay)
{
	RSRCD->suspend_callouts();

	call_out("release_callouts", delay);
}

void set_max_releases(int new_max)
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (new_max < 1) {
		error("Invalid argument");
	}

	max_releases = new_max;
}

/* rsrcd hooks */

void suspend_callouts()
{
	if (previous_program() == RSRCD) {
		LOGD->post_message("system", LOG_INFO, "Suspending callouts");

		suspend = -1;
		wipe_releasers();
	} else {
		RSRCD->suspend_callouts();
	}
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

	if (suspend == 1) {
		charge_releasers();
	}

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

void release_callouts()
{
	if (previous_program() == RSRCD) {
		LOGD->post_message("system", LOG_INFO, "Releasing callouts");

		if (queue->empty()) {
			suspend = 0;
			LOGD->post_message("system", LOG_INFO, "Released callouts");
		} else {
			suspend = 1;
			charge_releasers();
		}
	} else {
		RSRCD->release_callouts();
	}
}

/* internal */

void release()
{
	mixed *callout;
	object obj;
	int handle;
	int callouts;
	int holes;

	ACCESS_CHECK(SYSTEM());

	releases--;

	switch(suspend) {
	case -1: /* suspended! */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while suspended");
		wipe_releasers();
		return;

	case 0: /* idle! */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while idle");
		wipe_releasers();
		return;

	case 1: /* draining */
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
			if (!unordered) {
				RSRCD->release_callout(nil, 0);
			}
			LOGD->post_message("system", LOG_INFO, "Released callouts");

			suspend = 0;
			wipe_releasers();

			if (callouts = queue->callouts() != 0) {
				error(callouts + " callouts unaccounted for");
			}

			if (holes = queue->holes() != 0) {
				error(holes + " holes unaccounted for");
			}
		}

		break;
	}
}

private int bypass(object obj)
{
	if (obj == this_object()) {
		return 1;
	}

	if ((suspend == 1) && unordered) {
		return 1;
	}

	return DRIVER->creator(object_name(obj)) == "System";
}
