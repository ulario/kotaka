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
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit "call_guard";
inherit "catalog";

nomask int _F_sys_create(int clone)
{
	string oname;
	string creator;

	object this;
	object programd;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	sscanf(oname, "%s#%*d", oname);

	creator = DRIVER->creator(oname);

	programd = find_object(PROGRAMD);

	if (programd) {
		object pinfo;
		string *ctors;
		string ctor;
		int i, sz;

		pinfo = PROGRAMD->query_program_info(
			status(this, O_INDEX)
		);

		if (pinfo) {
			ctors = pinfo->query_inherited_constructors();

			sz = sizeof(ctors);

			for (i = 0; i < sz; i++) {
				call_other(this, ctors[i]);
			}

			ctor = pinfo->query_constructor();

			if (ctor) {
				call_other(this, ctor);
			}
		}
	} else {
		ASSERT(creator == "System" || creator == "Bigstruct");
	}

	/* call higher-level creator function */
	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		create();
	} else {
		create(clone);
	}

	if (oname == USR_DIR + "/" + creator + "/initd") {
		INITD->add_subsystem(creator);
	}

	return 1;
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
