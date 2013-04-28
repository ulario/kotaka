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
		gc->draw("You don't exist.");
	} else if (!env) {
		gc->move_pen(0, 0);
		gc->draw("You are in the formless void.");
	} else {
		string *lines;
		string desc;
		int i;
		int sz;

		if (!(desc = env->query_property("look"))) {
			desc = "This place is boring.";
		}

		lines = explode(STRINGD->wordwrap(desc, 55), "\n");
		sz = sizeof(lines);

		for (i = 0; i < sz; i++) {
			gc->move_pen(0, i);
			gc->draw(lines[i]);
		}
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

private void draw_banner(object gc, object env)
{
	string brief;

	gc->set_clip(0, 0, 59, 3);
	gc->set_offset(0, 1);

	gc->set_color(0x08);
	gc->move_pen(0, 1);
	gc->draw(STRINGD->chars('-', 60));
	gc->set_color(0x07);
	gc->move_pen(3, 0);

	if (env) {
		brief = env->query_property("brief");

		if (!brief) {
			brief = "an unnamed location";
		}
	} else {
		brief = "nowhere";
	}

	gc->draw(brief);
}

private void draw_void(object gc)
{
	int x, y;

	gc->set_color(0x7);

	for (y = -8; y <= 8; y += 1) {
		for (x = -8; x <= 8; x += 1) {
			gc->move_pen(x, y);

			if (random(2)) {
				gc->set_color(7 ^ (1 << random(3)));
				gc->draw(":");
			}
		}
	}

	gc->set_color(0x0C);
}

private void draw_environment(object gc, object living, object env)
{
	string painter;

	int x, y;

	gc->set_clip(-8, -8, 8, 8);
	gc->set_offset(70, 10);

	if (painter = env->query_property("event:paint")) {
		painter->on_paint_text(gc, env, living);
	} else {
		gc->set_color(0x0F);
		gc->move_pen(0, 0);
		gc->draw("?");
	}
}

int position_sort(object a, object b)
{
	if (a->query_y_position() < b->query_y_position()) {
		return -1;
	}

	return 1;
}

private void draw_neighbors(object gc, object living, object *neighbors)
{
	float ox, oy;
	int sz, i;

	sz = sizeof(neighbors);

	SUBD->qsort(neighbors, 0, sz, "position_sort");

	gc->set_clip(-8, -8, 8, 8);

	for (i = 0; i < sz; i++) {
		float dx, dy;
		float vx, vy;

		object neighbor;
		string painter;

		neighbor = neighbors[i];

		if (painter = neighbor->query_property("event:paint")) {
			painter->on_paint_text(gc, neighbor, living);
		} else {
			default_painter(gc, neighbor, living);
		}
	}
}

private void draw_bsod(object gc)
{
	int x, y;

	gc->set_clip(-8, -8, 8, 8);
	gc->set_offset(70, 10);
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
}

string draw_look(object living, varargs int facing)
{
	int x, y, sz, i;
	object painter;
	object *envstack;
	object gc;

	ACCESS_CHECK(TEXT());

	painter = new_object(LWO_PAINTER);
	painter->set_size(80, 20);
	gc = painter->create_gc();

	draw_frame(gc);
	draw_background(gc);
	draw_banner(gc, living ? living->query_environment() : nil);

	draw_prose(gc, living);

	draw_tickmarks(gc);

	if (living) {
		object environment;

		envstack = ({ });

		draw_void(gc);

		environment = living->query_environment();

		if (environment) {
			while (environment) {
				envstack = ({ environment }) + envstack;
				environment = environment->query_environment();
			}

			sz = sizeof(envstack);
			envstack += ({ living });

			for (i = 0; i < sz; i++) {
				object *contents;

				draw_environment(gc, living, envstack[i]);
				draw_neighbors(gc, living, envstack[i]->query_inventory() - ({ envstack[i + 1] }));
			}
		}

		gc->move_pen(0, 0);
		gc->set_color(0x0F);
		gc->draw("@");
	} else {
		draw_bsod(gc);
	}

	return implode(painter->render_color(), "\n") + "\n";
}
