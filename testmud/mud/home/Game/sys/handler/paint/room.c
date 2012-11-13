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

void paint_text(object gc, object obj, object viewer)
{
	int x, y;

	int row;

	if (viewer->query_environment() == obj) {
		x = -(int)viewer->query_x_position() - 1;
		y = -(int)viewer->query_y_position() - 1;

		gc->set_color(0x03);

		gc->move_pen(x, y);
		gc->draw("+----+");
		gc->move_pen(x, y + 1);
		gc->draw("|    |");
		gc->move_pen(x, y + 2);
		gc->draw("|    |");
		gc->move_pen(x, y + 3);
		gc->draw("|    |");
		gc->move_pen(x, y + 4);
		gc->draw("|    |");
		gc->move_pen(x, y + 5);
		gc->draw("+----+");
	} else {
		float dx, dy;

		dx = obj->query_x_position() - viewer->query_x_position();
		dy = obj->query_y_position() - viewer->query_y_position();

		if (fabs(dx) > 10.0 || fabs(dy) > 10.0) {
			/* out of bounds */
			return;
		}

		x = (int)dx;
		y = (int)dy;

		gc->set_color(0x30);

		gc->move_pen(x, y);
		gc->draw("+--+");
		gc->move_pen(x, y + 1);
		gc->draw("|  |");
		gc->move_pen(x, y + 2);
		gc->draw("|  |");
		gc->move_pen(x, y + 3);
		gc->draw("+--+");
	}
}
