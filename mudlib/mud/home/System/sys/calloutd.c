#include <status.h>

#include <kernel/rsrc.h>

#include <kotaka/assert.h>
#include <kotaka/bigstruct.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

int suspend;			/* callouts suspended */
int handle;			/* releaser handle */

int lrc;			/* last reported callout count */
int lrh;			/* last reported hole count */

object cmap;	/* ([ oindex : ([ handle : iterator ]) ]) */
object cqueue;	/* ({ iterator : ({ obj, handle }) }) */

int begin, end;
int callouts, holes;

private int bypass(object obj);
int empty();
mixed *release();

/* private */

static void create()
{
	cmap = clone_object(BIGSTRUCT_ARRAY_OBJ);
	cqueue = clone_object(BIGSTRUCT_ARRAY_OBJ);

	cmap->set_size(0x7FFFFFFF);
	cqueue->set_size(0x10000000);

	begin = end = callouts = holes = handle = 0;

	catch {
		RSRCD->set_suspension_manager(this_object());
	}
}

static void destruct()
{
	if (suspend) {
		error("Cannot destruct while callouts are suspended");
	}

	destruct_object(cmap);
	destruct_object(cqueue);
}

private int object_index(object obj)
{
	int oindex;

	if (sscanf(object_name(obj), "%*s#%d", oindex)) {
		return oindex;
	}

	return status(obj, O_INDEX);
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
	if (changed_enough(callouts, lrc)) {
		lrc = callouts;
		LOGD->post_message("system", LOG_INFO, "callouts: " + callouts + " suspended");
	}

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

/* rsrcd hooks */

void suspend_callouts()
{
	if (previous_program() == RSRCD) {
		LOGD->post_message("system", LOG_INFO, "Suspending callouts");

		suspend = -1;

		if (handle) {
			remove_call_out(handle);
			handle = 0;
		}
	} else {
		RSRCD->suspend_callouts();
	}
}

void suspend(object obj, int handle)
{
	mixed *callout;

	mapping map;
	int oindex;

	ACCESS_CHECK(previous_program() == RSRCD);

	if (bypass(obj)) {
		RSRCD->release_callout(obj, handle);
		return;
	}

	if ((end + 1) & 0x0FFFFFFF == begin) {
		error("Suspension queue overflow");
	}

	oindex = object_index(obj);

	map = cmap->get_element(oindex);

	if (!map) {
		map = ([ ]);
		cmap->set_element(oindex, map);
	}

	map[handle] = end;

	cqueue->set_element(end++, ({ obj, handle }) );
	end &= 0x0FFFFFFF;
	callouts++;

	statcheck();
}

int remove_callout(object obj, int handle)
{
	mapping map;
	int oindex;
	int qindex;

	ACCESS_CHECK(previous_program() == RSRCD);

	oindex = object_index(obj);

	map = cmap->get_element(oindex);

	if (!map || map[handle]) {
		return FALSE;
	}

	qindex = map[handle];
	map[handle] = nil;

	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}

	callouts--;
	holes++;

	statcheck();

	return TRUE;
}

void remove_callouts(object obj)
{
	mapping map;
	int oindex;
	int *handles;
	int *qindices;
	int sz;
	int osz;

	ACCESS_CHECK(previous_program() == RSRCD);

	oindex = object_index(obj);

	map = cmap->get_element(oindex);

	if (!map) {
		return;
	}

	cmap->set_element(oindex, nil);

	handles = map_indices(map);
	qindices = map_values(map);

	osz = sizeof(handles);

	for (sz = osz; --sz >= 0; ) {
		cqueue->set_element(qindices[sz], nil);
	}

	callouts -= osz;
	holes += osz;

	statcheck();
}

void release_callouts()
{
	if (previous_program() == RSRCD) {
		LOGD->post_message("system", LOG_INFO, "Releasing callouts");

		suspend = 1;

		/* start draining backed up callouts */
		if (!handle) {
			handle = call_out("do_release", 0);
		}
	} else {
		RSRCD->release_callouts();
	}
}

int query_suspend()
{
	return suspend;
}

/* internal */

static void do_release()
{
	mixed *callout;
	object obj;
	int ohandle;

	handle = 0;

	switch(suspend) {
	case -1: /* suspended! */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while suspended");
		return;

	case 0: /* idle! */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while idle");
		return;

	case 1: /* draining */
		callout = release();

		if (callout) {
			obj = callout[0];
			ohandle = callout[1];

			if (obj) {
				catch {
					RSRCD->release_callout(obj, ohandle);
				}
			}

			statcheck();
			handle = call_out("do_release", 0);
		} else {
			RSRCD->release_callout(nil, 0);
			LOGD->post_message("system", LOG_INFO, "Released callouts");

			suspend = 0;
			begin = end = 0;
			cmap->clear();
			cqueue->clear();

			if (callouts != 0) {
				error(callouts + " callouts unaccounted for");
			}

			if (holes != 0) {
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

	if (suspend == 1) {
		return 1;
	}

	return DRIVER->creator(object_name(obj)) == "System";
}

int empty()
{
	return begin == end;
}

mixed *release()
{
	mixed *callout;
	mixed map;
	int oindex;

	ACCESS_CHECK(SYSTEM());

	if (begin == end) {
		return nil;
	}

	callout = cqueue->get_element(begin);
	cqueue->set_element(begin++, nil);

	begin &= 0x0FFFFFFF;

	if (!callout) {
		holes--;
		return ({ nil, -1 });
	} else {
		callouts--;
	}

	if (!callout[0]) {
		return ({ nil, -1 });
	}

	oindex = object_index(callout[0]);
	map = cmap->get_element(oindex);

	ASSERT(map);
	ASSERT(map[callout[1]] != nil);

	map[callout[1]] = nil;

	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}

	return callout;
}
