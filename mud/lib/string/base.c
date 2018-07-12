/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
string bin(int val)
{
	int index;
	string out;
	out = "";

	if (!val) {
		return "0";
	}

	while (val) {
		int bit;

		bit = val & 0x1;
		val >>= 1;

		out = " " + out;

		out[0] = '0' + bit;
	}

	return out;
}

string hex(int val)
{
	int index;
	string out;
	out = "";

	if (!val) {
		return "0";
	}

	while (val) {
		int nibble;

		nibble = val & 0xF;
		val >>= 4;

		out = " " + out;

		if (nibble > 9) {
			out[0] = 'a' + (nibble - 10);
		} else {
			out[0] = '0' + nibble;
		}
	}

	return out;
}
