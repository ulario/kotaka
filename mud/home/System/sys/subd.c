/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;

/* private functions */

/* public functions */

void recompile_kernel_library()
{
	string *names;
	mixed **dir;
	int i;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	dir = get_dir("/kernel/lib/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0 .. strlen(name) - 3];

		destruct_object("/kernel/lib/" + name);
	}

	dir = get_dir("/kernel/lib/api/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0..strlen(name) - 3];

		destruct_object("/kernel/lib/api/" + name);
	}

	dir = get_dir("/kernel/obj/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0 .. strlen(name) - 3];

		if (find_object("/kernel/obj/" + name)) {
			compile_object("/kernel/obj/" + name);
		}
	}

	dir = get_dir("/kernel/sys/*");
	names = dir[0];

	for (i = 0; i < sizeof(names); i++) {
		string name;

		name = names[i];
		name = name[0..strlen(name) - 3];

		if (find_object("/kernel/sys/" + name)) {
			compile_object("/kernel/sys/" + name);
		}
	}
}

void recompile_everything()
{
	object indices;
	object libqueue;
	object objqueue;
	object initdqueue;

	int i;
	int sz;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	ASSERT(find_object(PROGRAMD));

	libqueue = new_object(BIGSTRUCT_ARRAY_LWO);
	libqueue->claim();
	objqueue = new_object(BIGSTRUCT_ARRAY_LWO);
	objqueue->claim();
	initdqueue = new_object(BIGSTRUCT_ARRAY_LWO);
	initdqueue->claim();

	rlimits(0; -1) {
		indices = PROGRAMD->query_program_indices();
		sz = indices->query_size();

		for (i = 0; i < sz; i++) {
			int oindex;
			object pinfo;
			string path;

			oindex = indices->query_element(i);
			pinfo = PROGRAMD->query_program_info(oindex);
			path = pinfo->query_path();

			if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
				libqueue->push_back(path);
			} else if (sscanf(path, USR_DIR + "/%*s/initd") || path == "/initd") {
				initdqueue->push_back(path);
			} else {
				objqueue->push_back(path);
			}
		}

		while (!libqueue->empty()) {
			string path;

			path = libqueue->query_back();
			libqueue->pop_back();

			destruct_object(path);
		}

		objqueue->grant_access(find_object(SORTD), WRITE_ACCESS);
		SORTD->qsort(objqueue, 0, objqueue->query_size() - 1);

		while (!initdqueue->empty()) {
			string path;

			path = initdqueue->query_back();
			initdqueue->pop_back();

			catch {
				compile_object(path);
			}
		}

		while (!objqueue->empty()) {
			string path;

			path = objqueue->query_back();
			objqueue->pop_back();

			catch {
				compile_object(path);
			}
		}
	}
}
