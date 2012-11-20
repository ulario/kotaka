/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kernel/access.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <type.h>
#include <status.h>

inherit SECOND_AUTO;

/****************/
/* Declarations */
/****************/

/* variables */

string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
int upgrading;		/* are we upgrading or making a new compile? */

/***************/
/* Definitions */
/***************/

/* internal */

static void create()
{
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

private string *fetch_from_initd(object initd, string path)
{
	string ctor;
	string dtor;
	string err;

	err = catch(ctor = initd->query_constructor(path));

	if (!err) {
		err = catch(dtor = initd->query_destructor(path));
	}

	return ({ err, ctor, dtor });
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

/* external */

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_object_manager(this_object());
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

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

	int i;
	int sz;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	libqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	objqueue = new_object(BIGSTRUCT_DEQUE_LWO);

	rlimits(0; -1) {
		indices = PROGRAMD->get_object_indices();
		sz = indices->get_size();

		for (i = 0; i < sz; i++) {
			int oindex;
			object pinfo;
			string path;

			oindex = indices->get_element(i);
			pinfo = PROGRAMD->query_program_info(oindex);
			path = pinfo->query_path();

			if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
				libqueue->push_back(path);
			} else {
				objqueue->push_back(path);
			}
		}

		while (!libqueue->empty()) {
			string path;

			path = libqueue->get_front();
			libqueue->pop_front();

			destruct_object(path);
		}

		while (!objqueue->empty()) {
			string path;

			path = objqueue->get_front();
			objqueue->pop_front();

			catch {
				compile_object(path);
			}
		}
	}
}

void discover_objects()
{
	object libqueue;
	object objqueue;

	ACCESS_CHECK(PRIVILEGED());

	rlimits(0; -1) {
		libqueue = new_object(BIGSTRUCT_DEQUE_LWO);
		objqueue = new_object(BIGSTRUCT_DEQUE_LWO);

		scan_objects("/", libqueue, objqueue, nil);

		while (!libqueue->empty()) {
			string path;

			path = libqueue->get_front();
			libqueue->pop_front();

			destruct_object(path);
		}

		while (!objqueue->empty()) {
			string path;

			path = objqueue->get_front();
			objqueue->pop_front();

			compile_object(path);
		}
	}
}

void full_reset()
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	PROGRAMD->reset_program_database();

	discover_objects();
}

object query_orphans()
{
	object orphans;
	object indices;
	int sz, i;

	orphans = new_object(BIGSTRUCT_ARRAY_LWO);
	orphans->grant_access(previous_object(), READ_ACCESS);

	indices = PROGRAMD->query_program_indices();
	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		string path;

		pinfo = PROGRAMD->query_program_info(indices->get_element(i));

		if (!file_info(path = pinfo->query_path() + ".c")) {
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

/* kernel library hooks */

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

void compile(string owner, object obj, string *source, string inherited ...)
{
	string path;
	string err;
	int is_kernel;
	int is_auto;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);

	if (path != DRIVER) {
		inherited |= ({ AUTO });
	}

	if (find_object(PROGRAMD)) {
		PROGRAMD->register_program(path, inherited, includes, nil, nil);
	}

	includes = nil;

	is_kernel = sscanf(path, "/kernel/%*s");

	if (is_kernel) {
		return;
	}

	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");

	if (!is_auto && !sizeof(({ SECOND_AUTO }) & inherited)) {
		error("Failure to inherit SECOND_AUTO: " + path);
	}

	if (path == USR_DIR + "/" + owner + "/initd" && !sizeof(({ LIB_INITD }) & inherited)) {
		error("Failure to inherit LIB_INITD: " + path);
	}

	if (upgrading) {
		upgrading = 0;

		obj->upgrading();
	}
}

void compile_lib(string owner, string path, string *source, string inherited ...)
{
	string err;
	string ctor;
	string dtor;
	object initd;
	int is_kernel;
	int is_auto;

	ACCESS_CHECK(KERNEL());

	if (path != AUTO) {
		inherited |= ({ AUTO });
	}

	is_kernel = sscanf(path, "/kernel/%*s");

	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");

	if (!is_kernel && !is_auto && !sizeof(({ SECOND_AUTO }) & inherited)) {
		error("Failure to inherit SECOND_AUTO: " + path);
	}

	if (!is_kernel) {
		initd = find_object(USR_DIR + "/" + owner + "/initd");
	}

	if (initd) {
		string *ret;

		ret = fetch_from_initd(initd, path);

		err = ret[0];
		ctor = ret[1];
		dtor = ret[2];
	}

	if (find_object(PROGRAMD)) {
		PROGRAMD->register_program(path, inherited, includes, ctor, dtor);
	}

	includes = nil;

	if (err) {
		error(err);
	}
}

void compile_failed(string owner, string path)
{
	ACCESS_CHECK(KERNEL());

	upgrading = 0;
	includes = nil;
}

void clone(string owner, object obj)
{
	object pinfo;
	string path;

	ACCESS_CHECK(KERNEL());
}

void destruct(string owner, object obj)
{
	int isclone;
	string name;
	string path;
	object pinfo;

	ACCESS_CHECK(KERNEL());

	name = object_name(obj);

	if (sscanf(name, "%s#%*d", path)) {
		return;
	}

	path = name;

	if (is_protected(path)) {
		error("Forbidden");
	}

	if (find_object(PROGRAMD)) {
		pinfo = PROGRAMD->query_program_info(status(obj, O_INDEX));
	}

	if (!pinfo) {
		return;
	}

	if (!sscanf(path, "/kernel/%*s")) {
		obj->_F_sys_destruct();
	}

	pinfo->set_destructed();
}

void destruct_lib(string owner, string path)
{
	object pinfo;

	ACCESS_CHECK(KERNEL());

	if (find_object(PROGRAMD)) {
		pinfo = PROGRAMD->query_program_info(status(path)[O_INDEX]);
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
