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
#include <config.h>
#include <kotaka/privilege.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>
#include <status.h>
#include <limits.h>

inherit "call_guard";

static int touch(string func)
{
	return 0;
}

nomask int _F_touch(string func)
{
	object this;
	string name;
	int oindex;
	int sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	this = this_object();
	name = object_name(this);

	if (!sscanf(name, "%*s#%d", oindex)) {
		oindex = status(this, O_INDEX);
	}

	if (find_object(PATCHD)) {
		string *patchers;

		patchers = PATCHD->query_patchers(this);

		if (patchers) {
			int i, sz;

			PATCHD->clear_patch(this);

			for (sz = sizeof(patchers), i = 0; i < sz; i++) {
				catch {
					call_limited(patchers[i]);
				}
			}
		}
	}

	return touch(func);
}

static void call_touch(object obj)
{
	if (SYSTEM()) {
		::call_touch(obj);
	} else {
		OBJECTD->call_touch(obj);
	}
}
