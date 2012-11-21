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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <game/paths.h>

void on_timer(object obj)
{
	obj->set_mass(obj->query_mass() + SUBD->rnd() * 0.05);

	destruct_object(obj);

	return;

	if (obj->query_mass() > 2.0) {
		destruct_object(obj);
	} else if (obj->query_mass() < 1.0) {
		/* grow! */
		obj->set_mass(obj->query_mass() + SUBD->rnd() * 0.05);
	} else {
		/* bloom! */
		float lx, ly;
		float px, py, pa;
		float dx, dy;

		object sprout;

		pa = SUBD->rnd() * SUBD->pi() * 2.0;

		px = 8.0 * sin(pa);
		py = 8.0 * cos(pa);

		lx = obj->query_x_position();
		ly = obj->query_y_position();

		sprout = clone_object("~/obj/object");
		sprout->add_archetype(GAME_INITD->query_master("tree"));
		sprout->move(obj->query_environment());
		sprout->set_x_position(lx + px);
		sprout->set_y_position(ly + py);

		"../create/tree"->on_create(sprout);
	}
}
