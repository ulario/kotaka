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
#include <kernel/user.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>

#include <config.h>
#include <type.h>

#define VERIFY() \
do {\
	if (!client || client != previous_object())\
		error("Unauthorized caller: expected " +\
			STRINGD->mixed_sprint(client) + ", got " +\
			STRINGD->mixed_sprint(previous_object()));\
} while (0)

inherit sa SECOND_AUTO;
inherit wt LIB_WIZTOOL;

object client;
string directory;

string *messages;

static void create(int clone)
{
	if (clone) {
		::create(0);
		messages = ({ });
		directory = USR_DIR + "/" + query_owner();
	}
}

static void message(string str)
{
	messages += ({ str });
}

int num_messages()
{
	VERIFY();
	return sizeof(messages);
}

string *query_messages()
{
	VERIFY();
	return messages;
}

void clear_messages()
{
	VERIFY();
	messages = ({ });
}

private string normalize(string in)
{
	return DRIVER->normalize_path(in, directory, query_owner());
}

void set_client(object new_client)
{
	ACCESS_CHECK(previous_program() == PROXYD);

	client = new_client;
}

void set_directory(string new_directory)
{
	VERIFY();
	directory = normalize(new_directory);
}

string query_directory()
{
	VERIFY();
	return directory;
}

void add_user(string user)
{
	VERIFY();
	::add_user(user);
}

void remove_user(string user)
{
	VERIFY();
	::remove_user(user);
}

void set_access(string user, string file, int type)
{
	VERIFY();
	file = normalize(file);
	::set_access(user, file, type);
}

void set_global_access(string dir, int flag)
{
	VERIFY();
	dir = normalize(dir);
	::set_global_access(dir, flag);
}

void add_owner(string rowner)
{
	VERIFY();
	::add_owner(rowner);
}

void remove_owner(string rowner)
{
	VERIFY();
	::remove_owner(rowner);
}

void set_rsrc(string name, int max, int decay, int period)
{
	VERIFY();
	::set_rsrc(name, max, decay, period);
}

mixed *query_rsrc(string name)
{
	VERIFY();
	return ::query_rsrc(name);
}

void rsrc_set_limit(string rowner, string name, int max)
{
	VERIFY();
	::rsrc_set_limit(rowner, name, max);
}

mixed *rsrc_get(string owner, string name)
{
	VERIFY();
	return ::rsrc_get(owner, name);
}

int rsrc_incr(string rowner, string name, mixed index, int incr, varargs int force)
{
	VERIFY();
	return ::rsrc_incr(rowner, name, index, incr, force);
}

object compile_object(string path, string source...)
{
	VERIFY();
	path = normalize(path);
	return wt::compile_object(path, source...);
}

object clone_object(string path)
{
	VERIFY();
	path = normalize(path);
	return wt::clone_object(path);
}

int destruct_object(mixed obj)
{
	VERIFY();

	if (typeof(obj) == T_STRING) {
		obj = normalize(obj);
	}

	return wt::destruct_object(obj);
}

object new_object(string path)
{
	VERIFY();
	path = normalize(path);
	return wt::new_object(path);
}

mixed read_file(string path, varargs int offset, int size)
{
	VERIFY();
	path = normalize(path);
	return ::read_file(path, offset, size);
}

int write_file(string path, string str, varargs int offset)
{
	VERIFY();
	path = normalize(path);
	return ::write_file(path, str, offset);
}

int remove_file(string path)
{
	VERIFY();
	path = normalize(path);
	return ::remove_file(path);
}

int rename_file(string from, string to)
{
	VERIFY();
	from = normalize(from);
	to = normalize(to);

	return ::rename_file(from, to);
}

mixed *file_info(string path)
{
	VERIFY();
	path = normalize(path);
	return ::file_info(path);
}

mixed **get_dir(string path)
{
	VERIFY();
	path = normalize(path);
	return ::get_dir(path);
}

int make_dir(string path)
{
	VERIFY();
	path = normalize(path);
	return ::make_dir(path);
}

int remove_dir(string path)
{
	VERIFY();
	path = normalize(path);
	return ::remove_dir(path);
}

void swapout()
{
	VERIFY();
	::swapout();
}

void dump_state(varargs int increment)
{
	VERIFY();
	::dump_state(increment);
}

void shutdown(varargs int hotboot)
{
	VERIFY();
	::shutdown(hotboot);
}
