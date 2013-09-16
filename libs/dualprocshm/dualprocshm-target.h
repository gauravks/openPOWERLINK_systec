/**
********************************************************************************
\file   dualprocshm-target.h

\brief  Dual Processor Library - Target header file

This header file defines target specific macros (e.g. data types) and selects
the target specific header file (e.g. dualprocshm_microblaze.h).

*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2012 Kalycito Infotech Private Limited
              www.kalycito.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

#ifndef _INC_DUALPROCSHM_TARGET_H_
#define _INC_DUALPROCSHM_TARGET_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <stdint.h>

#if defined(__ZYNQ__)

//#error ZYNQ
#include "dualprocshm-zynq.h"

#else

#error "Paltform is not supported! Please point the target platform file in dualprocshm-platform.h "

#endif

//---------------------------------------------------------
// include section header file with null macros
#include <section-default.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

/**
\name Data types
If the following data types are not defined in the environment, then they are
set to those provided by stdint.h.
*/
/**@{*/
#ifndef UINT8
#define UINT8               uint8_t
#endif

#ifndef UINT16
#define UINT16              uint16_t
#endif

#ifndef UINT32
#define UINT32              uint32_t
#endif

#ifndef BOOL
#define BOOL                uint8_t
#endif

#ifndef FALSE
#define FALSE               0x00
#endif

#ifndef TRUE
#define TRUE                0xFF
#endif
/**@}*/

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

UINT8* dualprocshm_getCommonMemAddr(UINT16* pSize_p);
UINT8* dualprocshm_getDynMapTableAddr(void);
void dualprocshm_targetReadData(UINT8* pBase_p, UINT16 Size_p, UINT8* pData_p);
void dualprocshm_targetWriteData(UINT8* pBase_p, UINT16 Size_p, UINT8* pData_p);
void dualprocshm_releaseCommonMemAddr(UINT16 pSize_p);
void dualprocshm_releaseDynMapTableAddr();

#endif /* _INC_DUALPROCSHM_TARGET_H_ */