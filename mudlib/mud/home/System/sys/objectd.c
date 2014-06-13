/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/algorithm.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
int upgrading;		/* are we upgrading or making a new compile? */
int is_initd;		/* current compilation is for an initd */
int is_kernel;		/* current compilation is for a kernel object */
int is_auto;		/* current compilation is for a second auto support library */

static void create()
{
	DRIVER->set_object_manager(this_object());
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

private string *fetch_from_initd(object initd, string path)
{
	return ({
		initd->query_constructor(path),
		initd->query_destructor(path),
		initd->query_toucher(path)
	});
}

private mixed query_include_file(string compiled, string from, string path)
{
	string creator;
	object initd;

	/* don't allow bypass of standard file */
	if (path == "/include/std.h") {
		return path;
	}

	creator = find_object(DRIVER)->creator(compiled);

	/* System has to be direct */
	if (creator == "System") {
		return path;
	}

	/* don't allow bypass of standard auto */
	if (path == "/include/AUTO" &&
		from == "/include/std.h") {
		return USR_DIR + "/System/include/second_auto.h";
	}

	if (initd = find_object(USR_DIR + "/" + creator + "/initd")) {
		return initd->include_file(compiled, from, path);
	}

	return path;
}

private void scan_objects_light(string path, mapping libqueue, mapping objqueue)
{
	string *names;
	int *sizes;
	mixed **dir;
	int i;

	path = find_object(DRIVER)->normalize_path(path, "/");

	dir = get_dir(path + "/*");
	names = dir[0];
	sizes = dir[1];

	for (i = 0; i < sizeof(names); i++) {
		string name;
		string opath;

		name = names[i];

		if (sizes[i] == -2) {
			scan_objects_light(path + "/" + name, libqueue, objqueue);
			continue;
		}

		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;

			opath = path + "/" + name[0 .. strlen(name) - 3];

			status = status(opath);

			if (!status) {
				continue;
			}

			if (sscanf(opath, "%*s" + INHERITABLE_SUBDIR)) {
				libqueue[opath] = 1;
			} else {
				objqueue[opath] = 1;
			}
		}
	}
}

