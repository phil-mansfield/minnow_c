#ifndef MNW_SEMVER_H_
#define MNW_SEMVER_H_

#include <stdbool.h>
#include <stdint.h>

const int semver_BUF_SIZE = 15;

enum semver_Stage {
    semver_DEV,
    semver_ALPHA,
    semver_BETA,
    semver_RC,
    semver_RELEASE
};

bool semver_Greater(uint32_t v1, uint32_t v2);
bool semver_Equals(uint32_t v1, uint32_t v2);

void semver_ToString(uint32_t version, char *buf);
uint32_t semver_FromString(char *s);

enum semver_Stage semver_Stage(uint32_t version);
uint8_t semver_Patch(uint32_t version);
uint8_t semver_Minor(uint32_t version);
uint8_t semver_Major(uint32_t version);

#endif
