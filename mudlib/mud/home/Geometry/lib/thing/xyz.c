/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/geometry.h>
#include <kotaka/paths/thing.h>
#include <kotaka/privilege.h>

inherit LIB_THING;

private int xpos, ypos, zpos;

void set_x_position(int new_xpos)
{
	xpos = 0;

	set_local_property("pos_x", new_xpos ? new_xpos : nil);
}

void set_y_position(int new_ypos)
{
	ypos = 0;

	set_local_property("pos_y", new_ypos ? new_ypos : nil);
}

void set_z_position(int new_zpos)
{
	zpos = 0;

	set_local_property("pos_z", new_zpos ? new_zpos : nil);
}

int query_x_position()
{
	mixed xp;
	xp = query_local_property("pos_x");

	return xpos ? xpos : (xp ? xp : 0);
}

int query_y_position()
{
	mixed yp;
	yp = query_local_property("pos_y");

	return ypos ? ypos : (yp ? yp : 0);
}

int query_z_position()
{
	mixed zp;
	zp = query_local_property("pos_z");

	return zpos ? zpos : (zp ? zp : 0);
}

static void move_notify(object old_env)
{
}

void xyz_convert()
{
	"~Kotaka/sys/channeld"->post_message("debug", "xyz", "Converted position for " + object_name(this_object()));
	set_x_position(query_x_position());
	set_y_position(query_y_position());
	set_z_position(query_z_position());
}
