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
#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>

#include <account/paths.h>
#include <game/paths.h>
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

private void default_painter(object neighbor, object living, object painter)
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

	painter->move_pen(x, y);

	switch(neighbor->query_property("id")) {
	case "wolf":
		painter->set_color(0x08);
		painter->draw("w");
		break;
	case "deer":
		painter->set_color(0x03);
		painter->draw("d");
		break;
	case "rock":
		painter->set_color(0x07);
		painter->draw("@");
		break;
	case "soil":
		painter->set_color(0x23);
		painter->draw(":");
		break;
	default:
		painter->set_color(0x0D);
		painter->draw("?");
		break;
	}
}

private void draw_background(object gc)
{
	int x, y;

	gc->set_clip(0, 0, 79, 19);
	gc->set_offset(0, 0);
	gc->set_color(0x04);

	for (y = 4; y < 20; y += 2) {
		for (x = 0; x < 60; x += 8) {
			gc->move_pen(x, y);
			gc->draw("+");
		}
	}

	for (y = 3; y < 20; y += 2) {
		for (x = 4; x < 60; x += 8) {
			gc->move_pen(x, y);
			gc->draw("+");
		}
	}
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
	}
}

private void draw_banner(object gc)
{
	gc->set_clip(0, 0, 79, 3);
	gc->set_offset(0, 0);

	gc->set_color(0x08);
	gc->move_pen(0, 0);
	gc->draw(STRINGD->chars(':', 60));
	gc->move_pen(0, 1);
	gc->draw(STRINGD->chars(':', 60));
	gc->move_pen(0, 2);
	gc->draw(STRINGD->chars(':', 60));
	gc->set_color(0x07);
	gc->move_pen(3, 1);
	gc->draw("Ularian Forest");
}

private void draw_map(object gc, object living)
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
		gc->set_color(0x20);

		for (y = -8; y <= 8; y++) {
			gc->move_pen(-8, y);
			gc->draw(STRINGD->chars('`', 17));
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

	draw_background(gc);
	draw_banner(gc);

	draw_prose(gc, living);

	draw_tickmarks(gc);
	draw_map(gc, living);

	gc->set_color(0x03);

	if (environment) {
		float ox, oy;
		int sz;

		float pi, sin, cos;

		pi = SUBD->pi();

		sin = sin((float)facing * pi / 180.0);
		cos = cos((float)facing * pi / 180.0);

		contents = environment->query_inventory() - ({ living });
		sz = sizeof(contents);

		SUBD->qsort(contents, 0, sz, "position_sort");

		gc->set_clip(-8, -8, 8, 8);

		for (i = 0; i < sz; i++) {
			float dx, dy;
			float vx, vy;

			object neighbor;
			object painthandler;

			neighbor = contents[i];

			if (painthandler = neighbor->query_property("painter")) {
				painthandler->paint_text(neighbor, living, gc);
			} else {
				default_painter(neighbor, living, gc);
			}
		}
	}

	if (living) {
		gc->move_pen(0, 0);
		gc->set_color(0x0F);
		gc->draw("@");
	}

	return implode(painter->render_color(), "\n") + "\n";
}

string titled_name(string name, int class)
{
	string username;

	username = name;

	if (name) {
		name = STRINGD->to_title(name);
	} else {
		name = "guest";
	}

	switch(class) {
	case 0:
		name = "\033[1;34m" + name + "\033[0m";
		break;
	case 1:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mMr. " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mMs. " + name + "\033[0m";
			break;
		}
		break;
	case 2:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mSir " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mDame " + name + "\033[0m";
			break;
		}
		break;
	case 3:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mLord " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mLady " + name + "\033[0m";
			break;
		}
		break;
	}

	return name;
}

void send_to_all(string phrase)
{
	int sz;
	object *users;

	ACCESS_CHECK(TEXT());

	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

void send_to_all_except(string phrase, object *exceptions)
{
	int sz;
	object *users;

	ACCESS_CHECK(TEXT());

	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();
	users -= exceptions;

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

int query_user_class(string username)
{
	if (!username) {
		return 0;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		return 0;
	}

	if (KERNELD->access(username, "/", FULL_ACCESS)) {
		return 3;
	}

	if (sizeof( KERNELD->query_users() & ({ username }) )) {
		return 2;
	}

	return 1;
}

object mega_inventory(object root)
{
	object obj;
	object *inv;
	int sz, i;
	object list;
	object queue;

	list = new_object(BIGSTRUCT_ARRAY_LWO);
	list->grant_access(previous_object(), FULL_ACCESS);
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
	queue->push_back(root);

	while (!queue->empty()) {
		object obj;

		obj = queue->get_front();
		queue->pop_front();

		list->push_back(obj);

		inv = obj->query_inventory();
		sz = sizeof(inv);

		for (i = 0; i < sz; i++) {
			queue->push_back(inv[i]);
		}
	}

	return list;
}
