/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

object linebuf;

static void create(int clone)
{
	if (clone) {
		::create();
		linebuf = new_object("~/lwo/linebuf");
	}
}

private void pour()
{
	string line;
	int count;

	count = 0;

	for (;;) {
		if (linebuf->empty() || count >= 20) {
			break;
		}

		line = linebuf->getline();
		send_out(line + "\n");

		count++;
	}

	if (linebuf->empty()) {
		pop_state();
	} else {
		send_out("(more ...)");
	}
}

void set_text(string text)
{
	linebuf->load(text);
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	pour();
}

void receive_in(string str)
{
	ACCESS_CHECK(previous_object() == query_user());

	pour();
}
