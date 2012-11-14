/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
float *measure_delta(object a, object b)
{
	object cc;

	float dx, dy, dz;

	cc = SUBD->query_common_container(a, b);

	if (!cc) {
		error("Objects not on same plane of existence");
	}

	while (b != cc) {
		dx += b->query_x_position();
		dy += b->query_y_position();
		dz += b->query_z_position();
		b = b->query_environment();
	}

	while (a != cc) {
		dx -= a->query_x_position();
		dy -= a->query_y_position();
		dz -= a->query_z_position();
		a = a->query_environment();
	}

	return ({ dx, dy, dz });
}
