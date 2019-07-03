/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit "~System/lib/struct/multimap";

static void test()
{
	mapping map;
	int i;

	map = ([ ]);

	rlimits (0; 1000000) {
		for (i = 2; i < (1 << 28); i = (i * 3) / 2) {
			set_multimap(map, i, i);
			ASSERT(query_multimap(map, i) == i);
		}

		for (i = 2; i < (1 << 28); i = (i * 3) / 2) {
			ASSERT(query_multimap(map, i) == i);
			set_multimap(map, i, nil);
			ASSERT(query_multimap(map, i) == nil);
		}
	}

	compact_multimap(map);

	ASSERT(map_sizeof(map) == 0);

	LOGD->post_message("debug", LOG_DEBUG, "Tested multimap");
}

void schedule_test()
{
	ACCESS_CHECK(TEST());

	call_out("test", 0);
}
