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

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		create();
	} else {
		create(clone);
	}

	return 1;
}

static void destruct(varargs int clone)
{
}

nomask void _F_sys_destruct()
{
	string oname;
	string creator;

	object this;
	object programd;

	int clone;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	clone = !!sscanf(oname, "%*s#");

	sscanf(oname, "%s#%*d", oname);

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		destruct();
	} else {
		destruct(clone);
	}

	creator = DRIVER->creator(oname);

	programd = find_object(PROGRAMD);

	if (programd) {
		object pinfo;
		string *dtors;
		string dtor;
		int i, sz;

		pinfo = PROGRAMD->query_program_info(
			status(this, O_INDEX)
		);

		if (pinfo) {
			dtor = pinfo->query_destructor();

			if (dtor) {
				call_other(this, dtor);
			}

			dtors = pinfo->query_inherited_destructors();

			for (sz = sizeof(dtors) - 1; sz >= 0; --sz) {
				call_other(this, dtors[i]);
			}
		}
	} else {
		ASSERT(creator == "System" || creator == "Bigstruct");
	}

	set_object_name(nil);
}

void upgrading()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}
