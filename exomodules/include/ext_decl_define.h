/**
 * @file    ext_decl_define.h
 *
 * @brief   This file does not use the standard multiple-include guards because
 * it is designed to modify the behavior of a single include file rather than a
 * compilation module. Each time it is included, it undefines the macros in it
 * (if they are defined) and then defines them again according to the existing
 * macro settings (DECLARE_GLOBALS).
 *
 * @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifdef DeclareVariable
    #undef DeclareVariable
#endif
#ifdef EXT_DECL
    #undef EXT_DECL
    #undef InitVar
#endif

#ifdef DECLARE_GLOBALS
    #define EXT_DECL
    #define InitVar 1
    #define DeclareVariable(_Type, _Name, _Init) _Type _Name = _Init
#else
    #define EXT_DECL extern
    #define InitVar 0
    #define DeclareVariable(_Type, _Name, _Init) extern _Type _Name
#endif
