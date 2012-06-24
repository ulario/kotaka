#include <kernel/kernel.h>
#include <config.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

static void create()
{
}

/* lets users and subsystems alike store stuff secretly */
string proper_path(string given, string user)
{
	return DRIVER->normalize_path(given, USR_DIR + "/Secret/" + user);
}

void validate_path(string path, string user)
{
	if (path != USR_DIR + "/Secret/" + user) {
		PERMISSION_CHECK(
			sscanf(path, USR_DIR + "/Secret/" + user + "/%*s")
		);
	}
}

/* files */
string read_file(string file, varargs int offset, int size)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::read_file(file, offset, size);
}

int remove_file(string file)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::remove_file(file);
}

int rename_file(string from, string to)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	from = proper_path(from, user);
	validate_path(from, user);

	to = proper_path(to, user);
	validate_path(to, user);

	return ::rename_file(from, to);
}

int write_file(string file, string str, varargs int offset)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::write_file(file, str, offset);
}

mixed *file_info(string file)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	file = proper_path(file, user);
	validate_path(file, user);

	return ::file_info(file);
}

/* void file_info(); */

/* directories */

mixed **get_dir(string path)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	path = proper_path(path, user);
	validate_path(path, user);

	return ::get_dir(path);
}

int make_dir(string path)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	path = proper_path(path, user);
	validate_path(path, user);

	return ::make_dir(path);
}

int remove_dir(string path)
{
	string user;

	user = DRIVER->creator(object_name(previous_object()));

	ACCESS_CHECK(user);

	path = proper_path(path, user);
	validate_path(path, user);

	return ::remove_dir(path);
}
