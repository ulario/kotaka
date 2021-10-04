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
#include <kotaka/paths/verb.h>
#include <kotaka/paths/intermud.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "irouterdel";
}

string *query_help_contents()
{
	return ({ "Usage: irouterdel <router>", "Removes a router from IntermudD's router list" });
}

void main(object actor, mapping roles)
{
	string router;

	if (query_user()->query_class() < 3) {
		send_out("Only an administrator can remove a router.\n");
		return;
	}

	router = roles["raw"];

	if (!router) {
		send_out("Usage: irouterdel <router>\n");
		return;
	}

	if (!sscanf(router, "*%*s")) {
		send_out("I3 router names must begin with an asterisk.\n");
		return;
	}

	INTERMUDD->remove_router(router);

	send_out("Router removed.\n");
}
