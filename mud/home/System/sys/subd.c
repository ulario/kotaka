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
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;
inherit LIB_SYSTEM;

/* private functions */

private void scan_objects(string path, object libqueue, object objqueue, object notqueue)
{
	string *names;
	int *sizes;
	mixed **dir;
	int i;

	path = find_object(DRIVER)->normalize_path(path, "/");

	if (path[strlen(path) - 1] != '/') {
		path += "/";
	}

	dir = get_dir(path + "*");
	names = dir[0];
	sizes = dir[1];

	for (i = 0; i < sizeof(names); i++) {
		string name;
		string opath;

		name = names[i];

		if (sizes[i] == -2) {
			scan_objects(path + name, libqueue, objqueue, notqueue);
			continue;
		}

		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;

			opath = path + name[0 .. strlen(name) - 3];

			status = status(opath);

			if (!status) {
				if (notqueue) {
					notqueue->push_back(opath);
				}
				continue;
			}

			if (sscanf(opath, "%*s" + INHERITABLE_SUBDIR)) {
				if (libqueue) {
					libqueue->push_back(opath);
				}
			} else {
				if (objqueue) {
					objqueue->push_back(opath);
				}
			}
		}
	}
}

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

object query_orphans()
{
	object orphans;
	object indices;
	int sz, i;

	orphans = new_object(BIGSTRUCT_ARRAY_LWO);
	orphans->claim();
	orphans->grant_access(previous_object(), READ_ACCESS);

	indices = PROGRAMD->query_program_indices();
	sz = indices->query_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		string path;

		pinfo = PROGRAMD->query_program_info(indices->query_element(i));

		if (!file_info((path = pinfo->query_path()) + ".c")) {
			orphans->push_back(path);
		}
	}

	return orphans;
}

void discover_objects()
{
	ACCESS_CHECK(PRIVILEGED());

	rlimits(0; -1) {
		object libqueue;
		object objqueue;
		int sz;

		libqueue = new_object(BIGSTRUCT_ARRAY_LWO);
		libqueue->claim();
		objqueue = new_object(BIGSTRUCT_ARRAY_LWO);
		objqueue->claim();

		scan_objects("/", libqueue, objqueue, nil);

		for (sz = libqueue->query_size(); --sz >= 0; ) {
			destruct_object(libqueue->query_element(sz));
		}

		for (sz = objqueue->query_size(); --sz >= 0; ) {
			compile_object(objqueue->query_element(sz));
		}
	}
}

atomic void full_reset()
{
	object ind;
	object paths;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	rlimits(0; -1) {
		ind = PROGRAMD->query_program_indices();
		paths = new_object(BIGSTRUCT_ARRAY_LWO);
		paths->claim();

		while (!ind->empty()) {
			object pinfo;

			pinfo = PROGRAMD->query_program_info(ind->query_back());
			ind->pop_back();

			paths->push_back(pinfo->query_path());
		}

		PROGRAMD->reset_program_database();

		discover_objects();

		while (!paths->empty()) {
			string path;

			path = paths->query_back();
			paths->pop_back();

			if (!status(path)) {
				continue;
			}

			if (PROGRAMD->query_program_index(path) == -1) {
				LOGD->post_message("system", LOG_INFO, "Restoring orphaned program " + path);
				PROGRAMD->register_program(path, ({ }), ({ }));
			}
		}
	}
}

object query_dormant()
{
	object notlist;

	notlist = new_object(BIGSTRUCT_ARRAY_LWO);
	notlist->claim();
	notlist->grant_access(previous_object(), READ_ACCESS);

	scan_objects("/", nil, nil, notlist);

	return notlist;
}
