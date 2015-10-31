/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/checkarg.h>

/* need to change this to a generic buffer */
/* need to be able to "charge up" the buffer */

string buf;

static void create(int clone)
{
	buf = "";
}

void load(string new_buf)
{
	CHECKARG(new_buf, 1, "load");
	buf = new_buf;
}

int empty()
{
	return buf == "";
}

string getline()
{
	string line;

	line = nil;

	if (sscanf(buf, "%s\n%s", line, buf)) {
		return line;
	} else if (buf != "") {
		line = buf;
		buf = "";
		return line;
	}
}
