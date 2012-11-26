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
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object genv;
	object env;

	if (!actor) {
		send_out("Odd...without a body you have no legs.\nPlease inhabit an object first.\n");
		return;
	}

	env = actor->query_environment();

	if (env) {
		genv = env->query_environment();

		if (genv) {
			float ax, ay;

			ax = actor->query_x_position();
			ay = actor->query_y_position();

			if (ax < 0.0 || ax > 4.0 || ay < 0.0 || ay > 4.0) {
				send_out("Don't know how you escaped, but you need to be in bounds of the shack to exit it.\n");
			} else {
				actor->move(genv);
				send_out("You exit.\n");
			}
		} else {
			send_out("You can't just exit the universe!\n");
		}
	} else {
		send_out("You are already nowhere.\n");
	}
}
