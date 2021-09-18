/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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
#include <kernel/version.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <kotaka/version.h>
#include <game/paths.h>
#include <status.h>

inherit "/lib/math/random";
inherit "/lib/string/case";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Charfix";
}

string *query_help_contents()
{
	return ({
		"Charfix audits your character set up, and fixes it if needed."
	});
}

void main(object actor, mapping roles)
{
	int newchar;
	object body, ghost, human, template, user, world;
	string name;

	if (roles["raw"]) {
		send_out("Usage: charfix\n");
		return;
	}

	user = query_user();
	name = user->query_name();

	if (!name) {
		send_out("You must log in first.\n");
		return;
	}

	template = IDD->find_object_by_name("templates:" + name);

	if (!template) {
		send_out("You don't have a character yet.\n");
		send_out("Please run \"chargen\".\n");
		return;
	}

	if (!template->has_local_detail(nil)) {
		send_out("Template is missing default detail.\n");
		template->add_local_detail(nil);
		template->add_local_snoun(nil, name);
		template->add_local_snoun(nil, "body");
		template->add_local_pnoun(nil, "bodies");
		template->add_local_adjective(nil, name);
		template->set_local_description(nil, "brief", to_title(name));
		send_out("Added default detail.\n");
	}

	ghost = IDD->find_object_by_name("ghosts:" + name);

	if (!ghost) {
		send_out("Your soul does not exist.\n");
		send_out("Either something is seriously wrong or your character has suffered the ultimate fate.\n");
		send_out("If you have any questions ask a wizard.\n");
		return;
	}

	if (!ghost->query_virtual()) {
		send_out("Ghost is not virtual.\n");
		ghost->set_virtual(1);
		send_out("Made ghost virtual.\n");
	}

	if (!ghost->has_local_detail(nil)) {
		send_out("Ghost is missing the default detail.\n");
		ghost->add_local_detail(nil);
		ghost->add_local_snoun(nil, "ghost");
		ghost->add_local_pnoun(nil, "ghost");
		ghost->set_local_adjectives(nil, ({ "ghost", name }) );
		ghost->set_local_description(nil, "brief", to_title(name) + "'s ghost");
	}

	body = IDD->find_object_by_name("players:" + name);

	if (!body) {
		send_out("Your body does not exist.\n");
		send_out("Either you are dead and your corpse is gone,\n");
		send_out("or something went awry.\n\n");
		send_out("If something went wrong, ask a wizard.\n");
		send_out("Otherwise, use the \"reincarnate\" command to create a new body.\n");
		return;
	}

	if (!body->has_local_detail(nil)) {
		send_out("Body is missing the default detail.\n");
		body->add_local_detail(nil);
		body->add_local_snoun(nil, "body");
		body->add_local_pnoun(nil, "bodies");
		body->add_local_adjective(nil, name);
		send_out("Default detail added.\n");
	}

	if (!body->query_character_lwo()) {
		send_out("Body is missing character stats.\n");
		body->initialize_character(10 + random(11), random(11), 30 + random(11));
		send_out("Character stats rolled.\n");
	}

	if (!body->query_living_lwo()) {
		send_out("You're dead, please \"resurrect\".\n");
		return;
	}

	if (ghost->query_environment() != body) {
		send_out("Your soul was not inside your body.\n");
		ghost->move(body);
		send_out("Moved your soul into your body.\n");
	}

	if (body->query_possessor() != ghost) {
		send_out("Your soul was not possessing your body.\n");
		ghost->possess(body);
		send_out("Possessed the body with the soul.\n");
	}

	if (!body->query_environment()) {
		object world;

		send_out("Your body was nowhere.\n");

		world = IDD->find_object_by_name("planets:earth");

		if (!world) {
			send_out("The start world is missing, yell at a wizard!\n");
			return;
		}

		body->move(world);
		send_out("Body moved.\n");
	}
}
