/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020, 2021  Raymond Jennings
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

private string primitive_brief(object thing)
{
	string brief;

	thing->patch_detail();

	brief = thing->query_description(nil, "brief");

	if (brief) {
		return brief;
	}

	brief = thing->query_property("id");

	if (brief) {
		return brief;
	}

	return "nondescript object";
}

string generate_brief_proper(object thing)
{
	string brief;

	brief = primitive_brief(thing);

	return brief;
}

string generate_brief_definite(object thing)
{
	if (thing->query_property("is_proper")) {
		return generate_brief_proper(thing);
	}

	return "the " + primitive_brief(thing);
}

string generate_brief_indefinite(object thing)
{
	string brief;

	if (thing->query_property("is_proper")) {
		return generate_brief_proper(thing);
	}

	if (thing->query_property("is_definite")) {
		return generate_brief_definite(thing);
	}

	brief = primitive_brief(thing);

	if (strlen(brief) > 0) {
		switch(to_lower(brief)[0]) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			return "an " + brief;
		}
	}

	return "a " + brief;
}

string generate_list(string *phrases)
{
	int sz;

	switch(sizeof(phrases)) {
	case 0:
		return nil;
	case 1:
		return phrases[0];
	case 2:
		return phrases[0] + " and " + phrases[1];
	default:
		sz = sizeof(phrases);
		return implode(phrases[0 .. sz - 2], ", ") + ", and " + phrases[sz - 1];
	}
}
