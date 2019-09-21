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
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;
inherit "/lib/string/char";

string *query_parse_methods()
{
	return ({ "raw" });
}

string make_bar(int cur, int max, int len)
{
	int pips;
	string buf;

	pips = (cur * len / max);

	if (cur * 3 < max) {
		buf = "\033[1;31m";
	} else if (cur * 3 < max * 2) {
		buf = "\033[1;33m";
	} else {
		buf = "\033[1;32m";
	}

	buf += chars('=', pips);
	buf += "\033[0m";
	buf += spaces(len - pips);

	return buf;
}

string make_xp_bar(int cur, int max, int len)
{
	int pips;
	string buf;

	pips = (cur * len / max);

	buf = "\033[1;36m";
	buf += chars('=', pips);
	buf += "\033[0m";
	buf += spaces(len - pips);

	return buf;
}

string from_number(int num)
{
	if (num > 99) {
		return "OV";
	} else if (num <= -10) {
		return "UN";
	} else if (num < 0) {
		return "" + num;
	} else if (num < 10) {
		return " " + num;
	} else {
		return "" + num;
	}
}

string make_abi_number(int cur, int max)
{
	string buf;

	buf = "\033[1m";

	if (cur < max) {
		buf += "\033[31m";
	} else if (cur > max) {
		buf += "\033[36m";
	}

	buf += from_number(cur);
	buf += "\033[37m/";
	buf += from_number(max);
	buf += "\033[0m";

	return buf;
}

string query_help_title()
{
	return "Score";
}

string *query_help_contents()
{
	return ({ "Shows a mockup of what a score command may give in the future for your character." });
}

void main(object actor, mapping roles)
{
	object achar;
	object aliv;

	int attack, defense, hp, max;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	achar = actor->query_character_lwo();

	if (!achar) {
		send_out("You are not a character.\n");
		return;
	}

	aliv = actor->query_living_lwo();

	if (!aliv) {
		send_out("Dead.\n");
		return;
	}

	attack = achar->query_attack();
	defense = achar->query_defense();
	max = achar->query_max_hp();
	hp = aliv->query_hp();

	send_out("HP: " + hp + "/" + max + "\n");
	send_out("Atk: " + attack + "\n");
	send_out("Def: " + defense + "\n");

	if (hp < max / 4) {
		send_out("You are seriously injured!\n");
	} else if (hp < max / 2) {
		send_out("You are hurt.\n");
	}
}
