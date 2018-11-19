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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;
inherit LIB_SYSTEM;
inherit "~/lib/struct/list";
inherit "~/lib/utility/compile";

mixed **upgrades;	/* upgrade list */
string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
int upgrading;		/* are we upgrading or making a new compile? */
object progdb;		/* program database */

static void create()
{
	progdb = new_object(SPARSE_ARRAY);

	DRIVER->set_object_manager(this_object());
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!progdb) {
		progdb = new_object(SPARSE_ARRAY);
	}
}

/* helpers */

private object fetch_program_info(int index)
{
	object pinfo;

	if (progdb) {
		return progdb->query_element(index);
	}
}

private object setup_ghost_program_info(string path, int index)
{
	object pinfo;

	if (!progdb) {
		progdb = new_object(SPARSE_ARRAY);
	}

	pinfo = new_object(PROGRAM_INFO);
	pinfo->set_path(path);

	progdb->set_element(index, pinfo);

	return pinfo;
}

private int *inherited_indices(string *inherited)
{
	int sz;
	int *ret;

	for (sz = sizeof(inherited), ret = allocate(sz); --sz >= 0; ) {
		ret[sz] = status(inherited[sz], O_INDEX);
	}

	return ret;
}

private object setup_program_info(string path, string *inherited)
{
	int index;
	object pinfo;

	int *inh;

	index = status(path, O_INDEX);

	pinfo = fetch_program_info(index);

	if (!pinfo) {
		pinfo = setup_ghost_program_info(path, index);
	}

	inh = inherited_indices(inherited);

	pinfo->set_includes(includes[..]);
	pinfo->set_inherits(inh);

	if (!sscanf(path, "/kernel/%*s")) {
		object initd;
		int i;
		int sz;
		string creator;

		string constructor, destructor, patcher;
		string *iconstructors, *idestructors, *ipatchers;

		creator = DRIVER->creator(path);

		if (creator) {
			initd = find_object(USR_DIR + "/" + creator + "/initd");
		} else {
			initd = find_object("/initd");
		}

		if (initd) {
			constructor = initd->query_constructor(path);
			destructor = initd->query_destructor(path);
			patcher = initd->query_patcher(path);
		}

		pinfo->set_constructor(constructor);
		pinfo->set_destructor(destructor);
		pinfo->set_patcher(patcher);

		sz = sizeof(inherited);

		iconstructors = ({ });
		idestructors = ({ });
		ipatchers = ({ });

		for (i = 0; i < sz; i++) {
			object libpinfo;
			int libindex;
			string lconstructor;
			string ldestructor;
			string lpatcher;

			libindex = inh[i];

			libpinfo = fetch_program_info(libindex);

			if (!libpinfo) {
				LOGD->post_message("system", LOG_WARNING, "Unable to access program_info for inherited object " + inherited[i]);

				continue;
			}

			lconstructor = libpinfo->query_constructor();
			ldestructor = libpinfo->query_destructor();
			lpatcher = libpinfo->query_patcher();

			/* call inherited first */
			iconstructors |= libpinfo->query_inherited_constructors();
			idestructors |= libpinfo->query_inherited_destructors();
			ipatchers |= libpinfo->query_inherited_patchers();

			if (lconstructor) {
				iconstructors |= ({ lconstructor });
			}

			if (ldestructor) {
				idestructors |= ({ ldestructor });
			}

			if (lpatcher) {
				ipatchers |= ({ lpatcher });
			}
		}

		pinfo->set_inherited_constructors(iconstructors - ({ constructor }));
		pinfo->set_inherited_destructors(idestructors - ({ destructor }));
		pinfo->set_inherited_patchers(ipatchers - ({ patcher }));
	}

	progdb->set_element(index, pinfo);

	return pinfo;
}

private string query_include_file(string compiled, string from, string path)
{
	string creator;

	if (path == "/include/std.h") {
		return path;
	}

	creator = DRIVER->creator(compiled);

	if (creator == "System") {
		return path;
	}

	if (path == "/include/AUTO"
		&& from == "/include/std.h"
		&& (!creator || compiled != USR_DIR + "/" + creator + "/_code")) {
		return USR_DIR + "/System/include/second_auto.h";
	}

	return path;
}

static void destruct_object(object obj)
{
	if (obj) {
		::destruct_object(obj);
	}
}

