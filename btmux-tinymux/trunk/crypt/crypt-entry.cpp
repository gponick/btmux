/*
 * UFC-crypt: ultra fast crypt(3) implementation
 *
 * Copyright (C) 1991, 1992, 1993, 1996, 1997 Free Software Foundation, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * crypt entry points
 *
 * @(#)crypt-entry.c    1.2 12/20/96
 *
 */

#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>

#ifndef STATIC
#define STATIC static
#endif

#ifndef DOS
#include "ufc-crypt.h"
#else
/*
 * Thanks to greg%wind@plains.NoDak.edu (Greg W. Wettstein)
 * for DOS patches
 */
#include "pl.h"
#include "ufc.h"
#endif
#include "crypt.h"
#include "crypt-private.h"

/* Prototypes for local functions.  */
extern void _ufc_setup_salt_r(const char *s, struct crypt_data *__data);
extern void _ufc_mk_keytab_r(char *key, struct crypt_data *__data);
extern void _ufc_dofinalperm_r (ufc_long *res, struct crypt_data *__data);
extern void _ufc_output_conversion_r (ufc_long v1, ufc_long v2,
                      const char *salt,
                      struct crypt_data *__data);

extern char *__md5_crypt_r (const char *key, const char *salt, char *buffer,
                int buflen);
extern char *__md5_crypt (const char *key, const char *salt);

/* Define our magic string to mark salt for MD5 encryption
   replacement.  This is meant to be the same as for other MD5 based
   encryption implementations.  */
static const char md5_salt_prefix[] = "$1$";

/* For use by the old, non-reentrant routines (crypt/encrypt/setkey)  */
extern struct crypt_data _ufc_foobar;

/*
 * UNIX crypt function
 */

char *
crypt_r (const char *key, const char *salt, struct crypt_data *data)
{
  ufc_long res[4];
  char ktab[9];
  ufc_long xx = 25; /* to cope with GCC long long compiler bugs */

#ifdef _LIBC
  /* Try to find out whether we have to use MD5 encryption replacement.  */
  if (strncmp (md5_salt_prefix, salt, sizeof (md5_salt_prefix) - 1) == 0)
    return __md5_crypt_r (key, salt, (char *) data,
              sizeof (struct crypt_data));
#endif

  /*
   * Hack DES tables according to salt
   */
  _ufc_setup_salt_r (salt, data);

  /*
   * Setup key schedule
   */
  memset(ktab, 0, sizeof(ktab));
  strncpy (ktab, key, 8);
  _ufc_mk_keytab_r (ktab, data);

  /*
   * Go for the 25 DES encryptions
   */
  memset(res, 0, sizeof (res));
  _ufc_doit_r (xx,  data, &res[0]);

  /*
   * Do final permutations
   */
  _ufc_dofinalperm_r (res, data);

  /*
   * And convert back to 6 bit ASCII
   */
  _ufc_output_conversion_r (res[0], res[1], salt, data);
  return data->crypt_3_buf;
}
//weak_alias (__crypt_r, crypt_r)

char *crypt(const char *key, const char *salt)
{
#ifdef _LIBC
  /* Try to find out whether we have to use MD5 encryption replacement.  */
  if (strncmp (md5_salt_prefix, salt, sizeof (md5_salt_prefix) - 1) == 0)
    return __md5_crypt (key, salt);
#endif

  return crypt_r (key, salt, &_ufc_foobar);
}


/*
 * To make fcrypt users happy.
 * They don't need to call init_des.
 */
#ifdef _LIBC
weak_alias (crypt, fcrypt)
#else
char *
__fcrypt (const char *key, const char *salt)
{
  return crypt (key, salt);
}
#endif
