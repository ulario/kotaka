/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

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

	buf += STRINGD->chars('=', pips);
	buf += "\033[0m";
	buf += STRINGD->spaces(len - pips);

	return buf;
}

string make_xp_bar(int cur, int max, int len)
{
	int pips;
	string buf;

	pips = (cur * len / max);

	buf = "\033[1;36m";
	buf += STRINGD->chars('=', pips);
	buf += "\033[0m";
	buf += STRINGD->spaces(len - pips);

	return buf;
}

mixed **query_roles()
{
	return ({ });
}

void main(object actor, mapping roles)
{
	object user;
	object body;
	object painter;
	object gc;
	string hps, mps, eps, xps;
	int hp, mp, ep, xp;
	int i, j;

	user = query_user();
	body = user->query_body();

	hp = random(21);
	mp = random(21);
	ep = random(21);
	xp = random(21);

	painter = new_object(LWO_PAINTER);

	send_out("Str ---/--- Cha ---/---\n");
	send_out("Dex ---/--- Int ---/---\n");
	send_out("Con ---/--- Wis ---/---\n\n");

	send_out("HP [" + make_bar(hp, 20, 20) + "]\n");
	send_out("MP [" + make_bar(mp, 20, 20) + "]\n");
	send_out("EP [" + make_bar(ep, 20, 20) + "]\n");
	send_out("XP [" + make_xp_bar(hp, 20, 20) + "]\n\n");
}
