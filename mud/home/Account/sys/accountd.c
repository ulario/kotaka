/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <kotaka/checkarg.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>
#include <type.h>
#include <status.h>

mapping accounts;

static void create()
{
	accounts = ([ ]);
}

void register_account(string name, string password)
{
	object account;

	ACCESS_CHECK(TEXT() || VERB());

	CHECKARG(name, 1, "register_account");

	if (accounts[name]) {
		error("Duplicate account");
	}

	account = clone_object("../obj/account");
	account->set_name(name);
	account->set_password(password);

	accounts[name] = account;
}

void unregister_account(string name)
{
	ACCESS_CHECK(TEXT() || VERB());

	CHECKARG(name, 1, "unregister_account");

	if (!accounts[name]) {
		error("No such account");
	}

	destruct_object(accounts[name]);

	if (SECRETD->file_info("accounts/" + name)) {
		if (SECRETD->file_info("accounts/" + name + ".old")) {
			SECRETD->remove_file("accounts/" + name + ".old");
		}

		SECRETD->rename_file("accounts/" + name, "accounts/" + name + ".old");
	} else {
		LOGD->post_message("system", LOG_WARNING, "Unregistering unsaved account " + name);
	}
}

int query_is_registered(string name)
{
	CHECKARG(name, 1, "query_is_registered");

	return !!accounts[name];
}

string *query_accounts()
{
	return map_indices(accounts);
}

void change_password(string name, string new_password)
{
	ACCESS_CHECK(TEXT() || ACCOUNT() || VERB() || KADMIN());

	CHECKARG(name, 1, "change_password");

	if (!accounts[name]) {
		error("No such account");
	}

	accounts[name]->set_password(new_password);
}

int authenticate(string name, string password)
{
	ACCESS_CHECK(INTERFACE());

	CHECKARG(name, 1, "authenticate");

	if (!accounts[name]) {
		error("No such account");
	}

	if (accounts[name]->authenticate(password)) {
		return TRUE;
	}

	return FALSE;
}

string *list_account_properties(string name)
{
	CHECKARG(name, 1, "list_account_properties");

	if (!accounts[name]) {
		error("No such account");
	}

	return accounts[name]->list_properties();
}

mixed query_account_property(string name, string property)
{
	CHECKARG(name, 1, "query_account_property");
	CHECKARG(property, 2, "query_account_property");

	if (!accounts[name]) {
		error("No such account");
	}

	return accounts[name]->query_property(property);
}

void set_account_property(string name, string property, mixed value)
{
	CHECKARG(name, 1, "set_account_property");
	CHECKARG(property, 2, "set_account_property");

	if (!accounts[name]) {
		error("No such account");
	}

	accounts[name]->set_property(property, value);
}

void save()
{
	string *names;
	int sz;

	ACCESS_CHECK(VERB() || ACCOUNT());

	LOGD->post_message("system", LOG_NOTICE, "Saving accounts");

	SECRETD->make_dir(".");
	SECRETD->make_dir("accounts");

	names = SECRETD->get_dir("accounts/*")[0];

	ASSERT(names);

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;

		name = names[sz];

		if (SECRETD->file_info("accounts/" + name + ".old")) {
			SECRETD->remove_file("accounts/" + name + ".old");
		}

		SECRETD->rename_file("accounts/" + names[sz], "accounts/" + name + ".old");
	}

	names = map_indices(accounts);

	for (sz = sizeof(names); --sz >= 0; ) {
		object account;
		string name;

		name = names[sz];

		account = accounts[name];

		account->save();
	}
}

void restore()
{
	string *remove;
	string *names;
	int sz;

	ACCESS_CHECK(VERB() || ACCOUNT());

	LOGD->post_message("system", LOG_NOTICE, "Restoring accounts");

	names = SECRETD->get_dir("accounts/*")[0];

	if (sizeof(names) == ST_ARRAYSIZE) {
		error("Account count overflow");
	}

	remove = map_indices(accounts) - names;

	for (sz = sizeof(remove); --sz >= 0; ) {
		destruct_object(accounts[remove[sz]]);
	}

	for (sz = sizeof(names); --sz >= 0; ) {
		object account;
		string name;

		name = names[sz];

		if (sscanf(name, "%*s.old")) {
			continue;
		}

		if (!accounts[name]) {
			accounts[name] = clone_object("../obj/account");
		}

		account = accounts[name];

		account->set_name(name);
		account->restore();
	}
}
