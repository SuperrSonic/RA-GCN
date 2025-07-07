/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2014 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *  sha1.h
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in the SHA1Context, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 *
 */

#ifndef __RARCH_HASH_H
#define __RARCH_HASH_H

#include <stdint.h>
#include <stddef.h>

#include <compat/msvc.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Hashes sha256 and outputs a human readable string
 * for comparing with the cheat XML values. */
void sha256_hash(char *out, const uint8_t *in, size_t size);

#ifdef HAVE_ZLIB
#include <zlib.h>

static inline uint32_t crc32_calculate(const uint8_t *data, size_t length)
{
   return crc32(0, data, length);
}

static inline uint32_t crc32_adjust(uint32_t crc, uint8_t data)
{
   /* zlib and nall have different
    * assumptions on "sign" for this function. */
   return ~crc32(~crc, &data, 1);
}
#else
uint32_t crc32_calculate(const uint8_t *data, size_t length);
uint32_t crc32_adjust(uint32_t crc, uint8_t data);
#endif

typedef struct SHA1Context
{
   unsigned Message_Digest[5]; /* Message Digest (output)          */

   unsigned Length_Low;        /* Message length in bits           */
   unsigned Length_High;       /* Message length in bits           */

   unsigned char Message_Block[64]; /* 512-bit message blocks      */
   int Message_Block_Index;    /* Index into message block array   */

   int Computed;               /* Is the digest computed?          */
   int Corrupted;              /* Is the message digest corruped?  */
} SHA1Context;

void SHA1Reset(SHA1Context *);
int SHA1Result(SHA1Context *);
void SHA1Input( SHA1Context *, const unsigned char *, unsigned);

#endif

