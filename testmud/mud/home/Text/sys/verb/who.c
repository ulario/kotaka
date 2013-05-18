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
#include <kernel/user.h>
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, mapping roles)
{
	object user;
	object *users;
	string **lists;
	int sz, i;
	int is_wiz;

	lists = ({ ({ }), ({ }), ({ }) });

	user = query_user();

	if (user->query_class() >= 2) {
		is_wiz = 1;
	}

	send_out("User list\n");
	send_out("---------\n");

	users = TEXT_USERD->query_users();
	sz = sizeof(users);

	for (i = 0; i < sz; i++) {
		object user;
		string name;
		int class;
		string buf;

		user = users[i];
		name = user->query_username();
		class = user->query_class();

		buf = TEXT_SUBD->titled_name(name, class);

		if (is_wiz) {
			while (user <- LIB_USER) {
				user = user->query_conn();
			}

			buf += " (" + query_ip_number(user) + ")";
		}

		lists[3 - class] += ({ buf });
	}

	for (i = 0; i < 3; i++) {
		if (sizeof(lists[i])) {
			int j;
			string *list;

			list = lists[i];
			sz = sizeof(list);
			SUBD->qsort(list, 0, sz);

			switch(i) {
			case 0: send_out("Administrators:\n"); break;
			case 1: send_out("Wizards:\n"); break;
			case 2: send_out("Players:\n"); break;
			}

			for (j = 0; j < sz; j++) {
				send_out(list[j] + "\n");
			}

			send_out("\n");
		}
	}

	users = TEXT_USERD->query_guests();

	switch (sizeof(users)) {
	case 1:
		send_out("There is " + sizeof(users) + " guest connected.\n\n");
		break;
	case 0:
		break;
	default:
		send_out("There are " + sizeof(users) + " guests connected.\n\n");
	}

	if (is_wiz && (sz = sizeof(users))) {
		string *list;
		list = allocate(sz);

		for (i = 0; i < sz; i++) {
			user = users[i];

			while (user <- LIB_USER) {
				user = user->query_conn();
			}

			list[i] = query_ip_number(user);
		}

		send_out("Guest IPs: " + implode(list, ", ") + "\n\n");
	}
}
