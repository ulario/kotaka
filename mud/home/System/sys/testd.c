/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2016  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kernel/rsrc.h>

inherit SECOND_AUTO;
inherit LIB_SYSTEM;

static void create()
{
}

void test()
{
	ACCESS_CHECK(TEST());

	call_out("report", 0);

	KERNELD->set_rsrc("test", -1, 0, 0);

	KERNELD->rsrc_incr("Test", "test", 0, random(5));
	KERNELD->rsrc_incr("Test", "test", 1, random(5));
	KERNELD->rsrc_incr("Test", "test", 1.0, random(5));
	KERNELD->rsrc_incr("Test", "test", "foo", random(5));
	KERNELD->rsrc_incr("Test", "test", ({ }), random(5));
	KERNELD->rsrc_incr("Test", "test", ([ ]), random(5));
}

static void report()
{
	LOGD->post_message("debug", LOG_DEBUG,
		mixed_sprint(KERNELD->rsrc_get("Test", "test"))
	);
	call_out("cleanup", 0);
}

static void cleanup()
{
	string *owners;
	int sz;

	owners = KERNELD->query_owners();

	for (sz = sizeof(owners); --sz > 0; ) {
		mixed *rsrc;
		string owner;

		owner = owners[sz];

		rsrc = KERNELD->rsrc_get(owner, "test");

		KERNELD->rsrc_incr(owner, "test", nil, -rsrc[RSRC_USAGE], TRUE);
	}

	KERNELD->remove_rsrc("test");
}
