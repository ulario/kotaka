/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/verb.h>
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

	sz = status(ST_OTABSIZE);

	list = new_object(BIGSTRUCT_DEQUE_LWO);

	for (i = 0; i < sz; i++) {
		object obj;

		obj = find_object(path + "#" + i);

		if (obj) {
			list->push_back(obj);
			tcount++;
		}
	}

	call_out("do_allcall", 0, list, func);

	send_out(tcount + " object calls scheduled.\n");
}

static void do_allcall(object list, string func)
{
	object obj;

	obj = list->query_front();
	list->pop_front();

	if (!list->empty()) {
		call_out("do_allcall", 0, list, func);
	}

	if (obj) {
		call_other(obj, func);
	}
}
