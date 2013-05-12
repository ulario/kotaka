/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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

inherit LIB_HANDLER;

void on_paint_text(object gc, object obj, object viewer)
{
	int dx, dy, dz;

	int row;

	({ dx, dy, dz }) = GAME_SUBD->query_position_difference(viewer, obj);

	if (obj->is_container_of(viewer)) {
		gc->set_color(0x03);

		gc->move_pen(dx - 1, dy - 4);	gc->draw("+----+");
		gc->move_pen(dx - 1, dy - 3);	gc->draw("|    |");
		gc->move_pen(dx - 1, dy - 2);	gc->draw("|    |");
		gc->move_pen(dx - 1, dy - 1);	gc->draw("|    |");
		gc->move_pen(dx - 1, dy);	gc->draw("|    |");
		gc->move_pen(dx - 1, dy + 1);	gc->draw("+----+");

		gc->set_color(0x30);

		gc->move_pen(dx, dy - 3);	gc->draw("|___");
		gc->move_pen(dx, dy - 2);	gc->draw("__|_");
		gc->move_pen(dx, dy - 1);	gc->draw("_|__");
		gc->move_pen(dx, dy);		gc->draw("___|");
	} else {
		gc->set_color(0x30);

		gc->move_pen(dx, dy - 3);
		gc->draw("/--\\");
		gc->move_pen(dx, dy - 2);
		gc->draw("|  |");
		gc->move_pen(dx, dy - 1);
		gc->draw("|  |");
		gc->move_pen(dx, dy - 0);
		gc->draw("|__|");
	}
}
