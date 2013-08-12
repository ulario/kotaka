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
#include <kotaka/paths/thing.h>

inherit "~Thing/lib/thing";

private int xpos, ypos, zpos;

/*********************/
/* Position handling */
/*********************/

static void move_notify(object old_env)
{
	object common;
	object new_env;

	new_env = query_environment();

	if (!old_env || !new_env) {
		return;
	}

	common = THING_SUBD->query_common_container(old_env, new_env);

	if (!common) {
		xpos = 0;
		ypos = 0;
		zpos = 0;
		return;
	}

	for (; old_env != common; old_env = old_env->query_environment()) {
		xpos += old_env->query_x_position();
		ypos += old_env->query_y_position();
		zpos += old_env->query_z_position();
	}

	for (; new_env != common; new_env = new_env->query_environment()) {
		xpos -= new_env->query_x_position();
		ypos -= new_env->query_y_position();
		zpos -= new_env->query_z_position();
	}
}

void set_x_position(int new_xpos)
{
	xpos = new_xpos;
}

void set_y_position(int new_ypos)
{
	ypos = new_ypos;
}

void set_z_position(int new_zpos)
{
	zpos = new_zpos;
}

int query_x_position()
{
	return xpos;
}

int query_y_position()
{
	return ypos;
}

int query_z_position()
{
	return zpos;
}
