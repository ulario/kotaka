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

/* drawing the feedback screen: */

#define XM (80 - 18)
#define YM (1)

private void draw_tickmarks(object painter)
{
	int i;

	for (i = 0; i < 17; i += 1) {
		if (i % 4 == 0) {
			painter->set_color(0x8F);
		} else {
			painter->set_color(0x88);
		}

		painter->move_pen(XM + i, YM - 1);
		painter->draw("|");
		painter->move_pen(XM + i, YM + 17);
		painter->draw("|");
		painter->move_pen(XM - 1, YM + i);
		painter->draw("-");
		painter->move_pen(XM + 17, YM + i);
		painter->draw("-");
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

	x = (int)dx + 8;
	y = (int)dy + 8;

	if (x < 0 || x > 16 || y < 0 || y > 16) {
		return;
	}

	painter->move_pen(XM + x, YM + y);

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

string draw_look(object living, varargs int facing)
{
	int x, y, i;
	object painter;
	object environment;
	object *contents;

	ACCESS_CHECK(TEXT());

	painter = new_object(LWO_PAINTER);
	painter->set_size(80, 20);

	if (!living) {
		painter->set_color(0x47);
		for (i = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars(' ', 17));
		}
		painter->move_pen(XM + 5, YM + 7);
		painter->set_color(0x74);
		painter->draw(" Error ");
		painter->set_color(0x47);
		painter->move_pen(XM + 5, YM + 9);
		painter->draw("No body");
	} else if (!(environment = living->query_environment())) {
		painter->set_color(0x7);

		for (i = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars(':', 17));
		}
	} else {
		painter->set_color(0x20);

		for (y = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars('`', 17));
		}
	}

	draw_tickmarks(painter);
	painter->set_color(0x03);

	if (environment) {
		float ox, oy;
		int sz;

		float pi, sin, cos;

		pi = SUBD->pi();

		sin = sin((float)facing * pi / 180.0);
		cos = cos((float)facing * pi / 180.0);

		contents = environment->query_inventory() - ({ living });
		sz = sizeof(contents);

		for (i = 0; i < sz; i++) {
			float dx, dy;
			float vx, vy;

			object neighbor;
			object painthandler;

			neighbor = contents[i];

			if (painthandler = neighbor->query_property("painter")) {
				painthandler->paint_text(neighbor, living, painter);
			} else {
				default_painter(neighbor, living, painter);
			}
		}
	}

	if (living) {
		painter->move_pen(XM + 8, YM + 8);
		painter->set_color(0x0F);
		painter->draw("@");
	}

	painter->set_color(0x07);
	painter->move_pen(0, 0);
	painter->draw("Ularian Forest");

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
