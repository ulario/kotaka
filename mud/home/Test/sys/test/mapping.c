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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <type.h>
#include <status.h>

private object test_mapping_1()
{
	mapping arr;
	object map;
	int i;

	arr = ([ ]);

	map = new_object("/lwo/struct/mapping");
	map->set_type(T_INT);

	for (i = 1; i < 1 << 25; i *= 3) {
		map->set_element(i, i);

		ASSERT(map->query_element(i) == i);
	}

	return map;
}

private object test_mapping_2(int count)
{
	object map;
	int i;

	map = new_object("/lwo/struct/mapping");
	map->set_type(T_INT);

	for (i = 0; i < count; i++) {
		int a, b;

		a = random(1048576);
		b = random(1048576);

		map->set_element(a, b);

		ASSERT(map->query_element(a) == b);
	}

	return map;
}

static void test()
{
	mixed *mtime1, *mtime2;
	float diff;
	int count;
	int sec;
	mixed msec;
	object map;

	map = test_mapping_1();

	count = 1;

	for (;;) {
		mtime1 = millitime();

		map = test_mapping_2(count);

		mtime2 = millitime();

		diff = (float)(mtime2[0] - mtime1[0]) + (mtime2[1] - mtime1[1]);

		if (diff > 0.25) {
			break;
		}

		count <<= 2;
	}

	msec = (int)floor(diff * 1000.0 + 0.5);

	sec = msec / 1000;
	msec %= 1000;

	msec = "000" + msec;
	msec = msec[strlen(msec) - 3 ..];

	LOGD->post_message("debug", LOG_DEBUG, "Tested mapping");
	LOGD->post_message("system", LOG_NOTICE, "Big mapping benchmark, " + (int)((float)count / diff) + " ops/sec");
}

void schedule_test()
{
	ACCESS_CHECK(TEST());

	call_out("test", 0);
}
