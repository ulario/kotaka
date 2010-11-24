#include <config.h>
#include <kernel/user.h>
#include <kotaka/paths.h>

inherit SECOND_AUTO;
inherit LIB_WIZTOOL;

static void message(string msg)
{
	error("Must override");
}

static mixed clone_object(varargs mixed args...)
{
	error("Must override");
}

static mixed compile_object(varargs mixed args...)
{
	error("Must override");
}
