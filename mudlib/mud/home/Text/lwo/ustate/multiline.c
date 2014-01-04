/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/text.h>

inherit TEXT_LIB_USTATE;

string text;

static void create(int clone)
{
	if (clone) {
		::create();
		text = "";
	}
}

private void prompt()
{
	send_out(": ");
}

static void receive_in(string str)
{
	if (str == ".") {
		send_in(text);
		text = "";
		return;
	} else if (strlen(str) >= 2 && str[0 .. 1] == "..") {
		str = str[1 ..];
	}

	text += str;
	text += "\n";
}

static void input_over()
{
	prompt();
}
