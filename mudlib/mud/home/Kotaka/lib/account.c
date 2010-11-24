#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#ifndef ACCOUNTD
#define ACCOUNTD "~Common/sys/accountd"
#endif

inherit LIB_DEEP_COPY;

int dirty;
string username;
mapping properties;

static void create()
{
	properties = ([ ]);
	dirty = 0;
}

int is_dirty()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	return dirty;
}

void set_dirty()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	dirty = 1;
}

void clear_dirty()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	dirty = 0;
}

void set_username(string new_username)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	username = new_username;
}

string query_username()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	return username;
}

void set_property(string key, mixed value)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());
	
	ACCOUNTD->validate_property(key, value);
	
	properties[key] = value;
	
	dirty = 1;
}

mixed query_property(string key)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());
	
	return properties[key];
}

string *list_properties()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	return map_indices(properties);
}

void load(mapping input)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	properties = deep_copy(input);
}

mapping save()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	return deep_copy(properties);
}
