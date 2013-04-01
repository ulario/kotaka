/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <game/paths.h>
#include <catalog/paths.h>

void on_create(object obj)
{
	object door1;
	object door2;
	object doormaster;

	mapping data;

	doormaster = CATALOGD->lookup_object("building:parts:door");

	door1 = clone_object(THING);
	door1->add_archetype(doormaster);
	door1->move(obj->query_environment());
	door1->set_x_position(2.0);
	door1->set_y_position(3.0);

	door2 = clone_object(THING);
	door2->add_archetype(doormaster);
	door2->move(obj);
	door2->set_x_position(2.0);
	door2->set_y_position(4.0);

	data = obj->query_property("data");
	data["doors"] = ({ door1, door2 });
	obj->set_property("data", data);
}
