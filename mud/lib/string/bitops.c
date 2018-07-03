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

inherit "trim";

string string_truncate(string line, int bits)
{
	int bytes;
	int mask;

	bytes = (bits + 7) >> 3;
	mask = 0xFF >> ((8 - bits) & 0x7);

	if (strlen(line) > bytes) {
		line = line[0 .. bytes - 1];
	} else while (strlen(line) < bytes) {
		line += "\000";
	}

	if (strlen(line) == bytes) {
		line[bytes - 1] &= mask;
	}

	return rtrim_null(line);
}

string string_rshift(string a, int bits)
{
	int buf;
	int index;

	a = a[bits / 8 ..];
	bits %= 8;

	buf = a[0];

	for (index = 0; index < strlen(a); index++) {
		buf |= ((index + 1) < strlen(a) ? a[index + 1] : 0) << 8;
		a[index] = (buf >> bits) & 255;
		buf >>= 8;
	}

	return rtrim_null(a);
}

string string_lshift(string a, int bits)
{
	a = nulls(bits / 8) + a;
	bits %= 8;

	return rtrim_null(string_rshift("\000" + a, 8 - bits));
}

string string_and(string a, string b)
{
	int i;

	if (strlen(a) < strlen(b)) {
		string c;
		c = a;
		a = b;
		b = c;
	}

	for (i = 0; i <= strlen(b); i++) {
		b[i] &= a[i];
	}

	return b;
}

string string_or(string a, string b)
{
	int i;

	if (strlen(a) < strlen(b)) {
		string c;
		c = a;
		a = b;
		b = c;
	}

	for (i = 0; i <= strlen(b); i++) {
		a[i] |= b[i];
	}

	return a;
}

string string_xor(string a, string b)
{
	int i;

	if (strlen(a) < strlen(b)) {
		string c;
		c = a;
		a = b;
		b = c;
	}

	for (i = 0; i <= strlen(b); i++) {
		a[i] ^= b[i];
	}

	return a;
}

string string_nand(string a, string b)
{
	int lim;
	int i;

	lim = strlen(a);

	if (lim > strlen(b)) {
		lim = strlen(b);
	}

	for (i = 0; i < lim; i++) {
		a[i] &= ~b[i];
	}

	return a;
}
