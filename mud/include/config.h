/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2014, 2015  Raymond Jennings
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
# define USR_DIR		"/home"	/* default user directory */
# define INHERITABLE_SUBDIR	"/lib/"
# define CLONABLE_SUBDIR	"/obj/"
# define LIGHTWEIGHT_SUBDIR	"/lwo/"
# define CREATOR		"_F_sys_create"

# define SYS_PERSISTENT		/* off by default */

# define CALLOUTRSRC	TRUE	/* don't have callouts as a resource */

# if defined( __NETWORK_PACKAGE__ ) || defined( __NETWORK_EXTENSIONS__ )
#  define SYS_NETWORKING	/* Network package is enabled */
# endif
