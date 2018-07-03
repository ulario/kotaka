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
#include <kotaka/paths/utility.h>
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/geometry.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

private void draw_object(object gc, object viewer, object obj);
private void draw_contents(object gc, object viewer, object obj);

private void draw_tickmarks(object gc)
{
	int i;

	gc->set_layer("canvas");
	gc->set_offset(69, 10);
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

private void draw_grid(object gc)
{
	int x, y;

	gc->set_layer("grid");
	gc->set_offset(8, 8);
	gc->set_clip(-4, -4, 4, 4);

	for (y = -4; y <= 4; y += 4) {
		for (x = -4; x <= 4; x += 4) {
			gc->move_pen(x, y);
			gc->draw("+");
		}
	}
}

private void default_painter(object gc, object neighbor, object viewer)
{
	string s;

	int dx, dy, dz;
	int ox, oy;

	int x, y;
	mixed color;

	if (neighbor->is_container_of(viewer)) {
		return;
	}

	({ dx, dy, dz }) = GEOMETRY_SUBD->query_position_difference(viewer, neighbor);

	x = (int)dx;
	y = (int)dy;

	if (x < -8 || x > 8 || y < -8 || y > 8) {
		return;
	}

	gc->set_layer("sprites");
	gc->move_pen(x, y);

	color = neighbor->query_property("paint_color");

	if (color == nil) {
		gc->set_color(0x8f);
	} else {
		if (color != color & 0xff) {
			gc->set_color(0x0d);
			gc->draw("E");
			return;
		} else {
			gc->set_color(color);
		}
	}

	s = neighbor->query_property("paint_character");

	if (s) {
		if (strlen(s) == 1) {
			gc->draw(s[0 .. 0]);
		} else {
			gc->set_color(0x0d);
			gc->draw("E");
			return;
		}
	} else {
		s = neighbor->query_property("brief");

		if (s && strlen(s) > 0) {
			gc->draw(s[0 .. 0]);
		} else {
			gc->draw("?");
		}
	}
}

private string inventory_list(object *inv)
{
	string *desc;
	int i, sz;

	desc = ({ });
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		desc += ({ TEXT_SUBD->generate_brief_indefinite(inv[i]) });
	}

	return TEXT_SUBD->generate_list(desc);
}

private string prose(object viewer)
{
	object env;

	if (viewer) {
		env = viewer->query_environment();
	}

	if (!viewer) {
		return "You don't exist.\n";
	} else if (!env) {
		return "You are in the formless void.\n";
	} else {
		string *lines;
		string desc;
		object *inv;
		int i;
		int sz;

		if (!(desc = env->query_property("look"))) {
			desc = "This place is boring.";
		}

		desc = STRINGD->wordwrap(desc, 55) + "\n";

		inv = env->query_inventory() - ({ viewer });

		{
			int sz;

			for (sz = sizeof(inv); --sz >= 0; ) {
				if (inv[sz]->query_property("is_invisible") && this_user()->query_class() < 2) {
					inv[sz] = nil;
				}
			}

			inv -= ({ nil });
		}

		if (sizeof(inv)) {
			desc += "\n" + STRINGD->wordwrap("You see " + inventory_list(inv) + ".", 55) + "\n";
		}

		return desc;
	}
}

private void draw_prose(object gc, object viewer)
{
	object env;
	string *lines;
	int i, sz;

	if (viewer) {
		env = viewer->query_environment();
	}

	gc->set_layer("canvas");
	gc->set_clip(0, 0, 60, 15);
	gc->set_offset(2, 4);

	gc->set_color(0x07);
	gc->move_pen(0, 0);

	lines = explode(prose(viewer), "\n");
	sz = sizeof(lines);

	for (i = 0; i < sz; i++) {
		gc->move_pen(0, i);
		gc->draw(lines[i]);
	}
}

private void draw_frame(object gc)
{
	int x, y;

	gc->set_layer("canvas");
	gc->set_clip(0, 0, 79, 21);
	gc->set_offset(0, 0);

	gc->set_color(0x07);

	gc->move_pen(1, 0);
	gc->draw(STRINGD->chars('-', 78));

	gc->move_pen(1, 20);
	gc->draw(STRINGD->chars('-', 78));

	for (y = 1; y < 20; y++) {
		gc->move_pen(0, y);
		gc->draw("|");
		gc->move_pen(59, y);
		gc->draw("|");
		gc->move_pen(79, y);
		gc->draw("|");
	}

	gc->set_color(0x07);

	gc->move_pen(0, 0);
	gc->draw("+");
	gc->move_pen(79, 0);
	gc->draw("+");

	gc->move_pen(0, 20);
	gc->draw("+");
	gc->move_pen(79, 20);
	gc->draw("+");
}

private void draw_banner(object gc, object env)
{
	string brief;

	gc->set_layer("canvas");
	gc->set_clip(0, 0, 59, 3);
	gc->set_offset(0, 1);

	gc->set_color(0x08);
	gc->move_pen(1, 1);
	gc->draw(STRINGD->chars('-', 58));
	gc->set_color(0x07);
	gc->move_pen(2, 0);

	if (env) {
		brief = env->query_property("brief");

		if (!brief) {
			brief = "an unnamed location";
		}
	} else {
		brief = "nowhere";
	}

	if (brief[0] >= 'a' && brief[0] <= 'z') {
		brief[0] -= ('a' - 'A');
	}

	gc->draw(brief);
}

