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

string *snouns;
string *pnouns;
string *adjectives;
mapping descriptions;

string *query_snouns()
{
	return snouns ? snouns[..] : ({ });
}

string *query_pnouns()
{
	return pnouns ? pnouns[..] : ({ });
}

string *query_adjectives()
{
	return adjectives ? adjectives[..] : ({ });
}

string query_description(string type)
{
	return descriptions ? descriptions[type] : nil;
}

void set_snouns(string *new_snouns)
{
	snouns = new_snouns[..];
}

void set_pnouns(string *new_pnouns)
{
	pnouns = new_pnouns[..];
}

void set_adjectives(string *new_adjectives)
{
	adjectives = new_adjectives[..];
}

void set_description(string type, string description)
{
	if (!descriptions) {
		descriptions = ([ ]);
	}

	descriptions[type] = description;

	if (!map_sizeof(descriptions)) {
		descriptions = nil;
	}
}
