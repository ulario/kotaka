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
/** Grants proxies

A proxy is used to grant user level permissions to objects, or to allow
one subsystem to take advantage of grants made by other subsystems.

*/
#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>

inherit SECOND_AUTO;

int audit;

static void create()
{
}

object get_proxy(string user)
{
	object proxy;
	int firstchar;

	string creator;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	creator = DRIVER->creator(previous_program());

	CHECKARG(user && user != "" &&
		STRINGD->is_valid_username(user), 1, "get_proxy");

	switch(user) {
	case "System":
		/* System doesn't need proxies */
		error("Permission denied");

	case "Common":
	case "Game":
		/* Infrastructure can only be proxied by itself */
		PERMISSION_CHECK(creator == user);
		break;

	default:
		/* Proxies for users must be initiated by that user */
		PERMISSION_CHECK(this_user());
		PERMISSION_CHECK(this_user()->query_name() == user);
	}

	proxy = new_object("~/lwo/proxy", user);

	/* only the object who requested a proxy is allowed to use it */
	proxy->set_client(previous_object());

	if (audit) {
		INITD->message("User access proxy being issued to " +
			creator + " for " + user + ", assigned to " +
			object_name(previous_object()));
	}

	return proxy;
}

void enable_audit()
{
	ACCESS_CHECK(SYSTEM());

	audit = 1;
}

void disable_audit()
{
	ACCESS_CHECK(SYSTEM());

	audit = 0;
}
