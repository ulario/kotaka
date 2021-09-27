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

inherit "detail";
inherit "thing";
inherit "support";

static string query_ip();

static string object_text(object obj)
{
	string subbuffer;
	string buffer;
	string name;

	subbuffer = "<p>Object owner: " + obj->query_owner() + "<br />\n";

	if (name = obj->query_object_name()) {
		subbuffer += "Object name: " + name + "<br />\n";
	}

	if (query_ip() == "127.0.0.1") {
		subbuffer += "<a href=\"destruct.lpc?obj=" + urlencode(object2string(obj)) + "\">Destruct</a><br />\n";
	}

	buffer = oinfobox("Object", 1, subbuffer);

	if (obj <- "~Thing/lib/thing") {
		string *details;

		details = obj->query_local_details();

		if (sizeof(details)) {
			buffer += "<table>\n";
			buffer += "<tr>\n";
			buffer += "<td class=\"mainpane\">\n";
		}

		buffer += thing_text(obj);

		if (sizeof(details)) {
			buffer += "</td>\n";
			buffer += "<td class=\"subpane\">\n";
			buffer += detail_text(obj, details);
			buffer += "</td>\n";
			buffer += "</tr>\n";
			buffer += "</table>\n";
		}

		if (query_ip() == "127.0.0.1") {
			buffer += thing_form(obj);
		}
	}

	return buffer;
}
