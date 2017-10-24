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
#include <kernel/access.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;
inherit "~/lib/system/list";

/* private functions */

private void scan_objects(string path, mixed **libs, mixed **objs, mixed **nots)
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
			scan_objects(path + name, libs, objs, nots);
			continue;
		}

		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;

			opath = path + name[0 .. strlen(name) - 3];

			status = status(opath);

			if (!status) {
				if (nots) {
					list_push_back(nots, opath);
				}
				continue;
			}

			if (sscanf(opath, "%*s" + INHERITABLE_SUBDIR)) {
				if (libs) {
					list_push_back(libs, opath);
				}
			} else {
				if (objs) {
					list_push_back(objs, opath);
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
	mixed **libs, **objs, **initds;

	int i;
	int sz;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	ASSERT(find_object(PROGRAMD));

	libs = ({ nil, nil });
	objs = ({ nil, nil });
	initds = ({ nil, nil });

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

			if (pinfo->query_destructed()) {
				continue;
			}

			if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
				list_push_back(libs, path);
			} else if (sscanf(path, USR_DIR + "/%*s/initd") || path == "/initd") {
				list_push_back(initds, path);
			} else {
				list_push_back(objs, path);
			}
		}

		while (!list_empty(libs)) {
			string path;

			path = list_back(libs);
			list_pop_back(libs);

			destruct_object(path);
		}

		while (!list_empty(initds)) {
			string path;

			path = list_back(initds);
			list_pop_back(initds);

			catch {
				compile_object(path);
			}
		}

		while (!list_empty(objs)) {
			string path;

			path = list_back(objs);
			list_pop_back(objs);

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
		mixed *libs;
		mixed *objs;

		libs = ({ nil, nil });
		objs = ({ nil, nil });

		scan_objects("/", libs, objs, nil);

		while (!list_empty(libs)) {
			destruct_object(list_back(libs));
			list_pop_back(libs);
		}

		while (!list_empty(objs)) {
			compile_object(list_back(objs));
			list_pop_back(objs);
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

mixed **query_dormant()
{
	mixed **nots;

	nots = ({ nil, nil });

	scan_objects("/", nil, nil, nots);

	return nots;
}
