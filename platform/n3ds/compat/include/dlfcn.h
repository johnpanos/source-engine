// Nintendo 3DS compatibility: the dynamic loader interface with no loader.
// 3DS products are statically composed (scripts/waifulib/static_composition.py);
// every first-party module is linked into the program, and dlopen refuses
// every request by name so a hidden loader dependency fails visibly.
#ifndef N3DS_COMPAT_DLFCN_H
#define N3DS_COMPAT_DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY 0x0001
#define RTLD_NOW 0x0002
#define RTLD_GLOBAL 0x0100
#define RTLD_LOCAL 0x0000
#define RTLD_NOLOAD 0x0004
#define RTLD_NODELETE 0x1000
#define RTLD_DEFAULT ( (void *)0 )
#define RTLD_NEXT ( (void *)-1 )

typedef struct
{
	const char *dli_fname;
	void *dli_fbase;
	const char *dli_sname;
	void *dli_saddr;
} Dl_info;

void *dlopen( const char *file, int mode );
void *dlsym( void *handle, const char *name );
int dlclose( void *handle );
char *dlerror( void );
int dladdr( const void *address, Dl_info *info );

#ifdef __cplusplus
}
#endif

#endif
