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
	return "Devour";
}

string *query_help_contents()
{
	return ({
		"Consume the fallen body of your foe in battle.",
		"You will recover HP equal to 1/8th of the meal's max HP.",
		"WARNING: Only works on dead combatants."
	});
}

atomic private void do_devour(object actor, object dob)
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

	if (dliv) {
		send_out("It's alive!\n");
		return;
	}

	aliv->add_hp(dchar->query_max_hp() / 8);

	emit_from(actor, actor, " hungrily ", ({ "devour", "devours" }), " ", dob, ".");
	dob->self_destruct();
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

	if (!actor->query_living_lwo()) {
		send_out("Alas, the dead have no appetite.\n");
	}

	dob = roles["dob"];

	if (!dob) {
		send_out("Devour what?\n");
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
			send_out("You can't devour yourself.\n");
		} else if (dob->query_environment() != actor->query_environment()) {
			send_out("You can only attack someone in the same room.\n");
		} else {
			do_devour(actor, dob);
		}
		break;

	case T_ARRAY:
		send_out("You can only attack one foe at a time.\n");
	}
}
