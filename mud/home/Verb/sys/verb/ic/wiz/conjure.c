/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021, 2022  Raymond Jennings
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
#include <game/paths.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Conjure";
}

string *query_help_contents()
{
	return ({ "Creates a raw thing." });
}

void main(object actor, mapping roles)
{
	object thing;

	if (query_user()->query_class() < 2) {
		send_out("You must be a wizard to use this command.\n");
		return;
	}

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	send_out("You conjure up a thing.\n");

	thing = GAME_INITD->create_thing();
	thing->add_local_detail(nil);
	thing->set_local_snouns(nil, ({ "thing" }) );
	thing->move(actor);
}
