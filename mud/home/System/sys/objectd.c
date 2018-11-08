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

void upgrade()
{
	ACCESS_CHECK(SYSTEM());
}

/* private */

private mixed query_include_file(string compiled, string from, string path)
{
	string creator;
	object initd;

	if (path == "/include/std.h") {
		return path;
	}

	creator = find_object(DRIVER)->creator(compiled);

	if (creator == "System") {
		return path;
	}

	if (path == "/include/AUTO"
		&& from == "/include/std.h"
		&& compiled != USR_DIR + "/admin/_code") {
		return USR_DIR + "/System/include/second_auto.h";
	}

	return path;
}

private void compile_common(string owner, string path, string *source, string *inherited)
{
	object initd;
	object pinfo;

	if (find_object(PROGRAMD)) {
		pinfo = PROGRAMD->register_program(path, inherited, includes);
	}

	/* we don't get to tell kernel objects what they inherit */
	if (sscanf(path, "/kernel/%*s")) {
		return;
	}

	/* everyting outside auto must inherit it */
	if (!is_auto && !sizeof(({ SECOND_AUTO }) & inherited)) {
		error("Failure to inherit SECOND_AUTO: " + path);
	}

	if (is_initd) {
		return;
	}

	if (pinfo) {
		initd = find_object(initd_of(owner));
	}

	if (initd) {
		pinfo->set_constructor(initd->query_constructor(path));
		pinfo->set_destructor(initd->query_destructor(path));
		pinfo->set_patcher(initd->query_patcher(path));
	}

	includes = nil;
}

private void set_flags(string path)
{
	string creator;

	is_kernel = sscanf(path, "/kernel/%*s");
	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");

	creator = DRIVER->creator(path);

	is_initd = (path == initd_of(creator));
}

/* program management */

/* klib hooks */

void gather_inherits(mapping map, int oindex)
{
	int *inh;
	int sz;
	object pinfo;

	pinfo = PROGRAMD->query_program_info(oindex);
	inh = pinfo->query_inherits();

	if (!inh) {
		LOGD->post_message("system", LOG_WARNING, "Attempted to gather inherits for ghost program " + pinfo->query_path());
		return;
	}

	for (sz = sizeof(inh) - 1; sz >= 0; --sz) {
		int i;

		i = inh[sz];

		if (!map[i]) {
			map[i] = 1;

			gather_inherits(map, i);
		}
	}
}

atomic void compiling(string path)
{
	ACCESS_CHECK(KERNEL());

	includes = ({ "/include/std.h" });

	if (sscanf(path, USR_DIR + "/%*s/_code")) {
		/* klib wiztool "code" command, ignore */
		return;
	}

	if (!find_object(PROGRAMD)) {
		switch(DRIVER->creator(path)) {
		case "System":
		case "Bigstruct":
			break;

		default:
			error("Cannot compile " + path + " without ProgramD");
		}
	}

	if (path == "/initd" || sscanf(path, USR_DIR + "/%*s/initd")) {
		/* this is an initd */
	} else {
		string creator;
		string initd;

		creator = DRIVER->creator(path);

		if (creator) {
			initd = USR_DIR + "/" + creator + "/initd";
		} else {
			initd = "/initd";
		}

		if (!find_object(initd)) {
			error("Cannot compile " + path + " without " + initd);
		}
	}

	if (find_object(path)) {
		upgrading = 1;
	}
}

void do_upgrade(object obj)
{
	ACCESS_CHECK(SYSTEM());

	obj->upgrade();
}

