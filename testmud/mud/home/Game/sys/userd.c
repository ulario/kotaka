#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>

mapping users;
mapping guests;

static void create()
{
	users = ([ ]);
	guests = ([ ]);
}

void upgrade()
{
	if (!guests) {
		guests = ([ ]);
	}

	LOGD->post_message("game", LOG_INFO, "Game user manager upgraded.");
}

void add_user(string name, object user)
{
	ACCESS_CHECK(GAME());

	if (users[name]) {
		error("Duplicate user");
	}

	users[name] = user;
}

void delete_user(string name)
{
	ACCESS_CHECK(GAME());

	if (!users[name]) {
		error("No such user");
	}

	users[name] = nil;
}

object query_user(string name)
{
	ACCESS_CHECK(GAME());

	return users[name];
}

void add_guest(object user)
{
	if (guests[user]) {
		error("Duplicate guest");
	}

	guests[user] = 1;
}

void delete_guest(object user)
{
	if (!guests[user]) {
		error("No such guest");
	}

	guests[user] = nil;
}

int query_is_guest(object user)
{
	return !!guests[user];
}

void promote_guest(string name, object user)
{
	ACCESS_CHECK(GAME());

	if (!guests[user]) {
		error("No such guest");
	}

	if (users[name]) {
		error("Duplicate user");
	}

	guests[user] = nil;
	users[name] = user;
}

void rename_user(string oldname, string newname)
{
	ACCESS_CHECK(GAME());

	if (!users[oldname]) {
		error("No such user");
	}

	if (users[newname]) {
		error("Duplicate user");
	}

	users[newname] = users[oldname];
	users[oldname] = nil;
}

object query_user(string name)
{
	return users[name];
}

string *query_names()
{
	return map_indices(users);
}

object *query_users()
{
	return map_values(users);
}

object *query_guests()
{
	return map_indices(guests);
}
