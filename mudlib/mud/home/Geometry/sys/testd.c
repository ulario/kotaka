/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2014  Raymond Jennings
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
#include <kotaka/assert.h>

static void create()
{
	call_out("test", 0);
}

void kaboom()
{
	destruct_object(this_object());
}

void test()
{
	object grid;
	string row;

	call_out("kaboom", 0);

	grid = new_object("~/lwo/grid");

	grid->set_size(28, 5);

	ASSERT(grid->query_row(0) == "\000\000\000\000");

	grid->set_cell(4, 4, 1);

	ASSERT(grid->query_row(0) == "\000\000\000\000");
	ASSERT(grid->query_row(4) == "\020\000\000\000");

	grid->set_row_bits(2, 5, 19, 1);

	/* 01234567 89012345 6789 */
	row = grid->query_row(2);

	CHANNELD->post_message("debug", nil, "Row 0: " + row[0] + ", should be " + '\340');
	CHANNELD->post_message("debug", nil, "Row 1: " + row[1] + ", should be " + '\377');
	CHANNELD->post_message("debug", nil, "Row 2: " + row[2] + ", should be " + '\017');
	ASSERT(grid->query_row(2) == "\340\377\017\000");
}

void upgrading()
{
	call_out("test", 0);
}
