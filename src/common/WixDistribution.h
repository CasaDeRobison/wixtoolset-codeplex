//-------------------------------------------------------------------------------------------------
// <copyright file="WixDistribution.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Contains disribution specific items, such as Product Name
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#ifndef __WIXDISTRIBUTION_FILE_H_
#define __WIXDISTRIBUTION_FILE_H_

#ifdef VER_PRODUCT_NAME
    #undef VER_PRODUCT_NAME
#endif
#define VER_PRODUCT_NAME "WiX Toolset"

#endif // __WIXDISTRIBUTION_FILE_H_
