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
#include <game/paths.h>

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

private void default_painter(object gc, object neighbor, object viewer)
{
	string s;

	int dx, dy, dz;
	int ox, oy;

	int x, y;

	({ dx, dy, dz }) = GAME_SUBD->query_position_difference(viewer, neighbor);

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
	gc->set_color(0x8D);
	s = neighbor->query_property("brief");

	if (s && strlen(s) > 0) {
		gc->draw(s[0 .. 0]);
	} else {
		gc->draw("?");
	}
}

private void draw_background(object gc)
{
	gc->set_clip(0, 0, 79, 19);
	gc->set_offset(0, 3);
	gc->set_color(0x04);
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
		object *inv;
		int i;
		int sz;

		if (!(desc = env->query_property("look"))) {
			desc = "This place is boring.";
		}

		lines = explode(STRINGD->wordwrap(desc, 55), "\n");

		inv = env->query_inventory() - ({ actor });

		if (sizeof(inv)) {
			lines += ({ "" });

			lines += explode(STRINGD->wordwrap("You see " + inventory_list(inv), 55) + ".", "\n");
		}

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
}

private void draw_environment(object gc, object viewer, object env)
{
	string painter;

	int x, y;

	gc->set_clip(-8, -8, 8, 8);
	gc->set_offset(70, 10);

	if (painter = env->query_property("event:paint")) {
		painter->on_paint_text(gc, env, viewer);
	} else {
		gc->set_color(0x0F);
		gc->move_pen(0, 0);
		gc->draw("?");
	}
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

private string draw_object(object gc, object viewer, object obj)
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

void draw_contents(object gc, object obj, object viewer)
{
	int sz, i;
	object *inv;

	inv = obj->query_inventory();
	inv -= ({ nil });
	sz = sizeof(inv);

	SUBD->qsort(inv, 0, sizeof(inv), "position_sort");

	for (i = 0; i < sz; i++) {
		if (inv[i] == viewer) {
			continue;
		}

		if (inv[i]->is_container_of(viewer)) {
			/* we will draw this on the next layer */
			continue;
		}

		draw_object(gc, viewer, inv[i]);
	}
}

string draw_look(object viewer)
{
	int x, y, sz, i;
	object painter;
	object *envstack;
	object gc;

	ACCESS_CHECK(TEXT() || GAME());

	painter = new_object(LWO_PAINTER);
	painter->set_size(80, 20);
	painter->add_layer("default");
	painter->add_layer("exits");
	painter->add_layer("sprites");
	gc = painter->create_gc();
	gc->set_layer("default");

	draw_frame(gc);
	draw_background(gc);
	draw_banner(gc, viewer ? viewer->query_environment() : nil);

	draw_prose(gc, viewer);

	draw_tickmarks(gc);

	if (viewer) {
		object env;

		gc->set_clip(-8, -8, 8, 8);

		env = viewer->query_environment();

		if (env) {
			look_object(gc, viewer, env);
		}

		gc->move_pen(0, 0);
		gc->set_color(0x0F);
		gc->draw("@");
	} else {
		draw_bsod(gc);
	}

	return implode(painter->render_color(), "\n") + "\n";
}
