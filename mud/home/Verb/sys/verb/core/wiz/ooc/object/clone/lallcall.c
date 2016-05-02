/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
		send_out("Usage: lallcall <blueprint name> <function>\n");
		return;
	}

	st = status(path);

	if (!st) {
		send_out("No such object.\n");
		return;
	}

	call_other(path, func);

	sz = status(ST_OTABSIZE);

	call_out("lazy_allcall", 0, path, func, sz - 1);
}

static void lazy_allcall(string path, string func, int oindex)
{
	object obj;

	obj = find_object(path + "#" + oindex);

	if (obj) {
		call_other(obj, func);
	}

	if (oindex == 0) {
		LOGD->post_message("debug", LOG_DEBUG, "Lazy allcall finished.");
	} else {
		call_out("lazy_allcall", 0, path, func, oindex - 1);
	}
}
