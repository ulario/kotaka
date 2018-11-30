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
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <status.h>

inherit LIB_VERB;

static void nuke(object proxy, string path, int index)
{
	object obj;

	if (!proxy) {
		LOGD->post_message("system", LOG_ERR, "Aborting clone nuke (proxy destructed)");

		return;
	}

	--index;

	if (obj = find_object(path + "#" + index)) {
		proxy->destruct_object(obj);
	}

	if (index) {
		call_out("lazy_nuke", 0, path, index - 1, proxy);
	}
}

static void upgrade()
{
	wipe_callouts();
}

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string path;
	object proxy;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to nuke clones.\n");
		return;
	}

	path = roles["raw"];
	proxy = PROXYD->get_proxy(query_user()->query_name());

	call_out("nuke", 0, proxy, path, status(ST_OTABSIZE));
}

void lazy_nuke(varargs mixed dummy...)
{
}