static void upgrade_object(object obj)
{
	if (obj && function_object("upgrade", obj)) {
		obj->upgrade();
	}
}

static void process()
{
	object obj;

	if (!upgrades || list_empty(upgrades)) {
		upgrades = nil;
		return;
	}

	call_out("process", 0);

	obj = list_front(upgrades);
	list_pop_front(upgrades);

	if (obj && function_object("upgrade", obj)) {
		obj->upgrade();
	}
}

/* klib hooks */

void compiling(string path)
{
	string creator;
	string initd;

	ACCESS_CHECK(previous_program() == DRIVER);

	includes = ({ "/include/std.h" });

	if (sscanf(path, USR_DIR + "/%*s/_code")) {
		/* klib wiztool "code" command, ignore it */
		return;
	}

	creator = DRIVER->creator(path);

	if (creator) {
		initd = USR_DIR + "/" + creator + "/initd";
	} else {
		initd = "/initd";
	}

	if (path != initd && !find_object(initd)) {
		error("Cannot compile " + path + " without " + initd);
	}

	if (find_object(path)) {
		upgrading = 1;
	}
}

void compile(string owner, object obj, string *source, string inherited ...)
{
	string creator;
	string path;
	object pinfo;
	int *inh;
	int sz;

	ACCESS_CHECK(previous_program() == DRIVER);

	path = object_name(obj);

	if (path != DRIVER) {
		inherited = ({ AUTO }) + inherited;
	}

	pinfo = setup_program_info(path, inherited);

	if (sscanf(path, "/kernel/%*s")) {
		return;
	}

	creator = DRIVER->creator(path);

	if (creator && path == USR_DIR + "/" + creator + "/_code") {
		call_out("destruct_object", 0, obj);

		return;
	}

	if (upgrading) {
		string patcher;
		string *patchers;

		upgrading = 0;

		if (function_object("upgrading", obj)) {
			catch {
				obj->upgrading();
			}
		}

		call_out("upgrade_object", 0, obj);

		patchers = pinfo->query_inherited_patchers();
		patcher = pinfo->query_patcher();

		if (patcher) {
			patchers |= ({ patcher });
		}

		if (sizeof(patchers)) {
			PATCHD->enqueue_patchers(obj, patchers);
		}
	} else if (sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s")) {
		pinfo->clear_clones();
		pinfo->set_clones_valid(1);
	}
}

void compile_lib(string owner, string path, string *source, string inherited ...)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	if (path != AUTO) {
		inherited = ({ AUTO }) + inherited;
	}

	setup_program_info(path, inherited);
}

void compile_failed(string owner, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	includes = nil;

	upgrading = 0;
}

void clone(string owner, object obj)
{
	object pinfo;

	ACCESS_CHECK(previous_program() == DRIVER);

	pinfo = fetch_program_info(status(obj, O_INDEX));

	pinfo->add_clone(obj);
}

void destruct(varargs mixed owner_arg, mixed obj_arg)
{
	if (previous_program() == DRIVER) {
		string owner;
		object obj;
		object pinfo;

		string path;

		owner = owner_arg;
		obj = obj_arg;

		pinfo = fetch_program_info(status(obj, O_INDEX));

		if (!pinfo) {
			return;
		}

		if (sscanf(object_name(obj), "%s#%*d", path)) {
			/* clone */
			pinfo->remove_clone(obj);
		} else {
			pinfo->set_destructed();
		}
	} else {
		ACCESS_CHECK(previous_program() == OBJECTD);

		LOGD->post_message("system", LOG_WARNING, "ObjectD destructing");
	}
}

void destruct_lib(string owner, string path)
{
	object pinfo;

	pinfo = fetch_program_info(status(path, O_INDEX));

	if (pinfo) {
		pinfo->set_destructed();
	}
}

void remove_program(string owner, string path, int timestamp, int index)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	if (progdb) {
		progdb->set_element(index, nil);
	}
}

mixed include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	path = query_include_file(compiled, from, path);

	if (file_info(path)) {
		includes |= ({ path });
		return path;
	}
}

