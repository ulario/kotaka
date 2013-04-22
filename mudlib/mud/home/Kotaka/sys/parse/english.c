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

mixed *iclause_verb(mixed *input)
{
	return ({ ({ "I", input[0], nil, nil }) });
}

mixed *iclause_verb_np(mixed *input)
{
	return ({ ({ "I", input[0], input[1], nil }) });
}

mixed *iclause_verb_np_pp(mixed *input)
{
	return ({ ({ "I", input[0], input[1], input[2] }) });
}

/*
iclause: verb ? iclause_verb
iclause: verb np ? iclause_verb_np
iclause: verb np pp ? iclause_verb_np_pp

pp: prep np ? pp_prep_np

np: adjl noun ? np_adjl_noun
np: art adjl noun ? np_art_adjl_noun
*/
mixed *pp_prep_np(mixed *input)
{
	return ({ ({ "P", input[0], input[1] }) });
}

mixed *np_adjl_noun(mixed *input)
{
	int sz;

	sz = sizeof(input);

	return ({ ({ "N", nil, input[0 .. sz - 2], input[sz - 1] }) });
}

mixed *np_art_adjl_noun(mixed *input)
{
	int sz;

	sz = sizeof(input);

	return ({ ({ "N", input[0], input[1 .. sz - 2], input[sz - 1] }) });
}
