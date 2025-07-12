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

static const char *raylib_modules[] = {
    "rcore",
    "raudio",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};

#define RAYLIB_SRC_FOLDER "vendor/raylib-5.5/src/"

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
    const char* build_path = temp_sprintf("%s/raylib", bindir);
    if(!mkdir_if_not_exists(build_path)) return 1;
    nob_minimal_log_level = NOB_INFO;

    Cmd cmd = {0};

    File_Paths object_files = {0};
    Procs procs = {0};

    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
        const char *input_path = temp_sprintf(RAYLIB_SRC_FOLDER"%s.c", raylib_modules[i]);
        const char *output_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
        output_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);

        da_append(&object_files, output_path);

        if (needs_rebuild(output_path, &input_path, 1)) {
            cmd_append(&cmd, cc,
                "-ggdb", "-DPLATFORM_DESKTOP", 
            #ifndef _WIN32
                "-D_GLFW_X11", "-fPIC",
            #endif
                "-DSUPPORT_FILEFORMAT_FLAC=1",
                "-I"RAYLIB_SRC_FOLDER"external/glfw/include",
                "-c", input_path,
                "-o", output_path);
            da_append(&procs, cmd_run_async_and_reset(&cmd));
        }
    }

    if (!procs_wait_and_reset(&procs)) return 1;

    #ifndef _WIN32
    #define LIBRARY_OUT_FILENAME "libraylib.a"
    #else
    #define LIBRARY_OUT_FILENAME "raylib.lib"
    #endif

    #ifndef _WIN32
    #define AR "ar"
    #else
    #define AR "llvm-ar"
    #endif

    const char *libraylib_path = temp_sprintf("%s/"LIBRARY_OUT_FILENAME, build_path);

    if (needs_rebuild(libraylib_path, object_files.items, object_files.count)) {
        cmd_append(&cmd, AR, "-crs", libraylib_path);
        for (size_t i = 0; i < ARRAY_LEN(raylib_modules); ++i) {
            const char *input_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
            cmd_append(&cmd, input_path);
        }
        if (!cmd_run_sync_and_reset(&cmd)) return 1;
    }

    return 0;
}
