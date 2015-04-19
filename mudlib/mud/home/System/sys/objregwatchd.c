/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kernel/rsrc.h>
#include <status.h>

inherit SECOND_AUTO;

mapping cursors;
mapping scores;

static void create()
{
	cursors = ([ ]);
	scores = ([ ]);

	call_out("tick", 0);
}

static void check(string owner)
{
	object prev, next;
	object obj;

	obj = cursors[owner];

	if (!obj) {
		obj = KERNELD->first_link(owner);

		if (!obj) {
			return;
		}
	}

	prev = KERNELD->prev_link(obj);
	next = KERNELD->next_link(obj);

	if (!prev || !next) {
		LOGD->post_message("system", LOG_EMERG, "Fatal error: ObjRegD corruption for " + owner);
		KERNELD->shutdown();
		error("internal error");
	}

	if (KERNELD->next_link(prev) != obj) {
		LOGD->post_message("system", LOG_EMERG, "Fatal error: ObjRegD corruption for " + owner);
		KERNELD->shutdown();
		error("internal error");
	}

	if (KERNELD->prev_link(next) != obj) {
		LOGD->post_message("system", LOG_EMERG, "Fatal error: ObjRegD corruption for " + owner);
		KERNELD->shutdown();
		error("internal error");
	}

	cursors[owner] = next;
}

static void tick()
{
	string *owners;
	int sz;

	call_out("tick", 1.0);

	owners = KERNELD->query_owners();
	cursors &= owners;
	scores &= owners;

	for (sz = sizeof(owners); --sz >= 0; ) {
		string owner;

		owner = owners[sz];

		if (!scores[owner]) {
			scores[owner] = 0.0;
		}

		scores[owner] +=
			(float)KERNELD->rsrc_get(owner, "objects")[RSRC_USAGE]
			/ (float)status(ST_NOBJECTS);

		while (scores[owner] > 1.0) {
			scores[owner] -= 1.0;
			call_out("check", 0, owner);
		}
	}
}
