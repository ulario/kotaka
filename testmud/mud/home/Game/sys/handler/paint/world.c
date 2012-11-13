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
#include <kotaka/paths.h>
#include <kotaka/assert.h>

void paint_text(object gc, object obj, object viewer)
{
	int mx, my;
	int i;

	gc->set_color(0x03);

	for (i = -8; i <= 8; i++) {
		gc->move_pen(-8, i);
		gc->draw(STRINGD->chars(':', 17));
	}

	mx = -(int)viewer->query_x_position();
	my = -(int)viewer->query_y_position();

	gc->set_color(0x09);

	for (i = 1; i <= 2; i++) {
		gc->move_pen(mx - i, my - i);
		gc->draw("\\");
		gc->move_pen(mx + i, my + i);
		gc->draw("\\");
		gc->move_pen(mx - i, my + i);
		gc->draw("/");
		gc->move_pen(mx + i, my - i);
		gc->draw("/");
	}

	gc->move_pen(mx, my);
	gc->set_color(0x0F);
	gc->draw("X");
}
