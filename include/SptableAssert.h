#ifndef SPTABLE_ASSERT_H
#define SPTABLE_ASSERT_H

#ifdef _DEBUG

#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak()
#else
#error Unsupported compiler
#endif

#define BREAK(expr) DEBUG_BREAK;
#define SPTABLE_ASSERT(expr) if (expr) {} else { BREAK(expr) }

#else
#define SPTABLE_ASSERT(expr)
#endif //DEBUG

#endif //SPTABLE_ASSERT_H
