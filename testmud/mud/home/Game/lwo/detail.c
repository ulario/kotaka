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

/* use mappings, we need fast lookups */
mapping snames;
mapping pnames;
mapping adjectives;

/* we have data with each type */
mapping descriptions;

static void create(int clone)
{
}

/* singular names */

void clear_snames()
{
	snames = nil;
}

void add_sname(string sname)
{
	if (!snames) {
		snames = ([ ]);
	}

	snames[sname] = 1;
}

string *query_snames()
{
	return snames ? map_indices(snames) : ({ });
}

int test_sname(string sname)
{
	return snames ? !!snames[sname] : 0;
}

/* plural names */

void clear_pnames()
{
	pnames = nil;
}

void add_pname(string pname)
{
	if (!pnames) {
		pnames = ([ ]);
	}

	pnames[pname] = 1;
}

string *query_pnames()
{
	return pnames ? map_indices(pnames) : ({ });
}

int test_pname(string pname)
{
	return pnames ? !!pnames[pname] : 0;
}

/* adjectives */

void clear_adjectives()
{
	adjectives = nil;
}

void add_adjectives(string adjective)
{
	if (!adjectives) {
		adjectives = ([ ]);
	}

	adjectives[adjective] = 1;
}

string *query_adjectives()
{
	return adjectives ? map_indices(adjectives) : ({ });
}

int test_adjective(string adjective)
{
	return adjectives ? !!adjectives[adjective] : 0;
}

/* descriptions */

void clear_descriptions()
{
	descriptions = nil;
}

void add_description(string type, string value)
{
	if (!descriptions) {
		descriptions = ([ ]);
	}

	descriptions[type] = value;
}

void remove_description(string type)
{
	descriptions[type] = nil;
}

string query_descriptions()
{
	return descriptions[..];
}

string query_description(string type)
{
	return descriptions[type];
}
