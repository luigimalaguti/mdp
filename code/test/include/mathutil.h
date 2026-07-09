#ifndef MATHUTIL_H
#define MATHUTIL_H

// C header, usable from C++ too thanks to the extern "C" guard
#ifdef __cplusplus
extern "C" {
#endif

int add(int a, int b);
int mul(int a, int b);

#ifdef __cplusplus
}
#endif

#endif // MATHUTIL_H
