/**
********************************************************************************
\file   hostiflib_arm.c

\brief  Host Interface Library Support File - For ARM target

This file provides specific function definition for Zynq ARM(ps7_cortexa9_0) CPU
to support host interface

\ingroup module_hostiflib
*******************************************************************************/
/*------------------------------------------------------------------------------
Copyright (c) 2012 Kalycito Infotech Private Limited
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

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include "hostiflib_arm.h"

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
\brief  Flush Data cache for the specified range

\param  dwAddr_p    base address of the range to be flushed
\param  span_p      Range of addresses to be flushed


\return void

\ingroup module_hostiflib
*/
//------------------------------------------------------------------------------
void hostif_FlushDCacheRange(u32 dwAddr_p,u16 span_p)
{
    Xil_DCacheFlushRange(dwAddr_p, span_p);
}
//------------------------------------------------------------------------------
/**
\brief  Invalidate Data cache for the specified range

\param  dwAddr_p    base address of the range to be flushed
\param  span_p      Range of addresses to be flushed


\return void

\ingroup module_hostiflib
*/
//------------------------------------------------------------------------------
void hostif_InvalidateDCacheRange(u32 dwAddr_p,u16 span_p)
{
    Xil_DCacheInvalidateRange(dwAddr_p, span_p);
}
//------------------------------------------------------------------------------
/**
\brief  READ and WRITE functions with Invalidation and flushing

\param  base      base address to be READ/WRITE
\param  offset    Offset to be READ/WRITE

 */
//------------------------------------------------------------------------------
u32 ARM_READ32(void* base,u32 offset)
{
    u32 Address = (u32)base + offset;
    Xil_DCacheInvalidateRange(Address,4);
    return Xil_In32(Address);;
}
u16 ARM_READ16(void* base,u32 offset)
{
    u32 Address = (u32)base + offset;
    Xil_DCacheInvalidateRange(Address,2);
    return Xil_In16(Address);;
}
u8  ARM_READ8(void* base, u32 offset)
{
    u32 Address = (u32)base + offset;
    Xil_DCacheInvalidateRange(Address,1);
    return Xil_In8(Address);;
}

void ARM_WRITE32(void* base,u32 offset,u32 dword)
{
    u32 Address = (u32)base + offset;
    Xil_Out32(Address, dword);
    Xil_DCacheFlushRange(Address,4);
}
void ARM_WRITE16(void* base,u32 offset,u16 word)
{
    u32 Address = (u32)base + offset;
    Xil_Out16(Address, word);
    Xil_DCacheFlushRange(Address,2);
}
void ARM_WRITE8(void* base,u32 offset,u8 byte)
{
    u32 Address = (u32)base + offset;
    Xil_Out8(Address, byte);
    Xil_DCacheFlushRange(Address,1);
}

/**
 * EOF
 */
