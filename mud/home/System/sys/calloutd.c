/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kernel/rsrc.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

int suspend;			/* callouts suspended */
int handle;			/* releaser handle */
int dead;			/* if we're being destroyed */

object cmap;	/* ([ oindex : ([ handle : iterator ]) ]) */
object cqueue;	/* ({ iterator : ({ obj, handle }) }) */

int begin, end;

int empty();

private mixed *release();
private void alloc_queue();
private void free_queue();

private void wipe_callouts()
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

static void create()
{
	call_out("self_destruct", 0);
}

static void destruct()
{
	RSRCD->release_callouts();
	RSRCD->release_callout(nil, 0);

	wipe_callouts();

	rlimits (0; -1) {
		for (;;) {
			mixed *callout;
			object obj;
			int chandle;

			callout = release();

			if (!callout) {
				break;
			}

			({ obj, chandle }) = callout;

			if (chandle == -1) {
				break;
			}

			if (obj) {
				catch {
					RSRCD->release_callout(obj, chandle);
				}
			}
		}
	}

	free_queue();
}

private int object_index(object obj)
{
	int oindex;

	if (sscanf(object_name(obj), "%*s#%d", oindex)) {
		return oindex;
	}

	return status(obj, O_INDEX);
}

private void alloc_queue()
{
	cmap = new_object(BIGSTRUCT_ARRAY_LWO);
	cmap->claim();
	cqueue = new_object(BIGSTRUCT_ARRAY_LWO);
	cqueue->claim();

	cmap->set_size(status(ST_OTABSIZE));
	cqueue->set_size(0x40000000);
}

private void free_queue()
{
	if (cmap && !sscanf(object_name(cmap), "%*s#-1")) {
		destruct_object(cmap);
	} else {
		cmap = nil;
	}

	if (cqueue && !sscanf(object_name(cqueue), "%*s#-1")) {
		destruct_object(cqueue);
	} else {
		cqueue = nil;
	}
}

/* initd hooks */

void reboot()
{
	if (cmap) {
		cmap->set_size(status(ST_OTABSIZE));
	}
}

/* rsrcd hooks */

void suspend_callouts()
{
	if (previous_program() == RSRCD) {
		if (dead) {
			return;
		}

		suspend = -1;

		if (handle) {
			remove_call_out(handle);
			handle = 0;
		}

		if (!cmap) {
			alloc_queue();
		}
	} else {
		if (!SYSTEM()) {
			LOGD->post_message("system", LOG_WARNING, "Callout suspension is deprecated");
		}
		RSRCD->suspend_callouts();
	}
}

void suspend(object obj, int handle)
{
	mixed *callout;

	mapping map;
	int oindex;

	ACCESS_CHECK(previous_program() == RSRCD);

	catch {
		RSRCD->release_callout(obj, handle);
	}

	return;
}

int remove_callout(object obj, int handle)
{
	mapping map;
	int oindex;
	int qindex;

	ACCESS_CHECK(previous_program() == RSRCD);

	if (dead) {
		return TRUE;
	}

	oindex = object_index(obj);

	map = cmap->query_element(oindex);

	if (!map || map[handle] == nil) {
		return FALSE;
	}

	qindex = map[handle];
	map[handle] = nil;

	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}

	cqueue->set_element(qindex, nil);

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

	if (dead) {
		return;
	}

	oindex = object_index(obj);

	map = cmap->query_element(oindex);

	if (!map) {
		return;
	}

	cmap->set_element(oindex, nil);

	qindices = map_values(map);

	osz = sizeof(qindices);

	for (sz = osz; --sz >= 0; ) {
		cqueue->set_element(qindices[sz], nil);
	}
}

void release_callouts()
{
	if (previous_program() == RSRCD) {
		if (dead) {
			return;
		}

		suspend = 1;

		/* Don't send any more callouts our way */
		RSRCD->release_callout(nil, 0);

		/* start draining backed up callouts */
		if (!handle) {
			handle = call_out("do_release", 0);
		}
	} else {
		RSRCD->release_callouts();
	}
}

/* public */

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
	case -1: /* suspended */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while suspended");
		return;

	case 0: /* idle */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while idle");
		return;

	case 1: /* draining */
		callout = release();

		if (callout) {
			obj = callout[0];
			ohandle = callout[1];

			handle = call_out("do_release", 0);

			if (obj) {
				RSRCD->release_callout(obj, ohandle);
			}
		} else {
			RSRCD->release_callout(nil, 0);

			if (cmap) {
				free_queue();
			}

			suspend = 0;
			begin = end = 0;
		}

		break;
	}
}

private mixed *release()
{
	mixed *callout;
	mixed map;
	int oindex;

	if (begin == end) {
		return nil;
	}

	callout = cqueue->query_element(begin);
	cqueue->set_element(begin++, nil);

	begin &= 0x3FFFFFFF;

	if (!callout) {
		return ({ nil, -1 });
	}

	if (!callout[0]) {
		return ({ nil, -1 });
	}

	oindex = object_index(callout[0]);
	map = cmap->query_element(oindex);

	ASSERT(map);
	ASSERT(map[callout[1]] != nil);

	map[callout[1]] = nil;

	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}

	return callout;
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	call_out("self_destruct", 0);
}

static void self_destruct()
{
	destruct_object(this_object());
}
