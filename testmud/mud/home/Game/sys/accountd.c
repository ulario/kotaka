#include <kotaka/privilege.h>

int max_uid;

mapping passwords;

private void save();
private void restore();

static void create()
{
	max_uid = 1;

	passwords = ([ ]);

	restore();
}

void register_account(string name, string password)
{
	ACCESS_CHECK(GAME());

	if (passwords[name]) {
		error("Duplicate account");
	}

	passwords[name] = hash_string("MD5", password);
	save();
}

void unregister_account(string name)
{
	ACCESS_CHECK(GAME());

	if (!passwords[name]) {
		error("No such account");
	}

	passwords[name] = nil;
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

	return passwords[name] == hash_string("MD5", password);
}

void change_password(string name, string newpass)
{
	ACCESS_CHECK(GAME());

	if (!passwords[name]) {
		error("No such account");
	}

	passwords[name] = hash_string("MD5", newpass);
}

private void save()
{
	save_object("accountd.o");
}

private void restore()
{
	restore_object("accountd.o");
}
