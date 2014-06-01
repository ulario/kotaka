/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2014  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kernel/user.h>

inherit LIB_SYSTEM_USER;

static void create(int clone)
{
}

int login(string str)
{
	connection(previous_object());

	message("HTTP/1.1 404 No handler\n");
	message("Connection: close\n");
	message("\n");
	message("<h1>No handler</h1>\n");
	message("<p>There is no handler for that path.</p>\n");

	return MODE_DISCONNECT;
}

void logout(int quit)
{
	destruct_object(this_object());
}

int receive_message()
{
	return MODE_DISCONNECT;
}

int message_done()
{
	return MODE_DISCONNECT;
}
