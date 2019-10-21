#include <assert.h>
#include <stddef.h>

struct exception {};

struct drop_check {
    bool* ok;
    ~drop_check() {
        if (ok)
            *ok = true;
    }
};

extern "C" {
    void rust_catch_callback(void (*cb)());

    static void callback() {
        throw exception();
    }

    void throw_cxx_exception() {
        try {
            rust_catch_callback(callback);
            assert(false && "unreachable");
        } catch (exception e) {
            return;
        }
        assert(false && "did not catch thrown C++ exception");
    }

    void cxx_catch_callback(void (*cb)(), bool* ok) {
        drop_check x;
#ifdef _WIN32
        // On windows catch (...) may or may not catch foreign exceptions
        // depending on the compiler and compiler options, so we don't check it
        // here.
        x.ok = ok;
#else
        x.ok = NULL;
#endif
        try {
            cb();
        } catch (exception e) {
            assert(false && "shouldn't be able to catch a rust panic");
        } catch (...) {
            // Foreign exceptions are caught by catch (...). We only set the ok
            // flag if we successfully caught the panic. The destructor of
            // drop_check will then set the flag to true if it is executed.
            x.ok = ok;
            throw;
        }
    }
}
