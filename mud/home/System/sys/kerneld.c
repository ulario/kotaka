/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <kernel/access.h>
#include <kernel/user.h>
#include <kernel/objreg.h>
#include <kernel/rsrc.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

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
	return ::access(user, file, access);
}

void add_user(string user)
{
	ACCESS_CHECK(SYSTEM() || INTERFACE());

	::add_user(user);
}

void remove_user(string user)
{
	ACCESS_CHECK(SYSTEM() || INTERFACE());

	::remove_user(user);
}

string *query_users()
{
	return ::query_users();
}

void set_access(string user, string file, int type)
{
	ACCESS_CHECK(SYSTEM());

	::set_access(user, file, type);
}

mapping query_user_access(string user)
{
	return ::query_user_access(user);
}

mapping query_file_access(string path)
{
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

string *query_global_access()
{
	return ::query_global_access();
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
	return ::query_rsrc(name);
}

string *query_resources()
{
	return ::query_resources();
}

void rsrc_set_limit(string owner, string name, int max)
{
	if (!SYSTEM()) {
		ACCESS_CHECK(owner);

		ACCESS_CHECK(previous_program() == USR_DIR + "/" + owner + "/initd");
	}

	::rsrc_set_limit(owner, name, max);
}

mixed *rsrc_get(string owner, string name)
{
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
	return ::query_connections();
}

object find_user(string name)
{
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

void dump_state(varargs int incr)
{
	ACCESS_CHECK(PRIVILEGED());

	::dump_state(incr);
}

void shutdown(varargs int hotboot)
{
	ACCESS_CHECK(PRIVILEGED());

	::shutdown(hotboot);
}

object get_wiztool()
{
	object wiztool;
	object user;

	ACCESS_CHECK(INTERFACE() || PRIVILEGED());
	ACCESS_CHECK(user = this_user());

	return new_object("~/lwo/wiztool", user->query_name());
}

void reset_accessd()
{
	ACCESS_CHECK(PRIVILEGED() || VERB());

	ACCESSD->reset();
}


void save_accessd()
{
	ACCESS_CHECK(PRIVILEGED() || VERB());

	ACCESSD->save();
}

void restore_accessd()
{
	ACCESS_CHECK(PRIVILEGED() || VERB());

	ACCESSD->restore();
}

/* public utility functions */

atomic void fix_filequota()
{
	ACCESS_CHECK(PRIVILEGED() || VERB());

	rlimits(0; -1) {
		string *names;
		int *sizes;
		mixed *dummy;
		int sz;
		string *owners;
		string *add;

		mapping counts;
		counts = ([ "System": 0, nil: 0 ]);

		({ names, sizes, dummy, dummy }) = get_dir("/*");

		for (sz = sizeof(names); --sz >= 0; ) {
			string name;
			int size;

			name = names[sz];
			size = sizes[sz];

			if (size == -2) {
				/* directory */
				if ("/" + name == USR_DIR) {
					string *unames;
					int *usizes;
					int usz;

					counts[nil]++;

					({ unames, usizes, dummy, dummy }) = get_dir(USR_DIR + "/*");

					for (usz = sizeof(unames); --usz >= 0; ) {
						string uname;

						uname = unames[usz];

						if (usizes[usz] == -2) {
							if (!counts[uname]) {
								counts[uname] = 0;
							}
							counts[uname] += DRIVER->file_size(USR_DIR + "/" + uname, 1);
						} else {
							counts[nil] += DRIVER->file_size(USR_DIR + "/" + uname);
						}
					}
				} else {
					counts[(name == "kernel") ? "System" : nil] += DRIVER->file_size("/" + name, 1);
				}
			} else {
				counts[nil] += DRIVER->file_size("/" + name);
			}
		}

		/* escheat ownerless files to nil */

		names = map_indices(counts);
		owners = ::query_owners();

		add = names - owners;

		for (sz = sizeof(add); --sz >= 0; ) {
			string name;

			name = add[sz];

			counts[nil] += counts[name];
			counts[name] = nil;
		}

		/* adjust quota */

		names = map_indices(counts);
		sizes = map_values(counts);

		for (sz = sizeof(names); --sz >= 0; ) {
			string name;
			int size;
			int usage;

			name = names[sz];
			size = sizes[sz];

			usage = ::rsrc_get(name, "filequota")[RSRC_USAGE];

			if (usage != size) {
				DRIVER->message("Adjusting filequota of " + (name ? name : "Ecru") + ": recorded " + usage + " but found " + size + ", adding " + (size - usage) + "\n");
				::rsrc_incr(name, "filequota", nil, size - usage, 1);
			}
		}
	}
}
