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
void on_paint_text(object gc, object obj, object viewer)
{
	int i;

	gc->set_layer("view");

	if (!obj->is_container_of(viewer)) {
		return;
	}

	gc->set_color(0x4C);

	for (i = -8; i <= 8; i += 2) {
		gc->move_pen(-8, i);
		gc->draw("OoOoOoOoOoOoOoOoO");
	}

	for (i = -7; i <= 7; i += 2) {
		gc->move_pen(-8, i);
		gc->draw("oOoOoOoOoOoOoOoOo");
	}
}
