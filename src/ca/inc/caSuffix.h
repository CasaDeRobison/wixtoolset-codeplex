#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="caSuffix.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Platform specific suffix defines/utilities.
//    Must be kept in sync with caSuffix.wxi.
// </summary>
//-------------------------------------------------------------------------------------------------

#if defined _WIN64 // x64 and ia64 get this one
#define PLATFORM_DECORATION(f) f ## L"_64"
#else
#define PLATFORM_DECORATION(f) f
#endif
