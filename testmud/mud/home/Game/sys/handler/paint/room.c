/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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

void paint_text(object obj, object viewer, object painter)
{
	float dx, dy;
	int xlow, ylow;
	int xhigh, yhigh;

	int row;

	ASSERT(obj->query_environment() == viewer->query_environment());

	dx = obj->query_x_position() - viewer->query_x_position();
	dy = obj->query_y_position() - viewer->query_y_position();

	xlow = (int)dx;
	ylow = (int)dy;

	painter->set_color(0x03);

	for (row = ylow; row < ylow + 4; row++) {
		painter->move_pen(xlow + 61, row + 1);
		painter->draw("::::");
	}
}
