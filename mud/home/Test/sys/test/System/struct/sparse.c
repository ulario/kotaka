/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020  Raymond Jennings
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

void test()
{
	object sparse;
	int i;

	ACCESS_CHECK(TEST());

	sparse = new_object("~System/lwo/struct/sparse_array");

	rlimits (0; 1000000) {
		for (i = 2; i < (1 << 28); i = (i * 3) / 2) {
			sparse->set_element(i, i);
			ASSERT(sparse->query_element(i) == i);
		}

		for (i = 2; i < (1 << 28); i = (i * 3) / 2) {
			ASSERT(sparse->query_element(i) == i);
			sparse->set_element(i, nil);
			ASSERT(sparse->query_element(i) == nil);
		}
	}

	LOGD->post_message("debug", LOG_DEBUG, "Tested sparse array");
}