int position_sort(object a, object b)
{
	if (a->query_z_position() < b->query_z_position()) {
		return -1;
	}

	if (a->query_z_position() > b->query_z_position()) {
		return 1;
	}

	if (a->query_y_position() < b->query_y_position()) {
		return -1;
	}

	if (a->query_y_position() > b->query_y_position()) {
		return 1;
	}

	if (a->query_x_position() < b->query_x_position()) {
		return -1;
	}

	if (a->query_x_position() > b->query_x_position()) {
		return 1;
	}

	return 1;
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

private void draw_object(object gc, object viewer, object obj)
{
	mixed painter;

	if (painter = obj->query_property("event:paint")) {
		painter->on_paint_text(gc, obj, viewer);
	} else {
		default_painter(gc, obj, viewer);
	}
}

private string look_object(object gc, object viewer, object obj)
{
	object env;
	object *inv;
	int sz;

	if (obj->query_property("is_transparent")) {
		env = obj->query_environment();

		if (env) {
			look_object(gc, viewer, env);
		}
	}

	draw_object(gc, viewer, obj);
}

private void draw_contents(object gc, object viewer, object obj)
{
	int sz, i;
	object *inv;

	inv = obj->query_inventory();
	inv -= ({ nil });
	sz = sizeof(inv);

	SORTD->qsort(inv, 0, sizeof(inv), "position_sort");

	for (i = 0; i < sz; i++) {
		if (inv[i] == viewer) {
			/* viewer is drawn separately */
			continue;
		}

		if (inv[i]->is_container_of(viewer)) {
			/* we will draw this on the next layer */
			continue;
		}

		draw_object(gc, viewer, inv[i]);
	}
}

private string draw_look_xyz(object viewer)
{
	object oenv;
	int x, y, sz, i;
	object painter;
	object *envstack;
	object gc;
	string render;

	painter = new_object(LWO_PAINTER);
	painter->set_size(80, 21);

	painter->add_layer("canvas");
	painter->set_layer_size("canvas", 80, 21);
	painter->set_layer_position("canvas", 0, 0);

	painter->add_layer("view");
	painter->set_layer_size("view", 17, 17);
	painter->set_layer_position("view", 61, 2);

	painter->add_layer("grid");
	painter->set_layer_size("grid", 17, 17);
	painter->set_layer_position("grid", 61, 2);

	painter->add_layer("exits");
	painter->set_layer_size("exits", 17, 17);
	painter->set_layer_position("exits", 61, 2);

	painter->add_layer("sprites");
	painter->set_layer_size("sprites", 17, 17);
	painter->set_layer_position("sprites", 61, 2);

	gc = painter->create_gc();

	gc->set_layer("canvas");
	gc->set_clip(0, 0, 79, 21);
	gc->set_offset(0, 0);

	draw_frame(gc);
	draw_banner(gc, viewer ? viewer->query_environment() : nil);

	draw_prose(gc, viewer);

	draw_tickmarks(gc);
	draw_grid(gc);

	gc->set_layer("view");

	if (viewer) {
		object env;
		object envstack;

		gc->set_clip(-8, -8, 8, 8);
		gc->set_offset(8, 8);

		env = viewer->query_environment();
		oenv = env;
		envstack = new_object(BIGSTRUCT_DEQUE_LWO);
		envstack->claim();

		while (env && (env->query_property("is_transparent") || env->query_virtual())) {
			env = env->query_environment();

			if (env) {
				envstack->push_front(env);
			}
		}

		while (!envstack->empty()) {
			env = envstack->query_front();
			envstack->pop_front();
			draw_object(gc, viewer, env);
		}

		draw_object(gc, viewer, oenv);
		draw_contents(gc, viewer, oenv);

		gc->set_layer("sprites");
		gc->move_pen(0, 0);
		gc->set_color(0x0F);
		gc->draw("@");
	} else {
		draw_bsod(gc);
	}

	render = implode(painter->render_color(), "\n") + "\n";

	{
		object origin;

		origin = viewer->query_outer_origin();

		if (!origin) {
			render += "No origin.\n";
		} else {
			string csystem;

			csystem = origin->query_coordinate_system();

			render += "Coordinate system: " + csystem + "\n";
			render += "Origin: " + TEXT_SUBD->generate_brief_proper(origin) + "\n";

			switch(csystem) {
			case "xyz":
				{
					int *d;

					d = GEOMETRY_SUBD->query_position_difference(origin, viewer);

					render += "Your coordinates are: (" + d[0] + ", " + d[1] + ", " + d[2] + ")\n";
				}
				break;

			case "void":
				render += "You don't have any coordinates here.\n";
			}
		}
	}

	return render;
}

private string draw_void(object viewer)
{
	object env;
	string output;
	string csystem;
	object origin;

	env = viewer->query_environment();
	output = "";

	if (env) {
		output = TEXT_SUBD->generate_brief_proper(env) + "\n\n";
		output = STRINGD->to_upper(output[0 .. 0]) + output[1 ..];
	}

	output += prose(viewer);

	origin = viewer->query_outer_origin();

	if (origin) {
		csystem = origin->query_coordinate_system();

		output += "Coordinate system: " + csystem + "\n";
		output += "Origin: " + TEXT_SUBD->generate_brief_proper(origin) + "\n";
	} else {
		output += "No origin\n";
	}

	return output;
}

string draw_look(object viewer)
{
	object origin;

	ACCESS_CHECK(TEXT() || GAME() || VERB());

	origin = viewer->query_outer_origin();

	if (!origin) {
		return draw_void(viewer);
	}

	switch (origin->query_coordinate_system()) {
	case "xyz":
		return draw_look_xyz(viewer);

	case "void":
		return draw_void(viewer);
	}
}
