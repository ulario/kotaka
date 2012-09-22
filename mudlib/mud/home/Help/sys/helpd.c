#include <kotaka/privilege.h>
#include <kotaka/assert.h>

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

void add_topic(string topic, varargs string content)
{
	ACCESS_CHECK(PRIVILEGED());
	ASSERT(content);

	root_category->insert_entry(topic, 0)->set_content(content);
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

int test_topic(string topic)
{
	string *parts;
	string category;
	object subnode;
	int sz;

	parts = explode(topic, "/");

	if ((sz = sizeof(parts)) > 1) {
		topic = parts[sz - 1];
		subnode = root_category->find_node(implode(parts[0 .. sz - 2], "/"));
	} else {
		subnode = root_category;
	}

	if (!subnode) {
		return 0;
	}

	subnode = subnode->query_topic(topic);

	return !!subnode;
}

string query_content(string topic)
{
	string *parts;
	string category;
	object subnode;
	int sz;

	parts = explode(topic, "/");

	if ((sz = sizeof(parts)) > 1) {
		topic = parts[sz - 1];
		subnode = root_category->find_node(implode(parts[0 .. sz - 2], "/"));
	} else {
		subnode = root_category;
	}

	if (!subnode) {
		error("No such category");
	}

	subnode = subnode->query_topic(topic);

	if (!subnode) {
		error("No such topic");
	}

	return subnode->query_content();
}
