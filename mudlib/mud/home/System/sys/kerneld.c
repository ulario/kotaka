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
#include <kernel/kernel.h>
#include <kernel/access.h>
#include <kernel/objreg.h>
#include <kernel/rsrc.h>
#include <kernel/user.h>

#include <kotaka/log.h>
#include <kotaka/privilege.h>
#include <kotaka/paths.h>

inherit SECOND_AUTO;

inherit user API_USER;
inherit access API_ACCESS;
inherit objreg API_OBJREG;
inherit rsrc API_RSRC;

static void create()
{
	access::create();
	objreg::create();
	rsrc::create();
	user::create();
}

/**********/
/* access */
/**********/

int access(string user, string file, int access)
{
	ACCESS_CHECK(PRIVILEGED());

	return ::access(user, file, access);
}

void add_user(string user)
{
	ACCESS_CHECK(SYSTEM());

	::add_user(user);
}

void remove_user(string user)
{
	ACCESS_CHECK(SYSTEM());

	::remove_user(user);
}

string *query_users()
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_users();
}

void set_access(string user, string file, int type)
{
	ACCESS_CHECK(SYSTEM());

	::set_access(user, file, type);
}

mapping query_user_access(string user)
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_user_access(user);
}

mapping query_file_access(string path)
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_file_access(path);
}

void set_global_access(string dir, int flag)
{
	ACCESS_CHECK(
		SYSTEM()
		|| (
			dir
			== DRIVER->creator(
				object_name(previous_object())
			)
		)
	);

	::set_global_access(dir, flag);
}

/**********/
/* objreg */
/**********/

object first_link(string owner)
{
	return ::first_link(owner);
}

object prev_link(object obj)
{
	return ::prev_link(obj);
}

object next_link(object obj)
{
	return ::next_link(obj);
}

/********/
/* rsrc */
/********/

void add_owner(string owner)
{
	ACCESS_CHECK(SYSTEM());

	::add_owner(owner);
}

void remove_owner(string owner)
{
	ACCESS_CHECK(SYSTEM());

	::remove_owner(owner);
}

string *query_owners()
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_owners();
}

void set_rsrc(string name, int max, int decay, int period)
{
	ACCESS_CHECK(SYSTEM());

	::set_rsrc(name, max, decay, period);
}

void remove_rsrc(string name)
{
	ACCESS_CHECK(SYSTEM());

	::remove_rsrc(name);
}

mixed *query_rsrc(string name)
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_rsrc(name);
}

string *query_resources()
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_resources();
}

void rsrc_set_limit(string owner, string name, int max)
{
	ACCESS_CHECK(SYSTEM());

	::rsrc_set_limit(owner, name, max);
}

mixed *rsrc_get(string owner, string name)
{
	ACCESS_CHECK(PRIVILEGED());

	return ::rsrc_get(owner, name);
}

int rsrc_incr(string owner, string name, mixed index, int incr, varargs int force)
{
	ACCESS_CHECK(SYSTEM());

	::rsrc_incr(owner, name, index, incr, force);
}

/********/
/* user */
/********/

object *query_connections()
{
	ACCESS_CHECK(PRIVILEGED());

	return ::query_connections();
}

object find_user(string name)
{
	ACCESS_CHECK(PRIVILEGED());

	return ::find_user(name);
}

/*****************/
/* miscellaneous */
/*****************/

void swapout()
{
	ACCESS_CHECK(PRIVILEGED());

	::swapout();
}

void dump_state()
{
	ACCESS_CHECK(PRIVILEGED());

	::dump_state();
}

void shutdown()
{
	ACCESS_CHECK(PRIVILEGED());

	::shutdown();
}
