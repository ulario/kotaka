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
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object *users;
	string **lists;
	int sz, i;

	lists = ({ ({ }), ({ }), ({ }) });

	send_out("User list\n");
	send_out("---------\n");

	users = TEXT_USERD->query_users();
	sz = sizeof(users);

	for (i = 0; i < sz; i++) {
		lists[3 - users[i]->query_class()]
			+= ({ users[i]->query_username() });
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
				string name;

				name = list[j];

				send_out(TEXT_SUBD->titled_name(
					name, TEXT_SUBD->query_user_class(name)
				) + "\n");
			}

			send_out("\n");
		}
	}
}
