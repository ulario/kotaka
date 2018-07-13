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
#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>
#include <type.h>

inherit "~System/lib/struct/multimap";

private void test_mapping()
{
	mapping arr;
	object map;
	int i;

	arr = ([ ]);

	map = new_object("~/lwo/mapping");
	map->set_type(T_INT);

	for (i = 1; i < 1 << 25; i *= 3) {
		map->set_element(i, i);
		ASSERT(map->query_element(i) == i);
	}

	map->set_type(T_INT);

	for (i = 5000; --i >= 0; ) {
		int j, k;

		j = random(1000000);
		k = random(1000000);

		set_multimap(arr, j, k);
		map->set_element(j, k);

		ASSERT(map->query_element(j) == k);
	}

	LOGD->post_message("system", LOG_NOTICE, "Dump of map: " + STRINGD->hybrid_sprint(map->query_root()));
}

void test()
{
	ACCESS_CHECK(TEST());

	test_mapping();
}
