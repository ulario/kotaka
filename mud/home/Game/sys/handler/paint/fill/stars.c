/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2022  Raymond Jennings
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
inherit "/lib/string/char";

private void draw_starpatch(object gc, int x, int y)
{
	gc->set_color(0xf);

	gc->move_pen(x + 0, y - 2); gc->draw(".");
	gc->move_pen(x + 0, y + 0); gc->draw(".");
	gc->move_pen(x + 0, y + 2); gc->draw(".");
	gc->move_pen(x - 2, y - 1); gc->draw(".");
	gc->move_pen(x + 2, y - 1); gc->draw(".");
	gc->move_pen(x - 2, y + 1); gc->draw(".");
	gc->move_pen(x + 2, y + 1); gc->draw(".");
}

void on_paint_text(object gc, object obj, object viewer)
{
	int i;

	gc->set_layer("view");

	if (!obj->is_container_of(viewer)) {
		return;
	}

	gc->set_color(0x7);

	for (i = -8; i <= 8; i++) {
		gc->draw(spaces(17));
	}

	draw_starpatch(gc, -5, -5);
	draw_starpatch(gc, 7, 0);
	draw_starpatch(gc, 0, 7);
}
