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
#include <kotaka/paths/system.h>

inherit "/lib/string/case";
inherit LIB_EMIT;
inherit LIB_VERB;
inherit "~/lib/ic";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Live";
}

string *query_help_contents()
{
	return ({ "Resurrect after dying.  Requires your body to still exist, otherwise you'll have to reincarnate." });
}

void main(object actor, mapping roles)
{
	string name;

	object template;
	object ghost;
	object body;

	if (actor->query_living_lwo()) {
		send_out("You're already alive.\n");
		return;
	}

	name = actor->query_object_name();

	if (!name || !sscanf(name, "ghosts:%s", name)) {
		send_out("Only ghosts can resurrect.\n");
		return;
	}

	/* if we get this far as a ghost we should already have a template */
	template = IDD->find_object_by_name("templates:" + name);
	ASSERT(template);

	body = IDD->find_object_by_name("players:" + name);

	if (!body) {
		send_out("Bummer, your body is gone.\nYou'll have to reincarnate to get a new one.\n");
		return;
	}

	if (!body->query_character_lwo()) {
		send_out("BUG: Your body doesn't appear to be a valid character, yell at a wizard.\n");
		return;
	}

	send_out("Returning your soul to its body...\n");

	emit_from(actor, actor, " ", ({ "enter", "enters" }), " ", body);
	actor->move(body);
	actor->possess(body);

	send_out("Reviving you...\n");
	body->initialize_living();
	body->remove_local_snoun(nil, "corpse");
	body->remove_local_pnoun(nil, "corpses");
	body->remove_local_adjective(nil, "dead");
	body->set_local_description(nil, "brief", to_title(name));
	emit_from(body, body, " ", ({ "revive", "revives" }), "!");
}
