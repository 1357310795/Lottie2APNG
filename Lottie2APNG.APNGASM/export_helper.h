#define APNGASM_EXPORTS
#ifdef APNGASM_EXPORTS
#define APNGASM_API __declspec(dllexport)
#else
#define APNGASM_API __declspec(dllimport)
#endif

