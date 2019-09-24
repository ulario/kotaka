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
#include <type.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "english" });
}

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({ });
}

string query_help_title()
{
	return "Fight";
}

string *query_help_contents()
{
	return ({
		"Attacks another character.",
		"The target must itself be alive."
	});
}

private void do_attack(object actor, object dob)
{
	object achar, aliv;
	object dchar, dliv;
	int damage;

	achar = actor->query_character_lwo();

	if (!achar) {
		send_out("You are not a character.\n");
		return;
	}

	aliv = actor->query_living_lwo();

	if (!aliv) {
		send_out("You're dead, Jim.\n");
		return;
	}

	dchar = dob->query_character_lwo();

	if (!dchar) {
		send_out("That is not a character.\n");
		return;
	}

	dliv = dob->query_living_lwo();

	if (!dliv) {
		send_out("It's dead, Jim\n");
		return;
	}

	damage = achar->query_attack() - dchar->query_defense();

	if (damage <= 0) {
		send_out("Your attack is ineffective\n");
		emit_from(actor, actor, " harmlessly ", ({ "hit", "hits" }), " ", dob, ".");
		return;
	}

	if (dliv->subtract_hp(damage) <= 0) {
		emit_from(actor, actor, " ", ({ "kill", "kills" }), " ", dob, "!");
		dob->clear_living();
		emit_to(actor, actor, "You fatally deal " + damage + " points of damage!");
		emit_to(dob, dob, "You suffer " + damage + " points of damage and die!");
	} else {
		emit_from(actor, actor, " ", ({ "hit", "hits" }), " ", dob, ".");
		emit_to(actor, actor, "You deal " + damage + " points of damage.");
		emit_to(dob, dob, "You suffer " + damage + " points of damage.");
	}
}

void main(object actor, mapping roles)
{
	mixed dob;
	string look;
	object from;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		send_out("Fight what?\n");
		return;
	}

	if (dob[0]) {
		send_out("Your grammar stinks.\n");
		return;
	}

	dob = dob[1];

	switch(typeof(dob)) {
	case T_OBJECT:
		if (dob == actor) {
			send_out("You can't fight yourself.\n");
		} else if (dob->query_environment() != actor->query_environment()) {
			send_out("You can only attack someone in the same room.\n");
		} else {
			do_attack(actor, dob);
		}
		break;

	case T_ARRAY:
		send_out("You can only attack one foe at a time.\n");
	}
}
