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
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <type.h>
#include <status.h>

inherit "/lib/string/sprint";

private void test_mapping_1()
{
	mapping arr;
	object map;
	int i;

	arr = ([ ]);

	map = new_object("~/lwo/mapping");
	map->set_type(T_INT);

	for (i = 1; i < 1 << 25; i *= 3) {
		LOGD->post_message("system", LOG_NOTICE, "Test 1: setting element " + i);

		map->set_element(i, i);

		ASSERT(map->query_element(i) == i);
	}

	for (i = 1; i < 1 << 25; i *= 3) {
		LOGD->post_message("system", LOG_NOTICE, "Test 1: clearing element " + i);

		map->set_element(i, nil);

		ASSERT(map->query_element(i) == nil);
	}
}

private void test_mapping_2()
{
	mapping arr;
	object map;
	int i;
	mapping times;

	arr = ([ ]);

	map = new_object("~/lwo/mapping");
	map->set_type(T_INT);

	times = ([ ]);

	for (i = 0; i < status(ST_ARRAYSIZE); i++) {
		mixed *mtime1, *mtime2;
		mixed diff;

		LOGD->post_message("system", LOG_NOTICE, "Test 2: setting element " + i);


		mtime1 = millitime();

		map->set_element(i, i);

		mtime2 = millitime();

		diff = mtime2[0] - mtime1[0];
		diff = (float)diff + mtime2[1] - mtime1[1];

		times[i] = diff;

		ASSERT(map->query_element(i) == i);
	}

	for (i = 1; i < status(ST_ARRAYSIZE); i++) {
		LOGD->post_message("system", LOG_NOTICE, "Test 2: clearing element " + i);

		map->set_element(i, nil);

		ASSERT(map->query_element(i) == nil);
	}

	LOGD->post_message("system", LOG_NOTICE, "Profiling data: " + hybrid_sprint(times));
}

void test()
{
	mixed *mtime1, *mtime2;

	ACCESS_CHECK(TEST());

	mtime1 = millitime();

	test_mapping_1();
	test_mapping_2();

	mtime2 = millitime();
}
