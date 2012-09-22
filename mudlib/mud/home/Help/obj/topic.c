#include <kotaka/privilege.h>

string content;

static void create(int clone)
{
}

static void destruct(int clone)
{
}

void set_content(string new_content)
{
	ACCESS_CHECK(HELP());

	content = new_content;
}

string query_content()
{
	ACCESS_CHECK(HELP());

	return content;
}
