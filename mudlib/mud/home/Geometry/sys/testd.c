/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/channel.h>
#include <kotaka/assert.h>

static void create()
{
	call_out("test", 0);
}

void test()
{
	object grid;
	string row;

	grid = new_object("~/lwo/grid");

	grid->set_size(28, 5);

	ASSERT(grid->query_row(0) == "\000\000\000\000");

	grid->set_cell(4, 4, 1);

	ASSERT(grid->query_row(0) == "\000\000\000\000");
	ASSERT(grid->query_row(4) == "\020\000\000\000");

	grid->set_row_bits(2, 5, 19, 1);

	/* 01234567 89012345 6789 */
	row = grid->query_row(2);

	ASSERT(grid->query_row(2) == "\340\377\017\000");
}
