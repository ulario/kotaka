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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>

inherit LIB_OBJECT;

/* keep it short and sweet and to the point for now */
float density;		/* kg/l */
float mass;		/* kg */

static void create()
{
	/* be sensible, one kilogram */
	density = 1.0;
	mass = 1.0;
}

/* kilograms per liter */
void set_density(float new_density)
{
	if (new_density <= 0.0) {
		error("Invalid density");
	}

	density = new_density;
}

/* kilograms */
void set_mass(float new_mass)
{
	if (new_mass < 0.0) {
		error("Negative mass");
	}

	mass = new_mass;
}

/* kilograms per liter */
float query_density()
{
	return density;
}

/* kilograms */
float query_mass()
{
	return mass;
}

/* return volume in cubic meters */
float query_volume()
{
	return 1000.0 * mass / density;
}
