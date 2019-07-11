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
#include <kernel/user.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;
inherit "/lib/sort";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Who";
}

string *query_help_contents()
{
	return ({ "Shows who is online." });
}

void main(object actor, mapping roles)
{
	int class;
	object user;
	object *users;
	mapping lists;
	string myname;
	int sz, i;
	int is_wiz;

	lists = ([
		4: ({ }),
		3: ({ }),
		2: ({ }),
		1: ({ })
	]);

	user = query_user();
	myname = user->query_username();
	class = user->query_class();

	send_out("User list\n");
	send_out("---------\n");

	users = TEXT_USERD->query_users();
	sz = sizeof(users);

	for (i = 0; i < sz; i++) {
		object ruser;
		string name;
		int uclass;
		int invisible;
		string buf;

		ruser = users[i];
		name = ruser->query_username();

		uclass = ruser->query_class();

		if (ACCOUNTD->query_account_property(name, "invisible")) {
			invisible = 1;
		}

		/* you can always see yourself */
		if (invisible && name != myname) {
			switch(class) {
			case 0:
			case 1:
				continue;
			case 2:
			case 3:
			case 4:
				if (uclass > class) {
					continue;
				}
			}
		}

		buf = ruser->query_titled_name();

		if (invisible) {
			buf += " (invisible)";
		}

		if (class > 2 && class >= uclass) {
			while (ruser <- LIB_USER) {
				ruser = ruser->query_conn();
			}

			buf += " (" + query_ip_number(ruser) + ")";
		}

		lists[uclass] += ({ buf });
	}

	for (i = 4; i > 0; i--) {
		if (sizeof(lists[i])) {
			int j;
			string *list;

			list = lists[i];
			sz = sizeof(list);
			qsort(list, 0, sz);

			switch(i) {
			case 4: send_out("Owners:\n"); break;
			case 3: send_out("Administrators:\n"); break;
			case 2: send_out("Wizards:\n"); break;
			case 1: send_out("Players:\n"); break;
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

	if (class >= 2 && (sz = sizeof(users))) {
		string *list;
		list = allocate(sz);

		for (i = 0; i < sz; i++) {
			object user, conn;

			user = users[i];
			conn = user->query_conn();

			while (conn && conn <- LIB_USER) {
				user = conn;
				conn = conn->query_conn();
			}

			if (conn) {
				list[i] = query_ip_number(conn);
			} else {
				list[i] = "(broken chain ending at " + object_name(user) + ")";
			}
		}

		send_out("Guest IPs:\n" + implode(list, "\n") + "\n\n");
	}
}
