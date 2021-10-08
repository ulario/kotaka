/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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
#include <kotaka/assert.h>
#include <status.h>

inherit LIB_VERB;
inherit "/lib/sort";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Pinfo";
}

string *query_help_contents()
{
	return ({ "Checks registered program info for a master object" });
}

void main(object actor, mapping roles)
{
	int sz;
	mixed *arr, index, val;
	object pinfo;
	string path;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can do that.\n");
		return;
	}

	path = roles["raw"];

	if (!path) {
		send_out("Usage: pinfo <path|index>\n");
	}

	if (sscanf(path, "%d", index)) {
		pinfo = OBJECTD->query_program_info(index);

		if (!pinfo) {
			send_out("Program not found\n");
			return;
		}

		path = pinfo->query_path();
	} else {
		index = status(path, O_INDEX);

		if (index == nil) {
			send_out("No such program is in service\n");
			return;
		}

		pinfo = OBJECTD->query_program_info(index);

		if (!pinfo) {
			send_out("Program not registered\n");
			return;
		}
	}

	send_out("Program: " + path + " (#" + index + ")");

	if (pinfo->query_destructed()) {
		send_out(" (destructed)");
	}

	arr = pinfo->query_inherits();

	if (!arr) {
		send_out(" (unregistered)");
	}

	if (sscanf(path, "%*s" + CLONABLE_SUBDIR)) {
		int clones;

		clones = pinfo->query_clone_count();

		send_out(" (" + clones + " " + (clones == 1 ? "clone" : "clones") + ")");
	}

	send_out("\n\n");

	if (arr && (sz = sizeof(arr))) {
		int i;

		send_out("Inherits:\n");

		for (i = 0; i < sz; i++) {
			object libpinfo;

			libpinfo = OBJECTD->query_program_info(arr[i]);

			if (libpinfo) {
				string libpath;

				libpath = libpinfo->query_path();

				if (libpinfo->query_destructed()) {
					send_out("    " + libpath + " (destructed #" + arr[i] + ")\n");
				} else {
					send_out("    " + libpath + " (#" + arr[i] + ")\n");
				}
			} else {
				send_out("    (unregistered #" + arr[i] + ")\n");
			}
		}

		send_out("\n");
	}

	arr = pinfo->query_includes();

	if (sz = sizeof(arr)) {
		int i;

		send_out("Includes:\n");

		for (i = 0; i < sz; i++) {
			send_out("    " + arr[i] + "\n");
		}

		send_out("\n");
	}

	val = pinfo->query_constructor();

	if (val) {
		send_out("Constructor: " + val + "\n");
	}

	arr = pinfo->query_inherited_constructors();

	if (sz = sizeof(arr)) {
		int i;

		send_out("Inherited constructors:\n");

		for (i = 0; i < sz; i++) {
			send_out("    " + arr[i] + "\n");
		}

		send_out("\n");
	}

	val = pinfo->query_destructor();

	if (val) {
		send_out("Destructor: " + val + "\n");
	}

	arr = pinfo->query_inherited_destructors();

	if (sz = sizeof(arr)) {
		int i;

		send_out("Inherited destructors:\n");

		for (i = 0; i < sz; i++) {
			send_out("    " + arr[i] + "\n");
		}

		send_out("\n");
	}

	val = pinfo->query_patcher();

	if (val) {
		send_out("Patcher: " + val + "\n");
	}

	arr = pinfo->query_inherited_patchers();

	if (sz = sizeof(arr)) {
		int i;

		send_out("Inherited patchers:\n");

		for (i = 0; i < sz; i++) {
			send_out("    " + arr[i] + "\n");
		}

		send_out("\n");
	}
}
