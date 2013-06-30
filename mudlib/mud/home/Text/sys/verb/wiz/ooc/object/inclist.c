/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <text/paths.h>
#include <status.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object ilist;
	int i, sz;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do an inclusion check.\n");
		return;
	}

	ilist = PROGRAMD->query_includers(args);

	if (!ilist) {
		send_out("No programs include that file.\n");
		return;
	}

	sz = ilist->get_size();
	send_out("There are " + ilist->get_size() + " programs including that file:\n");

	for (i = 0; i < sz; i++) {
		object pinfo;

		pinfo = PROGRAMD->query_program_info(ilist->get_element(i));
		send_out((pinfo ? pinfo->query_path() : "wtf#") + "\n");
	}
}
