/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020  Raymond Jennings
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

inherit "/lib/string/sprint";
inherit "../support";

static string thing_form(object obj)
{
	string buffer;

	buffer = "<p>Fun little boxes:</p>\n";

	buffer += "<form action=\"object.lpc?obj=" + object2string(obj) + "\" method=\"post\">\n";
	buffer += "Local mass: <input type=\"text\" name=\"localmass\" value=\"" + mixed_sprint(obj->query_local_mass()) + "\"/>\n";
	buffer += "<input type=\"submit\" value=\"change local mass\" />\n";
	buffer += "</form>\n";

	return oinfobox("Configuration", 2, buffer);
}
