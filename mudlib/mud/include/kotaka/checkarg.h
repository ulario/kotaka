#define BADARG(n, func) do {\
	error("Bad argument " + (n) + " for function " + (func));\
} while (0)

#define CHECKARG(arg, n, func) do {\
	if (!(arg)) {\
		BADARG((n), (func));\
	}\
} while (0)
