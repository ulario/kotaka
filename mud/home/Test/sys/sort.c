/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

#include <kotaka/privilege.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>

inherit "/lib/sort";

private void test_qsort()
{
	int *sortme;
	int i;

	sortme = allocate(500);

	for (i = 0; i < 500; i++) {
		sortme[i] = random(500);
	}

	qsort(sortme, 0, 500);

	for (i = 0; i < 499; i++) {
		ASSERT(sortme[i] <= sortme[i + 1]);
	}
}

void test()
{
	ACCESS_CHECK(TEST());

	LOGD->post_message("debug", LOG_DEBUG, "Starting sort test...");

	LOGD->post_message("debug", LOG_DEBUG, "Testing qsort...");
	test_qsort();
}
