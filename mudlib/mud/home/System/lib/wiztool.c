/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
