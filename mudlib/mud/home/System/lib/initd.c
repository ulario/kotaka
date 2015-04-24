/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

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

string query_toucher(string path)
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
	object list;

	list = PROGRAMD->query_program_indices();

	rlimits (0; -1) {
		while (!list->empty()) {
			int index;
			object pinfo;
			string file;

			index = list->query_back();
			list->pop_back();

			pinfo = PROGRAMD->query_program_info(index);

			if (!pinfo) {
				continue;
			}

			file = pinfo->query_path();

			if (!sscanf(file, USR_DIR + "/" + module + "/%*s")) {
				continue;
			}

			if (file_info(file + ".c")) {
				continue;
			}

			destruct_object(file);
		}
	}
}
