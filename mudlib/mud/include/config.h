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
