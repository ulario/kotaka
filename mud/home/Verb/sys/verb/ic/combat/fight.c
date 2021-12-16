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
inherit "~/lib/ic";

string *query_parse_methods()
{
	return ({ "english" });
}

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

int query_attack_bonus(object attacker)
{
	int sz, bonus;
	object *inv;

	inv = attacker->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		object obj;

		obj = inv[sz];

		if (obj->query_property("is_wielded")) {
			bonus += obj->query_property("attack_value");
		}
	}

	return bonus;
}

int query_defense_bonus(object defender)
{
	int sz, bonus;
	object *inv;

	inv = defender->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		object obj;

		obj = inv[sz];

		if (obj->query_property("is_worn")) {
			bonus += obj->query_property("defense_value");
		}
	}

	return bonus;
}

void main(object actor, mapping roles)
{
	mixed dob;
	string look;
	object from;
	object arena;

	arena = actor->query_environment();

	if (!arena) {
		send_out("It's going to be hard to fight someone without a field of battle.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		object *initiative;

		initiative = arena->query_property("initiative");

		if (!initiative || !sizeof(initiative)) {
			send_out("There isn't a fight in progress for you to join here.\n");
			return;
		}

		if (sizeof(initiative & ({ actor }) )) {
			send_out("You're already in a fight!\n");
			return;
		}

		initiative |= ({ actor });
		arena->set_property("initiative", initiative);

		send_out("You join the fight.\n");
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
			object *initiative;
			object achar, aliv;
			object dchar, dliv;
			int damage;

			if (!actor->query_living_lwo()) {
				break;
			}

			achar = actor->query_character_lwo();

			if (!achar) {
				send_out("You are not a character.\n");
				break;
			}

			aliv = actor->query_living_lwo();

			if (!aliv) {
				send_out("You're dead, Jim.\n");
				break;
			}

			dchar = dob->query_character_lwo();

			if (!dchar) {
				send_out("That is not a character.\n");
				break;
			}

			dliv = dob->query_living_lwo();

			if (!dliv) {
				send_out("It's dead, Jim\n");
				break;
			}

			initiative = arena->query_property("initiative");

			if (!initiative) {
				emit_from(actor, actor, ({ " pick", " picks" }), " a fight!");

				initiative = ({ });
			} else {
				if (initiative[0] != actor) {
					send_out("It's not your turn.\n");
					break;
				}
			}

			initiative = ({ actor }) | initiative | ({ dob });

			damage = achar->query_attack() + query_attack_bonus(actor)
				- dchar->query_defense() + query_defense_bonus(dob);

			if (damage <= 0) {
				send_out("Your attack is ineffective\n");
				emit_from(actor, actor, " harmlessly ", ({ "hit", "hits" }), " ", dob, ".");
			} else if (dliv->subtract_hp(damage) <= 0) {
				emit_from(actor, actor, " ", ({ "kill", "kills" }), " ", dob, "!");
				emit_to(actor, actor, "You fatally deal " + damage + " points of damage!");
				emit_to(dob, dob, "You suffer " + damage + " points of damage and die!");
				"~Action/sys/action/combat/die"->action( ([ "actor": dob ]) );
			} else {
				emit_from(actor, actor, " ", ({ "hit", "hits" }), " ", dob, ".");
				emit_to(actor, actor, "You deal " + damage + " points of damage.");
				emit_to(dob, dob, "You suffer " + damage + " points of damage.");
			}

			if (actor && !actor->query_living_lwo()) {
				initiative -= ({ actor });
			}

			if (dob && !dob->query_living_lwo()) {
				initiative -= ({ dob });
			}

			switch(sizeof(initiative)) {
			case 0:
				emit_from(actor, "Stalemate, nobody survived.");
				arena->set_property("initiative", nil);
				break;

			case 1:
				emit_from(actor, actor, ({ " are", " is" }), " the last one standing.");
				arena->set_property("initiative", nil);
				break;

			default:
				initiative -= ({ actor });
				initiative |= ({ actor });
				arena->set_property("initiative", initiative);
				emit_from(initiative[0], "It is now ", initiative[0], "'s turn.");
			}
		}
		break;

	case T_ARRAY:
		send_out("You can only attack one foe at a time.\n");
	}
}
