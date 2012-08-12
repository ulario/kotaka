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
	ACCESS_CHECK(GAME());

	if (passwords[name]) {
		error("Duplicate account");
	}

	passwords[name] = hash_string("SHA1", password);
	save();
}

void unregister_account(string name)
{
	ACCESS_CHECK(GAME());

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

int authenticate(string name, string password)
{
	ACCESS_CHECK(GAME());

	if (!passwords[name]) {
		error("No such account");
	}

	if (passwords[name] == hash_string("SHA1", password))
		return TRUE;

	if (passwords[name] == hash_string("MD5", password)) {
		passwords[name] = hash_string("SHA1", password);
		save();
		return TRUE;
	}

}

void change_password(string name, string newpass)
{
	ACCESS_CHECK(GAME());

	if (!passwords[name]) {
		error("No such account");
	}

	passwords[name] = hash_string("SHA1", newpass);

	save();
}

void force_save()
{
	ACCESS_CHECK(GAME() || KADMIN());

	save();
}

private void save()
{
	string buf;

	buf = STRINGD->hybrid_sprint( ({ properties, passwords }) );

	SECRETD->write_file("accounts-tmp", buf);
	SECRETD->remove_file("accounts");
	SECRETD->rename_file("accounts-tmp", "accounts");
}

private void restore()
{
	string buf;

	buf = SECRETD->read_file("accounts");

	if (!buf) {
		return;
	}

	({ properties, passwords }) = "~Kotaka/sys/parse/value"->parse(buf);
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
