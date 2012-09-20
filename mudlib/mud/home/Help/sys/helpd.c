#include <kotaka/privilege.h>

object help_root;

static void create()
{
	help_root = clone_object("~/obj/help");
}

static void destruct()
{
	if (help_root) {
		destruct_object(help_root);
	}
}

void reset()
{
	ACCESS_CHECK(PRIVILEGED());

	destruct_object(help_root);

	help_root = clone_object("~/obj/help");
}

void add_category(string category)
{
	ACCESS_CHECK(PRIVILEGED());

	help_root->insert_entry(category, 1);
}

void add_topic(string topic)
{
	ACCESS_CHECK(PRIVILEGED());

	help_root->insert_entry(topic, 0);
}


string *query_topics(string category)
{
	object subnode;

	if (category == "") {
		subnode = help_root;
	} else {
		subnode = help_root->find_node(category);
	}

	if (subnode) {
		return subnode->query_topics();
	} else {
		return nil;
	}
}

string *query_categories(string category)
{
	object subnode;

	if (category == "") {
		subnode = help_root;
	} else {
		subnode = help_root->find_node(category);
	}

	if (subnode) {
		return subnode->query_categories();
	} else {
		return nil;
	}
}

mapping query_index(string category)
{
	object subnode;

	if (category == "") {
		subnode = help_root;
	} else {
		subnode = help_root->find_node(category);
	}

	if (subnode) {
		return subnode->query_index();
	} else {
		return nil;
	}
}
