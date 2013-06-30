/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/paths.h>

private string name;

atomic void set_object_name(string new_name)
{
	CATALOGD->validate_name(new_name);

	if (name) {
		CATALOGD->remove_object(name);
		name = nil;
	}

	name = new_name;

	if (name) {
		CATALOGD->add_object(name, this_object());
	}
}

string query_object_name()
{
	return name;
}
