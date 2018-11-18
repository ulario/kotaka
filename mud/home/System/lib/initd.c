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
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/list";
inherit "~/lib/utility/compile";

void reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);
}

void hotboot()
{
	ACCESS_CHECK(previous_program() == MODULED);
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);
}

int forbid_inherit(string from, string path, int priv)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return 0;
}

int forbid_call(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return 0;
}

string include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return path;
}

string query_constructor(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return nil;
}

string query_destructor(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return nil;
}

string query_patcher(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return nil;
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);
}

void booted_module(string module)
{
	ACCESS_CHECK(previous_program() == MODULED);
}

void shutdown_module(string module)
{
	ACCESS_CHECK(previous_program() == MODULED);
}

static void purge_orphans(string module)
{
	rlimits (0; 1000000) {
		mixed **list;

		list = OBJECTD->query_program_indices();

		while (!list_empty(list)) {
			int index;
			object pinfo;
			string file;

			index = list_front(list);
			list_pop_front(list);

			pinfo = OBJECTD->query_program_info(index);

			if (!pinfo) {
				continue;
			}

			file = pinfo->query_path();

			if (DRIVER->creator(file) != module) {
				continue;
			}

			if (file_info(file + ".c")) {
				continue;
			}

			destruct_object(file);
		}
	}
}

void upgrade_purge()
{
	string module;

	ACCESS_CHECK(SYSTEM());

	module = DRIVER->creator(object_name(this_object()));

	ASSERT(module);

	purge_orphans(module);

	purge_dir(USR_DIR + "/" + module);
}

void upgrade_build()
{
	string module;

	ACCESS_CHECK(SYSTEM());

	module = DRIVER->creator(object_name(this_object()));

	ASSERT(module);

	compile_dir(USR_DIR + "/" + module);
}
