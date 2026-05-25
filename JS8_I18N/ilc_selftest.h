#ifndef ILC_SELFTEST_H
#define ILC_SELFTEST_H

// JS8CALL-CN in-app ILC self-test entry. [loc][test] -- invoked via the app's
// --ilc-selftest CLI option (see JS8_Main/main.cpp); not intended for upstream.

#include <QString>

// Run the ILC codec / framer / streaming self-test against the codebook found
// next to the executable. Writes a report to stdout; returns 0 on all-pass,
// non-zero on the first failing section.
int runIlcSelftest();

#endif // ILC_SELFTEST_H
