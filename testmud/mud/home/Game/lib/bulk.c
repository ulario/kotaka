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

inherit LIB_OBJECT;

/* for all derived calculations, object is presumed to be a cube */

/* two of three required */
float density;		/* kg/m^3 */
float mass;		/* kg */
float volume;		/* m^3 (can be derived from area or length) */

/* optional, derived from volume, can derive volume */
float length;		/* m */
float area;		/* m^2 */

int flexible;
int container;

float mass_capacity;	/* kg, optional, default = cvolume * density */
float volume_capacity;	/* m^3, optional, default = 0.9 * volume */
float length_capacity;	/* m, optional, default = 0.9 * length */
float area_capacity;	/* m^2, optional, default = cvolume / clength */

static void create()
{
	mass = -1.0;
	density = -1.0;
	volume = -1.0;
	area = -1.0;
	length = -1.0;
}

void set_base_density(float new_density)
{
	density = new_density;
}

void set_base_mass(float new_mass)
{
	mass = new_mass;
}

void set_base_volume(float new_volume)
{
	volume = new_volume;
}

void set_base_area(float new_area)
{
	area = new_area;
}

void set_base_length(float new_length)
{
	length = new_length;
}

/*
Global derivation
	mass = volume * density (if density defined and volume derivable)
	density = mass / volume (if mass defined and volume derivable)
	volume = mass / density (if mass and density defined)

Volume derivation
	length * area (if area defined)
	length ^ 3 (if length defined)
	area ^ 3/2 (if area defined)

Area derivation
	volume / length (if volume defined)
	length ^ 2 (if length defined)

Length derivation
	volume / area (if volume and area defined)
	volume ^ 1/3 (if volume defined)
*/
