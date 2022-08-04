/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/verb.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit LIB_VERB;

static void allcall(string path, string func, int index, int limit)
{
	object obj;

	obj = find_object(path + "#" + index);

	if (obj) {
		call_other(obj, func);
	}

	if (index < limit) {
		call_out("allcall", 0, path, func, index + 1, limit);
	} else {
		LOGD->post_message("debug", LOG_DEBUG, "Allcall finished");
	}
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	wipe_callouts();
}

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Allcall";
}

string *query_help_contents()
{
	return ({ "Calls a given function in all clones of a given master object" });
}

void main(object actor, mapping roles)
{
	string path;
	string func;
	string args;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to call all clones.\n");
		return;
	}

	args = roles["raw"];

	if (!args || !sscanf(args, "%s %s", path, func)) {
		send_out("Usage: allcall <blueprint name> <function>\n");
		return;
	}

	call_out("allcall", 0, path, func, 0, status(ST_OTABSIZE));
}
