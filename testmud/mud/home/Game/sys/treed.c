/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/bigstruct.h>
#include <game/paths.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

object queue;
int handle;

void create()
{
	queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
}

private object create_object()
{
	return GAME_INITD->create_object();
}

void enqueue(object tree)
{
	ACCESS_CHECK(GAME());

	queue->push_back(tree);

	if (!handle) {
		handle = call_out("dequeue", 0);
	}

	LOGD->post_message("tree", LOG_DEBUG, queue->get_size() + "");
}

private int crowded(object world, object self, int tx, int ty)
{
	/* no trees allowed within a radius of 4 meters */
	object *inv;
	int sz;
	int i;

	if (!world) {
		return 1;
	}

	inv = world->query_inventory();

	inv -= ({ self });

	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		int cx, cy;
		int dx, dy;
		object obj;

		obj = inv[i];

		if (obj->query_property("id") != "tree") {
			continue;
		}

		cx = obj->query_x_position();
		cy = obj->query_y_position();

		dx = cx - tx;
		dy = cy - ty;

		if (dx * dx + dy * dy < 16) {
			return 1;
		}
	}

	return 0;
}

static void dequeue()
{
	object tree;
	float mass;

	handle = 0;

	tree = queue->get_front();
	queue->pop_front();

	LOGD->post_message("tree", LOG_DEBUG, queue->get_size() + "");

	if (!queue->empty()) {
		handle = call_out("dequeue", 0);
	}

	if (!tree) {
		return;
	}

	tree->set_timer(300.0 + SUBD->rnd() * 300, -1.0);

	mass = tree->query_mass();

	if (
		crowded(
			tree->query_environment(), tree,
			tree->query_x_position(), tree->query_y_position()
		)
	) {
		if (mass <= 1.0) {
			destruct_object(tree);
			return;
		} else {
			if (mass > 1.0) {
				tree->set_mass(mass - sqrt(mass) * 0.1);
			} else {
				tree->set_mass(mass * 0.9);
			}
		}

		return; /* starving trees don't grow */
	}

	if (mass > 100.0) {
		/* bloom! */
		int lx, ly;
		int px, py;
		int sx, sy;
		float pa, pr;

		object sprout;

		tree->set_mass(90.0);

		pa = SUBD->rnd() * SUBD->pi() * 2.0;
		pr = SUBD->rnd() * 6.0 + 2.0;

		px = (int)(pr * sin(pa));
		py = (int)(pr * cos(pa));

		lx = tree->query_x_position();
		ly = tree->query_y_position();

		sx = px + lx;
		sy = py + ly;

		if (crowded(tree->query_environment(), nil, sx, sy)) {
			return;
		}

		sprout = create_object();
		sprout->set_property("id", "tree");
		sprout->add_archetype(CATALOGD->lookup_object("scenery:tree"));
		sprout->move(tree->query_environment());
		sprout->set_x_position(sx);
		sprout->set_y_position(sy);
		sprout->set_mass(1.0);

		sprout->query_property("event:create")->on_create(sprout);
	}

	tree->set_mass(mass + 1.0);
}
