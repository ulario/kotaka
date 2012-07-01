#include <kernel/kernel.h>
#include <kernel/user.h>
#include <kernel/access.h>

#include <kotaka/paths.h>

inherit auto SECOND_AUTO;
inherit user LIB_WIZTOOL;

/*
 * NAME:	compile_object()
 * DESCRIPTION:	compile_object wrapper
 */
static object compile_object(string path, string source...)
{
    int kernel;

    path = DRIVER->normalize_path(path, query_directory(), query_owner());
    kernel = sscanf(path, "/kernel/%*s");
    if ((sizeof(source) != 0 && kernel) ||
	!access(query_owner(), path,
		((sscanf(path, "%*s" + INHERITABLE_SUBDIR) != 0 ||
		  !DRIVER->creator(path)) && sizeof(source) == 0 && !kernel) ?
		 READ_ACCESS : WRITE_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return (sizeof(source) != 0) ?
	    auto::compile_object(path, source...) : auto::compile_object(path);
}

/*
 * NAME:	clone_object()
 * DESCRIPTION:	clone_object wrapper
 */
static object clone_object(string path)
{
    path = DRIVER->normalize_path(path, query_directory(), query_owner());
    if (sscanf(path, "/kernel/%*s") != 0 || !access(query_owner(), path, READ_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return auto::clone_object(path);
}