int touch(varargs mixed obj_arg, mixed func_arg)
{
	if (previous_program() == DRIVER) {
		object obj;
		string func;

		obj = obj_arg;
		func = func_arg;

		if (sscanf(object_name(obj), "/kernel/%*s")) {
			return 0;
		}

		if (function_object("_F_touch", obj)) {
			return obj->_F_touch(func);
		}
	} else {
		/* this is our own touch handler */
		ACCESS_CHECK(SYSTEM());
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

	initd = find_object(initd_of(DRIVER->creator(path)));

	if (initd) {
		return initd->forbid_inherit(from, path, priv);
	} else if (!initd && DRIVER->creator(path) != "System") {
		error("No initd loaded for " + path + " (compiling " + from + ")");
	}
}

/* subroutines */

void nuke_object(object obj)
{
	ACCESS_CHECK(VERB() || SYSTEM());

	TLSD->set_tls_value("System", "destruct_force", obj);

	destruct_object(obj);
}

private void register_ghosts_dir(string dir)
{
	string *names;
	int *sizes;
	mixed *objs;
	mixed **lists;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	lists = get_dir(dir + "/*");
	names = lists[0];
	sizes = lists[1];
	objs = lists[3];

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];
		path = dir + "/" + name;

		if (sizes[sz] == -2) {
			register_ghosts_dir(path);
		} else {
			if (!sscanf(path, "%s.c", path)) {
				continue;
			}

			if (objs[sz]) {
				setup_ghost_program_info(path, status(path, O_INDEX));
			}
		}
	}
}

void register_ghosts()
{
	ACCESS_CHECK(SYSTEM());

	register_ghosts_dir("/");
}

void discover_objects()
{
	ACCESS_CHECK(SYSTEM());

	purge_dir("/");

	recompile_dir("/");
}

void discover_clones()
{
	string *owners;
	int sz;

	mixed **masters;
	mixed **clones;

	ACCESS_CHECK(SYSTEM());

	masters = ({ nil, nil });
	clones = ({ nil, nil });

	owners = KERNELD->query_owners();

	for (sz = sizeof(owners); --sz >= 0; ) {
		object first;
		string owner;

		owner = owners[sz];
		first = KERNELD->first_link(owner);

		if (first) {
			object cursor;

			cursor = first;

			do {
				string name;

				name = object_name(cursor);

				if (sscanf(name, "%*s#%*d")) {
					list_push_back(clones, cursor);
				} else if (sscanf(name, "%*s" + CLONABLE_SUBDIR + "%*s")) {
					list_push_back(masters, cursor);
				}

				cursor = KERNELD->next_link(cursor);
			} while (cursor != first);
		}
	}

	while (!list_empty(masters)) {
		object pinfo;
		object master;
		string path;
		int index;

		master = list_front(masters);
		list_pop_front(masters);

		path = object_name(master);
		index = status(master, O_INDEX);

		pinfo = fetch_program_info(index);

		if (!pinfo) {
			pinfo = setup_ghost_program_info(path, index);

			if (find_object(path)) {
				pinfo->set_destructed();
			}
		}

		pinfo->clear_clones();
		pinfo->set_clones_valid(1);
	}

	while (!list_empty(clones)) {
		object pinfo;
		object clone;
		int index;

		clone = list_front(clones);
		list_pop_front(clones);

		index = status(clone, O_INDEX);
		pinfo = fetch_program_info(index);

		if (!pinfo) {
			string path;
			mixed mindex;

			sscanf(object_name(clone), "%s#%*d", path);

			pinfo = setup_ghost_program_info(path, index);

			mindex = status(path, O_INDEX);

			if (mindex == nil || mindex != index) {
				pinfo->set_destructed();
			}
		}

		pinfo->add_clone(clone);
	}
}

object query_program_info(int index)
{
	return fetch_program_info(index);
}

mixed **query_program_indices()
{
	ASSERT(progdb);

	return progdb->query_indices();
}

void reset()
{
	ACCESS_CHECK(PRIVILEGED() || VERB());

	rlimits (0; 1000000000) {
		progdb = new_object(SPARSE_ARRAY);

		register_ghosts();
		discover_clones();
		discover_objects();
	}
}

void full_rebuild()
{
	ACCESS_CHECK(PRIVILEGED() || VERB());

	rlimits (0; 1000000000) {
		purge_dir("/");
		compile_dir("/");
	}
}

void call_touch(object obj)
{
	ACCESS_CHECK(SYSTEM());

	::call_touch(obj);
}

void do_upgrade(object obj)
{
	ACCESS_CHECK(SYSTEM());

	call_out("upgrade_object", 0, obj);
}