private void scan_objects(string path, object libqueue, object objqueue, object notqueue)
{
	string *names;
	int *sizes;
	mixed **dir;
	int i;

	path = find_object(DRIVER)->normalize_path(path, "/");

	dir = get_dir(path + "/*");
	names = dir[0];
	sizes = dir[1];

	for (i = 0; i < sizeof(names); i++) {
		string name;
		string opath;

		name = names[i];

		if (sizes[i] == -2) {
			scan_objects(path + "/" + name, libqueue, objqueue, notqueue);
			continue;
		}

		if (strlen(name) > 2 && name[strlen(name) - 2 ..] == ".c") {
			string opath;
			mixed *status;
			int oindex;

			opath = path + "/" + name[0 .. strlen(name) - 3];

			status = status(opath);

			if (!status) {
				if (notqueue) {
					notqueue->push_back(opath + ".c");
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

private int is_protected(string path)
{
	if (sscanf(path, USR_DIR + "/Bigstruct/%*s/")) {
		return 1;
	}

	switch(path) {
	case INITD:
	case OBJECTD:
		return 1;
	}

	return 0;
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || INTERFACE());

	DRIVER->set_object_manager(this_object());
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || INTERFACE());

	DRIVER->set_object_manager(nil);
}

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
	objqueue = new_object(BIGSTRUCT_ARRAY_LWO);
	initdqueue = new_object(BIGSTRUCT_ARRAY_LWO);

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
			} else if (sscanf(path, USR_DIR + "/%*s/initd")) {
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

		SORTD->bqsort(objqueue, 0, objqueue->query_size() - 1);

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

void discover_objects()
{
	ACCESS_CHECK(PRIVILEGED());

	rlimits(0; -1) {
		mixed libqueue;
		mixed objqueue;
		int sz, i;

		libqueue = ([ ]);
		objqueue = ([ ]);

		scan_objects_light("/", libqueue, objqueue);

		libqueue = map_indices(libqueue);
		sz = sizeof(libqueue);

		for (i = 0; i < sz; i++) {
			destruct_object(libqueue[i]);
		}

		objqueue = map_indices(objqueue);
		sz = sizeof(objqueue);

		for (i = 0; i < sz; i++) {
			compile_object(objqueue[i]);
		}
	}
}

atomic void full_reset()
{
	object ind;
	object paths;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	ind = PROGRAMD->query_program_indices();
	paths = new_object(BIGSTRUCT_ARRAY_LWO);

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

object query_orphans()
{
	object orphans;
	object indices;
	int sz, i;

	orphans = new_object(BIGSTRUCT_ARRAY_LWO);
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

object query_dormant()
{
	object notlist;

	notlist = new_object(BIGSTRUCT_ARRAY_LWO);
	notlist->grant_access(previous_object(), READ_ACCESS);

	scan_objects("/", nil, nil, notlist);

	return notlist;
}

void compiling(string path)
{
	ACCESS_CHECK(KERNEL());

	if (path == DRIVER || path == AUTO) {
		includes = ({ });
	} else {
		includes = ({ "/include/std.h" });
	}

	if (find_object(path)) {
		upgrading = 1;
	}
}

private void compile_common(string owner, string path, string *source, string *inherited)
{
	object initd;
	object pinfo;

	if (find_object(PROGRAMD)) {
		pinfo = PROGRAMD->register_program(path, inherited, includes);
	}

	if (is_initd) {
		return;
	}

	if (is_kernel) {
		return;
	}

	if (!is_auto && !sizeof(({ SECOND_AUTO }) & inherited)) {
		error("Failure to inherit SECOND_AUTO: " + path);
	}

	if (pinfo) {
		initd = find_object(USR_DIR + "/" + owner + "/initd");
	}

	if (initd) {
		string *ret;

		ret = fetch_from_initd(initd, path);

		pinfo->set_constructor(ret[0]);
		pinfo->set_destructor(ret[1]);
		pinfo->set_toucher(ret[2]);
	}

	includes = nil;
}

private void set_flags(string path, string owner)
{
	is_kernel = sscanf(path, "/kernel/%*s");
	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");
	is_initd = (path == USR_DIR + "/" + owner + "/initd");
}

void compile(string owner, object obj, string *source, string inherited ...)
{
	string path;
	string err;
	int index;
	object pinfo;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);

	set_flags(path, owner);

	if (path != DRIVER) {
		inherited |= ({ AUTO });
	}

	if (sscanf(path, USR_DIR + "/%*s/_code")) {
		/* klib wiztool "code" command, ignore */
		return;
	}

	compile_common(owner, path, source, inherited);

	if (is_initd) {
		if (!sizeof(({ LIB_INITD }) & inherited)) {
			error("Failure to inherit LIB_INITD: " + path);
		}
	}

	if (upgrading) {
		upgrading = 0;

		if (!is_kernel) {
			if (function_object("upgrading", obj)) {
				catch {
					obj->upgrading();
				}
			}
		}
	}

	if (find_object(TOUCHD)) {
		pinfo = PROGRAMD->query_program_info(status(obj, O_INDEX));

		if (pinfo) {
			if (pinfo->query_toucher() || sizeof(pinfo->query_inherited_touchers())) {
				TOUCHD->touch_upgrade(path);
			}
		}
	}
}

void compile_lib(string owner, string path, string *source, string inherited ...)
{
	string ctor;
	string dtor;
	object initd;

	ACCESS_CHECK(KERNEL());

	set_flags(path, owner);

	if (path != AUTO) {
		inherited |= ({ AUTO });
	}

	compile_common(owner, path, source, inherited);
}

void compile_failed(string owner, string path)
{
	ACCESS_CHECK(KERNEL());

	upgrading = 0;
	includes = nil;
}

void clone(string owner, object obj)
{
	ACCESS_CHECK(KERNEL());

	if (find_object(CLONED)) {
		CLONED->add_clone(obj);
	}
}

void destruct(varargs mixed owner, mixed obj)
{
	int is_clone;
	string name;
	string path;
	object pinfo;

	if (!sscanf(previous_program(), "/kernel/%*s")) {
		ACCESS_CHECK(SYSTEM());
		/* regular destruct call */

		return;
	}

	ACCESS_CHECK(KERNEL());

	name = object_name(obj);

	if (sscanf(name, USR_DIR + "/%*s/_code")) {
		/* klib wiztool "code" command, ignore */
		return;
	}

	is_clone = sscanf(name, "%s#%*d", path);

	if (!path) {
		path = name;
	}

	if (!sscanf(path, "/kernel/%*s")) {
		obj->_F_sys_destruct();
	}

	if (is_clone) {
		if (find_object(CLONED)) {
			CLONED->remove_clone(obj);
		}
	} else {
		if (find_object(PROGRAMD)) {
			pinfo = PROGRAMD->query_program_info(status(obj, O_INDEX));
		}

		if (pinfo) {
			pinfo->set_destructed();
		}
	}
}

void destruct_lib(string owner, string path)
{
	object pinfo;

	ACCESS_CHECK(KERNEL());

	if (find_object(PROGRAMD)) {
		pinfo = PROGRAMD->query_program_info(status(path, O_INDEX));
	}

	if (!pinfo) {
		return;
	}

	pinfo->set_destructed();
}

void remove_program(string owner, string path, int timestamp, int index)
{
	ACCESS_CHECK(KERNEL());

	if (find_object(PROGRAMD)) {
		PROGRAMD->remove_program(index);
	}

	if (find_object(CLONED)) {
		CLONED->remove_program(index);
	}
}

mixed include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	path = query_include_file(compiled, from, path);

	includes |= ({ path });

	return path;
}

int touch(object obj, string function)
{
	ACCESS_CHECK(KERNEL());

	if (!sscanf(object_name(obj), "/kernel/%*s")) {
		obj->_F_touch(function);
	}
}

int forbid_call(string path)
{
	ACCESS_CHECK(KERNEL());
}

int forbid_inherit(string from, string path, int priv)
{
	object initd;

	ACCESS_CHECK(KERNEL());

	initd = find_object(USR_DIR + "/" + DRIVER->creator(path) + "/initd");

	if (!initd) {
		error("No initd loaded for " + path);
	}

	return initd->forbid_inherit(from, path, priv);
}

object find_all_clones(string path)
{
	object list;
	int i, sz;

	list = new_object(BIGSTRUCT_ARRAY_LWO);
	sz = status(ST_OTABSIZE);

	for (i = 0; i < sz; i++) {
		object obj;

		if (obj = find_object(path + "#" + i)) {
			list->push_back(obj);
		}
	}

	list->grant_access(previous_object(), FULL_ACCESS);
	list->grant_access(this_object(), 0);

	return list;
}