atomic void compile(string owner, object obj, string *source, string inherited ...)
{
	string path;
	string err;
	int index;

	ACCESS_CHECK(KERNEL());

	path = object_name(obj);

	set_flags(path);

	if (path != DRIVER) {
		inherited |= ({ AUTO });
	}

	if (sscanf(path, USR_DIR + "/%*s/_code")) {
		/* klib wiztool "code" command, ignore */
		return;
	}

	index = status(obj, O_INDEX);

	compile_common(owner, path, source, inherited);

	if (is_initd) {
		if (!sizeof(({ LIB_INITD }) & inherited)) {
			error("Failure to inherit LIB_INITD: " + path);
		}
	}

	if (upgrading) {
		upgrading = 0;

		if (!is_kernel) {
			if (find_object(PROGRAMD)) {
				int *programs;
				int sz;
				string *patchers;
				mapping inherits;

				inherits = ([ ]);
				gather_inherits(inherits, index);
				programs = map_indices(inherits) | ({ index });

				patchers = ({ });

				for (sz = sizeof(programs) - 1; sz >= 0; --sz) {
					object pinfo;
					string patcher;
					int program;

					program = programs[sz];

					pinfo = PROGRAMD->query_program_info(program);
					patcher = pinfo->query_patcher();

					if (patcher) {
						patchers |= ({ patcher });
					}
				}

				if (sizeof(patchers)) {
					PATCHD->enqueue_patchers(obj, patchers);
				}
			}

			call_out("do_upgrade", 0, obj);

			catch {
				obj->upgrading();
			}
		}
	}
}

atomic void compile_lib(string owner, string path, string *source, string inherited ...)
{
	string ctor;
	string dtor;
	object initd;

	ACCESS_CHECK(KERNEL());

	if (path != AUTO) {
		inherited |= ({ AUTO });
	}

	set_flags(path);

	compile_common(owner, path, source, inherited);
}

atomic void compile_failed(string owner, string path)
{
	ACCESS_CHECK(KERNEL());

	upgrading = 0;
	includes = nil;
}

atomic void clone(string owner, object obj)
{
	ACCESS_CHECK(KERNEL());
}

atomic void destruct(varargs mixed owner, mixed obj)
{
	int is_clone;
	int is_initd;
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

	set_flags(name);
	is_clone = sscanf(name, "%s#%*d", path);

	if (!path) {
		path = name;
	}

	if (!is_kernel) {
		object force;

		force = TLSD->query_tls_value("System", "destruct_force");

		if (obj != force) {
			obj->_F_sys_destruct();
		}
	}

	if (!is_clone) {
		if (find_object(PROGRAMD)) {
			pinfo = PROGRAMD->query_program_info(status(obj, O_INDEX));
		}

		if (pinfo) {
			pinfo->set_destructed();
		}
	}
}

atomic void destruct_lib(string owner, string path)
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

atomic void remove_program(string owner, string path, int timestamp, int index)
{
	ACCESS_CHECK(KERNEL());

	if (find_object(PROGRAMD)) {
		PROGRAMD->remove_program(index);
	}
}

atomic mixed include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	path = query_include_file(compiled, from, path);

	if (file_info(path)) {
		includes |= ({ path });
		return path;
	}
}

atomic int touch(varargs mixed obj, mixed func)
{
	if (previous_program() == DRIVER) {
		ASSERT(typeof(obj) == T_OBJECT);
		ASSERT(typeof(func) == T_STRING);

		if (!sscanf(object_name(obj), "/kernel/%*s")) {
			return obj->_F_touch(func);
		}
	} else if (sscanf(previous_program(), USR_DIR
		+ "/System" + INHERITABLE_SUBDIR + "auto/%*s")) {
		/* this is our own touch handler */
		/* obj is func */
	} else {
		error("Access denied");
	}
}

atomic int forbid_call(string path)
{
	ACCESS_CHECK(KERNEL());
}

atomic int forbid_inherit(string from, string path, int priv)
{
	object initd;

	ACCESS_CHECK(KERNEL());

	initd = find_object(initd_of(DRIVER->creator(path)));

	if (initd) {
		return initd->forbid_inherit(from, path, priv);
	} else if (!initd && DRIVER->creator(path) != "System") {
		error("No initd loaded for " + path);
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

	lists = get_dir(dir + (dir == "/" ? "*" : "/*"));
	names = lists[0];
	sizes = lists[1];
	objs = lists[3];

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];
		path = (dir == "/" ? "" : "/") + name;

		if (sizes[sz] == -2) {
			register_ghosts_dir(path);
		} else {
			if (!sscanf(path, "%*s.c", path)) {
				continue;
			}

			if (objs[sz]) {
				LOGD->post_message("system", LOG_NOTICE, "Registering ghost program " + path);
				PROGRAMD->register_program(path, nil, nil);
			}
		}
	}
}

void register_ghosts()
{
	ACCESS_CHECK(previous_program() == INITD);

	register_ghosts_dir("/");
}
