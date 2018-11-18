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
inherit "~System/lib/struct/list";

private void test_multimap()
{
	mapping map;
	int i;

	map = ([ ]);

	for (i = 1; i < (1 << 30); i *= 3) {
		set_multimap(map, i, i);
	}

	for (i = 1; i < (1 << 30); i *= 3) {
		ASSERT(query_multimap(map, i) == i);
		set_multimap(map, i, nil);
		ASSERT(query_multimap(map, i) == nil);
	}

	compact_multimap(map);

	ASSERT(map_sizeof(map) == 0);
}

private void test_sparse_array()
{
	object sparse;
	int i;

	sparse = new_object("~System/lwo/struct/sparse_array");

	for (i = 1; i < (1 << 30); i *= 3) {
		sparse->set_element(i, i);
	}

	for (i = 1; i < (1 << 30); i *= 3) {
		ASSERT(sparse->query_element(i) == i);
		sparse->set_element(i, i);
		ASSERT(sparse->query_element(i) == nil);
	}

}

void test()
{
	ACCESS_CHECK(TEST());

	test_multimap();
}
