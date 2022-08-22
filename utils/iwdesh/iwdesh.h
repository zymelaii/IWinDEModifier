typedef int (*fn_iwdesh_entry)(int, wchar_t*[]);

#define DEF_IWDESH_ENTRY(cmd, argc, argv) \
	extern "C" int iwdesh_entry_##cmd(int argc, wchar_t* argv[])

#define IWDESHENTRY(cmd) iwdesh_entry_##cmd