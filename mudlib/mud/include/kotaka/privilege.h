#include <config.h>
#include <kernel/kernel.h>
#include <kernel/user.h>
#define KOTAKA()	(sscanf(previous_program(), USR_DIR + "/Kotaka/%*s"))
#define HELP()		(sscanf(previous_program(), USR_DIR + "/Help/%*s"))
#define GAME()		(sscanf(previous_program(), USR_DIR + "/Game/%*s"))
#define KADMIN()	(previous_program(1) == LIB_WIZTOOL && \
			previous_program() == USR_DIR + "/admin/_code")
#define CODE()		(previous_program(1) == LIB_WIZTOOL && \
			sscanf(previous_program(), USR_DIR + "/%*s/_code"))
#define LOCAL()		(calling_object() == this_object())
#define PRIVILEGED()	(KOTAKA() || GAME() || SYSTEM() || KADMIN())
#define ACCESS_CHECK(x)	do { if (!(x)) error("Access denied"); } while (0)
#define PERMISSION_CHECK(x)	do { if (!(x)) error("Permission denied"); } while (0)
