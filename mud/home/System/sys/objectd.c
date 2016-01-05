/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/utility.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

string compiling;	/* path of object we are currently compiling */
string *includes;	/* include files of currently compiling object */
int upgrading;		/* are we upgrading or making a new compile? */
mapping old_inherits;	/* list of old inherits */
int is_initd;		/* current compilation is for an initd */
int is_kernel;		/* current compilation is for a kernel object */
int is_auto;		/* current compilation is for a second auto support library */
object upgrades;	/* list of objects needing upgrade */

static void create()
{
	DRIVER->set_object_manager(this_object());
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

/* private */

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

	return path;
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
		initd = find_object(MODULED->initd_of(owner));
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

private void set_flags(string path)
{
	string creator;

	is_kernel = sscanf(path, "/kernel/%*s");
	is_auto = sscanf(path, USR_DIR + "/System"
		+ INHERITABLE_SUBDIR + "auto/%*s");

	creator = DRIVER->creator(path);

	is_initd = (path == MODULED->initd_of(creator));
}

void upgrade_objects()
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	catch {
		string path;
		string *patches;

		({ path, patches }) = upgrades->query_back();
		upgrades->pop_back();

		if (sizeof(patches)) {
			TOUCHD->add_patches(path, patches);
		}

		catch {
			path->upgrade();
		}

		if (upgrades->empty()) {
			upgrades = nil;
		} else {
			SUSPENDD->queue_work("upgrade_objects");
		}
	} : {
		upgrades = nil;
	}
}

/* program management */

/* klib hooks */

void gather_inherits(mapping map, int oindex)
{
	int *inh;
	int sz;
	object pinfo;

	pinfo = PROGRAMD->query_program_info(oindex);

	if (!pinfo) {
		return;
	}

	inh = pinfo->query_inherits();

	for (sz = sizeof(inh) - 1; sz >= 0; --sz) {
		int i;

		i = inh[sz];

		if (!map[i]) {
			map[i] = 1;

			gather_inherits(map, i);
		}
	}
}

void compiling(string path)
{
	ACCESS_CHECK(KERNEL());

	includes = ({ "/include/std.h" });

	old_inherits = ([ ]);

	if (!find_object(PROGRAMD)) {
		switch(DRIVER->creator(path)) {
		case "System":
		case "Bigstruct":
			break;

		default:
			error("Not allowed to compile " + path + " without ProgramD loaded.");
		}
	}

	if (find_object(path)) {
		if (find_object(PROGRAMD)) {
			gather_inherits(old_inherits, status(path, O_INDEX));
		}

		upgrading = 1;
	}
}

void compile(string owner, object obj, string *source, string inherited ...)
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
			int *new_programs;
			int sz;
			string *patches;

			mapping new_inherits;

			new_inherits = ([ ]);

			if (find_object(PROGRAMD)) {
				gather_inherits(new_inherits, index);
			}

			new_programs = map_indices(new_inherits) - map_indices(old_inherits);
			new_programs |= ({ index });

			patches = ({ });

			for (sz = sizeof(new_programs) - 1; sz >= 0; --sz) {
				object pinfo;
				string toucher;

				if (find_object(PROGRAMD)) {
					pinfo = PROGRAMD->query_program_info(new_programs[sz]);
				}

				if (!pinfo) {
					continue;
				}

				toucher = pinfo->query_toucher();

				if (toucher) {
					patches |= ({ toucher });
				}
			}

			if (!upgrades) {
				SUSPENDD->suspend_system();
				SUSPENDD->queue_work("upgrade_objects");

				upgrades = new_object(BIGSTRUCT_DEQUE_LWO);
				upgrades->claim();
			}

			upgrades->push_back( ({ path, patches }) );

			catch {
				obj->upgrading();
			}
		}
	} else {
		old_inherits = nil;
	}
}

void compile_lib(string owner, string path, string *source, string inherited ...)
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

void compile_failed(string owner, string path)
{
	ACCESS_CHECK(KERNEL());

	upgrading = 0;
	includes = nil;
	old_inherits = nil;
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

	if (is_initd) {
		if (sizeof(MODULED->query_modules() & ({ owner }))) {
			error("Module must be shutdown before initd can be destroyed");
		}
	}

	if (!path) {
		path = name;
	}

	if (!is_kernel) {
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

int touch(varargs object obj, string func)
{
	if (previous_program() == DRIVER) {
		if (!sscanf(object_name(obj), "/kernel/%*s")) {
			obj->_F_touch(func);
		}
	} else if (sscanf(previous_program(), USR_DIR
		+ "/System" + INHERITABLE_SUBDIR + "auto/%*s")) {
	} else {
		error("Access denied");
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

	initd = find_object(MODULED->initd_of(DRIVER->creator(path)));

	if (initd) {
		return initd->forbid_inherit(from, path, priv);
	} else if (!initd && DRIVER->creator(path) != "System") {
		error("No initd loaded for " + path);
	}
}
