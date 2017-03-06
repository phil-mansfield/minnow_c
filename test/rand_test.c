#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "rand.h"

/* Note: this is the worst test in the Minnow library. Testing RNGs is a real
 * pain, so here we just look at them and see if they look roughly okay, which
 * will catch the most likely . The
 * assumption here is that we didn't accidentally break xorshift. */

void testSeed();
void testUint64();
void testUint64Lim();
void testFloat();
void testBool();

int main() {
    //testSeed();
    //testUint64();
    //testUint64Lim();
    //testFloat();
    //testBool();
    return 0;
}


void testSeed() {
    rand_State *s = rand_Seed(0, 30);
    for (int i = 0; i < 30; i++) {
        printf("%16"PRIx64" %16"PRIx64"\n", s[i][0], s[i][1]);
    }
    free(s);
}

void testUint64() {
    rand_State *s = rand_Seed(0, 1);
    for (int i = 0; i < 30; i++) {
        printf("%16"PRIx64"\n", rand_Uint64(s));
    }
    free(s);
}

void testUint64Lim() {
    rand_State *s = rand_Seed(0, 1);
    for (int i = 0; i < 30; i++) {
        printf("%"PRIu64"\n", rand_Uint63Lim(s, 5));
    }
    free(s);
}

void testFloat() {
    rand_State *s = rand_Seed(0, 1);
    for (int i = 0; i < 30; i++) {
        printf("%g\n", rand_Float(s));
    }
    free(s);
}

void testBool() {
    rand_State *s = rand_Seed(0, 1);
    for (int i = 0; i < 30; i++) {
        printf("%d\n", rand_Bool(s));
    }
    free(s);
}
