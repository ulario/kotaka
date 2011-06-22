/* keeps track of bans */

mapping username_bans;
mapping ip_bans;

static void create()
{
	username_bans = ([ ]);
}

void ban_username(string username)
{
	ACCESS_CHECK(GAME());

	if (username_bans[username]) {
		error("Username already banned");
	}

	username_bans[username] = 1;
}

void unban_username(string username)
{
	ACCESS_CHECK(GAME());

	if (!username_bans[username]) {
		error("Username not banned");
	}

	username_bans[username] = nil;
}

int query_is_banned(string username)
{
	return !!username_bans(username);
}
