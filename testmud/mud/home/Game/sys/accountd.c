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
	save_object("~/data/save/accountd.o");
}

private void restore()
{
	restore_object("~/data/save/accountd.o");
}
