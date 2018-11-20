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
	object this;
	object objectd;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	sscanf(oname, "%s#%*d", oname);

	objectd = find_object(OBJECTD);

	if (objectd) {
		object pinfo;

		pinfo = OBJECTD->query_program_info(status(this, O_INDEX));

		if (pinfo) {
			string ctor;
			mixed *ctors;
			int i, sz;

			ctor = pinfo->query_constructor();
			ctors = pinfo->query_inherited_constructors();

			if (ctors) {
				sz = sizeof(ctors);

				for (i = 0; i < sz; i++) {
					call_limited(ctors[i]);
				}
			}

			if (ctor) {
				call_limited(ctor);
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
	object this;
	object objectd;

	int clone;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	clone = !!sscanf(oname, "%*s#");

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		destruct();
	} else {
		destruct(clone);
	}

	objectd = find_object(OBJECTD);

	if (objectd) {
		object pinfo;

		pinfo = objectd->query_program_info(
			status(this, O_INDEX)
		);

		if (pinfo) {
			string dtor;
			string *dtors;
			int i, sz;

			dtor = pinfo->query_destructor();
			dtors = pinfo->query_inherited_destructors();

			if (dtor) {
				call_limited(dtor);
			}

			if (dtors) {
				sz = sizeof(dtors);

				for (i = sz - 1; i >= 0; i--) {
					call_limited(dtors[i]);
				}
			}
		}
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
