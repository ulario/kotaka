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
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/verb.h>
#include <kotaka/log.h>
#include <status.h>

inherit LIB_VERB;

void upgrade()
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	mixed *st;
	int i, sz;
	int tcount;
	string path;
	string func;
	object list;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to call all clones.\n");
		return;
	}

	if (sscanf(roles["raw"], "%s %s", path, func) != 2) {
		send_out("Usage: allcall <blueprint name> <function>\n");
		return;
	}

	if (find_object(path)) {
		call_other(path, func);
	}

	call_out("allcall", 0, path, func, status(ST_OTABSIZE), 0);
}

static void allcall(string path, string func, int oindex, int total)
{
	do {
		object obj;

		oindex--;

		obj = find_object(path + "#" + oindex);

		if (obj) {
			rlimits (0; 50000) {
				call_other(obj, func);
			}
			total++;
			break;
		}
	} while (oindex > 0 && status(ST_TICKS) > 75000);

	if (oindex) {
		call_out("allcall", 0, path, func, oindex, total);
	} else {
		LOGD->post_message("debug", LOG_DEBUG, "Allcall finished, " + total + " objects called");
	}
}