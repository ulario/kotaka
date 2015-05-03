/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2003, 2013, 2014, 2015  Raymond Jennings
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

object query_environment();
mixed query_local_property(string name);
void set_local_property(string name, mixed value);

void set_x_position(int new_xpos)
{
	set_local_property("pos_x", new_xpos ? new_xpos : nil);
}

void set_y_position(int new_ypos)
{
	set_local_property("pos_y", new_ypos ? new_ypos : nil);
}

void set_z_position(int new_zpos)
{
	set_local_property("pos_z", new_zpos ? new_zpos : nil);
}

void set_x_size(int new_xsize)
{
	set_local_property("size_x", new_xsize ? new_xsize : nil);
}

void set_y_size(int new_ysize)
{
	set_local_property("size_y", new_ysize ? new_ysize : nil);
}

void set_z_size(int new_zsize)
{
	set_local_property("size_z", new_zsize ? new_zsize : nil);
}

int query_x_position()
{
	mixed xp;

	xp = query_local_property("pos_x");

	return xp ? xp : 0;
}

int query_y_position()
{
	mixed yp;

	yp = query_local_property("pos_y");

	return yp ? yp : 0;
}

int query_z_position()
{
	mixed zp;

	zp = query_local_property("pos_z");

	return zp ? zp : 0;
}

int query_x_size()
{
	mixed xs;

	xs = query_local_property("size_x");

	return xs ? xs : 0;
}

int query_y_size()
{
	mixed ys;

	ys = query_local_property("size_y");

	return ys ? ys : 0;
}

int query_z_size()
{
	mixed zs;

	zs = query_local_property("size_z");

	return zs ? zs : 0;
}

void clear_xyz()
{
	set_local_property("pos_x", nil);
	set_local_property("pos_y", nil);
	set_local_property("pos_z", nil);
}

static void xyz_move_notify(object old_env)
{
	mixed nx, ny, nz;
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
