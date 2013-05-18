/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013 Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

string grammar;

private void decomment();

static void create()
{
	grammar = read_file("~/data/english.dpd");

	decomment();
}

void upgrading()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	grammar = read_file("~/data/english.dpd");

	decomment();
}

private void decomment()
{
	string *lines;
	int sz;

	lines = explode(grammar, "\n");
	lines -= ({ "" });

	for (sz = sizeof(lines) - 1; sz >= 0; sz--) {
		if (lines[sz][0] == '#') {
			lines[sz] = nil;
		}
	}

	lines -= ({ nil });

	grammar = implode(lines, "\n");

	LOGD->post_message("english", LOG_DEBUG, "Grammar:\n" + grammar);
}

mixed *parse(string input)
{
	return parse_string(grammar, input);
}

/*
statement: iclause ? statement_iclause
statement: iclause ',' evoke ? statement_iclause_evoke
statement: iclause evoke ? statement_iclause_evoke
*/
mixed *statement_iclause(mixed *input)
{
	return ({ "S", input[0], nil });
}

mixed *statement_iclause_evoke(mixed *input)
{
	string evoke;

	evoke = input[sizeof(input) - 1];

	return ({ "S", input[0], evoke[1 .. strlen(evoke) - 2] });
}

/*
iclause: verb ? iclause_verb
iclause: verb np ? iclause_verb_np
iclause: verb pp ? iclause_verb_pp
iclause: verb np pp ? iclause_verb_np_pp
*/
mixed *iclause_verb(mixed *input)
{
	return ({ ({ "V", input[0] }) });
}

mixed *iclause_verb_np(mixed *input)
{
	return ({ ({ "V", input[0], input[1] }) });
}

mixed *iclause_verb_pp(mixed *input)
{
	return ({ ({ "V", input[0], input[1] }) });
}

mixed *iclause_verb_np_pp(mixed *input)
{
	return ({ ({ "V", input[0], input[1], input[2] }) });
}

/*
pp: prep np ? pp_prep_np
np: adj noun ? np_adjl_noun
np: article adj noun ? np_article_adjl_noun
*/
mixed *pp_prep_np(mixed *input)
{
	return ({ ({ "P", input[0], input[1] }) });
}

mixed *np_adjl_noun(mixed *input)
{
	int sz;

	sz = sizeof(input);

	return ({ ({ "N", input[sz - 1], nil, input[0 .. sz - 2] }) });
}

mixed *np_article_adjl_noun(mixed *input)
{
	int sz;

	sz = sizeof(input);

	return ({ ({ "N", input[sz - 1], input[0], input[1 .. sz - 2] }) });
}
