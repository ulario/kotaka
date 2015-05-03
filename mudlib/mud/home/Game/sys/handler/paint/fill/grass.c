/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <game/paths.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>

#define RED	1
#define GREEN	2
#define BLUE	4
#define BOLD	8

void on_paint_text(object gc, object obj, object viewer)
{
	int mx, my;
	int i;

	gc->set_layer("view");

	if (!obj->is_container_of(viewer)) {
		return;
	}

	gc->set_color(0x2A);

	for (my = -8; my <= 8; my++) {
		gc->move_pen(-8, my);
		gc->draw(STRINGD->chars('\'', 17));
	}
}
