/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

/*
mixed **query_roles()
{
	return ({
		({ "iob", ({ "to" }), 0 })
	});
}
*/

void main(object actor, mapping roles)
{
	object user;
	object target;
	string name;
	string emote;

	int sz, i;

	object env;
	object *listeners;
	object *mobiles;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	user = query_user();

	emote = roles["raw"];

	if (emote == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("The emptiness ignores you.\n");
		return;
	}

	if (emote[strlen(emote) - 1] != '.') {
		emote += ".";
	}

	emit_from(actor, actor, " ", emote);
}
