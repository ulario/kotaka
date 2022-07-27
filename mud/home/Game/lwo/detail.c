/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020, 2021, 2022  Raymond Jennings
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
inherit "/lib/string/case";

string *snouns;
string *pnouns;
string *adjectives;

mapping descriptions;

private void downcase_snouns()
{
	int sz;

	for (sz = sizeof(snouns); --sz >= 0; ) {
		snouns[sz] = to_lower(snouns[sz]);
	}
}

private void downcase_pnouns()
{
	int sz;

	for (sz = sizeof(pnouns); --sz >= 0; ) {
		pnouns[sz] = to_lower(pnouns[sz]);
	}
}

private void downcase_adjectives()
{
	int sz;

	for (sz = sizeof(snouns); --sz >= 0; ) {
		adjectives[sz] = to_lower(adjectives[sz]);
	}
}

/* creator */

static void create(int clone)
{
	if (clone) {
		snouns = ({ });
		pnouns = ({ });
		adjectives = ({ });
		descriptions = ([ ]);
	}
}

/* nouns */

void add_snoun(string snoun)
{
	snouns |= ({ to_lower(snoun) });
}

void remove_snoun(string snoun)
{
	snouns -= ({ to_lower(snoun) });
}

void set_snouns(string *new_snouns)
{
	snouns = ({ }) | new_snouns - ({ nil });
}

string *query_snouns()
{
	return snouns[..];
}

void add_pnoun(string pnoun)
{
	pnouns |= ({ to_lower(pnoun) });
}

void remove_pnoun(string pnoun)
{
	pnouns -= ({ to_lower(pnoun) });
}

void set_pnouns(string *new_pnouns)
{
	pnouns = ({ }) | new_pnouns - ({ nil });
}

string *query_pnouns()
{
	return pnouns[..];
}

/* adjectives */

void add_adjective(string adjective)
{
	adjectives |= ({ to_lower(adjective) });
}

void remove_adjective(string adjective)
{
	adjectives -= ({ to_lower(adjective) });
}

void set_adjectives(string *new_adjectives)
{
	adjectives = ({ }) | new_adjectives - ({ nil });
}

string *query_adjectives()
{
	return adjectives[..];
}

/* descriptions */

string *query_descriptions()
{
	return map_indices(descriptions);
}

void set_description(string description, string text)
{
	descriptions[description] = text;
}

string query_description(string description)
{
	return descriptions[description];
}

/* saveload */

mapping save()
{
	return ([
		"snouns": sizeof(snouns) ? snouns[..] : nil,
		"pnouns": sizeof(pnouns) ? pnouns[..] : nil,
		"adjectives": sizeof(snouns) ? adjectives[..] : nil,
		"descriptions": map_sizeof(descriptions) ? descriptions : nil
	]);
}

void load(mapping data)
{
	mixed v;

	snouns = (v = data["snouns"]) ? ({ }) | v : ({ });
	pnouns = (v = data["pnouns"]) ? ({ }) | v : ({ });
	adjectives = (v = data["adjectives"]) ? ({ }) | v : ({ });
	descriptions = (v = data["descriptions"]) ? ([ ]) + v : ([ ]);
}
