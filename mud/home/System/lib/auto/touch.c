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

nomask int _F_touch(string func)
{
	object this;
	object pinfo;
	string patcher;
	string *patchers;
	int i, sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!find_object(PATCHD)) {
		return 0;
	}

	this = this_object();

	if (!PATCHD->query_marked(this)) {
		return 0;
	}

	pinfo = OBJECTD->query_program_info(status(this, O_INDEX));

	if (!pinfo) {
		LOGD->post_message("system", LOG_WARNING, "Attempted to patch " + object_name(this) + " without program_info");

		return 0;
	}

	patcher = pinfo->query_patcher();
	patchers = pinfo->query_inherited_patchers();

	if (patcher) {
		patchers |= ({ patcher });
	}

	PATCHD->clear_mark(this);

	for (sz = sizeof(patchers), i = 0; i < sz; i++) {
		catch {
			call_limited(patchers[i]);
		}
	}

	return 0;
}
