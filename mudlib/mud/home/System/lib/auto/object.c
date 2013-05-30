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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit "call_guard";
inherit "catalog";

static int free_objects()
{
	return status(ST_OTABSIZE) - status(ST_NOBJECTS);
}

nomask void _F_sys_create(int clone)
{
	int index;
	int sz;
	object pinfo;
	object programd;
	string base;
	string *ctors;
	string oname;

	oname = object_name(this_object());
	base = oname;
	sscanf(base, "%s#%*d", base);

	programd = find_object(PROGRAMD);

	if (!programd) {
		return;
	}

	if (DRIVER->creator(base) == "System") {
		return;
	}

	if (DRIVER->creator(base) == "Bigstruct") {
		return;
	}

	pinfo = programd->query_program_info(status(this_object(), O_INDEX));

	if (!pinfo) {
		return;
	}

	ctors = pinfo->query_inherited_constructors();
	sz = sizeof(ctors);

	for (index = 0; index < sz; index++) {
		call_other(this_object(), ctors[index]);
	}
}

static void destruct(varargs int clone)
{
}

nomask void _F_sys_destruct()
{
	int clone;
	int index;
	int sz;
	object pinfo;
	object programd;
	string base;
	string *dtors;
	string oname;

	ACCESS_CHECK(previous_program() == OBJECTD);

	oname = object_name(this_object());
	base = oname;
	sscanf(base, "%s#%*d", base);
	clone = !!sscanf(oname, "%*s#");

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		destruct();
	} else {
		destruct(clone);
	}

	if (DRIVER->creator(base) == "System") {
		return;
	}

	if (DRIVER->creator(base) == "Bigstruct") {
		return;
	}

	programd = find_object(PROGRAMD);

	if (!programd) {
		set_object_name(nil);

		return;
	}

	pinfo = programd->query_program_info(status(this_object(), O_INDEX));

	if (!pinfo) {
		set_object_name(nil);

		return;
	}

	dtors = pinfo->query_inherited_destructors();

	sz = sizeof(dtors);

	for (index = sz - 1; index >= 0; index--) {
		call_other(this_object(), dtors[index]);
	}

	set_object_name(nil);
}

void upgrading()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

void upgrade_failed()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

atomic static object compile_object(mixed args ...)
{
	object obj;

	obj = find_object(args[0]);

	if (!SYSTEM() && DRIVER->creator(args[0]) != "System" && !obj && free_objects() < 50) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

atomic static object load_object(mixed args ...)
{
	object obj;

	obj = find_object(args[0]);

	if (obj) {
		return obj;
	}

	if (!SYSTEM() && DRIVER->creator(args[0]) != "System" && !obj && free_objects() < 50) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object clone_object(mixed args ...)
{
	if (!SYSTEM() && query_owner() != "System" && free_objects() < 100) {
		error("Too many objects");
	}

	return ::clone_object(args ...);
}
