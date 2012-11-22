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

#include <game/paths.h>

void on_paint_text(object gc, object obj, object viewer)
{
	float dx, dy, dz;
	int x, y;
	float mass;

	int row;

	({ dx, dy, dz }) = GAME_SUBD->query_position_difference(viewer, obj);


	if (fabs(dx) > 10.0 || fabs(dy) > 10.0) {
		/* out of bounds */
		return;
	}

	x = (int)(dx);
	y = (int)(dy);

	mass = obj->query_mass();

	if (mass < 0.5) {
		gc->move_pen(x, y);
		gc->set_color(0x8A);
		gc->draw("^");
	} else if (mass < 2.5) {
		gc->move_pen(x, y);
		gc->set_color(0x83);
		gc->draw("|");
		gc->move_pen(x, y - 1);
		gc->set_color(0x8A);
		gc->draw("^");
	} else if (mass < 50.0) {
		gc->set_color(0x38);
		gc->move_pen(x, y);
		gc->draw("#");
		gc->move_pen(x, y - 1);
		gc->draw("#");

		gc->set_color(0x2A);
		gc->move_pen(x - 1, y - 2);
		gc->draw("//\\");
		gc->move_pen(x, y - 3);
		gc->draw("/");
	} else {
		gc->set_color(0x38);
		gc->move_pen(x, y);
		gc->draw("#");
		gc->move_pen(x, y - 1);
		gc->draw("#");
		gc->move_pen(x, y - 2);
		gc->draw("#");

		gc->set_color(0x2A);
		gc->move_pen(x - 2, y - 3);
		gc->draw("///\\\\");
		gc->move_pen(x - 1, y - 4);
		gc->draw("//\\");
		gc->move_pen(x, y - 5);
		gc->draw("/");
	}
}
