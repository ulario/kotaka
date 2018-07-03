/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;
	object painter;
	object gc;
	int x, y, b;

	user = query_user();

	painter = new_object(LWO_PAINTER);
	painter->set_size(16, 8);
	painter->add_layer("test");
	painter->set_layer_size("test", 16, 8);

	gc = painter->create_gc();
	gc->set_layer("test");
	gc->set_clip(0, 0, 15, 7);
	b = random(128);

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 16; x++) {
			gc->set_color((x + y * 16) ^ b);
			gc->move_pen(x, y);
			gc->draw("X");
		}
	}

	send_out(implode(painter->render_color(), "\n") + "\n");
}
