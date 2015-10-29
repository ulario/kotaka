/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
float rnd()
{
	mixed low;
	mixed high;

	low = random(1 << 18);
	high = random(1 << 18);

	return ldexp((float)low, -36) + ldexp((float)high, -18);
}

float bell_rnd(int degree)
{
	float sum;
	int i;

	for (i = 0; i < degree; i++) {
		sum += rnd();
	}

	return sum / (float)degree;
}

float pi()
{
	return atan(1.0) * 4.0;
}

int dice(int count, int faces)
{
	int sum;

	while (count) {
		sum += random(faces) + 1;
		count--;
	}

	return sum;
}
