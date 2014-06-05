/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <status.h>

mapping trackers; /* ([ owner : tracker ]) */
int handle;

static void create()
{
	trackers = ([ ]);
}

void enable()
{
	handle = call_out("check", 0);
}

void disable()
{
	object *turkeys;
	int sz;

	turkeys = map_values(trackers);

	for (sz = sizeof(turkeys) - 1; sz >= 0; --sz) {
		destruct_object(turkeys[sz]);
	}

	handle = 0;

	{
		mixed *callouts;

		callouts = status(this_object(), O_CALLOUTS);

		for (sz = sizeof(callouts) - 1; sz >= 0; --sz) {
			remove_call_out(callouts[sz][CO_HANDLE]);
		}
	}
}

static void check()
{
	string *owners;
	int sz, i;

	owners = KERNELD->query_owners();

	sz = sizeof(owners);

	for (i = 0; i < sz; i++) {
		string owner;
		object first;
		object tracker;

		owner = owners[i];

		if (trackers[owner]) {
			continue;
		}

		first = KERNELD->first_link(owner);

		if (!first) {
			continue;
		}

		tracker = clone_object("~/obj/objregwatch");

		trackers[owner] = tracker;

		tracker->activate(first);
	}
}
