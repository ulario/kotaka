#include <kotaka/paths.h>
#include <kotaka/log.h>

static void create()
{
}

void test()
{
	HELPD->add_topic("foo/bar");
	HELPD->add_topic("foo/baz");
	HELPD->add_topic("baz");
	HELPD->add_topic("foo/bar/baz");
	
	LOGD->post_message("test", LOG_INFO, "Topics listed by baz: " + 
		STRINGD->mixed_sprint(HELPD->fetch_topics("baz")));
	
	HELPD->clear();
}
