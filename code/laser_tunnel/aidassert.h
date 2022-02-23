#ifndef AIDASSERT_H
#define AIDASSERT_H

#ifdef NDEBUG
#define ASSERT(cond)
#else
#define ASSERT(cond) if (cond) {} else assertionFailure(F(#cond), F(__FILE__), __LINE__)

void assertionFailure(
    const __FlashStringHelper *cond,
    const __FlashStringHelper *file,
    int line
);
#endif


#endif
