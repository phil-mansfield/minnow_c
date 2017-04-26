#include "semver.h"
#include "debug.h"
#include <string.h>

typedef struct {
    uint8_t stage, major, minor, patch;
} semver;

bool semver_Greater(uint32_t v1, uint32_t v2) {
    return (v1 & 0xffffff) > (v2 & 0xffffff);
}

bool semver_Equals(uint32_t v1, uint32_t v2) {
    return (v1 & 0xffffff) == (v2 & 0xffffff);
}

void semver_ToString(uint32_t version, char *buf) {
    switch (semver_Stage(version)) {
        case semver_DEV:
            sprintf(buf, "%d.%d.%d-dev", semver_Major(version),
                    semver_Minor(version), semver_Patch(version));
            return;
        case semver_ALPHA:
            sprintf(buf, "%d.%d.%d-alpha", semver_Major(version),
                    semver_Minor(version), semver_Patch(version));
            return;
        case semver_BETA:
            sprintf(buf, "%d.%d.%d-beta", semver_Major(version),
                    semver_Minor(version), semver_Patch(version));
            return;
        case semver_RC:
            sprintf(buf, "%d.%d.%d-rc", semver_Major(version),
                    semver_Minor(version), semver_Patch(version));
            return;
        case semver_RELEASE:
            sprintf(buf, "%d.%d.%d", semver_Major(version),
                    semver_Minor(version), semver_Patch(version));
            return;
    }

    assert(0);
}

const int NUM_BUF_SIZE = 9;

uint32_t semver_FromString(char *s) {
    int n = (int) strlen(s);

    if (n > semver_BUF_SIZE - 1) {
        Panic("Cannot parse %s: too many characters.", s);
    }

    char numBuf[NUM_BUF_SIZE];
    int i = 0;
    for (; i < n && s[i] != '-'; i++);
    if (i > NUM_BUF_SIZE - 1) {
        Panic("Cannot parse %s: too many characters (probably using more"
              "than three digits in the version numbers).", s);
    }

    memcpy(numBuf, s, n);
    numBuf[i] = '\0';

    int major, minor, patch;
    int parseCount = sscanf(numBuf, "%d.%d.%d", &major, &minor, &patch);
    if (parseCount != 3) {
        Panic("Could not parse the version numbers in %s.", s);
    } else if (major > 256) {
        Panic("Major version in %s is too big.", s);
    } else if (minor > 256) {
        Panic("Minor version in %s is too big.", s);
    } else if (patch > 256) {
        Panic("Patch version in %s is too big.", s);
    }

    int stage;
    if (i == n) {        
        stage = 4;
    } else {
        if (!strcmp(s + i, "dev")) {
            stage = 0;
        } else if (!strcmp(s + i, "alpha")) {
            stage = 1;
        } else if (!strcmp(s + i, "beta")) {
            stage = 2;
        } else if (!strcmp(s + i, "rc")) {
            stage = 3;
        } else {
            Panic("Did not recognize stage string in %s.", s);
        }
    }

    return (((uint32_t) stage) << 24) + (((uint32_t) major) << 16) +
        (((uint32_t) minor) << 8) + (uint32_t) patch;
}

enum semver_Stage semver_Stage(uint32_t version) {
    switch (((semver*)&version)->stage) {
    case 0:
        return semver_DEV;
    case 1:
        return semver_ALPHA;
    case 2:
        return semver_BETA;
    case 3:
        return semver_RC;
    case 4:
        return semver_RELEASE;
    }

    assert(0);
}

uint8_t semver_Patch(uint32_t version) {
    return ((semver*)&version)->patch;
}

uint8_t semver_Minor(uint32_t version) {
    return ((semver*)&version)->minor;
}

uint8_t semver_Major(uint32_t version) {
    return ((semver*)&version)->major;
}
