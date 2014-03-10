/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2006, 2010, 2012, 2013, 2014  Raymond Jennings
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

string direction;
object destination;
object ret_exit;

void set_local_property(string name, mixed value);
mixed query_local_property(string name);

void set_direction(string new_direction)
{
	direction = nil;

	set_local_property("exit_direction", new_direction);
}

void set_destination(object new_destination)
{
	destination = nil;

	set_local_property("exit_destination", new_destination);
}

void set_return(object new_return)
{
	ret_exit = nil;

	set_local_property("exit_return", new_return);
}

string query_direction()
{
	if (direction) {
		return direction;
	}

	return query_local_property("exit_direction");
}

object query_destination()
{
	if (destination) {
		return destination;
	}

	return query_local_property("exit_destination");
}

object query_return()
{
	if (ret_exit) {
		return ret_exit;
	}

	return query_local_property("exit_return");
}

void exit_convert()
{
	set_return(query_return());
	set_destination(query_destination());
	set_direction(query_direction());
}
