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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <game/paths.h>
#include <thing/paths.h>

int crowded(object world, object self, float tx, float ty)
{
	/* no trees allowed within a radius of 4 meters */
	object *inv;
	int sz;
	int i;

	inv = world->query_inventory();

	inv -= ({ self });

	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		float cx, cy;
		float dx, dy;
		object obj;

		obj = inv[i];

		if (obj->query_property("id") != "tree") {
			continue;
		}

		cx = obj->query_x_position();
		cy = obj->query_y_position();

		dx = cx - tx;
		dy = cy - ty;

		if (dx * dx + dy * dy < 16.0) {
			return 1;
		}
	}

	return 0;
}

void on_timer(object obj)
{
	float mass;

	mass = obj->query_mass();

	if (
		crowded(
			obj->query_environment(), obj,
			obj->query_x_position(), obj->query_y_position()
		)
	) {
		if (mass < 0.01) {
			destruct_object(obj);
			return;
		} else {
			if (mass > 1.0) {
				obj->set_mass(mass - sqrt(mass) * 0.1);
			} else {
				obj->set_mass(mass * 0.9);
			}
		}

		return; /* starving trees don't grow */
	}

	if (mass > 100.0) {
		/* bloom! */
		float lx, ly;
		float px, py, pa;
		float dx, dy;
		float sx, sy;
		float pr;

		object sprout;

		obj->set_mass(90.0);

		pa = SUBD->rnd() * SUBD->pi() * 2.0;
		pr = SUBD->rnd() * 16.0 + 3.0;

		px = pr * sin(pa);
		py = pr * cos(pa);

		lx = obj->query_x_position();
		ly = obj->query_y_position();

		sx = px + lx;
		sy = py + ly;

		if (crowded(obj->query_environment(), nil, sx, sy)) {
			return;
		}

		sprout = clone_object(OBJ_THING);
		sprout->add_archetype(CATALOGD->lookup_object("tree"));
		sprout->set_property("id", "tree");
		sprout->move(obj->query_environment());
		sprout->set_x_position(sx);
		sprout->set_y_position(sy);
		sprout->set_mass(1.0);

		sprout->query_property("event:create")->on_create(sprout);
	}

	obj->set_mass(mass + 1.0);
	obj->set_timer(SUBD->rnd() * 30.0, 15.0);
}
