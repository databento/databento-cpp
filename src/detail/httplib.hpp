#pragma once

// The only place the library includes httplib. Including <httplib.h> directly
// risks compiling it with a different configuration than the rest of the
// library, which previously resulted in a segfault when databento-cpp was
// installed at the system level.
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>
