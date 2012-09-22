#include <kotaka/paths.h>
#include <kotaka/privilege.h>

/* keeps track of bans */

mapping bans;

private void save();
private void restore();

static void create()
{
	bans = ([ ]);
	restore();
}

void ban_username(string username)
{
	ACCESS_CHECK(GAME());

	if (username == "admin") {
		error("Cannot ban admin");
	}

	if (bans[username]) {
		error("Username already banned");
	}

	bans[username] = 1;
	save();
}

void unban_username(string username)
{
	ACCESS_CHECK(GAME());

	if (!bans[username]) {
		error("Username not banned");
	}

	bans[username] = nil;
	save();
}

int query_is_banned(string username)
{
	return !!bans[username];
}

string *query_bans()
{
	return map_indices(bans);
}

void force_save()
{
	ACCESS_CHECK(GAME() || KADMIN());

	save();
}

private void save()
{
	string buf;

	buf = STRINGD->hybrid_sprint(bans);

	SECRETD->write_file("bans-tmp", buf + "\n");
	SECRETD->remove_file("bans");
	SECRETD->rename_file("bans-tmp", "bans");
}

private void restore()
{
	string buf;

	buf = SECRETD->read_file("bans");

	if (buf) {
		bans = "~Kotaka/sys/parse/value"->parse(buf);
	}
}
