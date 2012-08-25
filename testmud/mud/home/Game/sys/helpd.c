#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>
#include <type.h>

object helpdb;	/* ([ key : ([ entry : 1 ]) ]) */

static void create()
{
	helpdb = clone_object(BIGSTRUCT_MAP_OBJ);
	helpdb->set_type(T_STRING);
}

static void destruct()
{
	destruct_object(helpdb);
}

private void add_to_index(string key, string topic)
{
	mapping list;

	list = helpdb->get_element(key);

	if (!list) {
		list = ([ ]);
		helpdb->set_element(key, list);
	}

	list[topic] = 1;
}

private void remove_from_index(string key, string topic)
{
	mapping list;

	list = helpdb->get_element(key);

	if (!list) {
		return;
	}

	list[topic] = nil;

	if (!map_sizeof(list)) {
		helpdb->set_element(key, nil);
	}
}

atomic void add_topic(string topic)
{
	string *parts;
	int i;
	int sz;

	ACCESS_CHECK(PRIVILEGED());

	parts = explode("/" + topic + "/", "/");

	if (sizeof(parts & ({ "" }))) {
		error("Invalid topic");
	}

	sz = sizeof(parts);

	for (i = 0; i < sz; i++) {
		add_to_index(implode(parts[i .. sz - 1], "/"), topic);
	}
}

atomic void remove_topic(string topic)
{
	string *parts;
	int i;
	int sz;

	ACCESS_CHECK(PRIVILEGED());

	parts = explode("/" + topic + "/", "/");

	if (sizeof(parts & ({ "" }))) {
		error("Invalid topic");
	}

	sz = sizeof(parts);

	for (i = 0; i < sz; i++) {
		remove_from_index(implode(parts[i .. sz - 1], "/"), topic);
	}
}

void reset()
{
	ACCESS_CHECK(PRIVILEGED());

	helpdb->clear();
}

string *query_topics(string key)
{
	mapping list;

	if (sizeof(explode("/" + key + "/", "/") & ({ "" }))) {
		error("Invalid key");
	}

	list = helpdb->get_element(key);

	if (!list) {
		return ({ });
	}

	return map_indices(list);
}

private void load_helpdir(string dir)
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	dirlist = get_dir("~/data/help/" + dir + "/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		if (sizes[i] == -2) {
			load_helpdir(dir + "/" + names[i]);
		} else if (sscanf(names[i], "%s.hlp", entry)) {
			add_topic(dir + "/" + entry);
		}
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

		if (sizes[i] == -2) {
			load_helpdir(names[i]);
		} else if (sscanf(names[i], "%s.hlp", entry)) {
			add_topic(entry);
		}
	}
}

void load_help()
{
	ACCESS_CHECK(PRIVILEGED());

	helpdb->clear();
	load_rootdir();
}
