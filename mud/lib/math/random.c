/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
	return ldexp((float)random(1 << 30), -30);
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

int dice(int count, int faces)
{
	int sum;

	while (count) {
		sum += random(faces) + 1;
		count--;
	}

	return sum;
}
