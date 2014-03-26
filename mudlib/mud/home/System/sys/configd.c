/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2014  Raymond Jennings
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
#include <config.h>
#include <kernel/kernel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

static void create()
{
	make_dir("/config");
}

/* lets users and subsystems alike store stuff secretly */
string proper_path(string given, string user)
{
	return DRIVER->normalize_path(given, "/config/" + user);
}

void validate_path(string path, string user)
{
	if (path != "/config/" + user) {
		PERMISSION_CHECK(
			sscanf(path, "/config/" + user + "/%*s")
		);
	}
}

/* files */
string read_file(string file, varargs int offset, int size)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::read_file(file, offset, size);
}

int remove_file(string file)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::remove_file(file);
}

int rename_file(string from, string to)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	from = proper_path(from, user);
	validate_path(from, user);

	to = proper_path(to, user);
	validate_path(to, user);

	return ::rename_file(from, to);
}

int write_file(string file, string str, varargs int offset)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::write_file(file, str, offset);
}

mixed *file_info(string file)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::file_info(file);
}

/* void file_info(); */

/* directories */

mixed **get_dir(string path)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	path = proper_path(path, user);
	validate_path(path, user);

	return ::get_dir(path);
}

int make_dir(string path)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	path = proper_path(path, user);
	validate_path(path, user);

	return ::make_dir(path);
}

int remove_dir(string path)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	path = proper_path(path, user);
	validate_path(path, user);

	return ::remove_dir(path);
}
