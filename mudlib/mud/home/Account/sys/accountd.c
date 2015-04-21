/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>
#include <type.h>

mapping accounts;
mapping properties;
mapping passwords;

static void create()
{
}

void upgrade()
{
	string *names;
	int sz;

	ACCESS_CHECK(ACCOUNT());

	if (!accounts) {
		accounts = ([ ]);
	}

	if (!passwords) {
		passwords = ([ ]);
	}

	if (!passwords) {
		passwords = ([ ]);
	}

	if (!properties) {
		properties = ([ ]);
	}

	names = map_indices(passwords);

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string password;

		object account;

		name = names[sz];

		account = clone_object("../obj/account");
		accounts[name] = account;
		account->set_name(name);
		account->set_hashed_password(passwords[name]);

		if (properties[name]) {
			string *propnames;
			int sz2;
			mapping prop;

			propnames = map_indices(properties[name]);

			for (sz2 = sizeof(propnames); --sz2 >= 0; ) {
				LOGD->post_message("debug", LOG_INFO, "Copying property" + propnames[sz2]);

				account->set_property(propnames[sz2],
					properties[name][propnames[sz2]]);
			}
		}

		account->save();
		passwords[name] = nil;
		properties[name] = nil;
	}

	ASSERT(map_sizeof(passwords) == 0);
	ASSERT(map_sizeof(properties) == 0);

	SECRETD->remove_file("account");
}

void register_account(string name, string password)
{
	object account;

	ACCESS_CHECK(TEXT() || VERB());

	if (accounts[name]) {
		error("Duplicate account");
	}

	account = clone_object("../obj/account");
	account->set_name(name);
	account->set_password(password);
	account->save();
}

void unregister_account(string name)
{
	ACCESS_CHECK(TEXT() || VERB());

	if (!accounts[name]) {
		error("No such account");
	}

	accounts[name]->delete_account();
}

int query_is_registered(string name)
{
	return !!accounts[name];
}

string *query_accounts()
{
	return map_indices(accounts);
}

void change_password(string name, string new_password)
{
	ACCESS_CHECK(TEXT() || ACCOUNT() || VERB());

	if (!accounts[name]) {
		error("No such account");
	}

	accounts[name]->set_password(new_password);
	accounts[name]->save();
}

int authenticate(string name, string password)
{
	ACCESS_CHECK(TEXT());

	if (!accounts[name]) {
		error("No such account");
	}

	if (accounts[name]->authenticate(password)) {
		change_password(name, password);
		return TRUE;
	}

	return FALSE;
}

mixed query_account_property(string name, string property)
{
	if (!accounts[name]) {
		error("No such account");
	}

	return accounts[name]->query_property(property);
}

void set_account_property(string name, string property, mixed value)
{
	if (!accounts[name]) {
		error("No such account");
	}

	accounts[name]->set_property(property, value);
	accounts[name]->save();
}

void force_save()
{
	string *names;
	int sz;

	names = map_indices(accounts);

	for (sz = sizeof(names); --sz >= 0; ) {
		object account;
		string name;

		name = names[sz];

		account = accounts[name];

		account->save();
	}
}

void force_restore()
{
	string *names;
	int sz;

	names = SECRETD->get_dir("accounts/*")[0];

	for (sz = sizeof(names); --sz >= 0; ) {
		object account;
		string name;

		name = names[sz];

		if (!accounts[name]) {
			accounts[name] = clone_object("../obj/account");
		}

		account = accounts[name];

		account->set_name(name);
		account->load();
	}
}
