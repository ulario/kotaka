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

	passwords[name] = password;
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

int authenticate(string name, string password)
{
	ACCESS_CHECK(GAME());

	if (!passwords[name]) {
		error("No such account");
	}

	return passwords[name] == password;
}

void change_password(string name, string oldpass, string newpass)
{
	ACCESS_CHECK(GAME());

	if (!passwords[name]) {
		error("No such account");
	}

	if (passwords[name] != oldpass) {
		error("Incorrect password");
	}

	passwords[name] = newpass;
}

int username_to_uid(string username)
{
	return -1;
}

private void save()
{
	save_object("accountd.o");
}

private void restore()
{
	restore_object("accountd.o");
}
