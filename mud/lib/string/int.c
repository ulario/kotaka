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
string pack_int(int value, varargs int be)
{
	string out;

	out = "    ";

	if (be) {
		out[0] = value & 0xFF;
		out[1] = (value >> 8) & 0xFF;
		out[2] = (value >> 16) & 0xFF;
		out[3] = (value >> 24) & 0xFF;
	} else {
		out[3] = value & 0xFF;
		out[2] = (value >> 8) & 0xFF;
		out[1] = (value >> 16) & 0xFF;
		out[0] = (value >> 24) & 0xFF;
	}
	return out;
}

int unpack_int(string str, varargs int be)
{
	int value;

	if (be) {
		value |= str[0];
		value |= str[1] << 8;
		value |= str[2] << 16;
		value |= str[3] << 24;
	} else {
		value |= str[3];
		value |= str[2] << 8;
		value |= str[1] << 16;
		value |= str[0] << 24;
	}

	return value;
}

string vpack_int(int value, varargs int be)
{
	string out;
	string char;

	char = " ";
	out = "";

	do {
		char[0] = (value & 0x7F) | 0x80;

		if (be) {
			out = char + out;
		} else {
			out = out + char;
		}

		value >>= 7;
	} while (value);

	out[strlen(out) - 1] &= 0x7F;

	return out;
}

int vunpack_int(string str, varargs int be)
{
	string char;
	int value;
	int index;

	char = " ";

	if (be) {
		for (index = 0; index < strlen(str); index++) {
			value <<= 7;
			char[0] = str[index];
			value = char[0] & 0x7F;
		}
	} else {
		for (index = strlen(str) - 1; index >= 0; index--) {
			value <<= 7;
			char[0] = str[index];
			value = char[0] & 0x7F;
		}
	}

	return value;
}
