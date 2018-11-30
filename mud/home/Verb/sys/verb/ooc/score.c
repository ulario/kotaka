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

void main(object actor, mapping roles)
{
	object user;
	object body;
	object painter;
	object gc;
	int hp, mp, ep, xp;
	int i, j;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	user = query_user();
	body = user->query_body();

	hp = random(21);
	mp = random(21);
	ep = random(21);
	xp = random(21);

	painter = new_object(LWO_PAINTER);

	send_out("     Phy   Men   Soc   Spi\n");
	send_out("Pow");
	for (i = 0; i < 4; i++) {
		send_out(" " + make_abi_number(random(11) + 5, random(11) + 5));
	}
	send_out("\n");
	send_out("Fin");
	for (i = 0; i < 4; i++) {
		send_out(" " + make_abi_number(random(11) + 5, random(11) + 5));
	}
	send_out("\n");
	send_out("Res");
	for (i = 0; i < 4; i++) {
		send_out(" " + make_abi_number(random(11) + 5, random(11) + 5));
	}
	send_out("\n\n");

	send_out("HP [" + make_bar(hp, 20, 20) + "]\n");
	send_out("MP [" + make_bar(mp, 20, 20) + "]\n");
	send_out("EP [" + make_bar(ep, 20, 20) + "]\n");
	send_out("XP [" + make_xp_bar(hp, 20, 20) + "]\n\n");
}