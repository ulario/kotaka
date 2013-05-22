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

/* verb phrase: ({ "V", NP }) */
/* prep phrase: ({ "P", NP }) */
/* evoke phrase: ({ "E", phrase }) */

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
}

mixed *parse(string input)
{
	return parse_string(grammar, input);
}

mixed *statement_vp_ppl_evoke(mixed *input)
{
	string evoke;
	int sz;

	sz = sizeof(input);

	if (sz > 2 && input[sz - 2] == ",") {
		input = input[0 .. sz - 3] + ({ input[sz - 1] });
		sz--;
	}

	evoke = input[sz - 1];

	return input[0 .. sz - 2] + ({ ({ "E", evoke[1 .. strlen(evoke) - 2] }) });
}

mixed *vp_verb(mixed *input)
{
	return ({ ({ "V", input[0], nil }) });
}

mixed *vp_verb_np(mixed *input)
{
	return ({ ({ "V", input[0], input[1] }) });
}

mixed *pp_prep_np(mixed *input)
{
	return ({ ({ "P", input[0], input[1] }) });
}

mixed *pp_prep(mixed *input)
{
	return ({ ({ "P", input[0], nil }) });
}

mixed *np_adjl_noun(mixed *input)
{
	return ({ ({ "N", input[..] }) });
}
