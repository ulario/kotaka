#define ASSERT(x)\
do {\
	if (!(x)) {\
		error("Assertion " + #x + " failed at " + __FILE__ + ":" + __LINE__);\
	}\
} while(0)
