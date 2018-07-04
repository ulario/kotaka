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
#include <kotaka/paths/system.h>
#include <kotaka/paths/string.h>
#include <kotaka/log.h>

inherit "~System/lib/struct/multimap";
inherit "~System/lib/struct/maparr";
inherit "~System/lib/struct/list";

private void test_maparr()
{
	mapping map;
	int i;

	for (i = 1; i < (1 << 30); i *= 3) {
		map = set_multilevel_map_arr(map, 3, i, i);

		LOGD->post_message("system", LOG_WARNING, STRINGD->mixed_sprint(map));
	}

	for (i = 1; i < (1 << 30); i *= 3) {
		ASSERT(query_multilevel_map_arr(map, 3, i) == i);
		map = set_multilevel_map_arr(map, 3, i, nil);

		LOGD->post_message("system", LOG_WARNING, STRINGD->mixed_sprint(map));
	}
}

private void test_multimap()
{
	mapping map;
	int i;

	map = ([ ]);

	for (i = 1; i < (1 << 30); i *= 3) {
		set_multimap(map, i, i);

		LOGD->post_message("system", LOG_WARNING, STRINGD->mixed_sprint(map));
	}

	for (i = 1; i < (1 << 30); i *= 3) {
		ASSERT(query_multimap(map, i) == i);
		set_multimap(map, i, nil);

		LOGD->post_message("system", LOG_WARNING, STRINGD->mixed_sprint(map));
	}
}

void test()
{
	ACCESS_CHECK(TEST());

	test_maparr();
	test_multimap();
}
