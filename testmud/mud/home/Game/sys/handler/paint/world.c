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
#include <game/paths.h>

void on_paint_text(object gc, object obj, object viewer)
{
	float dx, dy, dz;

	int mx, my;
	int i;

	for (my = -8; my <= 8; my++) {
		for (mx = -8; mx <= 8; mx++) {
			gc->move_pen(mx, my);

			if (SUBD->rnd() < 0.5) {
				gc->set_color(0x02);
				gc->draw(STRINGD->chars('.', 17));
			} else {
				gc->set_color(0x01);
				gc->draw(STRINGD->chars('\'', 17));
			}
		}
	}

	({ dx, dy, dz }) = GAME_SUBD->query_position_difference(viewer, obj);

	mx = (int)dx;
	my = (int)dy;

	gc->set_color(0x89);

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
	gc->set_color(0x8F);
	gc->draw("X");
}
