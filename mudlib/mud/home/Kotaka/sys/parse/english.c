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
#include <kotaka/log.h>

string grammar;

void decomment()
{
	string *lines;

	int i;
	int sz;

	lines = explode(grammar, "\n");
	sz = sizeof(lines);

	for (i = 0; i < sz; i++) {
		if (lines[i] == "") {
			lines[i] = nil;
		} else if (lines[i][0] == '#') {
			lines[i] = nil;
		}
	}

	lines -= ({ nil });
	grammar = implode(lines, "\n");
}

static void create()
{
	::create();

	LOGD->post_message("parse", LOG_INFO, "Loading grammar");
	grammar = read_file("~/data/parse/english.dpd");

	decomment();
}

void upgrade()
{
	LOGD->post_message("parse", LOG_INFO, "Reloading grammar");
	grammar = read_file("~/data/parse/english.dpd");

	decomment();
}

mixed parse(string input)
{
	return parse_string(grammar, input);
}

mixed *vp_verb(mixed *input)
{
	return ({ ({ "V", input[0], nil }) });
}

mixed *vp_verb_np(mixed *input)
{
	return ({ ({ "V", input[0], input[1] }) });
}

mixed *pp_prep(mixed *input)
{
	return ({ ({ "P", input[0], nil }) });
}

mixed *pp_prep_np(mixed *input)
{
	return ({ ({ "P", input[0], input[1] }) });
}

mixed *np_oart_oadjc_noun(mixed *input)
{
	return ({ ({ "N", input[0], input[1], input[2] }) });
}

mixed *oart(mixed *input)
{
	return ({ nil });
}

mixed *oart_art(mixed *input)
{
	return ({ input[0] });
}

mixed *oadjc(mixed *input)
{
	return ({ ({ }) });
}

mixed *oadjc_adjl(mixed *input)
{
	return ({ input });
}

mixed *noun_word(mixed *input)
{
}

mixed *verb_word(mixed *input)
{
}

mixed *prep_word(mixed *input)
{
}
