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
	int nx, ny, nz;
	object common;
	object new_env;

	new_env = query_environment();

	if (!old_env || !new_env) {
		return;
	}

	common = THING_SUBD->query_common_container(old_env, new_env);

	if (!common) {
		nx = 0;
		ny = 0;
		nz = 0;
	} else {
		nx = query_x_position();
		ny = query_y_position();
		nz = query_z_position();

		for (; old_env != common; old_env = old_env->query_environment()) {
			nx += old_env->query_x_position();
			ny += old_env->query_y_position();
			nz += old_env->query_z_position();
		}

		for (; new_env != common; new_env = new_env->query_environment()) {
			nx -= new_env->query_x_position();
			ny -= new_env->query_y_position();
			nz -= new_env->query_z_position();
		}
	}

	set_x_position(nx);
	set_y_position(ny);
	set_z_position(nz);
}

void xyz_convert()
{
	"~Kotaka/sys/channeld"->post_message("debug", "xyz", "Converted position for " + object_name(this_object()));
	set_x_position(query_x_position());
	set_y_position(query_y_position());
	set_z_position(query_z_position());
}
