#include <config.h>

/**********/
/* System */
/**********/

/* daemons */

#define DEFAULT_MANAGER	(USR_DIR + "/System/sys/default_manager")
#define SYSTEM_DEFAULT_USER \
			(USR_DIR + "/System/sys/default_user")
#define ERRORD		(USR_DIR + "/System/sys/errord")
#define FILTERD		(USR_DIR + "/System/sys/filterd")
#define CALLOUTD	(USR_DIR + "/System/sys/calloutd")
#define INITD		(USR_DIR + "/System/initd")
#define LISTD		(USR_DIR + "/System/sys/listd")
#define LOGD		(USR_DIR + "/System/sys/logd")
#define KERNELD		(USR_DIR + "/System/sys/kerneld")
#define OBJECTD		(USR_DIR + "/System/sys/objectd")
#define PORTD		(USR_DIR + "/System/sys/portd")
#define PROGRAMD	(USR_DIR + "/System/sys/programd")
#define PROXYD		(USR_DIR + "/System/sys/proxyd")
#define TESTD		(USR_DIR + "/System/sys/testd")
#define TLSD		(USR_DIR + "/System/sys/tlsd")
#define TOUCHD		(USR_DIR + "/System/sys/touchd")
#define TRASHD		(USR_DIR + "/System/sys/trashd")
#define WATCHDOGD	(USR_DIR + "/System/sys/watchdogd")
#define SECRETD		(USR_DIR + "/System/sys/secretd")
#define STATUSD		(USR_DIR + "/System/sys/statusd")

/* libs */

#define CALL_GUARD		(USR_DIR + "/System/lib/auto/call_guard")
#define LIB_FILTER		(USR_DIR + "/System/lib/filter")
#define LIB_SYSTEM_USER		(USR_DIR + "/System/lib/user")
#define LIB_MANAGER		(USR_DIR + "/System/lib/manager")
#define LIB_INITD		(USR_DIR + "/System/lib/initd")
#define LIB_BIN			(USR_DIR + "/System/lib/bin")
#define LIB_LIST		(USR_DIR + "/System/lib/list")
#define SECOND_AUTO		(USR_DIR + "/System/lib/auto/second_auto")
#define UTILITY_STRING		(USR_DIR + "/System/lib/utility/string")
#define UTILITY_COMPILE		(USR_DIR + "/System/lib/utility/compile")
#define UTILITY_ALGORITHM	(USR_DIR + "/System/lib/utility/algorithm")

/* lwos */

#define LWO_CINFO		(USR_DIR + "/System/lwo/conn_info")
#define PROGRAM_INFO		(USR_DIR + "/System/lwo/program_info")

/**********/
/* Kotaka */
/**********/

/* daemons */

#define ACCOUNTD	(USR_DIR + "/Kotaka/sys/accountd")
#define CHANNELD	(USR_DIR + "/Kotaka/sys/channeld")
#define SUBD		(USR_DIR + "/Kotaka/sys/subd")
#define HELPD		(USR_DIR + "/Kotaka/sys/helpd")
#define KOTAKA_USERD	(USR_DIR + "/Kotaka/sys/userd")
#define DUMPD		(USR_DIR + "/Kotaka/sys/dumpd")
#define PROPERTYD	(USR_DIR + "/Kotaka/sys/propertyd")
#define STRINGD		(USR_DIR + "/Kotaka/sys/stringd")
#define PARSE_DUMP	(USR_DIR + "/Kotaka/sys/parse/dump")

/* libs */

#define LIB_OBJECT		(USR_DIR + "/Kotaka/lib/object")
#define LIB_KOTAKA_USER		(USR_DIR + "/Kotaka/lib/user")
#define LIB_USTATE		(USR_DIR + "/Kotaka/lib/ustate")
#define LIB_DRIVER		(USR_DIR + "/Kotaka/lib/driver")
#define LIB_DEEP_COPY		(USR_DIR + "/Kotaka/lib/deep_copy")
