/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;
inherit "~Text/lib/sub";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Passwd";
}

string *query_help_contents()
{
	return ({ "Use this command to change your password." });
}

void main(object actor, mapping roles)
{
	string target;
	object user, passwd;

	target = roles["raw"];
	user = query_user();

	if (!target) {
		target = user->query_name();
	}

	if (!ACCOUNTD->query_is_registered(target)) {
		send_out("No such account.\n");
		return;
	}

	if (target != user->query_name()) {
		switch(query_user_class(target)) {
		case 4: /* owner */
			if (user->query_username() != "admin") {
				send_out("Only the mud owner can change the admin password.\n");
				return;
			}
			break;

		case 3: /* administrator */
			if (user->query_username() != "admin") {
				send_out("Only the mud owner can change an administrator's password.\n");
				return;
			}
			break;

		default:
			if (user->query_class() < 3) {
				send_out("Only an administrator can change someone else's password.\n");
				return;
			}
		}
	}

	passwd = query_user()->clone_ustate("passwd");
	passwd->set_name(target);
	query_ustate()->push_state(passwd);
}
