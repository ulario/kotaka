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
#include <kernel/kernel.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit "call_guard";
inherit "catalog";

static void create(varargs int clone)
{
}

nomask int _F_sys_create(int clone)
{
	string oname;
	string creator;

	object this;
	object objectd;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	sscanf(oname, "%s#%*d", oname);

	creator = DRIVER->creator(oname);

	objectd = find_object(OBJECTD);

	if (objectd) {
		object pinfo;

		pinfo = OBJECTD->query_program_info(status(this, O_INDEX));

		if (pinfo) {
			string *ctors;
			int i, sz;

			ctors = pinfo->query_constructors();
			sz = sizeof(ctors);

			for (i = 0; i < sz; i++) {
				call_other(this, ctors[i]);
			}
		}
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
	object objectd;

	int clone;
	int oindex;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	clone = !!sscanf(oname, "%*s#");

	if (!sscanf(oname, "%s#%d", oname, oindex)) {
		oindex = status(this, O_INDEX);
	}

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		destruct();
	} else {
		destruct(clone);
	}

	creator = DRIVER->creator(oname);

	objectd = find_object(OBJECTD);

	if (objectd) {
		object pinfo;

		pinfo = OBJECTD->query_program_info(
			status(this, O_INDEX)
		);

		if (pinfo) {
			string *dtors;
			int i, sz;

			dtors = pinfo->query_inherited_destructors();

			sz = sizeof(dtors);

			for (i = sz - 1; i >= 0; i--) {
				call_other(this, dtors[i]);
			}
		}
	}

	set_object_name(nil);
	PATCHD->clear_patch(this_object());
}

void upgrading()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}
