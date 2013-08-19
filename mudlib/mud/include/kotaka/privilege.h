#include <config.h>
#include <kernel/kernel.h>
#include <kernel/user.h>

#define ACCOUNT()	(sscanf(previous_program(), USR_DIR + "/Account/%*s"))
#define BIGSTRUCT()	(sscanf(previous_program(), USR_DIR + "/Bigstruct/%*s"))
#define CATALOG()	(sscanf(previous_program(), USR_DIR + "/Catalog/%*s"))
#define GAME()		(sscanf(previous_program(), USR_DIR + "/Game/%*s"))
#define HELP()		(sscanf(previous_program(), USR_DIR + "/Help/%*s"))
#define INTERMUD()	(sscanf(previous_program(), USR_DIR + "/Intermud/%*s"))
#define KOTAKA()	(sscanf(previous_program(), USR_DIR + "/Kotaka/%*s"))
#define TEST()		(sscanf(previous_program(), USR_DIR + "/Test/%*s"))
#define TEXT()		(sscanf(previous_program(), USR_DIR + "/Text/%*s"))
#define THING()		(sscanf(previous_program(), USR_DIR + "/Thing/%*s"))
#define VERB()		(sscanf(previous_program(), USR_DIR + "/Verb/%*s"))

#define KADMIN()	(previous_program(1) == LIB_WIZTOOL && \
			previous_program() == USR_DIR + "/admin/_code")
#define CODE()		(previous_program(1) == LIB_WIZTOOL && \
			sscanf(previous_program(), USR_DIR + "/%*s/_code"))
#define LOCAL()		(calling_object() == this_object())

#define INTERFACE()	(TEXT() || SYSTEM() || GAME() || INTERMUD() || VERB())
#define PRIVILEGED()	(KOTAKA() || GAME() || SYSTEM() || KADMIN() || TEST())
#define ACCESS_CHECK(x)	do { if (!(x)) error("Access denied"); } while (0)
#define PERMISSION_CHECK(x)	do { if (!(x)) error("Permission denied"); } while (0)
