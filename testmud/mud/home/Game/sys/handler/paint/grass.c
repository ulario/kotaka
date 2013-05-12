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
#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <game/paths.h>

inherit LIB_HANDLER;

#define RED	1
#define GREEN	2
#define BLUE	4
#define BOLD	8

void on_paint_text(object gc, object obj, object viewer)
{
	int mx, my;
	int i;

	for (my = -8; my <= 8; my++) {
		for (mx = -8; mx <= 8; mx++) {
			gc->move_pen(mx, my);

			switch(random(3)) {
			case 0:
				gc->set_color(0x20);
				break;
			case 1:
				gc->set_color(0x2A);
				break;
			case 2:
				gc->set_color(0x23);
				break;
			}
			switch(random(2)) {
			case 0:
				gc->draw(",");
				break;
			case 1:
				gc->draw("'");
				break;
			}
		}
	}
}
