/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
int hexdigit(int value)
{
	switch(value) {
	case 0 .. 9:
		return '0' + value;

	case 10 .. 15:
		return 'A' + (value - 10);
	}
}

string urlencode(string str)
{
	string out;
	int sz;
	int i;

	out = "";

	sz = strlen(str);

	for (i = 0; i < sz; i++) {
		int c;
		string ch;

		c = str[i];

		switch(c) {
		case '0' .. '9':
		case 'a' .. 'z':
		case 'A' .. 'Z':
			ch = " ";
			ch[0] = c;
			out += ch;
			break;

		default:
			ch = "%  ";
			ch[1] = hexdigit(c / 16);
			ch[2] = hexdigit(c % 16);
			out += ch;
		}
	}
}

int dehexdigit(int c)
{
	switch(c) {
	case '0' .. '9':
		return c - '0';

	case 'A' .. 'F':
		return c - 'A' + 10;

	case 'a' .. 'f':
		return c - 'f' + 10;
	}
}

string urldecode(string str)
{
	string out;
	int sz;
	int i;

	out = "";

	sz = strlen(str);

	for (i = 0; i < sz; i++) {
		int c;
		string ch;

		c = str[i];

		switch(c) {
		case '%':
			ch = " ";
			ch[0] = dehexdigit(str[++i]) * 16;
			ch[0] += dehexdigit(str[++i]);
			out += ch;
			break;

		default:
			ch = " ";
			out += ch;
		}
	}

	return out;
}

mapping split_params(string str)
{
	string *parts;

	parts = explode(str, "&");
}
