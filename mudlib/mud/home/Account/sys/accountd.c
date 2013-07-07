/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <type.h>

int max_uid;

mapping properties;
mapping passwords;

private void save();
private void restore();

static void create()
{
	max_uid = 1;

	restore();

	if (!passwords) {
		passwords = ([ ]);
	}

	if (!properties) {
		properties = ([ ]);
	}
}

void register_account(string name, string password)
{
	ACCESS_CHECK(TEXT());

	if (passwords[name]) {
		error("Duplicate account");
	}

	passwords[name] = hash_string("crypt", password, name);
	save();
}

void unregister_account(string name)
{
	ACCESS_CHECK(TEXT());

	if (!passwords[name]) {
		error("No such account");
	}

	passwords[name] = nil;
	properties[name] = nil;
	save();
}

int query_is_registered(string name)
{
	return !!passwords[name];
}

string *query_accounts()
{
	return map_indices(passwords);
}

void change_password(string name, string newpass)
{
	ACCESS_CHECK(TEXT() || ACCOUNT());

	if (!passwords[name]) {
		error("No such account");
	}

	passwords[name] = hash_string("crypt", newpass, name);

	save();
}

int authenticate(string name, string password)
{
	ACCESS_CHECK(TEXT());

	if (!passwords[name]) {
		error("No such account");
	}

	if (passwords[name] == hash_string("crypt", password, name)) {
		change_password(name, password);
		return TRUE;
	}

	if (passwords[name] == hash_string("SHA1", password)) {
		change_password(name, password);
		return TRUE;
	}

	if (passwords[name] == hash_string("MD5", password)) {
		change_password(name, password);
		return TRUE;
	}
}

void force_save()
{
	ACCESS_CHECK(INTERFACE() || KADMIN());

	save();
}

void force_restore()
{
	ACCESS_CHECK(INTERFACE() || KADMIN());

	restore();
}

private void save()
{
	string buf;

	buf = STRINGD->hybrid_sprint( ([ "properties" : properties,
		"passwords" : passwords ]) );

	SECRETD->remove_file("accounts-tmp");
	SECRETD->write_file("accounts-tmp", buf + "\n");
	SECRETD->remove_file("accounts");
	SECRETD->rename_file("accounts-tmp", "accounts");
}

private void restore()
{
	mapping map;
	string buf;

	buf = SECRETD->read_file("accounts");

	if (!buf) {
		return;
	}

	map = "~Kotaka/sys/parse/value"->parse(buf);

	properties = map["properties"];
	passwords = map["passwords"];
}

mixed query_account_property(string name, string property)
{
	if (!property) {
		error("Invalid property name");
	}

	if (!passwords[name]) {
		error("No such account");
	}

	if (properties[name]) {
		return properties[name][property];
	} else {
		return nil;
	}
}

void set_account_property(string name, string property, mixed value)
{
	mapping prop;

	if (!property) {
		error("Invalid property name");
	}

	if (!passwords[name]) {
		error("No such account");
	}

	switch(typeof(value)) {
	case T_ARRAY:
	case T_OBJECT:
	case T_MAPPING:
		error("Invalid property value");
	}

	prop = properties[name];

	if (!prop) {
		if (value == nil) {
			return;
		}

		properties[name] = prop = ([ ]);
	}

	prop[property] = value;

	if (!map_sizeof(prop)) {
		properties[name] = nil;
	}

	save();
}
