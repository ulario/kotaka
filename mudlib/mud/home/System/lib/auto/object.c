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
#include <kernel/kernel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit "call_guard";
inherit "catalog";

nomask void _F_sys_create(int clone)
{
	int index;
	int sz;
	object pinfo;
	object programd;
	string base;
	string ctor;
	string *ctors;
	string oname;

	ACCESS_CHECK(KERNEL());

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
	ctor = pinfo->query_constructor();

	sz = sizeof(ctors);

	for (index = 0; index < sz; index++) {
		call_other(this_object(), ctors[index]);
	}

	if (ctor) {
		call_other(this_object(), ctor);
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
	string dtor;
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

	dtor = pinfo->query_destructor();
	dtors = pinfo->query_inherited_destructors();

	if (dtor) {
		call_other(this_object(), dtor);
	}

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
