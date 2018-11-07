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
#include <kernel/kernel.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit "/lib/string/validate";

int audit;

static void create()
{
}

void check_security(string user, string creator)
{
	PERMISSION_CHECK(user != "System");

	if (user[0] >= 'a' && user[0] <= 'z') {
		object this_user;

		/* may be set by interface objects or filter objects */
		this_user = TLSD->query_tls_value("System", "this-user");

		if (!this_user) {
			this_user = this_user();
		}

		/* Proxies for users must be initiated by that user */
		PERMISSION_CHECK(this_user);
		PERMISSION_CHECK(this_user->query_name() == user);
	} else {
		/* Infrastructure can only be proxied by itself */
		PERMISSION_CHECK(creator == user);
	}
}

object get_proxy(string user)
{
	object proxy;
	int firstchar;

	string creator;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	creator = DRIVER->creator(previous_program());

	CHECKARG(user && user != "" &&
		is_valid_username(user), 1, "get_proxy");

	check_security(user, creator);

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

object get_wiztool(string user)
{
	object wiztool;
	int firstchar;

	string creator;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	creator = DRIVER->creator(previous_program());

	CHECKARG(user && user != "" &&
		is_valid_username(user), 1, "get_proxy");

	check_security(user, creator);

	wiztool = new_object("~/lwo/wiztool", user);

	if (audit) {
		INITD->message("Wiztool being issued to " +
			creator + " for " + user);
	}

	return wiztool;
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
