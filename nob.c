#ifdef _WIN32
# define EXE_SUFFIX ".exe"
#else
# define EXE_SUFFIX ""
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#ifdef _WIN32
int setenv(const char *name, const char *value, int overwrite) {
    if (name == NULL || *name == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    if (!overwrite) {
        size_t requiredSize;
        getenv_s(&requiredSize, NULL, 0, name);
        if (requiredSize > 0) return 0;
    }

    if (_putenv_s(name, value) != 0) return -1;

    return 0;
}
#endif

static bool walk_directory(
    File_Paths* dirs,
    File_Paths* c_sources,
    const char* path
) {
    DIR *dir = opendir(path);
    if(!dir) {
        nob_log(NOB_ERROR, "Could not open directory %s: %s", path, strerror(errno));
        return false;
    }
    errno = 0;
    struct dirent *ent;
    while((ent = readdir(dir))) {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0) continue;
        const char* fext = nob_get_ext(ent->d_name);
        size_t temp = nob_temp_save();
        const char* p = nob_temp_sprintf("%s/%s", path, ent->d_name); 
        Nob_File_Type type = nob_get_file_type(p);
        if(type == NOB_FILE_DIRECTORY) {
            da_append(dirs, p);
            if(!walk_directory(dirs, c_sources, p)) {
                closedir(dir);
                return false;
            }
            continue;
        }
        if(strcmp(fext, "c") == 0) {
            nob_da_append(c_sources, p);
            continue;
        }
        nob_temp_rewind(temp);
    }
    closedir(dir);
    return true;
}
int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    char* cc = getenv("CC");

    // TODO: automatic checks for the compiler 
    // available on the system. Maybe default to clang on bimbows
    #ifndef _WIN32
    if(!cc) cc = "cc";
    #else
    if(!cc) cc = "clang";
    #endif
    setenv("CC", cc, 0);
    char* bindir = getenv("BINDIR");
    if(!bindir) bindir = "bin";
    setenv("BINDIR", bindir, 0);

    nob_minimal_log_level = NOB_WARNING;
    if(!mkdir_if_not_exists(bindir)) return 1;
    if(!mkdir_if_not_exists(temp_sprintf("%s/bikeshed", bindir))) return 1;
    nob_minimal_log_level = NOB_INFO;
    
    // Building Raylib
    #define VENDOR_RAYLIB_NOB_FILEPATH "vendor/raylib-5.5/nob"
    #define VENDOR_RAYLIB_NOB_EXECUTABLE VENDOR_RAYLIB_NOB_FILEPATH EXE_SUFFIX
    
    Cmd cmd = { 0 };
    if(nob_needs_rebuild1(VENDOR_RAYLIB_NOB_EXECUTABLE, VENDOR_RAYLIB_NOB_FILEPATH".c")) {
        cmd_append(&cmd, NOB_REBUILD_URSELF(VENDOR_RAYLIB_NOB_EXECUTABLE, VENDOR_RAYLIB_NOB_FILEPATH".c"), "-I./");
#ifdef _WIN32
        cmd_append(&cmd, "-D_CRT_SECURE_NO_WARNINGS", "-Wno-deprecated-declarations");
#endif
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }
    cmd_append(&cmd, VENDOR_RAYLIB_NOB_EXECUTABLE);
    if(!cmd_run_sync_and_reset(&cmd)) return 1;


    File_Paths dirs = { 0 }, c_sources = { 0 };
    const char* src_dir = "src";
    size_t src_prefix_len = strlen(src_dir)+1;
    if(!walk_directory(&dirs, &c_sources, src_dir)) return 1;
    for(size_t i = 0; i < dirs.count; ++i) {
        nob_minimal_log_level = NOB_WARNING;
        if(!mkdir_if_not_exists(temp_sprintf("%s/bikeshed/%s", bindir, dirs.items[i] + src_prefix_len))) return 1;
        nob_minimal_log_level = NOB_INFO;
    }
    File_Paths objs = { 0 };
    String_Builder stb = { 0 };
    File_Paths pathb = { 0 };
    for(size_t i = 0; i < c_sources.count; ++i) {
        const char* src = c_sources.items[i];
        const char* out = temp_sprintf("%s/bikeshed/%.*s.o", bindir, (int)(strlen(src + src_prefix_len)-2), src + src_prefix_len);
        da_append(&objs, out);
        if(!nob_c_needs_rebuild1(&stb, &pathb, out, src)) continue;
        // C compiler
        cmd_append(&cmd, cc);
        // C standard
        cmd_append(&cmd, "-std=c11");
        // Warnings
        cmd_append(&cmd,
            "-Wall",
            "-Wextra",
    // on binbows there are SOOOO many warnings and some of them are unfixable so frick it
    #ifndef _WIN32
        #if 1
            "-pedantic",
            "-Werror",
        #endif
    #else
        "-D_CRT_SECURE_NO_WARNINGS", "-Wno-deprecated-declarations",
    #endif
        );
        // Include directories 
        cmd_append(&cmd,
            "-I", "vendor/raylib-5.5/src",
            "-I", "include",
        );
        // Actual compilation
        cmd_append(&cmd,
            "-MP", "-MMD", "-O1", "-g", "-c",
            src,
            "-o", out,
        );
        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    #define EXECUTABLE "bikeshed" EXE_SUFFIX

    if(needs_rebuild(EXECUTABLE, objs.items, objs.count)) {
        cmd_append(&cmd, cc, "-o", EXECUTABLE);
        da_append_many(&cmd, objs.items, objs.count);
        // Vendor libraries we link with
        cmd_append(&cmd, 
            "-Lbin/raylib",
        #ifndef _WIN32
            "-l:libraylib.a",
            "-lm"
        #else
            "-lraylib",
            "-luser32", "-lgdi32", "-lwinmm", "-lshell32", "-lkernel32", "-lopengl32", //bagilion windows things
        #endif
        );

        if(!cmd_run_sync_and_reset(&cmd)) return 1;
    }
}
