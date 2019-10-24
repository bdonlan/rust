#include <assert.h>
#include <stddef.h>
#include <stdio.h>

void println(const char* s) {
    puts(s);
    fflush(stdout);
}

struct exception {};

struct drop_check {
    bool* ok;
    ~drop_check() {
        printf("~drop_check\n");

        if (ok)
            *ok = true;
    }
};

extern "C" {
    void rust_catch_callback(void (*cb)(), bool* rust_ok);

    static void callback() {
        println("throwing C++ exception");
        throw exception();
    }

    void throw_cxx_exception() {
        bool rust_ok = false;
        try {
            rust_catch_callback(callback, &rust_ok);
            assert(false && "unreachable");
        } catch (exception e) {
            println("caught C++ exception");
            return;
        }
        assert(rust_ok);
        assert(false && "did not catch thrown C++ exception");
    }

    void cxx_catch_callback(void (*cb)(), bool* cxx_ok) {
        drop_check x;
#ifdef _WIN32
        // On windows catch (...) may or may not catch foreign exceptions
        // depending on the compiler and compiler options, so we don't check it
        // here.
        x.ok = cxx_ok;
#else
        x.ok = NULL;
#endif
        try {
            cb();
        } catch (exception e) {
            assert(false && "shouldn't be able to catch a rust panic");
        } /*catch (...) {
            println("caught foreign exception in catch (...)");
            // Foreign exceptions are caught by catch (...). We only set the ok
            // flag if we successfully caught the panic. The destructor of
            // drop_check will then set the flag to true if it is executed.
            x.ok = cxx_ok;
            throw;
        }*/
    }
}
