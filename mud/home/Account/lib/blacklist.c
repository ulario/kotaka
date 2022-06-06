#include <kernel/kernel.h>
#include <kotaka/paths/account.h>

static int is_control_garbage(string input)
{
	if (strlen(input) >= 1 && input[0] < ' ') {
		return 1;
	}

	return 0;
}

static int is_http_garbage(string input)
{
	if (strlen(input) >= 4 && input[0 .. 3] == "GET ") {
		return 1;
	}

	return 0;
}

static string garbage(string input)
{
	if (is_control_garbage(input)) {
		return "control";
	}

	if (is_http_garbage(input)) {
		return "http";
	}

	return nil;
}

static void siteban(string ip, string reason)
{
	string creator;
	mapping ban;

	ban = ([ ]);

	creator = DRIVER->creator(object_name(this_object()));

	ban["message"] = reason;
	ban["expire"] = time() + 90 * 86400;
	ban["issuer"] = creator;

	BAND->ban_site(ip + "/32", ban);
}
