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
#include <game/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object env;
	object *inv;

	float my_x;
	float my_y;
	
	float dx, dy;
	float dsquare;

	int i, sz;

	object *closest;

	if (!actor) {
		send_out("Odd...without a body you have no hands.\nPlease inhabit an object first.\n");
		return;
	}

	env = actor->query_environment();
	inv = env->query_inventory();
	inv -= ({ actor });

	sz = sizeof(inv);

	if (!sz) {
		send_out("There is nothing for you to take!");
		return;
	}

	closest = ({ inv[0] });

	my_x = actor->query_x_position();
	my_y = actor->query_y_position();

	dx = my_x - closest[0]->query_x_position();
	dy = my_y - closest[0]->query_y_position();

	dsquare = dx * dx + dy * dy;

	for (i = 1; i < sz; i++) {
		object obj;
		float trial;

		obj = inv[i];

		dx = my_x - obj->query_x_position();
		dy = my_y - obj->query_y_position();

		trial = dx * dx + dy * dy;

		if (trial < dsquare) {
			closest = ({ obj });
			dsquare = trial;
		} else if (trial == dsquare) {
			closest += ({ obj });
		}
	}

	send_out("There are " + sizeof(closest) + " objects within " + sqrt(dsquare) + " meters.\n");

	if (sqrt(dsquare) < 1.0) {
		object obj;

		if (sizeof(closest) > 1) {
			send_out("You take one of them.\n");
			obj = closest[random(sizeof(closest))];
		} else {
			send_out("You take it.\n");
			obj = closest[0];
		}

		obj->move(actor);

		obj->set_x_position(0.0);
		obj->set_y_position(0.0);
		obj->set_z_position(0.0);
	} else {
		if (sizeof(closest) > 1) {
			send_out("Sadly they are all out of reach for you.\n");
			return;
		} else {
			send_out("Alas, your arms are not long enough to take it from your present position!\n");
			return;
		}
	}

	send_out("looking around\n");
	send_out(GAME_SUBD->draw_look(actor, 0));
	send_out("done looking around\n");
}
