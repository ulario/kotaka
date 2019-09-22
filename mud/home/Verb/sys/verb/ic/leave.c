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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;
inherit "~/lib/ic";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Leave";
}

string *query_help_contents()
{
	return ({
		"Leaves a building or other object.",
		"Usually a leaveable object will have a default exit that will be used as" +
		" the exit, so this is just shorthand for going through that exit."
	});
}

void main(object actor, mapping roles)
{
	object env;
	object user;
	object def;

	if (roles["raw"] != "") {
		send_out("Just type \"leave\".\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("You're already in the void, how much more alone do you want to be?\n");
		return;
	}

	def = env->query_property("default_exit");

	if (def) {
		"~Action/sys/action/exit"->action(
			([
				"actor": actor,
				"dob": def
			])
		);
	} else {
		if (!env->query_environment()) {
			send_out("You're already at a root environment.\n");
			return;
		}

		user = query_user();

		if (user->query_class() < 2) {
			send_out("There doesn't appear to be an exit you could \"leave\" through.\n");
			return;
		}

		"~Action/sys/action/teleport"->action(
			([
				"actor": actor,
				"dob": env->query_environment()
			])
		);
	}
}
