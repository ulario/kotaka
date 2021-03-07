/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Quit";
}

string *query_help_contents()
{
	return ({ "Goes out of character, or if already ooc, logs out." });
}

void main(object actor, mapping roles)
{
	object body, user;

	if (roles["raw"]) {
		send_out("Usage: logout\n");
		return;
	}

	user = query_user();
	body = user->query_body();

	if (body) {
		"~Action/sys/action/logout"->action( ([ "actor": actor ]) );

		user->set_body(nil);
	} else {
		query_user()->quit("quit");
	}
}
