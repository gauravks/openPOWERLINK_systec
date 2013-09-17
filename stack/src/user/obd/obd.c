/****************************************************************************

  (c) SYSTEC electronic GmbH, D-07973 Greiz, August-Bebel-Str. 29
      www.systec-electronic.com

  Project:      openPOWERLINK

  Description:  source file for api function of EplOBD-Module

  License:

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    3. Neither the name of SYSTEC electronic GmbH nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without prior written permission. For written
       permission, please contact info@systec-electronic.com.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Severability Clause:

        If a provision of this License is or becomes illegal, invalid or
        unenforceable in any jurisdiction, that shall not affect:
        1. the validity or enforceability in that jurisdiction of any other
           provision of this License; or
        2. the validity or enforceability in other jurisdictions of that or
           any other provision of this License.

  -------------------------------------------------------------------------

                $RCSfile$

                $Author$

                $Revision$  $Date$

                $State$

                Build Environment:
                Microsoft VC7

  -------------------------------------------------------------------------

  Revision History:

  2006/06/02 k.t.:   start of the implementation, version 1.00
		     ->based on CANopen OBD-Modul

****************************************************************************/

#include "EplInc.h"
#include "obd.h"


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*          G L O B A L   D E F I N I T I O N S                            */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

//---------------------------------------------------------------------------
// const defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// local types
//---------------------------------------------------------------------------

// struct for instance table
static tEplObdInitParam             m_ObdInitParam;
static tEplObdStoreLoadObjCallback  m_fpStoreLoadObjCallback;

// decomposition of float
typedef union
{
    tEplObdReal32   m_flRealPart;
    int             m_nIntegerPart;

} tEplObdRealParts;


//---------------------------------------------------------------------------
// module global vars
//---------------------------------------------------------------------------

// This macro replace the unspecific pointer to an instance through
// the modul specific type for the local instance table. This macro
// must defined in each modul.
//#define tEplPtrInstance             tEplInstanceInfo MEM*

BYTE MEM            abEplObdTrashObject_g[8];


//---------------------------------------------------------------------------
// local function prototypes
//---------------------------------------------------------------------------

static tEplKernel   callObjectCallback(tEplObdCallback pfnCallback_p, tEplObdCbParam MEM* pCbParam_p);
static tEplObdSize  getDataSizeIntern(tEplObdSubEntryPtr pSubIndexEntry_p);
static tEplObdSize  getObdStringLen(void* pObjData_p, tEplObdSize objLen_p, tEplObdType objType_p);
#if (EPL_OBD_CHECK_OBJECT_RANGE != FALSE)
static tEplKernel   checkObjectRange(tEplObdSubEntryPtr pSubindexEntry_p, void * pData_p);
#endif
static tEplKernel   getVarEntry(tEplObdSubEntryPtr pSubindexEntry_p, tEplObdVarEntry MEM** ppVarEntry_p);
static tEplKernel   getEntry(UINT index_p, UINT subindex_p, tEplObdEntryPtr* ppObdEntry_p,
                             tEplObdSubEntryPtr* ppObdSubEntry_p);
static tEplObdSize  getObjectSize(tEplObdSubEntryPtr pSubIndexEntry_p);
static tEplKernel   getIndexIntern(tEplObdInitParam MEM* pInitParam_p, UINT index_p, tEplObdEntryPtr* ppObdEntry_p);
static tEplKernel   getSubindexIntern(tEplObdEntryPtr pObdEntry_p, UINT subIndex_p, tEplObdSubEntryPtr* ppObdSubEntry_p);
static tEplKernel   accessOdPartIntern(tEplObdPart currentOdPart_p, tEplObdEntryPtr pObdEnty_p, tEplObdDir direction_p);
static CONST void*  getObjectDefaultPtr (tEplObdSubEntryPtr pSubIndexEntry_p);
static void MEM*    getObjectCurrentPtr (tEplObdSubEntryPtr pSubIndexEntry_p);
#if (EPL_OBD_USE_STORE_RESTORE != FALSE)
static tEplKernel   callStoreCallback(tEplObdCbStoreParam MEM* pCbStoreParam_p);
#endif // (EPL_OBD_USE_STORE_RESTORE != FALSE)
static void         copyObjectData(void MEM* pDstData_p, CONST void* pSrcData_p, tEplObdSize objSize_p,
                                   tEplObdType objType_p);
static tEplKernel   callPostDefault(void *pData_p, tEplObdEntryPtr pObdEntry_p, tEplObdSubEntryPtr pSubIndex_p);
static void*        getObjectDataPtrIntern(tEplObdSubEntryPtr pSubindexEntry_p);
static tEplKernel   isNumericalIntern(tEplObdSubEntryPtr pObdSubEntry_p, BOOL* pfEntryNumerical_p);
static tEplKernel   writeEntryPre(UINT uiIndex_p, UINT subIndex_p, void* pSrcData_p, void** ppDstData_p,
                                  tEplObdSize Size_p, tEplObdEntryPtr* ppObdEntry_p, tEplObdSubEntryPtr* ppSubEntry_p,
                                  tEplObdCbParam MEM* pCbParam_p, tEplObdSize* pObdSize_p);
static tEplKernel   writeEntryPost(tEplObdEntryPtr pObdEntry_p, tEplObdSubEntryPtr pSubEntry_p,
                                   tEplObdCbParam MEM* pCbParam_p, void* pSrcData_p,
                                   void* pDstData_p, tEplObdSize obdSize_p);

//=========================================================================//
//                                                                         //
//          P U B L I C   F U N C T I O N S                                //
//                                                                         //
//=========================================================================//

