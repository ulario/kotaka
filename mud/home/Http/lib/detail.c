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

inherit "support";
inherit "/lib/string/sprint";

private string detail_box(object obj, string detail)
{
	string buffer;
	string *descriptions;
	string *words;
	int sz;

	buffer = "<p>";

	if (sizeof(words = obj->query_local_snouns(detail))) {
		buffer += "Singular nouns: " + implode(words, ", ") + "<br />\n";
	}

	if (sizeof(words = obj->query_local_pnouns(detail))) {
		buffer += "Plural nouns: " + implode(words, ", ") + "<br />\n";
	}

	if (sizeof(words = obj->query_local_adjectives(detail))) {
		buffer += "Adjectives: " + implode(words, ", ") + "<br />\n";
	}

	descriptions = obj->query_local_descriptions(detail);
	sz = sizeof(descriptions);

	if (sz) {
		int i;

		buffer += "</p>\n<p>";

		for (i = 0; i < sz; i++) {
			string description;

			description = descriptions[i];

			buffer += description + ": " + obj->query_description(detail, description) + "<br />\n";
		}

		buffer += "</p>\n";
	}

	if (detail) {
		return oinfobox(detail, 3, buffer);
	} else {
		return oinfobox("(default)", 3, buffer);
	}
}

static string detail_text(object obj, string *details)
{
	string subbuffer, buffer, *vetoes;
	int sz, i;

	sz = sizeof(details);

	vetoes = obj->query_vetoed_details();
	subbuffer = implode(vetoes, ", ");

	if (subbuffer == "") {
		buffer = "<h2>Details</h2>";
	} else {
		buffer = oinfobox("Details", 2, subbuffer);
	}

	for (i = 0; i < sz; i++) {
		buffer += detail_box(obj, details[i]);
	}

	return buffer;
}
