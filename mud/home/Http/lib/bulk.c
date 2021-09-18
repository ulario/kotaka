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

static string bulk_text(object obj)
{
	string buffer;

	if (obj->query_virtual()) {
		buffer = "Virtual<br />\n";
	} else {
		if (obj->query_flexible()) {
			buffer += "Flexible<br />\n";
		}

		buffer = "<span style=\"white-space: no wrap;\">\n";

		buffer += "Mass: " + mixed_sprint(obj->query_mass()) + " kg<br />\n";
		buffer += "Density: " + mixed_sprint(obj->query_density()) + " kg/l<br />\n";
		buffer += "Capacity: " + mixed_sprint(obj->query_capacity()) + " m^3<br />\n";
		buffer += "Max mass: " + mixed_sprint(obj->query_max_mass()) + " kg<br />\n</br />\n";

		if (obj->query_archetype()) {
			buffer += "Local mass: x" + mixed_sprint(obj->query_local_mass()) + "<br />\n";
			buffer += "Local density: x" + mixed_sprint(obj->query_local_density()) + "<br />\n";
			buffer += "Local capacity: x" + mixed_sprint(obj->query_local_capacity()) + "<br />\n";
			buffer += "Local max mass: x" + mixed_sprint(obj->query_local_max_mass()) + "\n";
		} else {
			buffer += "No archetype\n";
		}

		buffer += "</span>\n";
	}

	return oinfobox("Bulk", 3, buffer);
}
