#include <kotaka/privilege.h>

mixed content;

static void create(int clone)
{
}

static void destruct(int clone)
{
}

void set_content(mixed new_content)
{
	ACCESS_CHECK(HELP());

	content = new_content;
}

mixed query_content()
{
	ACCESS_CHECK(HELP());

	return content;
}
