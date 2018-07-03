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

static void call_touch(object obj)
{
	::call_touch(obj);
}

static int touch(string func)
{
	return 0;
}

nomask int _F_touch(string func)
{
	object this;
	string name;
	string *patchers;
	int oindex;
	int sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	this = this_object();
	name = object_name(this);

	if (!sscanf(name, "%*s#%d", oindex)) {
		oindex = status(this, O_INDEX);
	}

	patchers = PATCHD->query_patchers(this);

	if (patchers) {
		PATCHD->clear_patch(this);

		for (sz = sizeof(patchers) - 1; sz >= 0; --sz) {
			string user;
			string prog;
			string patcher;

			patcher = patchers[sz];

			rlimits(INT_MAX / 2; -1) {
				int stack;
				int ticks;

				catch {
					call_limited(patcher);
				}
			}
		}
	}

	return touch(func);
}

void system_patcher()
{
	LOGD->post_message("system", LOG_DEBUG, "System patcher called for " + object_name(this_object()));
}
