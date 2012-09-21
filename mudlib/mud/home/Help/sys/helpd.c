#include <kotaka/privilege.h>

object root_category;

static void create()
{
	root_category = clone_object("~/obj/category");
}

static void destruct()
{
	if (root_category) {
		destruct_object(root_category);
	}
}

void reset()
{
	ACCESS_CHECK(PRIVILEGED());

	destruct_object(root_category);

	root_category = clone_object("~/obj/category");
}

void add_category(string category)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->insert_entry(category, 1);
}

void add_topic(string topic)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->insert_entry(topic, 0);
}

void remove_category(string category)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->delete_entry(category, 1);
}

void remove_topic(string topic)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->delete_entry(topic, 0);
}

string *query_topics(varargs string category)
{
	object subnode;

	if (!category || category == "") {
		subnode = root_category;
	} else {
		subnode = root_category->find_node(category);
	}

	if (subnode) {
		return subnode->query_topics();
	} else {
		return nil;
	}
}

string *query_categories(varargs string category)
{
	object subnode;

	if (!category || category == "") {
		subnode = root_category;
	} else {
		subnode = root_category->find_node(category);
	}

	if (subnode) {
		return subnode->query_categories();
	} else {
		return nil;
	}
}

mapping query_index(varargs string category)
{
	object subnode;

	if (!category || category == "") {
		subnode = root_category;
	} else {
		subnode = root_category->find_node(category);
	}

	if (subnode) {
		return subnode->query_index();
	} else {
		return nil;
	}
}
