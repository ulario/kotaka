#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>
#include <type.h>

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

private void add_category(string category)
{
	help_root->insert_entry(category, 1);
}

private void add_topic(string topic)
{
	help_root->insert_entry(topic, 0);
}

private void load_helpdir(string dir)
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	add_category(dir);

	dirlist = get_dir("~/data/help/" + dir + "/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			load_helpdir(dir + "/" + entry);
		} else if (sscanf(names[i], "%s.hlp", entry)) {
			add_topic(dir + "/" + entry);
		}
	}
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

private void load_rootdir()
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	dirlist = get_dir("~/data/help/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			load_helpdir(entry);
		} else if (sscanf(entry, "%s.hlp", entry)) {
			add_topic(entry);
		}
	}
}

void load_help()
{
	ACCESS_CHECK(PRIVILEGED());

	reset();
	load_rootdir();
}
