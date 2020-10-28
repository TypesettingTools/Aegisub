// libass uses dirent in a function we don't use, so just provide a dummy version
typedef struct DIR { int dummy; } DIR;
typedef struct dirent { char *d_name; } dirent;
static inline DIR *opendir(const char *x) { return 0; }
static inline struct dirent *readdir(DIR *x) { return 0; }
static inline void closedir(DIR *x) { }