//---------------------------------------------------------------------------
//
// Function:    obd_init()
//
// Description: initializes the first instance
//
// Parameters:  pInitParam_p    = init parameter
//
// Return:      tEplKernel      =   errorcode
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_init(tEplObdInitParam MEM* pInitParam_p)
{

tEplKernel Ret;

    if (pInitParam_p == NULL)
    {
        Ret = kEplSuccessful;
        goto Exit;
    }

    Ret = obd_addInstance (
        pInitParam_p);

Exit:
    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_addInstance()
//
// Description: adds a new instance
//
// Parameters:  pInitParam_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_addInstance(tEplObdInitParam MEM* pInitParam_p)
{

tEplKernel Ret;


    // save init parameters
    EPL_MEMCPY (&m_ObdInitParam, pInitParam_p, sizeof (tEplObdInitParam));

    // clear callback function for command LOAD and STORE
    m_fpStoreLoadObjCallback = NULL;

    // initialize object dictionary
    // so all all VarEntries will be initialized to trash object and default values will be set to current data
    Ret = obd_accessOdPart (kEplObdPartAll, kEplObdDirInit);

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_deleteInstance()
//
// Description: delete instance
//
// Parameters:
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------
#if (EPL_USE_DELETEINST_FUNC != FALSE)
tEplKernel obd_deleteInstance(void)
{
    return kEplSuccessful;

}
#endif // (EPL_USE_DELETEINST_FUNC != FALSE)


//---------------------------------------------------------------------------
//
// Function:    obd_writeEntry()
//
// Description: Function writes data to an OBD entry. Strings
//              are stored with added '\0' character.
//
// Parameters:
//              uiIndex_p       =   Index of the OD entry
//              subIndex_p    =   Subindex of the OD Entry
//              pSrcData_p      =   Pointer to the data to write
//              Size_p          =   Size of the data in Byte
//
// Return:      tEplKernel      =   Errorcode
//
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_writeEntry(UINT index_p, UINT subIndex_p, void* pSrcData_p, tEplObdSize size_p)
{

tEplKernel              Ret;
tEplObdEntryPtr         pObdEntry;
tEplObdSubEntryPtr      pSubEntry;
tEplObdCbParam MEM      CbParam;
void MEM*               pDstData;
tEplObdSize             ObdSize;


    Ret = writeEntryPre (
                               index_p,
                               subIndex_p,
                               pSrcData_p,
                               &pDstData,
                               size_p,
                               &pObdEntry,
                               &pSubEntry,
                               &CbParam,
                               &ObdSize);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    Ret = writeEntryPost (
                                pObdEntry,
                                pSubEntry,
                                &CbParam,
                                pSrcData_p,
                                pDstData,
                                ObdSize);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_readEntry()
//
// Description: The function reads an object entry. The application
//              can always read the data even if attrib kEplObdAccRead
//              is not set. The attrib is only checked up for SDO transfer.
//
// Parameters:
//              index_p       = Index oof the OD entry to read
//              subIndex_p    = Subindex to read
//              pDstData_p      = pointer to the buffer for data
//              Offset_p        = offset in data for read access
//              pSize_p         = IN: Size of the buffer
//                                OUT: number of readed Bytes
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_readEntry(UINT index_p, UINT subIndex_p, void* pDstData_p, tEplObdSize* pSize_p)
{

tEplKernel                      Ret;
tEplObdEntryPtr                 pObdEntry;
tEplObdSubEntryPtr              pSubEntry;
tEplObdCbParam  MEM             CbParam;
void *                          pSrcData;
tEplObdSize                     ObdSize;

    ASSERT (pDstData_p != NULL);
    ASSERT (pSize_p != NULL);

    // get address of index and subindex entry
    Ret = getEntry (
        index_p, subIndex_p, &pObdEntry, &pSubEntry);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get pointer to object data
     pSrcData = getObjectDataPtrIntern (pSubEntry);

    // check source pointer
    if (pSrcData == NULL)
    {
        Ret = kEplObdReadViolation;
        goto Exit;
    }

    //------------------------------------------------------------------------
    // address of source data to structure of callback parameters
    // so callback function can change this data before reading
    CbParam.m_uiIndex   = index_p;
    CbParam.m_uiSubIndex= subIndex_p;
    CbParam.m_pArg      = pSrcData;
    CbParam.m_ObdEvent  = kEplObdEvPreRead;
    Ret = callObjectCallback (
        pObdEntry->m_fpCallback, &CbParam);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get size of data and check if application has reserved enough memory
    ObdSize = getDataSizeIntern (pSubEntry);

    // check if offset given and calc correct number of bytes to read
    if (*pSize_p < ObdSize)
    {
        Ret = kEplObdValueLengthError;
        goto Exit;
    }

    // read value from object
    EPL_MEMCPY (pDstData_p, pSrcData, ObdSize);

    if (pSubEntry->m_Type == kEplObdTypVString)
    {
        if (*pSize_p > ObdSize)
        {   // space left to set the terminating null-character
            ((char MEM*) pDstData_p)[ObdSize] = '\0';
            ObdSize++;
        }
    }
    *pSize_p = ObdSize;

    // write address of destination data to structure of callback parameters
    // so callback function can change this data after reading
    CbParam.m_pArg     = pDstData_p;
    CbParam.m_ObdEvent = kEplObdEvPostRead;
    Ret = callObjectCallback (
        pObdEntry->m_fpCallback, &CbParam);

Exit:

    return Ret;

}

//---------------------------------------------------------------------------
//
// Function:    obd_accessOdPart()
//
// Description: restores default values of one part of OD
//
// Parameters:  ObdPart_p
//              direction_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_accessOdPart (tEplObdPart obdPart_p, tEplObdDir  direction_p)
{

tEplKernel      Ret = kEplSuccessful;
BOOL            fPartFount;
tEplObdEntryPtr pObdEntry;

    //  part always has to be unequal to NULL
    pObdEntry = m_ObdInitParam.m_pGenericPart;
    ASSERTMSG (pObdEntry != NULL, "obd_accessOdPart(): no  OD part is defined!\n");

    // if obdPart_p is not valid fPartFound keeps FALSE and function returns kEplObdIllegalPart
    fPartFount = FALSE;

    // access to  part
    if ((obdPart_p & kEplObdPartGen) != 0)
    {
        fPartFount = TRUE;

        Ret = accessOdPartIntern (
            kEplObdPartGen, pObdEntry, direction_p);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }
    }

    // access to manufacturer part
    pObdEntry = m_ObdInitParam.m_pManufacturerPart;

    if ( ((obdPart_p & kEplObdPartMan) != 0) &&
         (pObdEntry != NULL) )
    {
        fPartFount = TRUE;

        Ret = accessOdPartIntern (
            kEplObdPartMan, pObdEntry, direction_p);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }
    }

    // access to device part
    pObdEntry = m_ObdInitParam.m_pDevicePart;

    if ( ((obdPart_p & kEplObdPartDev) != 0) &&
         (pObdEntry != NULL) )
    {
        fPartFount = TRUE;

        Ret = accessOdPartIntern (
            kEplObdPartDev, pObdEntry, direction_p);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }
    }

    #if (defined (EPL_OBD_USER_OD) && (EPL_OBD_USER_OD != FALSE))
    {
        // access to user part
        pObdEntry = m_ObdInitParam.m_pUserPart;

        if ( ((obdPart_p & kEplObdPartUsr) != 0) &&
             (pObdEntry != NULL) )
        {
            fPartFount = TRUE;

            Ret = accessOdPartIntern (kEplObdPartUsr, pObdEntry, direction_p);
            if (Ret != kEplSuccessful)
            {
                goto Exit;
            }
        }
    }
    #endif

    // no access to an OD part was done? illegal OD part was specified!
    if (fPartFount == FALSE)
    {
        Ret = kEplObdIllegalPart;
    }

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_defineVar()
//
// Description: defines a variable in OD
//
// Parameters:  pEplVarParam_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_defineVar (tEplVarParam MEM* pVarParam_p)
{

tEplKernel              Ret;
tEplObdVarEntry MEM*    pVarEntry;
tEplVarParamValid       VarValid;
tEplObdSubEntryPtr      pSubindexEntry;

    ASSERT (pVarParam_p != NULL);   // is not allowed to be NULL

    // get address of subindex entry
    Ret = getEntry (
        pVarParam_p->m_uiIndex,
        pVarParam_p->m_uiSubindex,
        NULL, &pSubindexEntry);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get var entry
    Ret = getVarEntry (pSubindexEntry, &pVarEntry);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    VarValid =  pVarParam_p->m_ValidFlag;

    // copy only this values, which valid flag is set
    if ((VarValid & kVarValidSize) != 0)
    {
        if (pSubindexEntry->m_Type != kEplObdTypDomain)
        {
        tEplObdSize DataSize;

            // check passed size parameter
            DataSize = getObjectSize(pSubindexEntry);
            if (DataSize != pVarParam_p->m_Size)
            {   // size of variable does not match
                Ret = kEplObdValueLengthError;
                goto Exit;
            }
        }
        else
        {   // size can be set only for objects of type DOMAIN
            pVarEntry->m_Size = pVarParam_p->m_Size;
        }
    }

    if ((VarValid & kVarValidData) != 0)
    {
       pVarEntry->m_pData = pVarParam_p->m_pData;
    }
/*
    #if (EPL_PDO_USE_STATIC_MAPPING == FALSE)
    {
        if ((VarValid & kVarValidCallback) != 0)
        {
           pVarEntry->m_fpCallback = pVarParam_p->m_fpCallback;
        }

        if ((VarValid & kVarValidArg) != 0)
        {
           pVarEntry->m_pArg = pVarParam_p->m_pArg;
        }
    }
    #endif
*/
    // Ret is already set to kEplSuccessful from ObdGetVarIntern()

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_getObjectDataPtr()
//
// Description: It returnes the current data pointer. But if object is an
//              constant object it returnes the default pointer.
//
// Parameters:  index_p    =   Index of the entry
//              uiSubindex_p =   Subindex of the entry
//
// Return:      void *    = pointer to object data
//
// State:
//
//---------------------------------------------------------------------------

void* obd_getObjectDataPtr (UINT index_p, UINT subIndex_p)
 {
tEplKernel          Ret;
void *       pData;
tEplObdEntryPtr     pObdEntry;
tEplObdSubEntryPtr  pObdSubEntry;


    // get pointer to index structure
    Ret = getIndexIntern (&m_ObdInitParam,
                                index_p,
                                &pObdEntry);
    if(Ret != kEplSuccessful)
    {
        pData = NULL;
        goto Exit;
    }

    // get pointer to subindex structure
    Ret = getSubindexIntern (pObdEntry,
                                subIndex_p,
                                &pObdSubEntry);
    if(Ret != kEplSuccessful)
    {
        pData = NULL;
        goto Exit;
    }
    // get Datapointer
    pData = getObjectDataPtrIntern(pObdSubEntry);

Exit:
    return pData;

}


#if (defined (EPL_OBD_USER_OD) && (EPL_OBD_USER_OD != FALSE))
//jba dead code!
//---------------------------------------------------------------------------
//
// Function:    obd_registerUserOd()
//
// Description: function registers the user OD
//
// Parameters:  pUserOd_p   =pointer to user ODd
//
// Return:     tEplKernel = errorcode
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_registerUserOd (tEplObdEntryPtr pUserOd_p)
{

    m_ObdInitParam.m_pUserPart = pUserOd_p;

    return kEplSuccessful;

}

#endif


//---------------------------------------------------------------------------
//
// Function:    obd_initVarEntry()
//
// Description: function to initialize VarEntry dependened on object type
//
// Parameters:  pVarEntry_p = pointer to var entry structure
//              Type_p      = object type
//              obdSize_p   = size of object data
//
// Returns:     none
//
// State:
//
//---------------------------------------------------------------------------

void obd_initVarEntry (tEplObdVarEntry MEM* pVarEntry_p, tEplObdType type_p, tEplObdSize obdSize_p)
{
/*
    #if (EPL_PDO_USE_STATIC_MAPPING == FALSE)
    {
        // reset pointer to VAR callback and argument
        pVarEntry_p->m_fpCallback  = NULL;
        pVarEntry_p->m_pArg = NULL;
    }
    #endif
*/

// 10-dec-2004 r.d.: this function will not be used for strings
    if ((type_p == kEplObdTypDomain))
//         (bType_p == kEplObdTypVString) /* ||
//         (bType_p == kEplObdTypOString) ||
//         (bType_p == kEplObdTypUString)    */ )
    {
        // variables which are defined as DOMAIN or VSTRING should not point to
        // trash object, because this trash object contains only 8 bytes. DOMAINS or
        // STRINGS can be longer.
        pVarEntry_p->m_pData = NULL;
        pVarEntry_p->m_Size  = 0;
    }
    else
    {
        // set address to variable data to trash object
        // This prevents an access violation if user forgets to call obd_defineVar()
        // for this variable but mappes it in a PDO.
        pVarEntry_p->m_pData = &abEplObdTrashObject_g[0];
        pVarEntry_p->m_Size  = obdSize_p;
    }

}


//---------------------------------------------------------------------------
//
// Function:    obd_getDataSize()
//
// Description: gets the data size of an object,
//              for string objects it returnes the string length
//              without terminating null-character
//
// Parameters:
//              index_p   =   Index
//              subIndex_p=   Subindex
//
// Return:      tEplObdSize
//
// State:
//
//---------------------------------------------------------------------------
tEplObdSize obd_getDataSize(UINT index_p, UINT subIndex_p)
{
tEplKernel          Ret;
tEplObdSize         ObdSize;
tEplObdEntryPtr     pObdEntry;
tEplObdSubEntryPtr  pObdSubEntry;


    // get pointer to index structure
    Ret = getIndexIntern (&m_ObdInitParam,
                                index_p,
                                &pObdEntry);
    if(Ret != kEplSuccessful)
    {
        ObdSize = 0;
        goto Exit;
    }

    // get pointer to subindex structure
    Ret = getSubindexIntern (pObdEntry,
                                subIndex_p,
                                &pObdSubEntry);
    if(Ret != kEplSuccessful)
    {
        ObdSize = 0;
        goto Exit;
    }

    // get size
    ObdSize = getDataSizeIntern (pObdSubEntry);
Exit:
    return ObdSize;
}
//---------------------------------------------------------------------------
//
// Function:    obd_getNodeId()
//
// Description: function returns nodeid from entry 0x1F93
//
//
// Parameters:
//
// Return:      UINT = Node Id
//
// State:
//
//---------------------------------------------------------------------------
UINT obd_getNodeId(void)
{
tEplKernel      Ret;
tEplObdSize     ObdSize;
BYTE            bNodeId;

    bNodeId = 0;
    ObdSize = sizeof(bNodeId);
    Ret = obd_readEntry(
                            EPL_OBD_NODE_ID_INDEX,
                            EPL_OBD_NODE_ID_SUBINDEX,
                            &bNodeId,
                            &ObdSize);
    if(Ret != kEplSuccessful)
    {
        bNodeId = EPL_C_ADR_INVALID;
        goto Exit;
    }

Exit:
    return (UINT) bNodeId;

}


//---------------------------------------------------------------------------
//
// Function:    obd_setNodeId()
//
// Description: function sets nodeid in entry 0x1F93
//
//
// Parameters:
//              uiNodeId_p  =   Node Id to set
//              NodeIdType_p=   Type on which way the Node Id was set
//
// Return:      tEplKernel = Errorcode
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_setNodeId(UINT nodeId_p, tEplObdNodeIdType nodeIdType_p)
{
tEplKernel  Ret;
tEplObdSize ObdSize;
BYTE        fHwBool;
BYTE        bNodeId;

    // check Node Id
    if(nodeId_p == EPL_C_ADR_INVALID)
    {
        Ret = kEplInvalidNodeId;
        goto Exit;
    }
    bNodeId = (BYTE)nodeId_p;
    ObdSize = sizeof(BYTE);
    // write NodeId to OD entry
    Ret = obd_writeEntry(
                            EPL_OBD_NODE_ID_INDEX,
                            EPL_OBD_NODE_ID_SUBINDEX,
                            &bNodeId,
                            ObdSize);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // set HWBOOL-Flag in Subindex EPL_OBD_NODE_ID_HWBOOL_SUBINDEX
    switch (nodeIdType_p)
    {
        // type unknown
        case kEplObdNodeIdUnknown:
        {
            fHwBool = OBD_FALSE;
            break;
        }

        case kEplObdNodeIdSoftware:
        {
            fHwBool = OBD_FALSE;
            break;
        }

        case kEplObdNodeIdHardware:
        {
            fHwBool = OBD_TRUE;
            break;
        }

        default:
        {
            fHwBool = OBD_FALSE;
        }

    }   // end of switch (NodeIdType_p)

    // write flag
    ObdSize = sizeof(fHwBool);
    Ret = obd_writeEntry(
                            EPL_OBD_NODE_ID_INDEX,
                            EPL_OBD_NODE_ID_HWBOOL_SUBINDEX,
                            &fHwBool,
                            ObdSize);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

Exit:
    return Ret;
}

//---------------------------------------------------------------------------
//
// Function:    obd_isNumerical()
//
// Description: function checks if a entry is numerical or not
//
//
// Parameters:
//              index_p           = Index
//              subIndex_p        = Subindex
//              pfEntryNumerical_p  = pointer to BOOL for returnvalue
//                                  -> TRUE if entry a numerical value
//                                  -> FALSE if entry not a numerical value
//
// Return:      tEplKernel = Errorcode
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_isNumerical(UINT index_p, UINT subIndex_p, BOOL* pfEntryNumerical_p)
{
tEplKernel          Ret;
tEplObdEntryPtr     pObdEntry;
tEplObdSubEntryPtr  pObdSubEntry;


    // get pointer to index structure
    Ret = getIndexIntern (&m_ObdInitParam,
                                index_p,
                                &pObdEntry);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get pointer to subindex structure
    Ret = getSubindexIntern (pObdEntry,
                                subIndex_p,
                                &pObdSubEntry);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    Ret = isNumericalIntern(pObdSubEntry, pfEntryNumerical_p);


Exit:
    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_getType()
//
// Description: function returns the data type of the specified entry
//
//
// Parameters:
//              index_p           = Index
//              subIndex_p        = Subindex
//              pType_p             = pointer to tEplObdType for returnvalue
//
// Return:      tEplKernel = Errorcode
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_getType(UINT index_p, UINT subIndex_p, tEplObdType* pType_p)
{
tEplKernel          Ret;
tEplObdEntryPtr     pObdEntry;
tEplObdSubEntryPtr  pObdSubEntry;


    // get pointer to index structure
    Ret = getIndexIntern (&m_ObdInitParam,
                                index_p,
                                &pObdEntry);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get pointer to subindex structure
    Ret = getSubindexIntern (pObdEntry,
                                subIndex_p,
                                &pObdSubEntry);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    *pType_p = pObdSubEntry->m_Type;

Exit:
    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_readEntryToLe()
//
// Description: The function reads an object entry from the byteoder
//              of the system to the little endian byteorder for numerical values.
//              For other types a normal read will be processed. This is usefull for
//              the PDO and SDO module. The application
//              can always read the data even if attrib kEplObdAccRead
//              is not set. The attrib is only checked up for SDO transfer.
//
// Parameters:
//              index_p       = Index of the OD entry to read
//              subIndex_p    = Subindex to read
//              pDstData_p      = pointer to the buffer for data
//              Offset_p        = offset in data for read access
//              pSize_p         = IN: Size of the buffer
//                                OUT: number of readed Bytes
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_readEntryToLe (UINT index_p, UINT subIndex_p, void* pDstData_p, tEplObdSize* pSize_p)
{
tEplKernel                      Ret;
tEplObdEntryPtr                 pObdEntry;
tEplObdSubEntryPtr              pSubEntry;
tEplObdCbParam  MEM             CbParam;
void *                          pSrcData;
tEplObdSize                     ObdSize;

    ASSERT (pDstData_p != NULL);
    ASSERT (pSize_p != NULL);

    // get address of index and subindex entry
    Ret = getEntry (
        index_p, subIndex_p, &pObdEntry, &pSubEntry);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get pointer to object data
     pSrcData = getObjectDataPtrIntern (pSubEntry);

    // check source pointer
    if (pSrcData == NULL)
    {
        Ret = kEplObdReadViolation;
        goto Exit;
    }

    //------------------------------------------------------------------------
    // address of source data to structure of callback parameters
    // so callback function can change this data before reading
    CbParam.m_uiIndex   = index_p;
    CbParam.m_uiSubIndex= subIndex_p;
    CbParam.m_pArg      = pSrcData;
    CbParam.m_ObdEvent  = kEplObdEvPreRead;
    Ret = callObjectCallback (
        pObdEntry->m_fpCallback, &CbParam);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get size of data and check if application has reserved enough memory
    ObdSize = getDataSizeIntern (pSubEntry);

    // check if offset given and calc correct number of bytes to read
    if (*pSize_p < ObdSize)
    {
        Ret = kEplObdValueLengthError;
        goto Exit;
    }

    // check if numerical type
    switch(pSubEntry->m_Type)
    {
        //-----------------------------------------------
        // types without ami
        case kEplObdTypVString:
        case kEplObdTypOString:
        case kEplObdTypDomain:
        default:
        {
            // read value from object
            EPL_MEMCPY (pDstData_p, pSrcData, ObdSize);

            if (pSubEntry->m_Type == kEplObdTypVString)
            {
                if (*pSize_p > ObdSize)
                {   // space left to set the terminating null-character
                    ((char MEM*) pDstData_p)[ObdSize] = '\0';
                    ObdSize++;
                }
            }
            break;
        }

        //-----------------------------------------------
        // numerical type which needs ami-write
        // 8 bit or smaller values
        case kEplObdTypBool:
        case kEplObdTypInt8:
        case kEplObdTypUInt8:
        {
            AmiSetByteToLe(pDstData_p, *((BYTE*)pSrcData));
            break;
        }

        // 16 bit values
        case kEplObdTypInt16:
        case kEplObdTypUInt16:
        {
            AmiSetWordToLe(pDstData_p, *((WORD*)pSrcData));
            break;
        }

        // 24 bit values
        case kEplObdTypInt24:
        case kEplObdTypUInt24:
        {
            AmiSetDword24ToLe(pDstData_p, *((DWORD*)pSrcData));
            break;
        }

        // 32 bit values
        case kEplObdTypInt32:
        case kEplObdTypUInt32:
        case kEplObdTypReal32:
        {
            AmiSetDwordToLe(pDstData_p, *((DWORD*)pSrcData));
            break;
        }

        // 40 bit values
        case kEplObdTypInt40:
        case kEplObdTypUInt40:
        {
            AmiSetQword40ToLe(pDstData_p, *((QWORD*)pSrcData));
            break;
        }

        // 48 bit values
        case kEplObdTypInt48:
        case kEplObdTypUInt48:
        {
            AmiSetQword48ToLe(pDstData_p, *((QWORD*)pSrcData));
            break;
        }

        // 56 bit values
        case kEplObdTypInt56:
        case kEplObdTypUInt56:
        {
            AmiSetQword56ToLe(pDstData_p, *((QWORD*)pSrcData));
            break;
        }

        // 64 bit values
        case kEplObdTypInt64:
        case kEplObdTypUInt64:
        case kEplObdTypReal64:
        {
            AmiSetQword64ToLe(pDstData_p, *((QWORD*)pSrcData));
            break;
        }

        // time of day
        case kEplObdTypTimeOfDay:
        case kEplObdTypTimeDiff:
        {
            AmiSetTimeOfDay(pDstData_p, ((tTimeOfDay*)pSrcData));
            break;
        }

    }// end of switch(pSubEntry->m_Type)

    *pSize_p = ObdSize;


    // write address of destination data to structure of callback parameters
    // so callback function can change this data after reading
    CbParam.m_pArg     = pDstData_p;
    CbParam.m_ObdEvent = kEplObdEvPostRead;
    Ret = callObjectCallback (
        pObdEntry->m_fpCallback, &CbParam);

Exit:

    return Ret;

}

//---------------------------------------------------------------------------
//
// Function:    obd_writeEntryFromLe()
//
// Description: Function writes data to an OBD entry from a source with
//              little endian byteorder to the od with system specuific
//              byteorder. Not numerical values will only by copied. Strings
//              are stored with added '\0' character.
//
// Parameters:
//              index_p       =   Index of the OD entry
//              subIndex_p    =   Subindex of the OD Entry
//              pSrcData_p      =   Pointer to the data to write
//              Size_p          =   Size of the data in Byte
//
// Return:      tEplKernel      =   Errorcode
//
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_writeEntryFromLe (UINT index_p, UINT subIndex_p, void* pSrcData_p, tEplObdSize size_p)
{
tEplKernel              Ret;
tEplObdEntryPtr         pObdEntry;
tEplObdSubEntryPtr      pSubEntry;
tEplObdCbParam MEM      CbParam;
void MEM*               pDstData;
tEplObdSize             ObdSize;
QWORD                   qwBuffer;
void*                   pBuffer = &qwBuffer;


    Ret = writeEntryPre (
                               index_p,
                               subIndex_p,
                               pSrcData_p,
                               &pDstData,
                               size_p,
                               &pObdEntry,
                               &pSubEntry,
                               &CbParam,
                               &ObdSize);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }


    // check if numerical type
    switch(pSubEntry->m_Type)
    {
        //-----------------------------------------------
        // types without ami
        default:
        {   // do nothing, i.e. use the given source pointer
            pBuffer = pSrcData_p;
            break;
        }

        //-----------------------------------------------
        // numerical type which needs ami-write
        // 8 bit or smaller values
        case kEplObdTypBool:
        case kEplObdTypInt8:
        case kEplObdTypUInt8:
        {
            *((BYTE*)pBuffer) = AmiGetByteFromLe(pSrcData_p);
            break;
        }

        // 16 bit values
        case kEplObdTypInt16:
        case kEplObdTypUInt16:
        {
            *((WORD*)pBuffer) = AmiGetWordFromLe(pSrcData_p);
            break;
        }

        // 24 bit values
        case kEplObdTypInt24:
        case kEplObdTypUInt24:
        {
            *((DWORD*)pBuffer) = AmiGetDword24FromLe(pSrcData_p);
            break;
        }

        // 32 bit values
        case kEplObdTypInt32:
        case kEplObdTypUInt32:
        case kEplObdTypReal32:
        {
            *((DWORD*)pBuffer) = AmiGetDwordFromLe(pSrcData_p);
            break;
        }

        // 40 bit values
        case kEplObdTypInt40:
        case kEplObdTypUInt40:
        {
            *((QWORD*)pBuffer) = AmiGetQword40FromLe(pSrcData_p);
            break;
        }

        // 48 bit values
        case kEplObdTypInt48:
        case kEplObdTypUInt48:
        {
            *((QWORD*)pBuffer) = AmiGetQword48FromLe(pSrcData_p);
            break;
        }

        // 56 bit values
        case kEplObdTypInt56:
        case kEplObdTypUInt56:
        {
            *((QWORD*)pBuffer) = AmiGetQword56FromLe(pSrcData_p);
            break;
        }

        // 64 bit values
        case kEplObdTypInt64:
        case kEplObdTypUInt64:
        case kEplObdTypReal64:
        {
            *((QWORD*)pBuffer) = AmiGetQword64FromLe(pSrcData_p);
            break;
        }

        // time of day
        case kEplObdTypTimeOfDay:
        case kEplObdTypTimeDiff:
        {
            AmiGetTimeOfDay(pBuffer, ((tTimeOfDay*)pSrcData_p));
            break;
        }

    }// end of switch(pSubEntry->m_Type)


    Ret = writeEntryPost (
                                pObdEntry,
                                pSubEntry,
                                &CbParam,
                                pBuffer,
                                pDstData,
                                ObdSize);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

Exit:

    return Ret;

}

//---------------------------------------------------------------------------
//
// Function:    obd_getAccessType()
//
// Description: Function returns accesstype of the entry
//
// Parameters:
//              index_p       =   Index of the OD entry
//              subIndex_p    =   Subindex of the OD Entry
//              pAccessTyp_p    =   pointer to buffer to store accesstype
//
// Return:      tEplKernel     =   errorcode
//
//
// State:
//
//---------------------------------------------------------------------------
tEplKernel obd_getAccessType(UINT index_p, UINT subIndex_p, tEplObdAccess* pAccessTyp_p)

{
tEplKernel          Ret;
tEplObdEntryPtr     pObdEntry;
tEplObdSubEntryPtr  pObdSubEntry;


    // get pointer to index structure
    Ret = getIndexIntern (&m_ObdInitParam,
                                index_p,
                                &pObdEntry);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get pointer to subindex structure
    Ret = getSubindexIntern (pObdEntry,
                                subIndex_p,
                                &pObdSubEntry);
    if(Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // get accessType
    *pAccessTyp_p = pObdSubEntry->m_Access;


Exit:
    return Ret;
}

//---------------------------------------------------------------------------
//
// Function:    obd_searchVarEntry()
//
// Description: gets variable from OD
//
// Parameters:  index_p       =   index of the var entry to search
//              uiSubindex_p    =   subindex of var entry to search
//              ppVarEntry_p    =   pointer to the pointer to the varentry
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

tEplKernel obd_searchVarEntry (UINT index_p, UINT subindex_p, tEplObdVarEntry MEM** ppVarEntry_p)
{

tEplKernel           Ret;
tEplObdSubEntryPtr   pSubindexEntry;

    // get address of subindex entry
    Ret = getEntry (
        index_p, subindex_p, NULL, &pSubindexEntry);
    if (Ret == kEplSuccessful)
    {
        // get var entry
        Ret = getVarEntry (pSubindexEntry, ppVarEntry_p);
    }

    return Ret;

}
//=========================================================================//
//                                                                         //
//          P R I V A T E   D E F I N I T I O N S                          //
//                                                                         //
//=========================================================================//

//---------------------------------------------------------------------------
//
// Function:    callObjectCallback()
//
// Description: calls callback function of an object or of a variable
//
// Parameters:  pfnCallback_p
//              pCbParam_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel callObjectCallback(tEplObdCallback pfnCallback_p, tEplObdCbParam MEM* pCbParam_p)
{

tEplKernel           Ret;
tEplObdCallback MEM  pfnCallback;

    ASSERT (pCbParam_p != NULL);

    Ret = kEplSuccessful;

    // check address of callback function before calling it
    if (pfnCallback_p != NULL)
    {
        // KEIL C51 V6.01 has a bug.
        // Therefore the parameter fpCallback_p has to be copied in local variable fpCallback.
        pfnCallback = pfnCallback_p;

        // call callback function for this object
        Ret = pfnCallback (pCbParam_p);
    }

    return Ret;
}

//---------------------------------------------------------------------------
//
// Function:    getDataSizeIntern()
//
// Description: gets the data size of an object.
//              for string objects it returnes the string length
//              without terminating null-character
//
// Parameters:  pSubIndexEntry_p
//
// Return:      tEplObdSize
//
// State:
//
//---------------------------------------------------------------------------

static tEplObdSize getDataSizeIntern(tEplObdSubEntryPtr pSubIndexEntry_p)
{

tEplObdSize DataSize;
void MEM*   pData;

    // If OD entry is defined by macro EPL_OBD_SUBINDEX_ROM_VSTRING
    // then the current pointer is always NULL. The function
    // returns the length of default string.
    DataSize = getObjectSize (pSubIndexEntry_p);

    if (pSubIndexEntry_p->m_Type == kEplObdTypVString)
    {
        // The pointer to current value can be received from getObjectCurrentPtr()
        pData = ((void MEM*) getObjectCurrentPtr (pSubIndexEntry_p));
        if (pData != NULL)
        {
            DataSize = getObdStringLen ((void *) pData, DataSize, pSubIndexEntry_p->m_Type);
        }

    }

    return DataSize;

}


//---------------------------------------------------------------------------
//
// Function:    getObdStringLen()
//
// Description: The function calculates the length of string. The '\0'
//              character is NOT included!!
//
// Parameters:  pObjData_p          = pointer to string
//              objLen_p            = max. length of object entry
//              bobjType_p          = object type (VSTRING, ...)
//
// Returns:     string length
//
// State:
//
//---------------------------------------------------------------------------

static tEplObdSize getObdStringLen(void* pObjData_p, tEplObdSize objLen_p, tEplObdType objType_p)
{

tEplObdSize    StrLen = 0;
BYTE *  pbString;

    if (pObjData_p == NULL)
    {
        goto Exit;
    }

    //----------------------------------------
    // Visible String: data format byte
    if (objType_p == kEplObdTypVString)
    {
        pbString = pObjData_p;

        for (StrLen = 0; StrLen < objLen_p; StrLen++)
        {
            if (*pbString == '\0')
            {
//                StrLen++;
                break;
            }

            pbString++;
        }
    }

    //----------------------------------------
    // other string types ...

Exit:
    return (StrLen);

}



#if (EPL_OBD_CHECK_OBJECT_RANGE != FALSE)

//---------------------------------------------------------------------------
//
// Function:    checkObjectRange()
//
// Description: function to check value range of object data
//
// NOTICE: The pointer of data (pData_p) must point out to an even address,
//         if ObjType is unequal to kEplObdTypInt8 or kEplObdTypUInt8! But it is
//         always realiced because pointer m_pDefault points always to an
//         array of the SPECIFIED type.
//
// Parameters:  pSubindexEntry_p
//              pData_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel checkObjectRange (tEplObdSubEntryPtr pSubindexEntry_p, void * pData_p)
{

tEplKernel      Ret;
const void *   pRangeData;

    ASSERTMSG (pSubindexEntry_p != NULL,
        "checkObjectRange(): no address to subindex struct!\n");

    Ret  = kEplSuccessful;

    // check if data range has to be checked
    if ((pSubindexEntry_p->m_Access & kEplObdAccRange) == 0)
    {
        goto Exit;
    }

    // get address of default data
    pRangeData = pSubindexEntry_p->m_pDefault;

    // jump to called object type
    switch ((tEplObdType) pSubindexEntry_p->m_Type)
    {
        // -----------------------------------------------------------------
        // ObdType kEplObdTypBool will not be checked because there are only
        // two possible values 0 or 1.

        // -----------------------------------------------------------------
        // ObdTypes which has to be check up because numerical values
        case kEplObdTypInt8:

            // switch to lower limit
            pRangeData = ((tEplObdInteger8 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdInteger8 *) pData_p) < *((tEplObdInteger8 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdInteger8 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdInteger8 *) pData_p) > *((tEplObdInteger8 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        case kEplObdTypUInt8:

            // switch to lower limit
            pRangeData = ((tEplObdUnsigned8 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdUnsigned8 *) pData_p) < *((tEplObdUnsigned8 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdUnsigned8*) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdUnsigned8 *) pData_p) > *((tEplObdUnsigned8 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        case kEplObdTypInt16:

            // switch to lower limit
            pRangeData = ((tEplObdInteger16 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdInteger16 *) pData_p) < *((tEplObdInteger16 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdInteger16 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdInteger16 *) pData_p) > *((tEplObdInteger16 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        case kEplObdTypUInt16:

            // switch to lower limit
            pRangeData = ((tEplObdUnsigned16 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdUnsigned16 *) pData_p) < *((tEplObdUnsigned16 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdUnsigned16 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdUnsigned16 *) pData_p) > *((tEplObdUnsigned16 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        case kEplObdTypInt32:

            // switch to lower limit
            pRangeData = ((tEplObdInteger32 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdInteger32 *) pData_p) < *((tEplObdInteger32 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdInteger32 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdInteger32 *) pData_p) > *((tEplObdInteger32 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        case kEplObdTypUInt32:

            // switch to lower limit
            pRangeData = ((tEplObdUnsigned32 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdUnsigned32 *) pData_p) < *((tEplObdUnsigned32 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdUnsigned32 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdUnsigned32 *) pData_p) > *((tEplObdUnsigned32 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        case kEplObdTypReal32:

            // switch to lower limit
            pRangeData = ((tEplObdReal32 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdReal32 *) pData_p) < *((tEplObdReal32 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdReal32 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdReal32 *) pData_p) > *((tEplObdReal32 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt40:
        case kEplObdTypInt48:
        case kEplObdTypInt56:
        case kEplObdTypInt64:

            // switch to lower limit
            pRangeData = ((signed QWORD *) pRangeData) + 1;

            // check if value is to low
            if (*((signed QWORD *) pData_p) < *((signed QWORD *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((signed QWORD *) pRangeData) + 1;

            // check if value is to high
            if (*((signed QWORD *) pData_p) > *((signed QWORD *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        // -----------------------------------------------------------------
        case kEplObdTypUInt40:
        case kEplObdTypUInt48:
        case kEplObdTypUInt56:
        case kEplObdTypUInt64:

            // switch to lower limit
            pRangeData = ((unsigned QWORD *) pRangeData) + 1;

            // check if value is to low
            if (*((unsigned QWORD *) pData_p) < *((unsigned QWORD *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((unsigned QWORD *) pRangeData) + 1;

            // check if value is to high
            if (*((unsigned QWORD *) pData_p) > *((unsigned QWORD *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        // -----------------------------------------------------------------
        case kEplObdTypReal64:

            // switch to lower limit
            pRangeData = ((tEplObdReal64 *) pRangeData) + 1;

            // check if value is to low
            if (*((tEplObdReal64 *) pData_p) < *((tEplObdReal64 *) pRangeData))
            {
                Ret = kEplObdValueTooLow;
                break;
            }

            // switch to higher limit
            pRangeData = ((tEplObdReal64 *) pRangeData) + 1;

            // check if value is to high
            if (*((tEplObdReal64 *) pData_p) > *((tEplObdReal64 *) pRangeData))
            {
                Ret = kEplObdValueTooHigh;
            }

            break;

        // -----------------------------------------------------------------
        case kEplObdTypTimeOfDay:
        case kEplObdTypTimeDiff:
            break;

        // -----------------------------------------------------------------
        // ObdTypes kEplObdTypXString and kEplObdTypDomain can not be checkt because
        // they have no numerical value.
        default:

            Ret = kEplObdUnknownObjectType;
            break;
    }

Exit:

    return Ret;

}
#endif // (EPL_OBD_CHECK_OBJECT_RANGE != FALSE)

//---------------------------------------------------------------------------
//
// Function:    writeEntryPre()
//
// Description: Function prepares write of data to an OBD entry. Strings
//              are stored with added '\0' character.
//
// Parameters:
//              index_p       =   Index of the OD entry
//              subIndex_p    =   Subindex of the OD Entry
//              pSrcData_p      =   Pointer to the data to write
//              Size_p          =   Size of the data in Byte
//
// Return:      tEplKernel      =   Errorcode
//
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel writeEntryPre (UINT index_p, UINT subIndex_p, void* pSrcData_p,
                                 void** ppDstData_p, tEplObdSize Size_p, tEplObdEntryPtr* ppObdEntry_p,
                                 tEplObdSubEntryPtr* ppSubEntry_p, tEplObdCbParam MEM* pCbParam_p,
                                 tEplObdSize*  pObdSize_p)
{

tEplKernel              Ret;
tEplObdEntryPtr         pObdEntry;
tEplObdSubEntryPtr      pSubEntry;
tEplObdAccess           Access;
void MEM*               pDstData;
tEplObdSize             ObdSize;
BOOL                    fEntryNumerical;

#if (EPL_OBD_USE_STRING_DOMAIN_IN_RAM != FALSE)
    tEplObdVStringDomain MEM    MemVStringDomain;
    void MEM*                   pCurrData;
#endif

    ASSERT (pSrcData_p != NULL);    // should never be NULL

    //------------------------------------------------------------------------
    // get address of index and subindex entry
    Ret = getEntry (
        index_p, subIndex_p, &pObdEntry, &pSubEntry);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    Access = (tEplObdAccess) pSubEntry->m_Access;

    // check access for write
    if ((Access & kEplObdAccConst) != 0)
    {
        Ret = kEplObdAccessViolation;
        goto Exit;
    }

    //------------------------------------------------------------------------
    // To use the same callback function for ObdWriteEntry as well as for
    // an SDO download call at first (kEplObdEvPre...) the callback function
    // with the argument pointer to object size.
    pCbParam_p->m_uiIndex    = index_p;
    pCbParam_p->m_uiSubIndex = subIndex_p;

    // Because object size and object pointer are
    // adapted by user callback function, re-read
    // this values.
    ObdSize = getObjectSize (pSubEntry);
    // get pointer to object data
    pDstData = (void MEM*) getObjectDataPtrIntern (pSubEntry);

    // 09-dec-2004 r.d.:
    //      Function obd_writeEntry() calls new event kEplObdEvWrStringDomain
    //      for String or Domain which lets called module directly change
    //      the data pointer or size. This prevents a recursive call to
    //      the callback function if it calls getEntry().
    #if (EPL_OBD_USE_STRING_DOMAIN_IN_RAM != FALSE)
    if ( (pSubEntry->m_Type == kEplObdTypVString) ||
         (pSubEntry->m_Type == kEplObdTypDomain)  ||
         (pSubEntry->m_Type == kEplObdTypOString))
    {
        if (pSubEntry->m_Type == kEplObdTypVString)
        {
            // reserve one byte for 0-termination
            // -as ObdSize -= 1;
            Size_p += 1;
        }

        // fill out new arg-struct
        MemVStringDomain.m_DownloadSize = Size_p;
        MemVStringDomain.m_ObjSize      = ObdSize;
        MemVStringDomain.m_pData        = pDstData;

        pCbParam_p->m_ObdEvent = kEplObdEvWrStringDomain;
        pCbParam_p->m_pArg     = &MemVStringDomain;
        //  call user callback
        Ret = callObjectCallback (
                pObdEntry->m_fpCallback, pCbParam_p);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }

        // write back new settings
        pCurrData = pSubEntry->m_pCurrent;
        if ((pSubEntry->m_Type == kEplObdTypVString)
            ||(pSubEntry->m_Type ==  kEplObdTypOString))
        {
            ((tEplObdVString MEM*) pCurrData)->m_Size    = MemVStringDomain.m_ObjSize;
            ((tEplObdVString MEM*) pCurrData)->m_pString = MemVStringDomain.m_pData;
        }
        else // if (pSdosTableEntry_p->m_bObjType == kEplObdTypDomain)
        {
        tEplObdVarEntry MEM*    pVarEntry = NULL;

            Ret = getVarEntry(pSubEntry, &pVarEntry);
            if (Ret != kEplSuccessful)
            {
                goto Exit;
            }
            if (pVarEntry == NULL)
            {
                Ret = kEplObdAccessViolation;
                goto Exit;
            }

            pVarEntry->m_Size  = MemVStringDomain.m_ObjSize;
            pVarEntry->m_pData = (void MEM*) MemVStringDomain.m_pData;
        }

        // Because object size and object pointer are
        // adapted by user callback function, re-read
        // this values.
        ObdSize  = MemVStringDomain.m_ObjSize;
        pDstData = (void MEM*) MemVStringDomain.m_pData;
    }
    #endif //#if (OBD_USE_STRING_DOMAIN_IN_RAM != FALSE)

    // access violation if adress to current value is NULL
    if (pDstData == NULL)
    {
        Ret = kEplObdAccessViolation;
        goto Exit;
    }

    // 07-dec-2004 r.d.: size from application is needed because callback function can change the object size
    // -as 16.11.04 CbParam.m_pArg     = &ObdSize;
    // 09-dec-2004 r.d.: CbParam.m_pArg     = &Size_p;
    pCbParam_p->m_pArg     = &ObdSize;
    pCbParam_p->m_ObdEvent = kEplObdEvInitWrite;
    Ret = callObjectCallback (
        pObdEntry->m_fpCallback, pCbParam_p);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    if (Size_p > ObdSize)
    {
        Ret = kEplObdValueLengthError;
        goto Exit;
    }

    if (pSubEntry->m_Type == kEplObdTypVString)
    {
        if (((char MEM*) pSrcData_p)[Size_p - 1] == '\0')
        {   // last byte of source string contains null character

            // reserve one byte in destination for 0-termination
            Size_p  -= 1;
        }
        else if (Size_p >= ObdSize)
        {   // source string is not 0-terminated
            // and destination buffer is too short
            Ret = kEplObdValueLengthError;
            goto Exit;
        }
    }

    Ret = isNumericalIntern(pSubEntry, &fEntryNumerical);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    if ((fEntryNumerical != FALSE)
        && (Size_p != ObdSize))
    {
        // type is numerical, therefore size has to fit, but it does not.
        Ret = kEplObdValueLengthError;
        goto Exit;
    }

    // use given size, because non-numerical objects can be written with shorter values
    ObdSize = Size_p;

    // set output parameters
    *pObdSize_p = ObdSize;
    *ppObdEntry_p = pObdEntry;
    *ppSubEntry_p = pSubEntry;
    *ppDstData_p = pDstData;

    // all checks are done
    // the caller may now convert the numerical source value to platform byte order in a temporary buffer

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    writeEntryPost()
//
// Description: Function finishes write of data to an OBD entry. Strings
//              are stored with added '\0' character.
//
// Parameters:
//              index_p       =   Index of the OD entry
//              subIndex_p    =   Subindex of the OD Entry
//              pSrcData_p      =   Pointer to the data to write
//              Size_p          =   Size of the data in Byte
//
// Return:      tEplKernel      =   Errorcode
//
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel writeEntryPost (tEplObdEntryPtr pObdEntry_p, tEplObdSubEntryPtr pSubEntry_p,
                                  tEplObdCbParam MEM* pCbParam_p, void* pSrcData_p,
                                  void* pDstData_p, tEplObdSize obdSize_p)
{

tEplKernel              Ret;


    // caller converted the source value to platform byte order
    // now the range of the value may be checked

    #if (EPL_OBD_CHECK_OBJECT_RANGE != FALSE)
    {
        // check data range
        Ret = checkObjectRange (pSubEntry_p, pSrcData_p);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }
    }
    #endif

    // now call user callback function to check value
    // write address of source data to structure of callback parameters
    // so callback function can check this data
    pCbParam_p->m_pArg     = pSrcData_p;
    pCbParam_p->m_ObdEvent = kEplObdEvPreWrite;
    Ret = callObjectCallback (
        pObdEntry_p->m_fpCallback, pCbParam_p);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    // copy object data to OBD
    EPL_MEMCPY (pDstData_p, pSrcData_p, obdSize_p);

    // terminate string with 0
    if (pSubEntry_p->m_Type == kEplObdTypVString)
    {
        ((char MEM*) pDstData_p)[obdSize_p] = '\0';
    }

    // write address of destination to structure of callback parameters
    // so callback function can change data subsequently
    pCbParam_p->m_pArg     = pDstData_p;
    pCbParam_p->m_ObdEvent = kEplObdEvPostWrite;
    Ret = callObjectCallback (
        pObdEntry_p->m_fpCallback, pCbParam_p);

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    getObjectSize()
//
// Description: function to get size of object
//              The function determines if an object type an fixed data type (BYTE, WORD, ...)
//              or non fixed object (string, domain). This information is used to decide
//              if download data are stored temporary or not. For objects with fixed data length
//              and types a value range checking can process.
//              For strings the function returns the whole object size not the
//              length of string.
//
// Parameters:  pSubIndexEntry_p
//
// Return:      tEplObdSize
//
// State:
//
//---------------------------------------------------------------------------

static tEplObdSize getObjectSize(tEplObdSubEntryPtr pSubIndexEntry_p)
{

tEplObdSize DataSize = 0;
void * pData;

    switch (pSubIndexEntry_p->m_Type)
    {
        // -----------------------------------------------------------------
        case kEplObdTypBool:

            DataSize = 1;
            break;

        // -----------------------------------------------------------------
        // ObdTypes which has to be check because numerical values
        case kEplObdTypInt8:
            DataSize = sizeof (tEplObdInteger8);
            break;

        // -----------------------------------------------------------------
        case kEplObdTypUInt8:
            DataSize = sizeof (tEplObdUnsigned8);
            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt16:
            DataSize = sizeof (tEplObdInteger16);
            break;

        // -----------------------------------------------------------------
        case kEplObdTypUInt16:
            DataSize = sizeof (tEplObdUnsigned16);
            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt32:
            DataSize = sizeof (tEplObdInteger32);
            break;

        // -----------------------------------------------------------------
        case kEplObdTypUInt32:
            DataSize = sizeof (tEplObdUnsigned32);
            break;

        // -----------------------------------------------------------------
        case kEplObdTypReal32:
            DataSize = sizeof (tEplObdReal32);
            break;

        // -----------------------------------------------------------------
        // ObdTypes which has to be not checked because not NUM values
        case kEplObdTypDomain:
        {
        tEplObdVarEntry MEM*    pVarEntry = NULL;
        tEplKernel              Ret;

            Ret = getVarEntry(pSubIndexEntry_p, &pVarEntry);
            if ((Ret == kEplSuccessful)
                && (pVarEntry != NULL))
            {
                DataSize = pVarEntry->m_Size;
            }
            break;
        }

        // -----------------------------------------------------------------
        case kEplObdTypVString:
        //case kEplObdTypUString:

            // If OD entry is defined by macro EPL_OBD_SUBINDEX_ROM_VSTRING
            // then the current pointer is always NULL. The function
            // returns the length of default string.
            pData = (void *) pSubIndexEntry_p->m_pCurrent;
            if ((void MEM*) pData != (void MEM*) NULL)
            {
                // The max. size of strings defined by STRING-Macro is stored in
                // tEplObdVString of current value.
                // (types tEplObdVString, tEplObdOString and tEplObdUString has the same members)
                DataSize = ((tEplObdVString MEM*) pData)->m_Size;
            }
            else
            {
                // The current position is not declared. The string
                // is located in ROM, therefore use default pointer.
                pData = (void *) pSubIndexEntry_p->m_pDefault;
                if ((CONST void ROM*) pData != (CONST void ROM*) NULL)
                {
                   // The max. size of strings defined by STRING-Macro is stored in
                   // tEplObdVString of default value.
                   DataSize = ((CONST tEplObdVString ROM*) pData)->m_Size;
                }
            }

            break;

        // -----------------------------------------------------------------
        case kEplObdTypOString:

            pData = (void *) pSubIndexEntry_p->m_pCurrent;
            if ((void MEM*) pData != (void MEM*) NULL)
            {
                // The max. size of strings defined by STRING-Macro is stored in
                // tEplObdVString of current value.
                // (types tEplObdVString, tEplObdOString and tEplObdUString has the same members)
                DataSize = ((tEplObdOString MEM*) pData)->m_Size;
            }
            else
            {
                // The current position is not declared. The string
                // is located in ROM, therefore use default pointer.
                pData = (void *) pSubIndexEntry_p->m_pDefault;
                if ((CONST void ROM*) pData != (CONST void ROM*) NULL)
                {
                   // The max. size of strings defined by STRING-Macro is stored in
                   // tEplObdVString of default value.
                   DataSize = ((CONST tEplObdOString ROM*) pData)->m_Size;
                }
            }
            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt24:
        case kEplObdTypUInt24:

            DataSize = 3;
            break;


        // -----------------------------------------------------------------
        case kEplObdTypInt40:
        case kEplObdTypUInt40:

            DataSize = 5;
            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt48:
        case kEplObdTypUInt48:

            DataSize = 6;
            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt56:
        case kEplObdTypUInt56:

            DataSize = 7;
            break;

        // -----------------------------------------------------------------
        case kEplObdTypInt64:
        case kEplObdTypUInt64:
        case kEplObdTypReal64:

            DataSize = 8;
            break;

        // -----------------------------------------------------------------
        case kEplObdTypTimeOfDay:
        case kEplObdTypTimeDiff:

            DataSize = 6;
            break;

        // -----------------------------------------------------------------
        default:
            break;
    }

    return DataSize;
}

//---------------------------------------------------------------------------
//
// Function:    getObjectDefaultPtr()
//
// Description: function to get the default pointer (type specific)
//
// Parameters:  pSubIndexEntry_p    = pointer to subindex structure
//
// Returns:     (CONST void *)      = pointer to default value
//
// State:
//
//---------------------------------------------------------------------------

static CONST void* getObjectDefaultPtr(tEplObdSubEntryPtr pSubIndexEntry_p)
{

CONST void*     pDefault;
tEplObdType     Type;

    ASSERTMSG (pSubIndexEntry_p != NULL, "getObjectDefaultPtr(): pointer to SubEntry not valid!\n");

    // get address to default data from default pointer
    pDefault = pSubIndexEntry_p->m_pDefault;
    if (pDefault != NULL)
    {
        // there are some special types, whose default pointer always is NULL or has to get from other structure
        // get type from subindex structure
        Type = pSubIndexEntry_p->m_Type;

        // check if object type is a string value
        if ((Type == kEplObdTypVString) /* ||
            (Type == kEplObdTypUString) */ )
        {

            // EPL_OBD_SUBINDEX_RAM_VSTRING
            //    tEplObdSize         m_Size;       --> size of default string
            //    char *    m_pDefString; --> pointer to  default string
            //    char *    m_pString;    --> pointer to string in RAM
            //
            pDefault = ((tEplObdVStringDef *) pDefault)->m_pDefString;
        }
        else if(Type == kEplObdTypOString)
        {
             pDefault = ((tEplObdOStringDef *) pDefault)->m_pDefString;
        }
    }

    return pDefault;

}


//---------------------------------------------------------------------------
//
// Function:    getVarEntry()
//
// Description: gets a variable entry of an object
//
// Parameters:  pSubindexEntry_p
//              ppVarEntry_p
//
// Return:      tCopKernel
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel getVarEntry (tEplObdSubEntryPtr pSubindexEntry_p, tEplObdVarEntry MEM** ppVarEntry_p)
{

tEplKernel Ret = kEplObdVarEntryNotExist;

    ASSERT (ppVarEntry_p != NULL);   // is not allowed to be NULL
    ASSERT (pSubindexEntry_p != NULL);

    // check VAR-Flag - only this object points to variables
    if ((pSubindexEntry_p->m_Access & kEplObdAccVar) != 0)
    {
        // check if object is an array
        if ((pSubindexEntry_p->m_Access & kEplObdAccArray) != 0)
        {
            *ppVarEntry_p = &((tEplObdVarEntry MEM*) pSubindexEntry_p->m_pCurrent)[pSubindexEntry_p->m_uiSubIndex - 1];
        }
        else
        {
            *ppVarEntry_p = (tEplObdVarEntry MEM*) pSubindexEntry_p->m_pCurrent;
        }

        Ret = kEplSuccessful;
    }

    return Ret;

}

//---------------------------------------------------------------------------
//
// Function:    getEntry()
//
// Description: gets a index entry from OD
//
// Parameters:  index_p       =   Index number
//              uiSubindex_p    =   Subindex number
//              ppObdEntry_p    =   pointer to the pointer to the entry
//              ppObdSubEntry_p =   pointer to the pointer to the subentry
//
// Return:      tEplKernel

//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel getEntry(UINT index_p, UINT uiSubindex_p, tEplObdEntryPtr* ppObdEntry_p,
                           tEplObdSubEntryPtr* ppObdSubEntry_p)
{

tEplObdEntryPtr         pObdEntry;
tEplObdCbParam MEM      CbParam;
tEplKernel              Ret;

    //------------------------------------------------------------------------
    // get address of entry of index
    Ret = getIndexIntern (&m_ObdInitParam, index_p, &pObdEntry);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    //------------------------------------------------------------------------
    // get address of entry of subindex
    Ret = getSubindexIntern (pObdEntry, uiSubindex_p, ppObdSubEntry_p);
    if (Ret != kEplSuccessful)
    {
        goto Exit;
    }

    //------------------------------------------------------------------------
    // call callback function to inform user/stack that an object will be searched
    // if the called module returnes an error then we abort the searching with kEplObdIndexNotExist
    CbParam.m_uiIndex    = index_p;
    CbParam.m_uiSubIndex = uiSubindex_p;
    CbParam.m_pArg       = NULL;
    CbParam.m_ObdEvent   = kEplObdEvCheckExist;
    Ret = callObjectCallback (
        pObdEntry->m_fpCallback, &CbParam);
    if (Ret != kEplSuccessful)
    {
        Ret = kEplObdIndexNotExist;
        goto Exit;
    }

    //------------------------------------------------------------------------
    // it is allowed to set ppObdEntry_p to NULL
    // if so, no address will be written to calling function
    if (ppObdEntry_p != NULL)
    {
        *ppObdEntry_p = pObdEntry;
    }

Exit:

    return Ret;

}
//---------------------------------------------------------------------------
//
// Function:    getObjectCurrentPtr()
//
// Description: function to get Current pointer (type specific)
//
// Parameters:  pSubIndexEntry_p
//
// Return:      void MEM*
//
// State:
//
//---------------------------------------------------------------------------

static void MEM* getObjectCurrentPtr(tEplObdSubEntryPtr pSubIndexEntry_p)
{

void MEM*       pData;
unsigned int    uiArrayIndex;
tEplObdSize     Size;

    pData = pSubIndexEntry_p->m_pCurrent;

    // check if constant object
    if (pData != NULL)
    {
        // check if object is an array
        if ((pSubIndexEntry_p->m_Access & kEplObdAccArray) != 0)
        {
            // calculate correct data pointer
            uiArrayIndex = pSubIndexEntry_p->m_uiSubIndex - 1;
            if ((pSubIndexEntry_p->m_Access & kEplObdAccVar) != 0)
            {
                Size = sizeof (tEplObdVarEntry);
            }
            else
            {
                Size = getObjectSize (pSubIndexEntry_p);
            }
            pData = ((BYTE MEM*) pData) + (Size * uiArrayIndex);
        }

        // check if VarEntry
        if ((pSubIndexEntry_p->m_Access & kEplObdAccVar) != 0)
        {
            // The data pointer is stored in VarEntry->pData
            pData = ((tEplObdVarEntry MEM*) pData)->m_pData;
        }

        // the default pointer is stored for strings in tEplObdVString
        else if ((pSubIndexEntry_p->m_Type == kEplObdTypVString) /* ||
            (pSubIndexEntry_p->m_Type == kEplObdTypUString)    */ )
        {
            pData = (void MEM*) ((tEplObdVString MEM*) pData)->m_pString;
        }
        else if (pSubIndexEntry_p->m_Type == kEplObdTypOString)
        {
            pData = (void MEM*) ((tEplObdOString MEM*) pData)->m_pString;
        }
    }

    return pData;

}


//---------------------------------------------------------------------------
//
// Function:    getIndexIntern()
//
// Description: gets a index entry from OD
//
// Parameters:  pInitParam_p
//              index_p
//              ppObdEntry_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel getIndexIntern (tEplObdInitParam MEM* pInitParam_p, UINT index_p, tEplObdEntryPtr* ppObdEntry_p)
{

tEplObdEntryPtr pObdEntry;
tEplKernel      Ret;
unsigned int    uiIndex;

#if (defined (EPL_OBD_USER_OD) && (EPL_OBD_USER_OD != FALSE))

unsigned int  nLoop;

    // if user OD is used then objekts also has to be searched in user OD
    // there is less code need if we do this in a loop
    nLoop = 2;

#endif

    ASSERTMSG (ppObdEntry_p != NULL, "getIndexIntern(): pointer to index entry is NULL!\n");

    Ret = kEplObdIndexNotExist;

    // get start address of OD part
    // start address depends on object index because
    // object dictionary is divided in 3 parts
    if ((index_p >= 0x1000) && (index_p < 0x2000))
    {
        pObdEntry = pInitParam_p->m_pGenericPart;
    }
    else if ((index_p >= 0x2000) && (index_p < 0x6000))
    {
        pObdEntry = pInitParam_p->m_pManufacturerPart;
    }

    // index range 0xA000 to 0xFFFF is reserved for DSP-405
    // DS-301 defines that range 0x6000 to 0x9FFF (!!!) is stored if "store" was written to 0x1010/3.
    // Therefore default configuration is OBD_INCLUDE_A000_TO_DEVICE_PART = FALSE.
    // But a CANopen Application which does not implement dynamic OD or user-OD but wants to use static objets 0xA000...
    // should set OBD_INCLUDE_A000_TO_DEVICE_PART to TRUE.

#if (EPL_OBD_INCLUDE_A000_TO_DEVICE_PART == FALSE)
    else if ((index_p >= 0x6000) && (index_p < 0x9FFF))
#else
    else if ((index_p >= 0x6000) && (index_p < 0xFFFF))
#endif
    {
        pObdEntry = pInitParam_p->m_pDevicePart;
    }


#if (defined (EPL_OBD_USER_OD) && (EPL_OBD_USER_OD != FALSE))

    // if index does not match in static OD then index only has to be searched in user OD
    else
    {
        // begin from first entry of user OD part
        pObdEntry = pInitParam_p->m_pUserPart;

        // no user OD is available
        if (pObdEntry == NULL)
        {
            goto Exit;
        }

        // loop must only run once
        nLoop = 1;
    }

    do
    {

#else

        // no user OD is available
        // so other object can be found in OD
        else
        {
            Ret = kEplObdIllegalPart;
            goto Exit;
        }

#endif

        // note:
        // The end of Index table is marked with m_uiIndex = 0xFFFF.
        // If this function will be called with wIndex_p = 0xFFFF, entry
        // should not be found. Therefor it is important to use
        // while{} instead of do{}while !!!

        // get first index of index table
        uiIndex = pObdEntry->m_uiIndex;

        // search Index in OD part
        while (uiIndex != EPL_OBD_TABLE_INDEX_END)
        {
            // go to the end of this function if index is found
            if (index_p == uiIndex)
            {
                // write address of OD entry to calling function
                *ppObdEntry_p = pObdEntry;
                Ret = kEplSuccessful;
                goto Exit;
            }

            // objects are sorted in OD
            // if the current index in OD is greater than the index which is to search then break loop
            // in this case user OD has to be search too
            if (index_p < uiIndex)
            {
                break;
            }

            // next entry in index table
            pObdEntry++;

            // get next index of index table
            uiIndex = pObdEntry->m_uiIndex;
        }

#if (defined (EPL_OBD_USER_OD) && (EPL_OBD_USER_OD != FALSE))

        // begin from first entry of user OD part
        pObdEntry = pInitParam_p->m_pUserPart;

        // no user OD is available
        if (pObdEntry == NULL)
        {
            goto Exit;
        }

        // switch next loop for user OD
        nLoop--;

    } while (nLoop > 0);

#endif

    // in this line Index was not found

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    getSubindexIntern()
//
// Description: gets a subindex entry from a index entry
//
// Parameters:  pObdEntry_p
//              bSubIndex_p
//              ppObdSubEntry_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel getSubindexIntern (tEplObdEntryPtr pObdEntry_p, UINT subIndex_p, tEplObdSubEntryPtr* ppObdSubEntry_p)
{

tEplObdSubEntryPtr pSubEntry;
unsigned int       nSubIndexCount;
tEplKernel         Ret;

    ASSERTMSG (pObdEntry_p != NULL, "getSubindexIntern(): pointer to index is NULL!\n");
    ASSERTMSG (ppObdSubEntry_p != NULL, "getSubindexIntern(): pointer to subindex is NULL!\n");

    Ret = kEplObdSubindexNotExist;

    // get start address of subindex table and count of subindices
    pSubEntry     = pObdEntry_p->m_pSubIndex;
    nSubIndexCount =  pObdEntry_p->m_uiCount;
    ASSERTMSG ((pSubEntry != NULL) && (nSubIndexCount > 0),
        "ObdGetSubindexIntern(): invalid subindex table within index table!\n");   // should never be NULL

    // search subindex in subindex table
    while (nSubIndexCount > 0)
    {
        // check if array is found
        if ((pSubEntry->m_Access & kEplObdAccArray) != 0)
        {
            // check if subindex is in range
            if (subIndex_p < pObdEntry_p->m_uiCount)
            {
                // update subindex number (subindex entry of an array is always in RAM !!!)
                pSubEntry->m_uiSubIndex = subIndex_p;
                *ppObdSubEntry_p = pSubEntry;
                Ret = kEplSuccessful;
                goto Exit;
            }
        }

        // go to the end of this function if subindex is found
        else if (subIndex_p == pSubEntry->m_uiSubIndex)
        {
            *ppObdSubEntry_p = pSubEntry;
            Ret = kEplSuccessful;
            goto Exit;
        }

        // objects are sorted in OD
        // if the current subindex in OD is greater than the subindex which is to search then break loop
        // in this case user OD has to be search too
        if (subIndex_p < pSubEntry->m_uiSubIndex)
        {
            break;
        }

        pSubEntry++;
        nSubIndexCount--;
    }

    // in this line SubIndex was not fount

Exit:

    return Ret;

}


//---------------------------------------------------------------------------
//
// Function:    obd_storeLoadObjCallback()
//
// Description: function set address to callbackfunction for command Store and Load
//
// Parameters:  fpCallback_p
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------
#if (EPL_OBD_USE_STORE_RESTORE != FALSE)
tEplKernel obd_storeLoadObjCallback (tEplObdStoreLoadObjCallback fpCallback_p)
{

    // set new address of callback function
    m_fpStoreLoadObjCallback = fpCallback_p;

    return kEplSuccessful;

}
#endif // (EPL_OBD_USE_STORE_RESTORE != FALSE)


//---------------------------------------------------------------------------
//
// Function:    accessOdPartIntern()
//
// Description: runs through OD and executes a job
//
// Parameters:  currentOdPart_p
//              pObdEnty_p
//              direction_p     = what is to do (load values from flash or EEPROM, store, ...)
//
// Return:      tEplKernel
//
// State:
//
//---------------------------------------------------------------------------

static tEplKernel accessOdPartIntern (tEplObdPart currentOdPart_p, tEplObdEntryPtr pObdEnty_p,
                                      tEplObdDir direction_p)
{

tEplObdSubEntryPtr          pSubIndex;
unsigned int                nSubIndexCount;
tEplObdAccess               Access;
void MEM*                   pDstData;
CONST void*                 pDefault;
tEplObdSize                 ObjSize;
tEplKernel                  Ret;
tEplObdVarEntry MEM*        pVarEntry = NULL;

#if (EPL_OBD_USE_STORE_RESTORE != FALSE)
tEplObdCbStoreParam MEM     CbStore;
#else
UNUSED_PARAMETER(currentOdPart_p);
#endif

    ASSERT (pObdEnty_p != NULL);

    Ret = kEplSuccessful;
#if (EPL_OBD_USE_STORE_RESTORE != FALSE)
    // prepare structure for STORE RESTORE callback function
    CbStore.m_bCurrentOdPart = (BYTE) currentOdPart_p;
    CbStore.m_pData          = NULL;
    CbStore.m_ObjSize        = 0;
#endif

    // command of first action depends on direction to access
    #if (EPL_OBD_USE_STORE_RESTORE != FALSE)
    if (direction_p == kEplObdDirLoad)
    {
        CbStore.m_bCommand = (BYTE) kEplObdCommOpenRead;

        // call callback function for previous command
        Ret = callStoreCallback (
            &CbStore);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }

        // set command for index and subindex loop
        CbStore.m_bCommand = (BYTE) kEplObdCommReadObj;
    }
    else if (direction_p == kEplObdDirStore)
    {
        CbStore.m_bCommand = (BYTE) kEplObdCommOpenWrite;

        // call callback function for previous command
        Ret = callStoreCallback (&CbStore);
        if (Ret != kEplSuccessful)
        {
            goto Exit;
        }

        // set command for index and subindex loop
        CbStore.m_bCommand = (BYTE) kEplObdCommWriteObj;
    }
    #endif // (EPL_OBD_USE_STORE_RESTORE != FALSE)

    // we should not restore the OD values here
    // the next NMT command "Reset Node" or "Reset Communication" resets the OD data
    if (direction_p != kEplObdDirRestore)
    {
        // walk through OD part till end is found
        while (pObdEnty_p->m_uiIndex != EPL_OBD_TABLE_INDEX_END)
        {
            // get address to subindex table and count of subindices
            pSubIndex     = pObdEnty_p->m_pSubIndex;
            nSubIndexCount = pObdEnty_p->m_uiCount;
            ASSERT ((pSubIndex != NULL) && (nSubIndexCount > 0));    // should never be NULL

            // walk through subindex table till all subinices were restored
            while (nSubIndexCount != 0)
            {
                Access = (tEplObdAccess) pSubIndex->m_Access;

                // get pointer to current and default data
                pDefault = getObjectDefaultPtr (pSubIndex);
                pDstData = getObjectCurrentPtr (pSubIndex);

                // NOTE (for kEplObdTypVString):
                //      The function returnes the max. number of bytes for a
                //      current string.
                //      r.d.: For stings the default-size will be read in other lines following (kEplObdDirInit).
                ObjSize  = getObjectSize (pSubIndex);

                // switch direction of OD access
                switch (direction_p)
                {
                    // --------------------------------------------------------------------------
                    // VarEntry structures has to be initialized
                    case kEplObdDirInit:

                        // If VAR-Flag is set, m_pCurrent means not address of data
                        // but address of tEplObdVarEntry. Address of data has to be get from
                        // this structure.
                        if ((Access & kEplObdAccVar) != 0)
                        {
                            getVarEntry (pSubIndex, &pVarEntry);
                            obd_initVarEntry (pVarEntry, pSubIndex->m_Type, ObjSize);
/*
                            if ((Access & kEplObdAccArray) == 0)
                            {
                                obd_initVarEntry (pSubIndex->m_pCurrent, pSubIndex->m_Type, ObjSize);
                            }
                            else
                            {
                                obd_initVarEntry ((tEplObdVarEntry MEM*) (((BYTE MEM*) pSubIndex->m_pCurrent) + (sizeof (tEplObdVarEntry) * pSubIndex->m_uiSubIndex)),
                                    pSubIndex->m_Type, ObjSize);
                            }
*/
                            // at this time no application variable is defined !!!
                            // therefore data can not be copied.
                            break;
                        }
                        else if (pSubIndex->m_Type == kEplObdTypVString)
                        {
                            // If pointer m_pCurrent is not equal to NULL then the
                            // string was defined with EPL_OBD_SUBINDEX_RAM_VSTRING. The current
                            // pointer points to struct tEplObdVString located in MEM.
                            // The element size includes the max. number of
                            // bytes. The element m_pString includes the pointer
                            // to string in MEM. The memory location of default string
                            // must be copied to memory location of current string.

                            pDstData = pSubIndex->m_pCurrent;
                            if (pDstData != NULL)
                            {
                                // 08-dec-2004: code optimization !!!
                                //              entries ((tEplObdVStringDef ROM*) pSubIndex->m_pDefault)->m_pString
                                //              and ((tEplObdVStringDef ROM*) pSubIndex->m_pDefault)->m_Size were read
                                //              twice. thats not necessary!

                                // For copying data we have to set the destination pointer to the real RAM string. This
                                // pointer to RAM string is located in default string info structure. (translated r.d.)
                                pDstData = (void MEM*) ((tEplObdVStringDef ROM*) pSubIndex->m_pDefault)->m_pString;
                                ObjSize  = ((tEplObdVStringDef ROM*) pSubIndex->m_pDefault)->m_Size;


                                ((tEplObdVString MEM*) pSubIndex->m_pCurrent)->m_pString = pDstData;
                                ((tEplObdVString MEM*) pSubIndex->m_pCurrent)->m_Size    = ObjSize;
                            }

                        }
                        else if(pSubIndex->m_Type == kEplObdTypOString)
                        {
                            pDstData = pSubIndex->m_pCurrent;
                            if (pDstData != NULL)
                            {
                                // 08-dec-2004: code optimization !!!
                                //              entries ((tEplObdOStringDef ROM*) pSubIndex->m_pDefault)->m_pString
                                //              and ((tEplObdOStringDef ROM*) pSubIndex->m_pDefault)->m_Size were read
                                //              twice. thats not necessary!

                                // For copying data we have to set the destination pointer to the real RAM string. This
                                // pointer to RAM string is located in default string info structure. (translated r.d.)
                                pDstData = (void MEM*) ((tEplObdOStringDef ROM*) pSubIndex->m_pDefault)->m_pString;
                                ObjSize  = ((tEplObdOStringDef ROM*) pSubIndex->m_pDefault)->m_Size;


                                ((tEplObdOString MEM*) pSubIndex->m_pCurrent)->m_pString = pDstData;
                                ((tEplObdOString MEM*) pSubIndex->m_pCurrent)->m_Size    = ObjSize;
                            }

                        }


                        // no break !! because copy of data has to done too.

                    // --------------------------------------------------------------------------
                    // all objects has to be restored with default values
                    case kEplObdDirRestore:

                        // 09-dec-2004 r.d.: optimization! the same code for kEplObdDirRestore and kEplObdDirLoad
                        //                   is replaced to function ObdCopyObjectData() with a new parameter.
                        // restore object data for init phase

                        copyObjectData (pDstData, pDefault, ObjSize, pSubIndex->m_Type);

                        // execute post default event
                        callPostDefault (pDstData, pObdEnty_p, pSubIndex);

                        break;

                    // --------------------------------------------------------------------------
                    // objects with attribute kEplObdAccStore has to be load from EEPROM or from a file
                    case kEplObdDirLoad:

                        // restore object data for init phase
                        copyObjectData (pDstData, pDefault, ObjSize, pSubIndex->m_Type);

                        // execute post default event
                        callPostDefault (pDstData, pObdEnty_p, pSubIndex);

                        // no break !! because callback function has to be called too.

                    // --------------------------------------------------------------------------
                    // objects with attribute kEplObdAccStore has to be stored in EEPROM or in a file
                    case kEplObdDirStore:

                        // when attribute kEplObdAccStore is set, then call callback function
                        #if (EPL_OBD_USE_STORE_RESTORE != FALSE)
                        if ((Access & kEplObdAccStore) != 0)
                        {
                            // fill out data pointer and size of data
                            CbStore.m_pData    = pDstData;
                            CbStore.m_ObjSize  = ObjSize;

                            // call callback function for read or write object
                            Ret = ObdCallStoreCallback (&CbStore);
                            if (Ret != kEplSuccessful)
                            {
                                goto Exit;
                            }
                        }
                        #endif // (EPL_OBD_USE_STORE_RESTORE != FALSE)
                        break;


                    // --------------------------------------------------------------------------
                    // if OD Builder key has to be checked no access to subindex and data should be made
                    case kEplObdDirOBKCheck:

                        // no break !! because we want to break the second loop too.


                    // --------------------------------------------------------------------------
                    // unknown Direction
                    default:

                        // so we can break the second loop earler
                        nSubIndexCount = 1;
                        break;
                }

                nSubIndexCount--;

                // next subindex entry
                if ((Access & kEplObdAccArray) == 0)
                {
                    pSubIndex++;
                    if ((nSubIndexCount > 0)
                        && ((pSubIndex->m_Access & kEplObdAccArray) != 0))
                    {
                        // next subindex points to an array
                        // reset subindex number
                        pSubIndex->m_uiSubIndex = 1;
                    }
                }
                else
                {
                    if (nSubIndexCount > 0)
                    {
                        // next subindex points to an array
                        // increment subindex number
                        pSubIndex->m_uiSubIndex++;
                    }
                }
            }

            // next index entry
            pObdEnty_p++;
        }
    }

    // -----------------------------------------------------------------------------------------
    // command of last action depends on direction to access
    if (direction_p == kEplObdDirOBKCheck)
    {

        goto Exit;
    }
    #if (EPL_OBD_USE_STORE_RESTORE != FALSE)
    else
    {
        if (direction_p == kEplObdDirLoad)
        {
            CbStore.m_bCommand = (BYTE) kEplObdCommCloseRead;
        }
        else if (direction_p == kEplObdDirStore)
        {
            CbStore.m_bCommand = (BYTE) kEplObdCommCloseWrite;
        }
        else if (direction_p == kEplObdDirRestore)
        {
            CbStore.m_bCommand = (BYTE) kEplObdCommClear;
        }
        else
        {
            goto Exit;
        }

        // call callback function for last command
        Ret = callStoreCallback (&CbStore);
    }
    #endif // (EPL_OBD_USE_STORE_RESTORE != FALSE)

//    goto Exit;

Exit:

    return Ret;

}


// ----------------------------------------------------------------------------
// Function:    copyObjectData()
//
// Description: checks pointers to object data and copy them from source to destination
//
// Parameters:  pDstData_p              = destination pointer
//              pSrcData_p              = source pointer
//              objSize_p               = size of object
//              objType_p               =
//
// Returns:     tEplKernel              = error code
// ----------------------------------------------------------------------------

static void copyObjectData (void MEM* pDstData_p, CONST void* pSrcData_p,
                            tEplObdSize objSize_p, tEplObdType objType_p)
{


tEplObdSize StrSize = 0;


    // it is allowed to set default and current address to NULL (nothing to copy)
    if (pDstData_p != NULL)
    {

        if (objType_p == kEplObdTypVString)
        {
            // The function calculates the really number of characters of string. The
            // object entry size can be bigger as string size of default string.
            // The '\0'-termination is NOT included. A string with no characters has a
            // size of 0.
            StrSize = getObdStringLen ((void *) pSrcData_p, objSize_p, kEplObdTypVString);

            // If the string length is greater than or equal to the entry size in OD then only copy
            // entry size - 1 and always set the '\0'-termination.
            if (StrSize >= objSize_p)
            {
                StrSize = objSize_p - 1;
            }
        }

        if (pSrcData_p != NULL)
        {
            // copy data
            EPL_MEMCPY (pDstData_p, pSrcData_p, objSize_p);

            if (objType_p == kEplObdTypVString)
            {
                ((char MEM*) pDstData_p)[StrSize] = '\0';
            }
        }
    }

}

// ----------------------------------------------------------------------------
// Function:    callPostDefault()
//
// Description: calls the callback function with post callback event
//
// Parameters:  pDstData_p              = data pointer
//              pObdEntry_p             = pointer to obd entry
//              pSubIndex_p             = pointer to obd subentry
//
// Returns:     tEplKernel              = error code
// ----------------------------------------------------------------------------
static tEplKernel callPostDefault (void *pData_p, tEplObdEntryPtr pObdEntry_p,
                                   tEplObdSubEntryPtr pSubIndex_p)
{
    tEplKernel          ret;
    tEplObdCbParam      cbParam;

    cbParam.m_uiIndex    = pObdEntry_p->m_uiIndex;
    cbParam.m_uiSubIndex = pSubIndex_p->m_uiSubIndex;
    cbParam.m_pArg     = pData_p;
    cbParam.m_ObdEvent = kEplObdEvPostDefault;
    ret = callObjectCallback ( pObdEntry_p->m_fpCallback,
                                    &cbParam);

    return ret;
}

//---------------------------------------------------------------------------
//
// Function:    isNumericalIntern()
//
// Description: function checks if a entry is numerical or not
//
//
// Parameters:
//              index_p           = Index
//              subIndex_p        = Subindex
//              pfEntryNumerical_p  = pointer to BOOL for returnvalue
//                                  -> TRUE if entry a numerical value
//                                  -> FALSE if entry not a numerical value
//
// Return:      tEplKernel = Errorcode
//
// State:
//
//---------------------------------------------------------------------------
static tEplKernel isNumericalIntern(tEplObdSubEntryPtr pObdSubEntry_p,
                                    BOOL* pfEntryNumerical_p)
{
tEplKernel          Ret = kEplSuccessful;


    // get Type
    if((pObdSubEntry_p->m_Type == kEplObdTypVString)
        || (pObdSubEntry_p->m_Type == kEplObdTypOString)
        || (pObdSubEntry_p->m_Type == kEplObdTypDomain))
    {   // not numerical types
        *pfEntryNumerical_p = FALSE;
    }
    else
    {   // numerical types
        *pfEntryNumerical_p = TRUE;
    }

    return Ret;

}


// -------------------------------------------------------------------------
// function to classify object type (fixed/non fixed)
// -------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Function:    callStoreCallback()
//
// Description: checks address to callback function and calles it when unequal
//              to NULL
//
// Parameters:
//              pCbStoreParam_p        = address to callback parameters
//
// Returns:     tEplKernel             = error code
// ----------------------------------------------------------------------------
#if (EPL_OBD_USE_STORE_RESTORE != FALSE)
static tEplKernel callStoreCallback(tEplObdCbStoreParam MEM* pCbStoreParam_p)
{

tEplKernel Ret = kEplSuccessful;

    ASSERT (pCbStoreParam_p != NULL);

    // check if function pointer is NULL - if so, no callback should be called
    if (m_fpStoreLoadObjCallback != NULL)
    {
        Ret = m_fpStoreLoadObjCallback(pCbStoreParam_p);
    }

    return Ret;

}
#endif // (EPL_OBD_USE_STORE_RESTORE != FALSE)
//---------------------------------------------------------------------------
//
// Function:    getObjectDataPtrIntern()
//
// Description: Function gets the data pointer of an object.
//              It returnes the current data pointer. But if object is an
//              constant object it returnes the default pointer.
//
// Parameters:  pSubindexEntry_p = pointer to subindex entry
//
// Return:      void *    = pointer to object data
//
// State:
//
//---------------------------------------------------------------------------

static void* getObjectDataPtrIntern(tEplObdSubEntryPtr pSubindexEntry_p)
{

void * pData;
tEplObdAccess Access;

    ASSERTMSG (pSubindexEntry_p != NULL, "getObjectDataPtrIntern(): pointer to SubEntry not valid!\n");

    // there are are some objects whose data pointer has to get from other structure
    // get access type for this object
    Access = pSubindexEntry_p->m_Access;

    // If object has access type = const,
    // only the default value exists.
    if ((Access & kEplObdAccConst) != 0)
    {
        // The pointer to default value can be received from ObdGetObjectDefaultPtr()
        pData = ((void *) getObjectDefaultPtr (pSubindexEntry_p));
    }
    else
    {
        // The pointer to current value can be received from ObdGetObjectCurrentPtr()
        pData = getObjectCurrentPtr (pSubindexEntry_p);
    }

    return pData;

}

// EOF

