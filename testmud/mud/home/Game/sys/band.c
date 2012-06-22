#include <kotaka/privilege.h>

/* keeps track of bans */

mapping username_bans;

private void save();
private void restore();

static void create()
{
	username_bans = ([ ]);
	restore();
}

void ban_username(string username)
{
	ACCESS_CHECK(GAME());

	if (username == "admin") {
		error("Cannot ban admin");
	}

	if (username_bans[username]) {
		error("Username already banned");
	}

	username_bans[username] = 1;
	save();
}

void unban_username(string username)
{
	ACCESS_CHECK(GAME());

	if (!username_bans[username]) {
		error("Username not banned");
	}

	username_bans[username] = nil;
	save();
}

int query_is_banned(string username)
{
	return !!username_bans[username];
}

void force_save()
{
	ACCESS_CHECK(GAME() || KADMIN());

	save();
}

private void save()
{
	save_object("~/data/save/band.o");
}

private void restore()
{
	restore_object("~/data/save/band.o");
}
