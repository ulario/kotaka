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
inherit "/lib/sort";

string *snouns;
string *pnouns;
string *adjectives;

mapping descriptions;

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

/* patch */

void patch_detail_downcase()
{
	int sz;
	string *arr;
	string *ind;

	arr = ({ });

	for (sz = sizeof(snouns); --sz >= 0; ) {
		arr |= ({ snouns[sz] });
	}

	snouns = arr;
	qsort(snouns, 0, sizeof(snouns));

	arr = ({ });

	for (sz = sizeof(pnouns); --sz >= 0; ) {
		arr |= ({ pnouns[sz] });
	}

	pnouns = arr;
	qsort(pnouns, 0, sizeof(pnouns));

	arr = ({ });

	for (sz = sizeof(adjectives); --sz >= 0; ) {
		arr |= ({ adjectives[sz] });
	}

	adjectives = arr;
	qsort(adjectives, 0, sizeof(adjectives));

	ind = map_indices(descriptions);

	for (sz = sizeof(ind); --sz >= 0; ) {
		string desc;
		string ldesc;

		desc = ind[sz];
		ldesc = to_lower(desc);

		if (desc == ldesc) {
			/* it's already lowercase, leave it alone */
			continue;
		}

		if (descriptions[ldesc]) {
			/* the correct version already exists, delete the anomaly */
			descriptions[desc] = nil;
			continue;
		}

		/* the current version is the wrong case, migrate it */
		descriptions[ldesc] = descriptions[desc];
		descriptions[desc] = nil;
	}
}
