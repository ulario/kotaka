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
#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>
#include <text/paths.h>

private void draw_tickmarks(object gc)
{
	int i;

	gc->set_offset(70, 10);
	gc->set_clip(-9, -9, 9, 9);

	for (i = -8; i <= 8; i += 1) {
		if (i % 4 == 0) {
			gc->set_color(0x8F);
		} else {
			gc->set_color(0x88);
		}

		gc->move_pen(i, -9);
		gc->draw("|");
		gc->move_pen(i, 9);
		gc->draw("|");
		gc->move_pen(-9, i);
		gc->draw("-");
		gc->move_pen(9, i);
		gc->draw("-");
	}
}

private void default_painter(object gc, object neighbor, object living)
{
	float dx, dy;
	float ox, oy;

	int x, y;

	dx = neighbor->query_x_position() - living->query_x_position();
	dy = neighbor->query_y_position() - living->query_y_position();

	/*
	vy = cos * dy - sin * dx;
	vx = sin * dy + cos * dx;
	*/

	x = (int)dx;
	y = (int)dy;

	if (x < -8 || x > 8 || y < -8 || y > 8) {
		return;
	}

	gc->move_pen(x, y);

	switch(neighbor->query_property("id")) {
	case "wolf":
		gc->set_color(0x08);
		gc->draw("w");
		break;
	case "deer":
		gc->set_color(0x03);
		gc->draw("d");
		break;
	case "rock":
		gc->set_color(0x07);
		gc->draw("@");
		break;
	case "soil":
		gc->set_color(0x23);
		gc->draw(":");
		break;
	default:
		gc->set_color(0x0D);
		gc->draw("?");
		break;
	}
}

private void draw_background(object gc)
{
	gc->set_clip(0, 0, 79, 19);
	gc->set_offset(0, 3);
	gc->set_color(0x04);
}

private void draw_prose(object gc, object actor)
{
	object env;

	if (actor) {
		env = actor->query_environment();
	}

	gc->set_clip(0, 0, 60, 15);
	gc->set_offset(0, 4);

	gc->set_color(0x07);

	if (!actor) {
		gc->move_pen(0, 0);
		gc->draw("How strange, you don't even exist.");
		gc->move_pen(0, 2);
		gc->draw("Your neighbors are undefined.");
	} else if (!env) {
		gc->move_pen(0, 0);
		gc->draw("You are in the void.");
		gc->move_pen(0, 2);
		gc->draw("You have no neighbors at all.");
	} else {
		gc->move_pen(0, 0);
		gc->draw("A boring room description.");
		gc->move_pen(0, 2);
		gc->draw("A boring inventory list.");
		gc->move_pen(0, 4);
		gc->draw("Your location is "
			+ actor->query_x_position() + ", "
			+ actor->query_y_position()
		);
	}
}

private void draw_frame(object gc)
{
	int y;

	gc->set_clip(0, 0, 79, 19);
	gc->set_offset(0, 0);

	gc->set_color(0x07);
	gc->move_pen(0, 0);
	gc->draw(STRINGD->chars('-', 80));

	for (y = 1; y < 20; y++) {
		gc->move_pen(60, y);
		gc->draw("|");
	}
}

private void draw_banner(object gc)
{
	gc->set_clip(0, 0, 59, 3);
	gc->set_offset(0, 1);

	gc->set_color(0x08);
	gc->move_pen(0, 1);
	gc->draw(STRINGD->chars('-', 60));
	gc->set_color(0x07);
	gc->move_pen(3, 0);
	gc->draw("Ularian Forest");
}

private void draw_environment(object gc, object living)
{
	object env;
	int x, y;

	gc->set_clip(-8, -8, 8, 8);
	gc->set_offset(70, 10);

	if (!living) {
		gc->set_color(0x47);

		for (y = -8; y <= 8; y++) {
			gc->move_pen(-8, y);
			gc->draw(STRINGD->chars(' ', 17));
		}

		gc->move_pen(-3, -1);
		gc->set_color(0x74);
		gc->draw(" Error ");
		gc->set_color(0x47);
		gc->move_pen(-3, 1);
		gc->draw("No body");
	} else if (!(env = living->query_environment())) {
		gc->set_color(0x7);

		for (y = -8; y <= 8; y += 1) {
			for (x = -8; x <= 8; x += 1) {
				gc->move_pen(x, y);
				gc->set_color(random(2) * 4);
				gc->draw(":");
			}
		}

		gc->set_color(0x0C);

		for (y = -6; y <= 6; y += 4) {
			for (x = -6; x <= 6; x += 4) {
				gc->move_pen(x, y);
				gc->draw("+");
			}
		}

		for (y = -8; y <= 8; y += 4) {
			for (x = -8; x <= 8; x += 4) {
				gc->move_pen(x, y);
				gc->draw("+");
			}
		}
	} else {
		object stack;
		string painter;

		stack = new_object(BIGSTRUCT_DEQUE_LWO);

		while (env) {
			stack->push_back(env);
			env = env->query_environment();
		}

		while (!stack->empty()) {
			env = stack->get_back();
			stack->pop_back();

			if (painter = env->query_property("painter")) {
				painter->on_paint_text(gc, env, living);
			} else {
				gc->set_color(0x0F);
				gc->move_pen(0, 0);
				gc->draw("?");
			}
		}
	}
}

int position_sort(object a, object b)
{
	if (a->query_y_position() < b->query_y_position()) {
		return -1;
	}

	return 1;
}

private void draw_neighbors(object gc, object living)
{
	object environment;

	if (living) {
		environment = living->query_environment();
	}

	if (environment) {
		float ox, oy;
		int sz, i;
		object *contents;

		contents = environment->query_inventory() - ({ living });
		sz = sizeof(contents);

		SUBD->qsort(contents, 0, sz, "position_sort");

		gc->set_clip(-8, -8, 8, 8);

		for (i = 0; i < sz; i++) {
			float dx, dy;
			float vx, vy;

			object neighbor;
			string painter;

			neighbor = contents[i];

			if (painter = neighbor->query_property("painter")) {
				painter->on_paint_text(gc, neighbor, living);
			} else {
				default_painter(gc, neighbor, living);
			}
		}
	}
}

string draw_look(object living, varargs int facing)
{
	int x, y, i;
	object painter;
	object environment;
	object *contents;
	object gc;

	ACCESS_CHECK(TEXT());

	painter = new_object(LWO_PAINTER);
	painter->set_size(80, 20);
	gc = painter->create_gc();

	if (living) {
		environment = living->query_environment();
	}

	draw_frame(gc);
	draw_background(gc);
	draw_banner(gc);

	draw_prose(gc, living);

	draw_tickmarks(gc);
	draw_environment(gc, living);
	draw_neighbors(gc, living);

	if (living) {
		gc->move_pen(0, 0);
		gc->set_color(0x0F);
		gc->draw("@");
	}

	return implode(painter->render_color(), "\n") + "\n";
}
