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

int char_is_whitespace(int char)
{
	if ((char == '\n') || (char == '\t') || (char == ' ')) {
		return 1;
	}

	return 0;
}

int char_to_lower(int char)
{
	if ((char <= 'Z') && (char >= 'A')) {
		char |= 0x20;
	}

	return char;
}

int char_to_upper(int char)
{
	if ((char <= 'z') && (char >= 'a'))
		char &= ~0x20;

	return char;
}
