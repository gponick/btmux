// stringutil.cpp -- string utilities.
//
// $Id: stringutil.cpp,v 1.71 2005/08/05 17:11:07 sdennis Exp $
//
// MUX 2.4
// Copyright (C) 1998 through 2004 Solid Vertical Domains, Ltd. All
// rights not explicitly given are reserved.
//
#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"

#include "ansi.h"
#include "pcre.h"

const bool mux_isprint[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 3
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 5
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  // 7

    0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0,  // 8
    0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // A
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

const bool mux_isdigit[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

const bool mux_ishex[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 5
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

const bool mux_isalpha[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

const bool mux_isalnum[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

const bool mux_isupper[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

const bool mux_islower[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

const bool mux_isspace[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

// The first character of an attribute name must be either alphabetic,
// '_', '#', '.', or '~'. It's handled by the following table.
//
// Characters thereafter may be letters, numbers, and characters from
// the set {'?!`/-_.@#$^&~=+<>()}. Lower-case letters are turned into
// uppercase before being used, but lower-case letters are valid input.
//
bool mux_AttrNameInitialSet[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,  // 7

    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

bool mux_AttrNameSet[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1,  // 3
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,  // 5
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,  // 7

    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

// Valid characters for an object name are all printable
// characters except those from the set {=&|}.
//
const bool mux_ObjectNameSet[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,  // 3
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 5
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0,  // 7

    0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0,  // 8
    0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // A
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

// Valid characters for a player name are all alphanumeric plus
// {`$_-.,'} plus SPACE depending on configuration.
//
bool mux_PlayerNameSet[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  // 5
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // B
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  // D
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
    1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0   // F
};

// Characters which should be escaped for the secure()
// function: '%$\[](){},;'.
//
const bool mux_issecure[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

// Characters which should be escaped for the escape()
// function: '%\[]{};,()^$'.
//
const bool mux_isescape[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

const bool ANSI_TokenTerminatorTable[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

const unsigned char mux_hex2dec[256] =
{
//  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
//
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 0
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 1
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 2
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,  // 3
    0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 4
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 5
    0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 6
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 7

    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 8
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // A
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // B
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // C
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // D
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // E
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   // F
};

const unsigned char mux_toupper[256] =
{
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
//
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 1
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 2
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 3
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 4
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 5
    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 6
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // 7

    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, // 8
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x8A, 0x9B, 0x8C, 0x9D, 0x8E, 0x9F, // 9
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, // A
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, // B
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, // C
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, // D
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, // E
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xF7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xFF  // F
};

const unsigned char mux_tolower[256] =
{
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
//
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 1
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 2
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 3
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 4
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 5
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 6
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // 7

    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x9A, 0x8B, 0x9C, 0x8D, 0x9E, 0x8F, // 8
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0xFF, // 9
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, // A
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, // B
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, // C
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xD7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xDF, // D
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, // E
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF  // F
};

const unsigned char mux_StripAccents[256] =
{
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
//
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 1
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 2
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 3
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 4
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 5
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 6
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // 7

    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, // 8
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, // 9
    0xA0, 0x21, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0x22, 0xAC, 0xAD, 0xAE, 0xAF, // A
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0x22, 0xBC, 0xBD, 0xBE, 0x3F, // B
    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0xC6, 0x43, 0x45, 0x45, 0x45, 0x45, 0x49, 0x49, 0x49, 0x49, // C
    0x44, 0x4E, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0xD7, 0x4F, 0x55, 0x55, 0x55, 0x55, 0x59, 0x50, 0x42, // D
    0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0xE6, 0x63, 0x65, 0x65, 0x65, 0x65, 0x69, 0x69, 0x69, 0x69, // E
    0x6F, 0x6E, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0xF7, 0x6F, 0x75, 0x75, 0x75, 0x75, 0x79, 0x70, 0x79, // F
};

// ANSI_lex - This function parses a string and returns two token types.
// The type identifies the token type of length nLengthToken0. nLengthToken1
// may also be present and is a token of the -other- type.
//
int ANSI_lex(int nString, const char *pString, int *nLengthToken0, int *nLengthToken1)
{
    *nLengthToken0 = 0;
    *nLengthToken1 = 0;

    const char *p = pString;

    for (;;)
    {
        // Look for an ESC_CHAR
        //
        p = strchr(p, ESC_CHAR);
        if (!p)
        {
            // This is the most common case by far.
            //
            *nLengthToken0 = nString;
            return TOKEN_TEXT_ANSI;
        }

        // We have an ESC_CHAR. Let's look at the next character.
        //
        if (p[1] != '[')
        {
            // Could be a '\0' or another non-'[' character.
            // Move the pointer to position ourselves over it.
            // And continue looking for an ESC_CHAR.
            //
            p = p + 1;
            continue;
        }

        // We found the beginning of an ANSI sequence.
        // Find the terminating character.
        //
        const char *q = p+2;
        while (ANSI_TokenTerminatorTable[(unsigned char)*q] == 0)
        {
            q++;
        }
        if (q[0] == '\0')
        {
            // There was no good terminator. Treat everything like text.
            // Also, we are at the end of the string, so just return.
            //
            *nLengthToken0 = q - pString;
            return TOKEN_TEXT_ANSI;
        }
        else
        {
            // We found an ANSI sequence.
            //
            if (p == pString)
            {
                // The ANSI sequence started it.
                //
                *nLengthToken0 = q - pString + 1;
                return TOKEN_ANSI;
            }
            else
            {
                // We have TEXT followed by an ANSI sequence.
                //
                *nLengthToken0 = p - pString;
                *nLengthToken1 = q - p + 1;
                return TOKEN_TEXT_ANSI;
            }
        }
    }
}

char *strip_ansi(const char *szString, size_t *pnString)
{
    static char Buffer[LBUF_SIZE];
    char *pBuffer = Buffer;

    const char *pString = szString;
    if (!pString)
    {
        if (pnString)
        {
            *pnString = 0;
        }
        *pBuffer = '\0';
        return Buffer;
    }
    size_t nString = strlen(szString);

    while (nString)
    {
        int nTokenLength0;
        int nTokenLength1;
        int iType = ANSI_lex(nString, pString, &nTokenLength0, &nTokenLength1);

        if (iType == TOKEN_TEXT_ANSI)
        {
            memcpy(pBuffer, pString, nTokenLength0);
            pBuffer += nTokenLength0;

            int nSkipLength = nTokenLength0 + nTokenLength1;
            nString -= nSkipLength;
            pString += nSkipLength;
        }
        else
        {
            // TOKEN_ANSI
            //
            nString -= nTokenLength0;
            pString += nTokenLength0;
        }
    }
    if (pnString)
    {
        *pnString = pBuffer - Buffer;
    }
    *pBuffer = '\0';
    return Buffer;
}

char *strip_accents(const char *szString, size_t *pnString)
{
    static char Buffer[LBUF_SIZE];
    char *pBuffer = Buffer;

    const char *pString = szString;
    if (pString)
    {
        while (*pString)
        {
            *pBuffer = mux_StripAccents(*pString);
            pBuffer++;
            pString++;
        }
    }
    if (pnString)
    {
        *pnString = pBuffer - Buffer;
    }
    *pBuffer = '\0';
    return Buffer;
}

#define ANSI_COLOR_INDEX_BLACK     0
#define ANSI_COLOR_INDEX_RED       1
#define ANSI_COLOR_INDEX_GREEN     2
#define ANSI_COLOR_INDEX_YELLOW    3
#define ANSI_COLOR_INDEX_BLUE      4
#define ANSI_COLOR_INDEX_MAGENTA   5
#define ANSI_COLOR_INDEX_CYAN      6
#define ANSI_COLOR_INDEX_WHITE     7
#define ANSI_COLOR_INDEX_DEFAULT   8

const ANSI_ColorState acsRestingStates[3] =
{
    {true,  false, false, false, false, ANSI_COLOR_INDEX_DEFAULT, ANSI_COLOR_INDEX_DEFAULT},
    {false, false, false, false, false, ANSI_COLOR_INDEX_WHITE,   ANSI_COLOR_INDEX_DEFAULT},
    {true,  false, false, false, false, ANSI_COLOR_INDEX_DEFAULT, ANSI_COLOR_INDEX_DEFAULT}
};

void ANSI_Parse_m(ANSI_ColorState *pacsCurrent, int nANSI, const char *pANSI,
                  bool *pbSawNormal)
{
    // If the last character isn't an 'm', then it's an ANSI sequence we
    // don't support, yet. TODO: There should be a ANSI_Parse() function
    // that calls into this one -only- if there's an 'm', but since 'm'
    // is the only command this game understands at the moment, it's easier
    // to put the test here.
    //
    if (pANSI[nANSI-1] != 'm')
    {
        return;
    }

    // Process entire string and update the current color state structure.
    //
    while (nANSI)
    {
        // Process the next attribute phrase (terminated by ';' or 'm'
        // typically).
        //
        const char *p = pANSI;
        while (mux_isdigit(*p))
        {
            p++;
        }
        size_t nLen = p - pANSI + 1;
        if (p[0] == 'm' || p[0] == ';')
        {
            // We have an attribute.
            //
            if (nLen == 2)
            {
                int iCode = pANSI[0] - '0';
                switch (iCode)
                {
                case 0:
                    // Normal.
                    //
                    *pacsCurrent = acsRestingStates[ANSI_ENDGOAL_NORMAL];
                    *pbSawNormal = true;
                    break;

                case 1:
                    // High Intensity.
                    //
                    pacsCurrent->bHighlite = true;
                    pacsCurrent->bNormal = false;
                    break;

                case 2:
                    // Low Intensity.
                    //
                    pacsCurrent->bHighlite = false;
                    pacsCurrent->bNormal = false;
                    break;

                case 4:
                    // Underline.
                    //
                    pacsCurrent->bUnder = true;
                    pacsCurrent->bNormal = false;
                    break;

                case 5:
                    // Blinking.
                    //
                    pacsCurrent->bBlink = true;
                    pacsCurrent->bNormal = false;
                    break;

                case 7:
                    // Reverse Video
                    //
                    pacsCurrent->bInverse = true;
                    pacsCurrent->bNormal = false;
                    break;
                }
            }
            else if (nLen == 3)
            {
                int iCode0 = pANSI[0] - '0';
                int iCode1 = pANSI[1] - '0';
                if (iCode0 == 3)
                {
                    // Foreground Color
                    //
                    if (iCode1 <= 7)
                    {
                        pacsCurrent->iForeground = iCode1;
                        pacsCurrent->bNormal = false;
                    }
                }
                else if (iCode0 == 4)
                {
                    // Background Color
                    //
                    if (iCode1 <= 7)
                    {
                        pacsCurrent->iBackground = iCode1;
                        pacsCurrent->bNormal = false;
                    }
                }
            }
        }
        pANSI += nLen;
        nANSI -= nLen;
    }
}

// The following is really 30 (E[0mE[1mE[4mE[5mE[7mE[33mE[43m) but we are
// being conservative.
//
#define ANSI_MAXIMUM_BINARY_TRANSITION_LENGTH 60

// Generate the minimal ANSI sequence that will transition from one color state
// to another.
//
char *ANSI_TransitionColorBinary
(
    ANSI_ColorState *acsCurrent,
    const ANSI_ColorState *pcsNext,
    int *nTransition,
    int  iEndGoal
)
{
    static char Buffer[ANSI_MAXIMUM_BINARY_TRANSITION_LENGTH+1];

    if (memcmp(acsCurrent, pcsNext, sizeof(ANSI_ColorState)) == 0)
    {
        *nTransition = 0;
        Buffer[0] = '\0';
        return Buffer;
    }
    ANSI_ColorState tmp = *acsCurrent;
    char *p = Buffer;

    if (pcsNext->bNormal)
    {
        // With NOBLEED, we can't stay in the normal mode. We must eventually
        // be on a white foreground.
        //
        pcsNext = &acsRestingStates[iEndGoal];
    }

    // Do we need to go through the normal state?
    //
    if (  tmp.bHighlite && !pcsNext->bHighlite
       || tmp.bUnder    && !pcsNext->bUnder
       || tmp.bBlink    && !pcsNext->bBlink
       || tmp.bInverse  && !pcsNext->bInverse
       || (  tmp.iBackground != ANSI_COLOR_INDEX_DEFAULT
          && pcsNext->iBackground == ANSI_COLOR_INDEX_DEFAULT)
       || (  tmp.iForeground != ANSI_COLOR_INDEX_DEFAULT
          && pcsNext->iForeground == ANSI_COLOR_INDEX_DEFAULT))
    {
        memcpy(p, ANSI_NORMAL, sizeof(ANSI_NORMAL)-1);
        p += sizeof(ANSI_NORMAL)-1;
        tmp = acsRestingStates[ANSI_ENDGOAL_NORMAL];
    }
    if (tmp.bHighlite != pcsNext->bHighlite)
    {
        memcpy(p, ANSI_HILITE, sizeof(ANSI_HILITE)-1);
        p += sizeof(ANSI_HILITE)-1;
    }
    if (tmp.bUnder != pcsNext->bUnder)
    {
        memcpy(p, ANSI_UNDER, sizeof(ANSI_UNDER)-1);
        p += sizeof(ANSI_UNDER)-1;
    }
    if (tmp.bBlink != pcsNext->bBlink)
    {
        memcpy(p, ANSI_BLINK, sizeof(ANSI_BLINK)-1);
        p += sizeof(ANSI_BLINK)-1;
    }
    if (tmp.bInverse != pcsNext->bInverse)
    {
        memcpy(p, ANSI_INVERSE, sizeof(ANSI_INVERSE)-1);
        p += sizeof(ANSI_INVERSE)-1;
    }
    if (tmp.iForeground != pcsNext->iForeground)
    {
        memcpy(p, ANSI_FOREGROUND, sizeof(ANSI_FOREGROUND)-1);
        p += sizeof(ANSI_FOREGROUND)-1;
        *p++ = pcsNext->iForeground + '0';
        *p++ = ANSI_ATTR_CMD;
    }
    if (tmp.iBackground != pcsNext->iBackground)
    {
        memcpy(p, ANSI_BACKGROUND, sizeof(ANSI_BACKGROUND)-1);
        p += sizeof(ANSI_BACKGROUND)-1;
        *p++ = pcsNext->iBackground + '0';
        *p++ = ANSI_ATTR_CMD;
    }
    *p = '\0';
    *nTransition = p - Buffer;
    return Buffer;
}

// The following is really 21 (%xn%xh%xu%xi%xf%xR%xr) but we are being conservative
//
#define ANSI_MAXIMUM_ESCAPE_TRANSITION_LENGTH 42

// Generate the minimal MU ANSI %-sequence that will transition from one color state
// to another.
//
char *ANSI_TransitionColorEscape
(
    ANSI_ColorState *acsCurrent,
    ANSI_ColorState *acsNext,
    int *nTransition
)
{
    static char Buffer[ANSI_MAXIMUM_ESCAPE_TRANSITION_LENGTH+1];
    static const char cForegroundColors[9] = "xrgybmcw";
    static const char cBackgroundColors[9] = "XRGYBMCW";

    if (memcmp(acsCurrent, acsNext, sizeof(ANSI_ColorState)) == 0)
    {
        *nTransition = 0;
        Buffer[0] = '\0';
        return Buffer;
    }
    ANSI_ColorState tmp = *acsCurrent;
    int  i = 0;

    // Do we need to go through the normal state?
    //
    if (  tmp.bBlink    && !acsNext->bBlink
       || tmp.bHighlite && !acsNext->bHighlite
       || tmp.bInverse  && !acsNext->bInverse
       || (  tmp.iBackground != ANSI_COLOR_INDEX_DEFAULT
          && acsNext->iBackground == ANSI_COLOR_INDEX_DEFAULT)
       || (  tmp.iForeground != ANSI_COLOR_INDEX_DEFAULT
          && acsNext->iForeground == ANSI_COLOR_INDEX_DEFAULT))
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = 'n';
        i = i + 3;
        tmp = acsRestingStates[ANSI_ENDGOAL_NORMAL];
    }
    if (tmp.bHighlite != acsNext->bHighlite)
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = 'h';
        i = i + 3;
    }
    if (tmp.bUnder != acsNext->bUnder)
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = 'u';
        i = i + 3;
    }
    if (tmp.bBlink != acsNext->bBlink)
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = 'f';
        i = i + 3;
    }
    if (tmp.bInverse != acsNext->bInverse)
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = 'i';
        i = i + 3;
    }
    if (tmp.iForeground != acsNext->iForeground)
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = cForegroundColors[acsNext->iForeground];
        i = i + 3;
    }
    if (tmp.iBackground != acsNext->iBackground)
    {
        Buffer[i  ] = '%';
        Buffer[i+1] = 'x';
        Buffer[i+2] = cBackgroundColors[acsNext->iBackground];
        i = i + 3;
    }
    Buffer[i] = '\0';
    *nTransition = i;
    return Buffer;
}

void ANSI_String_In_Init
(
    struct ANSI_In_Context *pacIn,
    const char *szString,
    int        iEndGoal
)
{
    pacIn->m_acs = acsRestingStates[iEndGoal];
    pacIn->m_p   = szString;
    pacIn->m_n   = strlen(szString);
    pacIn->m_bSawNormal = false;
}

void ANSI_String_Out_Init
(
    struct ANSI_Out_Context *pacOut,
    char *pField,
    int   nField,
    int   vwMax,
    int   iEndGoal
)
{
    pacOut->m_acs      = acsRestingStates[ANSI_ENDGOAL_NORMAL];
    pacOut->m_bDone    = false;
    pacOut->m_iEndGoal = iEndGoal;
    pacOut->m_n        = 0;
    pacOut->m_nMax     = nField;
    pacOut->m_p        = pField;
    pacOut->m_vw       = 0;
    pacOut->m_vwMax    = vwMax;
}

void ANSI_String_Skip
(
    struct ANSI_In_Context *pacIn,
    int   maxVisualWidth,
    int  *pnVisualWidth)
{
    *pnVisualWidth = 0;
    while (pacIn->m_n)
    {
        int nTokenLength0;
        int nTokenLength1;
        int iType = ANSI_lex(pacIn->m_n, pacIn->m_p, &nTokenLength0, &nTokenLength1);

        if (iType == TOKEN_TEXT_ANSI)
        {
            // Process TEXT
            //
            int nTextToSkip = maxVisualWidth - *pnVisualWidth;
            if (nTokenLength0 > nTextToSkip)
            {
                // We have reached the limits of the field
                //
                *pnVisualWidth += nTextToSkip;
                pacIn->m_p     += nTextToSkip;
                pacIn->m_n     -= nTextToSkip;
                return;
            }

            pacIn->m_p     += nTokenLength0;
            pacIn->m_n     -= nTokenLength0;
            *pnVisualWidth += nTokenLength0;

            if (nTokenLength1)
            {
                // Process ANSI
                //
                ANSI_Parse_m(&(pacIn->m_acs), nTokenLength1, pacIn->m_p, &(pacIn->m_bSawNormal));
                pacIn->m_p     += nTokenLength1;
                pacIn->m_n     -= nTokenLength1;
            }
        }
        else
        {
            // Process ANSI
            //
            ANSI_Parse_m(&(pacIn->m_acs), nTokenLength0, pacIn->m_p, &(pacIn->m_bSawNormal));
            pacIn->m_n     -= nTokenLength0;
            pacIn->m_p     += nTokenLength0;
        }
    }
}

// TODO: Rework comment block.
//
// ANSI_String_Copy -- Copy characters into a buffer starting at
// pField0 with maximum size of nField. Truncate the string if it would
// overflow the buffer -or- if it would have a visual with of greater
// than maxVisualWidth. Returns the number of ANSI-encoded characters
// copied to. Also, the visual width produced by this is returned in
// *pnVisualWidth.
//
// There are three ANSI color states that we deal with in this routine:
//
// 1. acsPrevious is the color state at the current end of the field.
//    It has already been encoded into the field.
//
// 2. acsCurrent is the color state that the current TEXT will be shown
//    with. It hasn't been encoded into the field, yet, and if we don't
//    have enough room for at least one character of TEXT, then it may
//    never be encoded into the field.
//
// 3. acsFinal is the required color state at the end. This is usually
//    the normal state or in the case of NOBLEED, it's a specific (and
//    somewhate arbitrary) foreground/background combination.
//
void ANSI_String_Copy
(
    struct ANSI_Out_Context *pacOut,
    struct ANSI_In_Context  *pacIn,
    int maxVisualWidth0
)
{
    // Check whether we have previous struck the session limits (given
    // by ANSI_String_Out_Init() for field size or visual width.
    //
    if (pacOut->m_bDone)
    {
        return;
    }

    // What is the working limit for visual width.
    //
    int vw = 0;
    int vwMax = pacOut->m_vwMax;
    if (maxVisualWidth0 < vwMax)
    {
        vwMax = maxVisualWidth0;
    }

    // What is the working limit for field size.
    //
    int nMax = pacOut->m_nMax;

    char *pField = pacOut->m_p;
    while (pacIn->m_n)
    {
        int nTokenLength0;
        int nTokenLength1;
        int iType = ANSI_lex(pacIn->m_n, pacIn->m_p, &nTokenLength0,
            &nTokenLength1);

        if (iType == TOKEN_TEXT_ANSI)
        {
            // We have a TEXT+[ANSI] phrase. The text length is given
            // by nTokenLength0, and the ANSI characters that follow
            // (if present) are of length nTokenLength1.
            //
            // Process TEXT part first.
            //
            // TODO: If there is a maximum size for the transitions,
            // and we have gobs of space, don't bother calculating
            // sizes so carefully. It might be faster

            // nFieldEffective is used to allocate and plan space for
            // the rest of the physical field (given by the current
            // nField length).
            //
            int nFieldEffective = nMax - 1; // Leave room for '\0'.

            int nTransitionFinal = 0;
            if (pacOut->m_iEndGoal <= ANSI_ENDGOAL_NOBLEED)
            {
                // If we lay down -any- of the TEXT part, we need to make
                // sure we always leave enough room to get back to the
                // required final ANSI color state.
                //
                if (memcmp( &(pacIn->m_acs),
                            &acsRestingStates[pacOut->m_iEndGoal],
                            sizeof(ANSI_ColorState)) != 0)
                {
                    // The color state of the TEXT isn't the final state,
                    // so how much room will the transition back to the
                    // final state take?
                    //
                    ANSI_TransitionColorBinary( &(pacIn->m_acs),
                                                &acsRestingStates[pacOut->m_iEndGoal],
                                                &nTransitionFinal,
                                                pacOut->m_iEndGoal);

                    nFieldEffective -= nTransitionFinal;
                }
            }

            // If we lay down -any- of the TEXT part, it needs to be
            // the right color.
            //
            int nTransition = 0;
            char *pTransition =
                ANSI_TransitionColorBinary( &(pacOut->m_acs),
                                            &(pacIn->m_acs),
                                            &nTransition,
                                            pacOut->m_iEndGoal);
            nFieldEffective -= nTransition;

            // If we find that there is no room for any of the TEXT,
            // then we're done.
            //
            // TODO: The visual width test can be done further up to save time.
            //
            if (  nFieldEffective <= nTokenLength0
               || vw + nTokenLength0 > vwMax)
            {
                // We have reached the limits of the field.
                //
                if (nFieldEffective > 0)
                {
                    // There was enough physical room in the field, but
                    // we would have exceeded the maximum visual width
                    // if we used all the text.
                    //
                    if (nTransition)
                    {
                        // Encode the TEXT color.
                        //
                        memcpy(pField, pTransition, nTransition);
                        pField += nTransition;
                    }

                    // Place just enough of the TEXT in the field.
                    //
                    int nTextToAdd = vwMax - vw;
                    if (nTextToAdd < nFieldEffective)
                    {
                        nFieldEffective = nTextToAdd;
                    }
                    memcpy(pField, pacIn->m_p, nFieldEffective);
                    pField += nFieldEffective;
                    pacIn->m_p += nFieldEffective;
                    pacIn->m_n -= nFieldEffective;
                    vw += nFieldEffective;
                    pacOut->m_acs = pacIn->m_acs;

                    // Was this visual width limit related to the session or
                    // the call?
                    //
                    if (vwMax != maxVisualWidth0)
                    {
                        pacOut->m_bDone = true;
                    }
                }
                else
                {
                    // Was size limit related to the session or the call?
                    //
                    pacOut->m_bDone = true;
                }
                pacOut->m_n += pField - pacOut->m_p;
                pacOut->m_nMax -= pField - pacOut->m_p;
                pacOut->m_p  = pField;
                pacOut->m_vw += vw;
                return;
            }

            if (nTransition)
            {
                memcpy(pField, pTransition, nTransition);
                pField += nTransition;
                nMax   -= nTransition;
            }
            memcpy(pField, pacIn->m_p, nTokenLength0);
            pField  += nTokenLength0;
            nMax    -= nTokenLength0;
            pacIn->m_p += nTokenLength0;
            pacIn->m_n -= nTokenLength0;
            vw += nTokenLength0;
            pacOut->m_acs = pacIn->m_acs;

            if (nTokenLength1)
            {
                // Process ANSI
                //
                ANSI_Parse_m(&(pacIn->m_acs), nTokenLength1, pacIn->m_p, &(pacIn->m_bSawNormal));
                pacIn->m_p += nTokenLength1;
                pacIn->m_n -= nTokenLength1;
            }
        }
        else
        {
            // Process ANSI
            //
            ANSI_Parse_m(&(pacIn->m_acs), nTokenLength0, pacIn->m_p, &(pacIn->m_bSawNormal));
            pacIn->m_n -= nTokenLength0;
            pacIn->m_p += nTokenLength0;
        }
    }
    pacOut->m_n += pField - pacOut->m_p;
    pacOut->m_nMax -= pField - pacOut->m_p;
    pacOut->m_p  = pField;
    pacOut->m_vw += vw;
}

int ANSI_String_Finalize
(
    struct ANSI_Out_Context *pacOut,
    int *pnVisualWidth
)
{
    char *pField = pacOut->m_p;
    if (pacOut->m_iEndGoal <= ANSI_ENDGOAL_NOBLEED)
    {
        int nTransition = 0;
        char *pTransition =
            ANSI_TransitionColorBinary( &(pacOut->m_acs),
                                        &acsRestingStates[pacOut->m_iEndGoal],
                                        &nTransition, pacOut->m_iEndGoal);
        if (nTransition)
        {
            memcpy(pField, pTransition, nTransition);
            pField += nTransition;
        }
    }
    *pField = '\0';
    pacOut->m_n += pField - pacOut->m_p;
    pacOut->m_p  = pField;
    *pnVisualWidth = pacOut->m_vw;
    return pacOut->m_n;
}

// Take an ANSI string and fit as much of the information as possible
// into a field of size nField. Truncate text. Also make sure that no color
// leaks out of the field.
//
int ANSI_TruncateToField
(
    const char *szString,
    int nField,
    char *pField0,
    int maxVisualWidth,
    int *pnVisualWidth,
    int  iEndGoal
)
{
    if (!szString)
    {
        pField0[0] = '\0';
        return 0;
    }
    struct ANSI_In_Context aic;
    struct ANSI_Out_Context aoc;
    ANSI_String_In_Init(&aic, szString, iEndGoal);
    ANSI_String_Out_Init(&aoc, pField0, nField, maxVisualWidth, iEndGoal);
    ANSI_String_Copy(&aoc, &aic, maxVisualWidth);
    return ANSI_String_Finalize(&aoc, pnVisualWidth);
}

char *ANSI_TruncateAndPad_sbuf(const char *pString, int nMaxVisualWidth, char fill)
{
    char *pStringModified = alloc_sbuf("ANSI_TruncateAndPad_sbuf");
    int nAvailable = SBUF_SIZE - nMaxVisualWidth;
    int nVisualWidth;
    int nLen = ANSI_TruncateToField(pString, nAvailable,
        pStringModified, nMaxVisualWidth, &nVisualWidth, ANSI_ENDGOAL_NORMAL);
    for (int i = nMaxVisualWidth - nVisualWidth; i > 0; i--)
    {
        pStringModified[nLen] = fill;
        nLen++;
    }
    pStringModified[nLen] = '\0';
    return pStringModified;
}

char *normal_to_white(const char *szString)
{
    static char Buffer[LBUF_SIZE];
    int nVisualWidth;
    ANSI_TruncateToField( szString,
                          sizeof(Buffer),
                          Buffer,
                          sizeof(Buffer),
                          &nVisualWidth,
                          ANSI_ENDGOAL_NOBLEED
                        );
    return Buffer;
}

typedef struct
{
    int len;
    char *p;
} LITERAL_STRING_STRUCT;

LITERAL_STRING_STRUCT MU_Substitutes[] =
{
    { 1, " "  },  // 0
    { 1, " "  },  // 1
    { 2, "%t" },  // 2
    { 2, "%r" },  // 3
    { 0, NULL },  // 4
    { 2, "%b" },  // 5
    { 2, "%%" },  // 6
    { 2, "%(" },  // 7
    { 2, "%)" },  // 8
    { 2, "%[" },  // 9
    { 2, "%]" },  // 10
    { 2, "%{" },  // 11
    { 2, "%}" },  // 12
    { 2, "\\\\" } // 13
};

const unsigned char MU_EscapeConvert[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 0, 4, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    1, 0, 0, 0, 0, 6, 0, 0, 7, 8, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9,13,10, 0, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11, 0,12, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

const unsigned char MU_EscapeNoConvert[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
//
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 0,  // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 7

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // F
};

// Convert raw character sequences into MUX substitutions (type = 1)
// or strips them (type = 0).
//
char *translate_string(const char *szString, bool bConvert)
{
    static char szTranslatedString[LBUF_SIZE];
    char *pTranslatedString = szTranslatedString;

    const char *pString = szString;
    if (!szString)
    {
        *pTranslatedString = '\0';
        return szTranslatedString;
    }
    size_t nString = strlen(szString);

    ANSI_ColorState acsCurrent;
    ANSI_ColorState acsPrevious;
    acsCurrent = acsRestingStates[ANSI_ENDGOAL_NOBLEED];
    acsPrevious = acsCurrent;
    bool bSawNormal = false;
    const unsigned char *MU_EscapeChar = (bConvert)? MU_EscapeConvert : MU_EscapeNoConvert;
    while (nString)
    {
        int nTokenLength0;
        int nTokenLength1;
        int iType = ANSI_lex(nString, pString, &nTokenLength0, &nTokenLength1);

        if (iType == TOKEN_TEXT_ANSI)
        {
            // Process TEXT
            //
            int nTransition = 0;
            if (bConvert)
            {
                char *pTransition = ANSI_TransitionColorEscape(&acsPrevious, &acsCurrent, &nTransition);
                safe_str(pTransition, szTranslatedString, &pTranslatedString);
            }
            nString -= nTokenLength0;

            while (nTokenLength0--)
            {
                unsigned char ch = *pString++;
                unsigned char code = MU_EscapeChar[ch];
                if (code)
                {
                    // The following can look one ahead off the end of the
                    // current token (and even at the '\0' at the end of the
                    // string, but this is acceptable. An extra look will
                    // always see either ESC from the next ANSI sequence,
                    // or the '\0' on the end of the string. No harm done.
                    //
                    if (ch == ' ' && pString[0] == ' ')
                    {
                        code = 5;
                    }
                    safe_copy_buf(MU_Substitutes[code].p,
                        MU_Substitutes[code].len, szTranslatedString,
                        &pTranslatedString);
                }
                else
                {
                    safe_chr(ch, szTranslatedString, &pTranslatedString);
                }
            }
            acsPrevious = acsCurrent;

            if (nTokenLength1)
            {
                // Process ANSI
                //
                ANSI_Parse_m(&acsCurrent, nTokenLength1, pString, &bSawNormal);
                pString += nTokenLength1;
                nString -= nTokenLength1;
            }
        }
        else
        {
            // Process ANSI
            //
            ANSI_Parse_m(&acsCurrent, nTokenLength0, pString, &bSawNormal);
            nString -= nTokenLength0;
            pString += nTokenLength0;
        }
    }
    *pTranslatedString = '\0';
    return szTranslatedString;
}

/* ---------------------------------------------------------------------------
 * munge_space: Compress multiple spaces to one space, also remove leading and
 * trailing spaces.
 */
char *munge_space(const char *string)
{
    char *buffer = alloc_lbuf("munge_space");
    const char *p = string;
    char *q = buffer;

    if (p)
    {
        // Remove initial spaces.
        //
        while (mux_isspace(*p))
            p++;

        while (*p)
        {
            while (*p && !mux_isspace(*p))
                *q++ = *p++;

            while (mux_isspace(*p))
            {
                p++;
            }

            if (*p)
                *q++ = ' ';
        }
    }

    // Remove terminal spaces and terminate string.
    //
    *q = '\0';
    return buffer;
}

/* ---------------------------------------------------------------------------
 * trim_spaces: Remove leading and trailing spaces.
 */
char *trim_spaces(char *string)
{
    char *buffer = alloc_lbuf("trim_spaces");
    char *p = string;
    char *q = buffer;

    if (p)
    {
        // Remove initial spaces.
        //
        while (mux_isspace(*p))
        {
            p++;
        }

        while (*p)
        {
            // Copy non-space characters.
            //
            while (*p && !mux_isspace(*p))
            {
                *q++ = *p++;
            }

            // Compress spaces.
            //
            while (mux_isspace(*p))
            {
                p++;
            }

            // Leave one space.
            //
            if (*p)
            {
                *q++ = ' ';
            }
        }
    }

    // Terminate string.
    //
    *q = '\0';
    return buffer;
}

/*
 * ---------------------------------------------------------------------------
 * * grabto: Return portion of a string up to the indicated character.  Also
 * * returns a modified pointer to the string ready for another call.
 */

char *grabto(char **str, char targ)
{
    char *savec, *cp;

    if (!str || !*str || !**str)
        return NULL;

    savec = cp = *str;
    while (*cp && *cp != targ)
        cp++;
    if (*cp)
        *cp++ = '\0';
    *str = cp;
    return savec;
}

int string_compare(const char *s1, const char *s2)
{
    if (  mudstate.bStandAlone
       || mudconf.space_compress)
    {
        while (mux_isspace(*s1))
        {
            s1++;
        }
        while (mux_isspace(*s2))
        {
            s2++;
        }

        while (  *s1 && *s2
              && (  (mux_tolower(*s1) == mux_tolower(*s2))
                 || (mux_isspace(*s1) && mux_isspace(*s2))))
        {
            if (mux_isspace(*s1) && mux_isspace(*s2))
            {
                // skip all other spaces.
                //
                do
                {
                    s1++;
                } while (mux_isspace(*s1));

                do
                {
                    s2++;
                } while (mux_isspace(*s2));
            }
            else
            {
                s1++;
                s2++;
            }
        }
        if (  *s1
           && *s2)
        {
            return 1;
        }

        if (mux_isspace(*s1))
        {
            while (mux_isspace(*s1))
            {
                s1++;
            }
            return *s1;
        }
        if (mux_isspace(*s2))
        {
            while (mux_isspace(*s2))
            {
                s2++;
            }
            return *s2;
        }
        if (  *s1
           || *s2)
        {
            return 1;
        }
        return 0;
    }
    else
    {
        return mux_stricmp(s1, s2);
    }
}

int string_prefix(const char *string, const char *prefix)
{
    int count = 0;

    while (*string && *prefix
          && (mux_tolower(*string) == mux_tolower(*prefix)))
    {
        string++, prefix++, count++;
    }
    if (*prefix == '\0')
    {
        // Matched all of prefix.
        //
        return count;
    }
    else
    {
        return 0;
    }
}

/*
 * accepts only nonempty matches starting at the beginning of a word
 */

const char *string_match(const char *src, const char *sub)
{
    if ((*sub != '\0') && (src))
    {
        while (*src)
        {
            if (string_prefix(src, sub))
            {
                return src;
            }

            // else scan to beginning of next word
            //
            while (mux_isalnum(*src))
            {
                src++;
            }
            while (*src && !mux_isalnum(*src))
            {
                src++;
            }
        }
    }
    return 0;
}

/*
 * ---------------------------------------------------------------------------
 * * replace_string: Returns an lbuf containing string STRING with all occurances
 * * of OLD replaced by NEW. OLD and NEW may be different lengths.
 * * (mitch 1 feb 91)
 */

char *replace_string(const char *old, const char *new0, const char *s)
{
    if (!s)
    {
        return NULL;
    }
    size_t olen = strlen(old);
    char *result = alloc_lbuf("replace_string");
    char *r = result;
    while (*s)
    {
        // Find next occurrence of the first character of OLD string.
        //
        const char *p;
        if (  olen
           && (p = strchr(s, old[0])))
        {
            // Copy up to the next occurrence of the first char of OLD.
            //
            size_t n = p - s;
            if (n)
            {
                safe_copy_buf(s, n, result, &r);
                s += n;
            }

            // If we are really at an complete OLD, append NEW to the result
            // and bump the input string past the occurrence of OLD.
            // Otherwise, copy the character and try matching again.
            //
            if (!strncmp(old, s, olen))
            {
                safe_str(new0, result, &r);
                s += olen;
            }
            else
            {
                safe_chr(*s, result, &r);
                s++;
            }
        }
        else
        {
            // Finish copying source string. No matches. No further
            // work to perform.
            //
            safe_str(s, result, &r);
            break;
        }
    }
    *r = '\0';
    return result;
}

// ---------------------------------------------------------------------------
// replace_tokens: Performs ## and #@ substitution.
//
char *replace_tokens
(
    const char *s,
    const char *pBound,
    const char *pListPlace,
    const char *pSwitch
)
{
    if (!s)
    {
        return NULL;
    }
    char *result = alloc_lbuf("replace_tokens");
    char *r = result;

    while (*s)
    {
        // Find next '#'.
        //
        const char *p = strchr(s, '#');
        if (p)
        {
            // Copy up to the next occurrence of the first character.
            //
            size_t n = p - s;
            if (n)
            {
                safe_copy_buf(s, n, result, &r);
                s += n;
            }

            if (  s[1] == '#'
               && pBound)
            {
                // BOUND_VAR
                //
                safe_str(pBound, result, &r);
                s += 2;
            }
            else if (  s[1] == '@'
                    && pListPlace)
            {
                // LISTPLACE_VAR
                //
                safe_str(pListPlace, result, &r);
                s += 2;
            }
            else if (  s[1] == '$'
                    && pSwitch)
            {
                // SWITCH_VAR
                //
                safe_str(pSwitch, result, &r);
                s += 2;
            }
            else
            {
                safe_chr(*s, result, &r);
                s++;
            }
        }
        else
        {
            // Finish copying source string. No matches. No further
            // work to perform.
            //
            safe_str(s, result, &r);
            break;
        }
    }
    *r = '\0';
    return result;
}

#if 0
// Returns the number of identical characters in the two strings.
//
int prefix_match(const char *s1, const char *s2)
{
    int count = 0;

    while (*s1 && *s2
          && (mux_tolower(*s1) == mux_tolower(*s2)))
    {
        s1++, s2++, count++;
    }

    // If the whole string matched, count the null.  (Yes really.)
    //
    if (!*s1 && !*s2)
    {
        count++;
    }
    return count;
}
#endif // 0

bool minmatch(char *str, char *target, int min)
{
    while (*str && *target
          && (mux_tolower(*str) == mux_tolower(*target)))
    {
        str++;
        target++;
        min--;
    }
    if (*str)
    {
        return false;
    }
    if (!*target)
    {
        return true;
    }
    return (min <= 0);
}

// --------------------------------------------------------------------------
// StringCloneLen: allocate memory and copy string
//
char *StringCloneLen(const char *str, size_t nStr)
{
    char *buff = (char *)MEMALLOC(nStr+1);
    ISOUTOFMEMORY(buff);
    memcpy(buff, str, nStr);
    buff[nStr] = '\0';
    return buff;
}

// --------------------------------------------------------------------------
// StringClone: allocate memory and copy string
//
char *StringClone(const char *str)
{
    return StringCloneLen(str, strlen(str));
}

#if 0
// --------------------------------------------------------------------------
// BufferCloneLen: allocate memory and copy buffer
//
char *BufferCloneLen(const char *pBuffer, unsigned int nBuffer)
{
    char *buff = (char *)MEMALLOC(nBuffer);
    ISOUTOFMEMORY(buff);
    memcpy(buff, pBuffer, nBuffer);
    return buff;
}
#endif // 0

/* ---------------------------------------------------------------------------
 * safe_copy_str, safe_copy_chr - Copy buffers, watching for overflows.
 */

void safe_copy_str(const char *src, char *buff, char **bufp, int nSizeOfBuffer)
{
    if (src == NULL) return;

    char *tp = *bufp;
    char *maxtp = buff + nSizeOfBuffer;
    while (tp < maxtp && *src)
    {
        *tp++ = *src++;
    }
    *bufp = tp;
}

void safe_copy_str_lbuf(const char *src, char *buff, char **bufp)
{
    if (src == NULL)
    {
        return;
    }

    char *tp = *bufp;
    char *maxtp = buff + LBUF_SIZE - 1;
    while (tp < maxtp && *src)
    {
        *tp++ = *src++;
    }
    *bufp = tp;
}

size_t safe_copy_buf(const char *src, size_t nLen, char *buff, char **bufc)
{
    size_t left = LBUF_SIZE - (*bufc - buff) - 1;
    if (left < nLen)
    {
        nLen = left;
    }
    memcpy(*bufc, src, nLen);
    *bufc += nLen;
    return nLen;
}

size_t safe_fill(char *buff, char **bufc, char chFill, size_t nSpaces)
{
    // Check for buffer limits.
    //
    size_t nBufferAvailable = LBUF_SIZE - (*bufc - buff) - 1;
    if (nSpaces > nBufferAvailable)
    {
        nSpaces = nBufferAvailable;
    }

    // Fill with spaces.
    //
    memset(*bufc, chFill, nSpaces);
    *bufc += nSpaces;
    return nSpaces;
}

bool matches_exit_from_list(char *str, const char *pattern)
{
    char *s;

    while (*pattern)
    {
        for (s = str;   // check out this one
             ( *s
             && (mux_tolower(*s) == mux_tolower(*pattern))
             && *pattern
             && (*pattern != EXIT_DELIMITER));
             s++, pattern++) ;

        // Did we match it all?
        //
        if (*s == '\0')
        {
            // Make sure nothing afterwards
            //
            while (mux_isspace(*pattern))
            {
                pattern++;
            }

            // Did we get it?
            //
            if (  !*pattern
               || (*pattern == EXIT_DELIMITER))
            {
                return true;
            }
        }
        // We didn't get it, find next string to test
        //
        while (  *pattern
              && *pattern++ != EXIT_DELIMITER)
        {
            ; // Nothing.
        }
        while (mux_isspace(*pattern))
        {
            pattern++;
        }
    }
    return false;
}

const char Digits100[201] =
"001020304050607080900111213141516171819102122232425262728292\
031323334353637383930414243444546474849405152535455565758595\
061626364656667686960717273747576777879708182838485868788898\
09192939495969798999";

size_t mux_ltoa(long val, char *buf)
{
    char *p = buf;

    if (val < 0)
    {
        *p++ = '-';
        val = -val;
    }
    unsigned long uval = (unsigned long)val;

    char *q = p;

    const char *z;
    while (uval > 99)
    {
        z = Digits100 + ((uval % 100) << 1);
        uval /= 100;
        *p++ = *z;
        *p++ = *(z+1);
    }
    z = Digits100 + (uval << 1);
    *p++ = *z;
    if (uval > 9)
    {
        *p++ = *(z+1);
    }

    size_t nLength = p - buf;
    *p-- = '\0';

    // The digits are in reverse order with a possible leading '-'
    // if the value was negative. q points to the first digit,
    // and p points to the last digit.
    //
    while (q < p)
    {
        // Swap characters are *p and *q
        //
        char temp = *p;
        *p = *q;
        *q = temp;

        // Move p and first digit towards the middle.
        //
        --p;
        ++q;

        // Stop when we reach or pass the middle.
        //
    }
    return nLength;
}

char *mux_ltoa_t(long val)
{
    static char buff[12];
    mux_ltoa(val, buff);
    return buff;
}

void safe_ltoa(long val, char *buff, char **bufc)
{
    static char temp[12];
    size_t n = mux_ltoa(val, temp);
    safe_copy_buf(temp, n, buff, bufc);
}

size_t mux_i64toa(INT64 val, char *buf)
{
    char *p = buf;

    if (val < 0)
    {
        *p++ = '-';
        val = -val;
    }
    UINT64 uval = (UINT64)val;

    char *q = p;

    const char *z;
    while (uval > 99)
    {
        z = Digits100 + ((uval % 100) << 1);
        uval /= 100;
        *p++ = *z;
        *p++ = *(z+1);
    }
    z = Digits100 + (uval << 1);
    *p++ = *z;
    if (uval > 9)
    {
        *p++ = *(z+1);
    }

    size_t nLength = p - buf;
    *p-- = '\0';

    // The digits are in reverse order with a possible leading '-'
    // if the value was negative. q points to the first digit,
    // and p points to the last digit.
    //
    while (q < p)
    {
        // Swap characters are *p and *q
        //
        char temp = *p;
        *p = *q;
        *q = temp;

        // Move p and first digit towards the middle.
        //
        --p;
        ++q;

        // Stop when we reach or pass the middle.
        //
    }
    return nLength;
}

#if 0
char *mux_i64toa_t(INT64 val)
{
    static char buff[22];
    mux_i64toa(val, buff);
    return buff;
}
#endif

void safe_i64toa(INT64 val, char *buff, char **bufc)
{
    static char temp[22];
    size_t n = mux_i64toa(val, temp);
    safe_copy_buf(temp, n, buff, bufc);
}

const char TableATOI[16][10] =
{
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9},
    { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19},
    { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29},
    { 30, 31, 32, 33, 34, 35, 36, 37, 38, 39},
    { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49},
    { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
    { 60, 61, 62, 63, 64, 65, 66, 67, 68, 69},
    { 70, 71, 72, 73, 74, 75, 76, 77, 78, 79},
    { 80, 81, 82, 83, 84, 85, 86, 87, 88, 89},
    { 90, 91, 92, 93, 94, 95, 96, 97, 98, 99}
};

long mux_atol(const char *pString)
{
    long sum = 0;
    int LeadingCharacter = 0;

    // Convert ASCII digits
    //
    unsigned int c1;
    unsigned int c0 = pString[0];
    if (!mux_isdigit(c0))
    {
        while (mux_isspace(pString[0]))
        {
            pString++;
        }
        LeadingCharacter = pString[0];
        if (  LeadingCharacter == '-'
           || LeadingCharacter == '+')
        {
            pString++;
        }
        c0 = pString[0];
        if (!mux_isdigit(c0))
        {
            return 0;
        }
    }

    do
    {
        c1 = pString[1];
        if (mux_isdigit(c1))
        {
            sum = 100 * sum + TableATOI[c0-'0'][c1-'0'];
            pString += 2;
        }
        else
        {
            sum = 10 * sum + (c0-'0');
            break;
        }
    } while (mux_isdigit(c0 = pString[0]));

    // Interpret sign
    //
    if (LeadingCharacter == '-')
    {
        sum = -sum;
    }
    return sum;
}

INT64 mux_atoi64(const char *pString)
{
    INT64 sum = 0;
    int LeadingCharacter = 0;

    // Convert ASCII digits
    //
    unsigned int c1;
    unsigned int c0 = pString[0];
    if (!mux_isdigit(c0))
    {
        while (mux_isspace(pString[0]))
        {
            pString++;
        }
        LeadingCharacter = pString[0];
        if (  LeadingCharacter == '-'
           || LeadingCharacter == '+')
        {
            pString++;
        }
        c0 = pString[0];
        if (!mux_isdigit(c0))
        {
            return 0;
        }
    }

    do
    {
        c1 = pString[1];
        if (mux_isdigit(c1))
        {
            sum = 100 * sum + TableATOI[c0-'0'][c1-'0'];
            pString += 2;
        }
        else
        {
            sum = 10 * sum + (c0-'0');
            break;
        }
    } while (mux_isdigit(c0 = pString[0]));

    // Interpret sign
    //
    if (LeadingCharacter == '-')
    {
        sum = -sum;
    }
    return sum;
}

// Floating-point strings match one of the following patterns:
//
// [+-]?[0-9]?(.[0-9]+)([eE][+-]?[0-9]{1,3})?
// [+-]?[0-9]+(.[0-9]?)([eE][+-]?[0-9]{1,3})?
// +Inf
// -Inf
// Ind
// NaN
//
bool ParseFloat(PARSE_FLOAT_RESULT *pfr, const char *str, bool bStrict)
{
    memset(pfr, 0, sizeof(PARSE_FLOAT_RESULT));

    // Parse Input
    //
    unsigned char ch;
    pfr->pMeat = str;
    if (  !mux_isdigit(*str)
       && *str != '.')
    {
        while (mux_isspace(*str))
        {
            str++;
        }

        pfr->pMeat = str;
        if (*str == '-')
        {
            pfr->iLeadingSign = '-';
            str++;
        }
        else if (*str == '+')
        {
            pfr->iLeadingSign = '+';
            str++;
        }

        if (  !mux_isdigit(*str)
           && *str != '.')
        {
            // Look for three magic strings.
            //
            ch = mux_toupper(str[0]);
            if (ch == 'I')
            {
                // Could be 'Inf' or 'Ind'
                //
                ch = mux_toupper(str[1]);
                if (ch == 'N')
                {
                    ch = mux_toupper(str[2]);
                    if (ch == 'F')
                    {
                        // Inf
                        //
                        if (pfr->iLeadingSign == '-')
                        {
                            pfr->iString = IEEE_MAKE_NINF;
                        }
                        else
                        {
                            pfr->iString = IEEE_MAKE_PINF;
                        }
                        str += 3;
                        goto LastSpaces;
                    }
                    else if (ch == 'D')
                    {
                        // Ind
                        //
                        pfr->iString = IEEE_MAKE_IND;
                        str += 3;
                        goto LastSpaces;
                    }
                }
            }
            else if (ch == 'N')
            {
                // Could be 'Nan'
                //
                ch = mux_toupper(str[1]);
                if (ch == 'A')
                {
                    ch = mux_toupper(str[2]);
                    if (ch == 'N')
                    {
                        // Nan
                        //
                        pfr->iString = IEEE_MAKE_NAN;
                        str += 3;
                        goto LastSpaces;
                    }
                }
            }
            return false;
        }
    }

    // At this point, we have processed the leading sign, handled all
    // the magic strings, skipped the leading spaces, and best of all
    // we either have a digit or a decimal point.
    //
    pfr->pDigitsA = str;
    while (mux_isdigit(*str))
    {
        pfr->nDigitsA++;
        str++;
    }

    if (*str == '.')
    {
        str++;
    }

    pfr->pDigitsB = str;
    while (mux_isdigit(*str))
    {
        pfr->nDigitsB++;
        str++;
    }

    if (  pfr->nDigitsA == 0
       && pfr->nDigitsB == 0)
    {
        return false;
    }

    ch = mux_toupper(*str);
    if (ch == 'E')
    {
        // There is an exponent portion.
        //
        str++;
        if (*str == '-')
        {
            pfr->iExponentSign = '-';
            str++;
        }
        else if (*str == '+')
        {
            pfr->iExponentSign = '+';
            str++;
        }
        pfr->pDigitsC = str;
        while (mux_isdigit(*str))
        {
            pfr->nDigitsC++;
            str++;
        }

        if (  pfr->nDigitsC < 1
           || 4 < pfr->nDigitsC)
        {
            return false;
        }
    }

LastSpaces:

    pfr->nMeat = str - pfr->pMeat;

    // Trailing spaces.
    //
    while (mux_isspace(*str))
    {
        str++;
    }

    if (bStrict)
    {
        return (!*str);
    }
    else
    {
        return true;
    }
}

#define ATOF_LIMIT 100
static const double powerstab[10] =
{
            1.0,
           10.0,
          100.0,
         1000.0,
        10000.0,
       100000.0,
      1000000.0,
     10000000.0,
    100000000.0,
   1000000000.0
};

double mux_atof(char *szString, bool bStrict)
{
    PARSE_FLOAT_RESULT pfr;
    if (!ParseFloat(&pfr, szString, bStrict))
    {
        return 0.0;
    }

    if (pfr.iString)
    {
        // Return the double value which corresponds to the
        // string when HAVE_IEEE_FORMAT.
        //
#ifdef HAVE_IEEE_FP_FORMAT
        return MakeSpecialFloat(pfr.iString);
#else // HAVE_IEEE_FP_FORMAT
        return 0.0;
#endif // HAVE_IEEE_FP_FORMAT
    }

    // See if we can shortcut the decoding process.
    //
    double ret;
    if (  pfr.nDigitsA <= 9
       && pfr.nDigitsC == 0)
    {
        if (pfr.nDigitsB <= 9)
        {
            if (pfr.nDigitsB == 0)
            {
                // This 'floating-point' number is just an integer.
                //
                ret = (double)mux_atol(pfr.pDigitsA);
            }
            else
            {
                // This 'floating-point' number is fixed-point.
                //
                double rA = (double)mux_atol(pfr.pDigitsA);
                double rB = (double)mux_atol(pfr.pDigitsB);
                double rScale = powerstab[pfr.nDigitsB];
                ret = rA + rB/rScale;

                // As it is, ret is within a single bit of what a
                // a call to atof would return. However, we can
                // achieve that last lowest bit of precision by
                // computing a residual.
                //
                double residual = (ret - rA)*rScale;
                ret += (rB - residual)/rScale;
            }
            if (pfr.iLeadingSign == '-')
            {
                ret = -ret;
            }
            return ret;
        }
    }

    const char *p = pfr.pMeat;
    size_t n = pfr.nMeat;

    // We need to protect certain libraries from going nuts from being
    // force fed lots of ASCII.
    //
    char *pTmp = NULL;
    if (n > ATOF_LIMIT)
    {
        pTmp = alloc_lbuf("mux_atof");
        memcpy(pTmp, p, ATOF_LIMIT);
        pTmp[ATOF_LIMIT] = '\0';
        p = pTmp;
    }

    ret = mux_strtod(p, NULL);

    if (pTmp)
    {
        free_lbuf(pTmp);
    }

    return ret;
}

extern char *mux_dtoa(double d, int mode, int nRequest, int *iDecimalPoint,
                       int *sign, char **rve);

char *mux_ftoa(double r, bool bRounded, int frac)
{
    static char buffer[100];
    char *q = buffer;
    char *rve = NULL;
    int iDecimalPoint = 0;
    int bNegative = 0;
    int mode = 0;
    int nRequest = 50;

    if (bRounded)
    {
        mode = 3;
        nRequest = frac;
        if (50 < nRequest)
        {
            nRequest = 50;
        }
        else if (nRequest < -20)
        {
            nRequest = -20;
        }
    }
    char *p = mux_dtoa(r, mode, nRequest, &iDecimalPoint, &bNegative, &rve);
    int nSize = rve - p;
    if (nSize > 50)
    {
        nSize = 50;
    }
    if (bNegative)
    {
        *q++ = '-';
    }
    if (iDecimalPoint == 9999)
    {
        // Inf or NaN
        //
        memcpy(q, p, nSize);
        q += nSize;
    }
    else if (nSize <= 0)
    {
        // Zero
        //
        *q++ = '0';
        if (  bRounded
           && 0 < nRequest)
        {
            *q++ = '.';
            memset(q, '0', nRequest);
            q += nRequest;
        }
    }
    else if (  iDecimalPoint <= -6
            || 18 <= iDecimalPoint)
    {
        *q++ = *p++;
        if (1 < nSize)
        {
            *q++ = '.';
            memcpy(q, p, nSize-1);
            q += nSize-1;
        }
        *q++ = 'E';
        q += mux_ltoa(iDecimalPoint-1, q);
    }
    else if (iDecimalPoint <= 0)
    {
        // iDecimalPoint = -5 to 0
        //
        *q++ = '0';
        *q++ = '.';
        memset(q, '0', -iDecimalPoint);
        q += -iDecimalPoint;
        memcpy(q, p, nSize);
        q += nSize;
        if (bRounded)
        {
            int nPad = nRequest - (nSize - iDecimalPoint);
            if (0 < nPad)
            {
                memset(q, '0', nPad);
                q += nPad;
            }
        }
    }
    else
    {
        // iDecimalPoint = 1 to 17
        //
        if (nSize <= iDecimalPoint)
        {
            memcpy(q, p, nSize);
            q += nSize;
            memset(q, '0', iDecimalPoint - nSize);
            q += iDecimalPoint - nSize;
            if (  bRounded
               && 0 < nRequest)
            {
                *q++ = '.';
                memset(q, '0', nRequest);
                q += nRequest;
            }
        }
        else
        {
            memcpy(q, p, iDecimalPoint);
            q += iDecimalPoint;
            p += iDecimalPoint;
            *q++ = '.';
            memcpy(q, p, nSize - iDecimalPoint);
            q += nSize - iDecimalPoint;
            if (bRounded)
            {
                int nPad = nRequest - (nSize - iDecimalPoint);
                if (0 < nPad)
                {
                    memset(q, '0', nPad);
                    q += nPad;
                }
            }
        }
    }
    *q = '\0';
    return buffer;
}

bool is_integer(char *str, int *pDigits)
{
    int nDigits = 0;
    if (pDigits)
    {
        *pDigits = 0;
    }

    // Leading spaces.
    //
    while (mux_isspace(*str))
    {
        str++;
    }

    // Leading minus or plus
    //
    if (*str == '-' || *str == '+')
    {
        str++;

        // Just a sign by itself isn't an integer.
        //
        if (!*str)
        {
            return false;
        }
    }

    // Need at least 1 integer
    //
    if (!mux_isdigit(*str))
    {
        return false;
    }

    // The number (int)
    //
    do
    {
        str++;
        nDigits++;
    } while (mux_isdigit(*str));

    if (pDigits)
    {
        *pDigits = nDigits;
    }

    // Trailing Spaces.
    //
    while (mux_isspace(*str))
    {
        str++;
    }

    return (!*str);
}

bool is_rational(char *str)
{
    // Leading spaces.
    //
    while (mux_isspace(*str))
    {
        str++;
    }

    // Leading minus or plus sign.
    //
    if (*str == '-' || *str == '+')
    {
        str++;

        // But not if just a sign.
        //
        if (!*str)
        {
            return false;
        }
    }

    // Need at least one digit.
    //
    bool got_one = false;
    if (mux_isdigit(*str))
    {
        got_one = true;
    }

    // The number (int)
    //
    while (mux_isdigit(*str))
    {
        str++;
    }

    // Decimal point.
    //
    if (*str == '.')
    {
        str++;
    }

    // Need at least one digit
    //
    if (mux_isdigit(*str))
    {
        got_one = true;
    }

    if (!got_one)
    {
        return false;
    }

    // The number (fract)
    //
    while (mux_isdigit(*str))
    {
        str++;
    }

    // Trailing spaces.
    //
    while (mux_isspace(*str))
    {
        str++;
    }

    // There must be nothing else after the trailing spaces.
    //
    return (!*str);
}

bool is_real(char *str)
{
    PARSE_FLOAT_RESULT pfr;
    return ParseFloat(&pfr, str);
}

// mux_strtok_src, mux_strtok_ctl, mux_strtok_parse.
//
// These three functions work together to replace the functionality of the
// strtok() C runtime library function. Call mux_strtok_src() first with
// the string to parse, then mux_strtok_ctl() with the control
// characters, and finally mux_strtok_parse() to parse out the tokens.
//
// You may call mux_strtok_ctl() to change the set of control characters
// between mux_strtok_parse() calls, however keep in mind that the parsing
// may not occur how you intend it to as mux_strtok_parse() does not
// consume -all- of the controlling delimiters that separate two tokens.
// It consumes only the first one.
//
void mux_strtok_src(MUX_STRTOK_STATE *tts, char *arg_pString)
{
    if (!tts || !arg_pString) return;

    // Remember the string to parse.
    //
    tts->pString = arg_pString;
}

void mux_strtok_ctl(MUX_STRTOK_STATE *tts, char *pControl)
{
    if (!tts || !pControl) return;

    // No character is a control character.
    //
    memset(tts->aControl, 0, sizeof(tts->aControl));

    // The NULL character is always a control character.
    //
    tts->aControl[0] = 1;

    // Record the user-specified control characters.
    //
    while (*pControl)
    {
        tts->aControl[(unsigned char)*pControl] = 1;
        pControl++;
    }
}

char *mux_strtok_parseLEN(MUX_STRTOK_STATE *tts, int *pnLen)
{
    *pnLen = 0;
    if (!tts)
    {
        return NULL;
    }
    char *p = tts->pString;
    if (!p)
    {
        return NULL;
    }

    // Skip over leading control characters except for the NUL character.
    //
    while (tts->aControl[(unsigned char)*p] && *p)
    {
        p++;
    }

    char *pReturn = p;

    // Skip over non-control characters.
    //
    while (tts->aControl[(unsigned char)*p] == 0)
    {
        p++;
    }

    // What is the length of this token?
    //
    *pnLen = p - pReturn;

    // Terminate the token with a NUL.
    //
    if (p[0])
    {
        // We found a non-NUL delimiter, so the next call will begin parsing
        // on the character after this one.
        //
        tts->pString = p+1;
    }
    else
    {
        // We hit the end of the string, so the end of the string is where
        // the next call will begin.
        //
        tts->pString = p;
    }

    // Did we find a token?
    //
    if (*pnLen > 0)
    {
        return pReturn;
    }
    else
    {
        return NULL;
    }
}

char *mux_strtok_parse(MUX_STRTOK_STATE *tts)
{
    int nLen;
    char *p = mux_strtok_parseLEN(tts, &nLen);
    if (p)
    {
        p[nLen] = '\0';
    }
    return p;
}

// This function will filter out any characters in the the set from
// the string.
//
char *RemoveSetOfCharacters(char *pString, char *pSetToRemove)
{
    static char Buffer[LBUF_SIZE];
    char *pBuffer = Buffer;

    int nLen;
    int nLeft = sizeof(Buffer) - 1;
    char *p;
    MUX_STRTOK_STATE tts;
    mux_strtok_src(&tts, pString);
    mux_strtok_ctl(&tts, pSetToRemove);
    for ( p = mux_strtok_parseLEN(&tts, &nLen);
          p && nLeft;
          p = mux_strtok_parseLEN(&tts, &nLen))
    {
        if (nLeft < nLen)
        {
            nLen = nLeft;
        }
        memcpy(pBuffer, p, nLen);
        pBuffer += nLen;
        nLeft -= nLen;
    }
    *pBuffer = '\0';
    return Buffer;
}

void ItemToList_Init(ITL *p, char *arg_buff, char **arg_bufc,
    char arg_chPrefix, char arg_chSep)
{
    p->bFirst = true;
    p->chPrefix = arg_chPrefix;
    p->chSep = arg_chSep;
    p->buff = arg_buff;
    p->bufc = arg_bufc;
    p->nBufferAvailable = LBUF_SIZE - (*arg_bufc - arg_buff) - 1;
}

bool ItemToList_AddInteger(ITL *pContext, int i)
{
    char smbuf[SBUF_SIZE];
    char *p = smbuf;
    if (  !pContext->bFirst
       && pContext->chSep)
    {
        *p++ = pContext->chSep;
    }
    if (pContext->chPrefix)
    {
        *p++ = pContext->chPrefix;
    }
    p += mux_ltoa(i, p);
    size_t nLen = p - smbuf;
    if (nLen > pContext->nBufferAvailable)
    {
        // Out of room.
        //
        return false;
    }
    if (pContext->bFirst)
    {
        pContext->bFirst = false;
    }
    memcpy(*(pContext->bufc), smbuf, nLen);
    *(pContext->bufc) += nLen;
    pContext->nBufferAvailable -= nLen;
    return true;
}

bool ItemToList_AddStringLEN(ITL *pContext, size_t nStr, char *pStr)
{
    size_t nLen = nStr;
    if (  !pContext->bFirst
       && pContext->chSep)
    {
        nLen++;
    }
    if (pContext->chPrefix)
    {
        nLen++;
    }
    if (nLen > pContext->nBufferAvailable)
    {
        // Out of room.
        //
        return false;
    }
    char *p = *(pContext->bufc);
    if (pContext->bFirst)
    {
        pContext->bFirst = false;
    }
    else if (pContext->chSep)
    {
        *p++ = pContext->chSep;
    }
    if (pContext->chPrefix)
    {
        *p++ = pContext->chPrefix;
    }
    memcpy(p, pStr, nStr);
    *(pContext->bufc) += nLen;
    pContext->nBufferAvailable -= nLen;
    return true;
}

bool ItemToList_AddString(ITL *pContext, char *pStr)
{
    size_t nStr = strlen(pStr);
    return ItemToList_AddStringLEN(pContext, nStr, pStr);
}

void ItemToList_Final(ITL *pContext)
{
    **(pContext->bufc) = '\0';
}

// mux_stricmp - Compare two strings ignoring case.
//
int mux_stricmp(const char *a, const char *b)
{
    while (  *a
          && *b
          && mux_tolower(*a) == mux_tolower(*b))
    {
        a++;
        b++;
    }
    int c1 = mux_tolower(*a);
    int c2 = mux_tolower(*b);
    if (c1 < c2)
    {
        return -1;
    }
    else if (c1 > c2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// mux_memicmp - Compare two buffers ignoring case.
//
int mux_memicmp(const void *p1_arg, const void *p2_arg, size_t n)
{
    unsigned char *p1 = (unsigned char *)p1_arg;
    unsigned char *p2 = (unsigned char *)p2_arg;
    while (  n
          && mux_tolower(*p1) == mux_tolower(*p2))
    {
        n--;
        p1++;
        p2++;
    }
    if (n)
    {
        int c1 = mux_tolower(*p1);
        int c2 = mux_tolower(*p2);
        if (c1 < c2)
        {
            return -1;
        }
        else if (c1 > c2)
        {
            return 1;
        }
    }
    return 0;
}

// mux_strlwr - Convert string to all lower case.
//
void mux_strlwr(char *a)
{
    while (*a)
    {
        *a = mux_tolower(*a);
        a++;
    }
}

// mux_strupr - Convert string to all upper case.
//
void mux_strupr(char *a)
{
    while (*a)
    {
        *a = mux_toupper(*a);
        a++;
    }
}

#ifdef WIN32
#define VSNPRINTF _vsnprintf
#else // WIN32
#ifdef NEED_VSPRINTF_DCL
extern char *vsprintf(char *, char *, va_list);
#endif // NEED_VSPRINTF_DCL
#define VSNPRINTF vsnprintf
#endif // WIN32

// mux_vsnprintf - Is an sprintf-like function that will not overflow
// a buffer of specific size. The size is give by count, and count
// should be chosen to include the '\0' termination.
//
// Returns: A number from 0 to count-1 that is the string length of
// the returned (possibly truncated) buffer.
//
int DCL_CDECL mux_vsnprintf(char *buff, int count, const char *fmt, va_list va)
{
    // From the manuals:
    //
    // vsnprintf returns the number of characters written, not
    // including the terminating '\0' character.
    //
    // It returns a -1 if an output error occurs.
    //
    // It can return a number larger than the size of the buffer
    // on some systems to indicate how much space it -would- have taken
    // if not limited by the request.
    //
    // On Win32, it can fill the buffer completely without a
    // null-termination and return -1.


    // To favor the Unix case, if there is an output error, but
    // vsnprint doesn't touch the buffer, we avoid undefined trash by
    // null-terminating the buffer to zero-length before the call.
    // Not sure that this happens, but it's a cheap precaution.
    //
    buff[0] = '\0';

    // If Unix version does start touching the buffer, null-terminates,
    // and returns -1, we are still safe. However, if Unix version
    // touches the buffer writes garbage, and then returns -1, we may
    // pass garbage, but this possibility seems very unlikely.
    //
    int len = VSNPRINTF(buff, count, fmt, va);
    if (len < 0 || len > count-1)
    {
        if (buff[0] == '\0')
        {
            // vsnprintf did not touch the buffer.
            //
            len = 0;
        }
        else
        {
            len = count-1;
        }
    }
    buff[len] = '\0';
    return len;
}

// This function acts like fgets except that any data on the end of the
// line past the buffer size is truncated instead of being returned on
// the next call.
//
int GetLineTrunc(char *Buffer, size_t nBuffer, FILE *fp)
{
    size_t lenBuffer = 0;
    if (fgets(Buffer, nBuffer, fp))
    {
        lenBuffer = strlen(Buffer);
    }
    if (lenBuffer <= 0)
    {
        memcpy(Buffer, "\n", 2);
        return 1;
    }
    if (Buffer[lenBuffer-1] != '\n')
    {
        // The line was too long for the buffer. Continue reading until the
        // end of the line.
        //
        char TruncBuffer[SBUF_SIZE];
        size_t lenTruncBuffer;
        do
        {
            if (!fgets(TruncBuffer, sizeof(TruncBuffer), fp))
            {
                break;
            }
            lenTruncBuffer = strlen(TruncBuffer);
        }
        while (TruncBuffer[lenTruncBuffer-1] != '\n');
    }
    return lenBuffer;
}

// Method: Boyer-Moore-Horspool
//
// This method is a simplification of the Boyer-Moore String Searching
// Algorithm, but a useful one. It does not require as much temporary
// storage, and the setup costs are not as high as the full Boyer-Moore.
//
// If we were searching megabytes of data instead of 8KB at most, then
// the full Boyer-Moore would make more sense.
//
#define BMH_LARGE 32767
void BMH_Prepare(BMH_State *bmhs, int nPat, const char *pPat)
{
    if (nPat <= 0)
    {
        return;
    }
    int k;
    for (k = 0; k < 256; k++)
    {
        bmhs->m_d[k] = nPat;
    }

    char chLastPat = pPat[nPat-1];
    bmhs->m_skip2 = nPat;
    for (k = 0; k < nPat - 1; k++)
    {
        bmhs->m_d[(unsigned char)pPat[k]] = nPat - k - 1;
        if (pPat[k] == chLastPat)
        {
            bmhs->m_skip2 = nPat - k - 1;
        }
    }
    bmhs->m_d[(unsigned char)chLastPat] = BMH_LARGE;
}

int BMH_Execute(BMH_State *bmhs, int nPat, const char *pPat, int nSrc, const char *pSrc)
{
    if (nPat <= 0)
    {
        return -1;
    }
    for (int i = nPat-1; i < nSrc; i += bmhs->m_skip2)
    {
        while ((i += bmhs->m_d[(unsigned char)(pSrc[i])]) < nSrc)
        {
            ; // Nothing.
        }
        if (i < BMH_LARGE)
        {
            break;
        }
        i -= BMH_LARGE;
        int j = nPat - 1;
        const char *s = pSrc + (i - j);
        while (--j >= 0 && s[j] == pPat[j])
        {
            ; // Nothing.
        }
        if (j < 0)
        {
            return s-pSrc;
        }
    }
    return -1;
}

int BMH_StringSearch(int nPat, const char *pPat, int nSrc, const char *pSrc)
{
    BMH_State bmhs;
    BMH_Prepare(&bmhs, nPat, pPat);
    return BMH_Execute(&bmhs, nPat, pPat, nSrc, pSrc);
}

void BMH_PrepareI(BMH_State *bmhs, int nPat, const char *pPat)
{
    if (nPat <= 0)
    {
        return;
    }
    int k;
    for (k = 0; k < 256; k++)
    {
        bmhs->m_d[k] = nPat;
    }

    char chLastPat = pPat[nPat-1];
    bmhs->m_skip2 = nPat;
    for (k = 0; k < nPat - 1; k++)
    {
        bmhs->m_d[mux_toupper(pPat[k])] = nPat - k - 1;
        bmhs->m_d[mux_tolower(pPat[k])] = nPat - k - 1;
        if (pPat[k] == chLastPat)
        {
            bmhs->m_skip2 = nPat - k - 1;
        }
    }
    bmhs->m_d[mux_toupper(chLastPat)] = BMH_LARGE;
    bmhs->m_d[mux_tolower(chLastPat)] = BMH_LARGE;
}

int BMH_ExecuteI(BMH_State *bmhs, int nPat, const char *pPat, int nSrc, const char *pSrc)
{
    if (nPat <= 0)
    {
        return -1;
    }
    for (int i = nPat-1; i < nSrc; i += bmhs->m_skip2)
    {
        while ((i += bmhs->m_d[(unsigned char)(pSrc[i])]) < nSrc)
        {
            ; // Nothing.
        }
        if (i < BMH_LARGE)
        {
            break;
        }
        i -= BMH_LARGE;
        int j = nPat - 1;
        const char *s = pSrc + (i - j);
        while (  --j >= 0
              && mux_toupper(s[j]) == mux_toupper(pPat[j]))
        {
            ; // Nothing.
        }
        if (j < 0)
        {
            return s-pSrc;
        }
    }
    return -1;
}

int BMH_StringSearchI(int nPat, const char *pPat, int nSrc, const char *pSrc)
{
    BMH_State bmhs;
    BMH_PrepareI(&bmhs, nPat, pPat);
    return BMH_ExecuteI(&bmhs, nPat, pPat, nSrc, pSrc);
}

// ---------------------------------------------------------------------------
// cf_art_except:
//
// Add an article rule to the ruleset.
//

extern void DCL_CDECL cf_log_syntax(dbref player, char *cmd, const char *fmt, ...);

CF_HAND(cf_art_rule)
{
    char* pCurrent = str;

    while (mux_isspace(*pCurrent))
    {
        pCurrent++;
    }
    char* pArticle = pCurrent;
    while (  !mux_isspace(*pCurrent)
          && *pCurrent != '\0')
    {
        pCurrent++;
    }
    if (*pCurrent == '\0')
    {
        cf_log_syntax(player, cmd, "No article or regexp specified.");
        return -1;
    }

    bool bUseAn = false;
    bool bOkay = false;

    if (pCurrent - pArticle <= 2)
    {
        if (mux_tolower(pArticle[0]) == 'a')
        {
            if (mux_tolower(pArticle[1]) == 'n')
            {
                bUseAn = true;
                bOkay = true;
            }

            if (mux_isspace(pArticle[1]))
            {
                bOkay = true;
            }
        }
    }

    if (!bOkay)
    {
        *pCurrent = '\0';
        cf_log_syntax(player, cmd, "Invalid article '%s'.", pArticle);
        return -1;
    }

    while (mux_isspace(*pCurrent))
    {
        pCurrent++;
    }

    if (*pCurrent == '\0')
    {
        cf_log_syntax(player, cmd, "No regexp specified.");
        return -1;
    }

    const char *errptr;
    int erroffset;
    pcre* reNewRegexp = pcre_compile(pCurrent, 0, &errptr, &erroffset, NULL);
    if (!reNewRegexp)
    {
        cf_log_syntax(player, cmd, "Error processing regexp '%s':.",
              pCurrent, errptr);
        return -1;
    }

    pcre_extra *study = pcre_study(reNewRegexp, 0, &errptr);

    // Push new rule at head of list.
    ArtRuleset** arRules = (ArtRuleset **) vp;
    ArtRuleset* arNewRule = (ArtRuleset *) MEMALLOC(sizeof(ArtRuleset));

    arNewRule->m_pNextRule = *arRules;
    arNewRule->m_bUseAn = bUseAn;
    arNewRule->m_pRegexp = reNewRegexp;
    arNewRule->m_pRegexpStudy = study;

    *arRules = arNewRule;
    return 0;
}

