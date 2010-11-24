#include <kotaka/paths.h>

object make_lwo(string path);
object make_clone(string path);
object create_object(varargs object LIB_OBJECT *parents);
atomic void destroy_object(object LIB_OBJECT turkey,
	varargs int recursive);
