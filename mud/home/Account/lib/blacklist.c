/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2022  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/paths/account.h>

static int is_control_garbage(string input)
{
	if (strlen(input) >= 1 && input[0] < ' ') {
		return 1;
	}

	return 0;
}

static int is_high_garbage(string input)
{
	if (strlen(input) >= 1 && input[0] & 0x80) {
		return 1;
	}
}

static int is_http_garbage(string input)
{
	if (strlen(input) >= 4 && input[0 .. 3] == "GET ") {
		return 1;
	}

	return 0;
}

static string garbage(string input)
{
	if (is_control_garbage(input)) {
		return "control";
	}

	if (is_high_garbage(input)) {
		return "high";
	}

	if (is_http_garbage(input)) {
		return "http";
	}

	return nil;
}

static void siteban(string ip, string reason)
{
	string creator;
	mapping ban;

	ban = ([ ]);

	creator = DRIVER->creator(object_name(this_object()));

	ban["message"] = reason;
	ban["expire"] = time() + 90 * 86400;
	ban["issuer"] = creator;

	BAND->ban_site(ip + "/32", ban);
}
