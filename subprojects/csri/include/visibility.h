/*****************************************************************************
 * asa: portable digital subtitle renderer
 *****************************************************************************
 * Copyright (C) 2007  David Lamparter
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/


#ifndef _VISIBILITY_H
#define _VISIBILITY_H

#ifdef HAVE_CONFIG_H
#include "acconf.h"
#endif

#if defined(_WIN32) && !defined(CSRI_NO_EXPORT)
#define export __declspec(dllexport)
#define internal
#define hidden
#elif HAVE_GCC_VISIBILITY
#define export __attribute__((visibility ("default")))
#define internal __attribute__((visibility ("internal")))
#define hidden __attribute__((visibility ("hidden")))
#else
#define export
#define internal
#define hidden
#endif

#endif /* _VISIBILITY_H */
