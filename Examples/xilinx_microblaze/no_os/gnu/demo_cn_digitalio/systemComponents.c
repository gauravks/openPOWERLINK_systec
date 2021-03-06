/**
********************************************************************************
\file       systemComponents.c

\brief      Module which contains of processor specific functions
            (microblaze version)

Provides all functions which are platform dependent for the application of the
directIO example.

Copyright (c) 2012, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
Copyright (c) 2012, SYSTEC electronik GmbH
Copyright (c) 2012, Kalycito Infotech Private Ltd.
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
*******************************************************************************/


//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include "systemComponents.h"

#include "xgpio_l.h"
#include "mb_interface.h"
#include "xilinx_irq.h"

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
// local types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------
static DWORD bEplStatusLeds = 0;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------


//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief               init the processor peripheral

Enable/invalidate the instruction and data cache. Reset the Leds and init
the interrupt.
*/
//------------------------------------------------------------------------------
void SysComp_initPeripheral(void)
{
    #if XPAR_MICROBLAZE_USE_ICACHE
        microblaze_invalidate_icache();
        microblaze_enable_icache();
    #endif

    #if XPAR_MICROBLAZE_USE_DCACHE
        microblaze_invalidate_dcache();
        microblaze_enable_dcache();
    #endif

    #ifdef XPAR_LEDS_OUTPUT_BASEADDR
        XGpio_WriteReg(XPAR_LEDS_OUTPUT_BASEADDR, XGPIO_TRI_OFFSET, 0);
    #endif

    #ifdef XPAR_POWERLINK_LED_BASEADDR
        XGpio_WriteReg(XPAR_POWERLINK_LED_BASEADDR, XGPIO_TRI_OFFSET, 0);
    #endif

    initInterrupts();
}

//------------------------------------------------------------------------------
/**
\brief               enable global interrupts

Call enable interrupts function in the xilinx target folder
*/
//------------------------------------------------------------------------------
void SysComp_enableInterrupts(void)
{
    enableInterrupts();
}

//------------------------------------------------------------------------------
/**
\brief               free the processor cache

Flush and disable the instruction and data cache.
*/
//------------------------------------------------------------------------------
void SysComp_freeProcessorCache(void)
{
    #if XPAR_MICROBLAZE_USE_DCACHE
        microblaze_invalidate_dcache();
        microblaze_disable_dcache();
    #endif

    #if XPAR_MICROBLAZE_USE_ICACHE
        microblaze_invalidate_icache();
        microblaze_disable_icache();
    #endif
}


//------------------------------------------------------------------------------
/**
\brief              read the node ID from the available peripheral

This function reads the node id from the given board peripheral. If the board
is not supporting node switches zero is returned.

\return             nodeId
\retval             [1-239]         the given node id
*/
//------------------------------------------------------------------------------
BYTE SysComp_getNodeId(void)
{
    BYTE nodeId = 0;

    /* read dip switches for node id */
#ifdef NODE_SWITCH_BASE
    nodeId = XGpio_ReadReg(NODE_SWITCH_BASE, 0);
#endif

    return nodeId;
}

//------------------------------------------------------------------------------
/**
\brief              set the powerlink led

This function sets the powerlink status or error led.

\param              bBitNum_p       powerlink status (1: state; 2: error)
*/
//------------------------------------------------------------------------------
void SysComp_setPowerlinkStatus(BYTE bBitNum_p)
{
    bEplStatusLeds |= bBitNum_p;

    #ifdef STATUS_LEDS_BASE
        XGpio_WriteReg(STATUS_LEDS_BASE, XGPIO_DATA_OFFSET, bEplStatusLeds);
    #endif
}

//------------------------------------------------------------------------------
/**
\brief             reset the powerlink led

This function resets the powerlink status or error led.

\param             bBitNum_p       powerlink status (1: state; 2: error)
*/
//------------------------------------------------------------------------------
void SysComp_resetPowerlinkStatus(BYTE bBitNum_p)
{
    bEplStatusLeds &= ~bBitNum_p;

    #ifdef STATUS_LEDS_BASE
        XGpio_WriteReg(STATUS_LEDS_BASE, XGPIO_DATA_OFFSET, bEplStatusLeds);
    #endif
}
