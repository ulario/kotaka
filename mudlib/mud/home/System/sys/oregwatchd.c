/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kernel/tls.h>

#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <kotaka/bigstruct.h>

#include <type.h>
#include <status.h>

inherit SECOND_AUTO;

static void create()
{
	call_out("verify_objregd", 1);
}

private void verify_objregd_owner(string owner)
{
	mapping seen;
	object first;
	object obj;

	seen = ([ ]);
	first = KERNELD->first_link(owner);

	if (!first) {
		return;
	}

	obj = first;

	do {
		seen[obj] = 1;
		obj = KERNELD->next_link(obj);

		if (!obj) {
			shutdown();
			LOGD->post_message("system", LOG_EMERG, "Fatal error:  Corrupted ObjRegD database for " + owner);
			break;
		}
	} while (!seen[obj]);
}

static void verify_objregd()
{
	rlimits (0; -1) {
		string *owners;
		int i, sz;

		owners = KERNELD->query_owners();

		sz = sizeof(owners);

		for (i = 0; i < sz; i++) {
			verify_objregd_owner(owners[i]);
		}

		call_out("verify_objregd", 1);
	}
}
