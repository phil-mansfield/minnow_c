#!/bin/bash
clang -Wno-float-equal -Wno-missing-noreturn -Weverything -Wno-padded -Werror -lm register.c quant.c funcs.c
