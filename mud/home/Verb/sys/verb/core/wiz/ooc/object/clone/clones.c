/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/verb.h>
#include <kotaka/log.h>
#include <status.h>

inherit LIB_VERB;

int handle;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string path;
	mixed *st;
	int i, sz;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a clone check.\n");
		return;
	}

	path = roles["raw"];

	if (handle) {
		remove_call_out(handle);
	}

	handle = call_out("lazy_clones", 0, path, status(ST_OTABSIZE) - 1, time());
}

static void lazy_clones(string path, int oindex, varargs int time)
{
	object obj;

	obj = find_object(path + "#" + oindex);

	if (obj) {
		LOGD->post_message("debug", LOG_DEBUG, "Found " + object_name(obj));
	}

	if (oindex == 0) {
		handle = 0;
		LOGD->post_message("debug", LOG_DEBUG, "Lazy clone enumeration finished.");
	} else {
		int newtime;

		newtime = time();

		if (time < newtime) {
			LOGD->post_message("debug", LOG_DEBUG, "Lazy clone enumeration: " + oindex + " slots left to check.");
			time = time();
		}

		handle = call_out("lazy_clones", 0, path, oindex - 1, time);
	}
}
