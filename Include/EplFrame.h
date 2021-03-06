/****************************************************************************

  (c) SYSTEC electronic GmbH, D-08468 Heinsdorfergrund, Am Windrad 2
      www.systec-electronic.com

  Project:      openPOWERLINK

  Description:  include file for EPL frames

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
                    GCC V3.4

  -------------------------------------------------------------------------

  Revision History:

  2006/05/22 d.k.:   start of the implementation, version 1.00


****************************************************************************/

#ifndef _EPL_FRAME_H_
#define _EPL_FRAME_H_


//---------------------------------------------------------------------------
// const defines
//---------------------------------------------------------------------------


// defines for EplFrame.m_wFlag
#define EPL_FRAME_FLAG1_RD          0x01    // ready                                    (PReq, PRes)
#define EPL_FRAME_FLAG1_ER          0x02    // exception reset (error signalling)       (SoA)
#define EPL_FRAME_FLAG1_EA          0x04    // exception acknowledge (error signalling) (PReq, SoA)
#define EPL_FRAME_FLAG1_EC          0x08    // exception clear (error signalling)       (StatusRes)
#define EPL_FRAME_FLAG1_EN          0x10    // exception new (error signalling)         (PRes, StatusRes)
#define EPL_FRAME_FLAG1_MS          0x20    // multiplexed slot                         (PReq)
#define EPL_FRAME_FLAG1_PS          0x40    // prescaled slot                           (SoC)
#define EPL_FRAME_FLAG1_MC          0x80    // multiplexed cycle completed              (SoC)
#define EPL_FRAME_FLAG2_RS          0x07    // number of pending requests to send       (PRes, StatusRes, IdentRes)
#define EPL_FRAME_FLAG2_PR          0x38    // priority of requested asynch. frame      (PRes, StatusRes, IdentRes)
#define EPL_FRAME_FLAG2_PR_SHIFT    3       // shift of priority of requested asynch. frame

// error history/status entry types
#define EPL_ERR_ENTRYTYPE_STATUS        0x8000
#define EPL_ERR_ENTRYTYPE_HISTORY       0x0000
#define EPL_ERR_ENTRYTYPE_EMCY          0x4000
#define EPL_ERR_ENTRYTYPE_MODE_ACTIVE   0x1000
#define EPL_ERR_ENTRYTYPE_MODE_CLEARED  0x2000
#define EPL_ERR_ENTRYTYPE_MODE_OCCURRED 0x3000
#define EPL_ERR_ENTRYTYPE_MODE_MASK     0x3000
#define EPL_ERR_ENTRYTYPE_PROF_VENDOR   0x0001
#define EPL_ERR_ENTRYTYPE_PROF_EPL      0x0002
#define EPL_ERR_ENTRYTYPE_PROF_MASK     0x0FFF

// defines for EPL version / PDO version
#define EPL_VERSION_SUB             0x0F  // sub version
#define EPL_VERSION_MAIN            0xF0  // main version


#define EPL_FRAME_OFFSET_DST_MAC        0
#define EPL_FRAME_OFFSET_SRC_MAC        6
#define EPL_FRAME_OFFSET_ETHER_TYPE     12
#define EPL_FRAME_OFFSET_MSG_TYPE       14
#define EPL_FRAME_OFFSET_DST_NODEID     15
#define EPL_FRAME_OFFSET_SRC_NODEID     16
#define EPL_FRAME_OFFSET_PDO_PAYLOAD    24

// defines for bit fields SyncControl and SyncStatus
#define EPL_SYNC_PRES_TIME_FIRST_VALID          0x00000001
#define EPL_SYNC_PRES_TIME_SECOND_VALID         0x00000002
#define EPL_SYNC_SYNC_MN_DELAY_FIRST_VALID      0x00000004
#define EPL_SYNC_SYNC_MN_DELAY_SECOND_VALID     0x00000008
#define EPL_SYNC_PRES_FALL_BACK_TIMEOUT_VALID   0x00000010
#define EPL_SYNC_DEST_MAC_ADDRESS_VALID         0x00000020
#define EPL_SYNC_PRES_MODE_RESET                0x40000000
#define EPL_SYNC_PRES_MODE_SET                  0x80000000

//---------------------------------------------------------------------------
// typedef
//---------------------------------------------------------------------------


// $$$ d.k.: move this definition to global.h
// byte-align structures
#ifdef _MSC_VER
#    pragma pack( push, packing )
#    pragma pack( 1 )
#    define PACK_STRUCT
#elif defined( __GNUC__ )
#    define PACK_STRUCT    __attribute__((packed))
#else
#    error you must byte-align these structures with the appropriate compiler directives
#endif


typedef struct
{
    // Offset 17
    BYTE                    m_le_bRes1;                     // reserved
    // Offset 18
    BYTE                    m_le_bFlag1;                    // Flags: MC, PS
    // Offset 19
    BYTE                    m_le_bFlag2;                    // Flags: res
    // Offset 20
    tEplNetTime             m_le_NetTime;                   // supported if D_NMT_NetTimeIsRealTime_BOOL is set
    // Offset 28
    QWORD                   m_le_RelativeTime;              // in us (supported if D_NMT_RelativeTime_BOOL is set)

} PACK_STRUCT tEplSocFrame;

typedef struct
{
    // Offset 17
    BYTE                    m_le_bRes1;                     // reserved
    // Offset 18
    BYTE                    m_le_bFlag1;                    // Flags: MS, EA, RD
    // Offset 19
    BYTE                    m_le_bFlag2;                    // Flags: res
    // Offset 20
    BYTE                    m_le_bPdoVersion;
    // Offset 21
    BYTE                    m_le_bRes2;                     // reserved
    // Offset 22
    WORD                    m_le_wSize;
    // Offset 24
    BYTE                    m_le_abPayload[256 /*D_NMT_IsochrRxMaxPayload_U16*/];

} PACK_STRUCT tEplPreqFrame;

typedef struct
{
    // Offset 17
    BYTE                    m_le_bNmtStatus;                // NMT state
    // Offset 18
    BYTE                    m_le_bFlag1;                    // Flags: MS, EN, RD
    // Offset 19
    BYTE                    m_le_bFlag2;                    // Flags: PR, RS
    // Offset 20
    BYTE                    m_le_bPdoVersion;
    // Offset 21
    BYTE                    m_le_bRes2;                     // reserved
    // Offset 22
    WORD                    m_le_wSize;
    // Offset 24
    BYTE                    m_le_abPayload[256 /*D_NMT_IsochrRxMaxPayload_U16
                                        / D_NMT_IsochrTxMaxPayload_U16*/];

} PACK_STRUCT tEplPresFrame;

typedef struct
{
    // Offset 23
    BYTE                    m_le_bReserved;
    DWORD                   m_le_dwSyncControl;
    DWORD                   m_le_dwPResTimeFirst;
    DWORD                   m_le_dwPResTimeSecond;
    DWORD                   m_le_dwSyncMnDelayFirst;
    DWORD                   m_le_dwSyncMnDelaySecond;
    DWORD                   m_le_dwPResFallBackTimeout;
    BYTE                    m_be_abDestMacAddress[6];
    
} PACK_STRUCT tEplSyncRequest;

typedef union
{
    // Offset 23
    tEplSyncRequest         m_SyncRequest;

} tEplSoaPayload;

typedef struct
{
    // Offset 17
    BYTE                    m_le_bNmtStatus;                // NMT state
    // Offset 18
    BYTE                    m_le_bFlag1;                    // Flags: EA, ER
    // Offset 19
    BYTE                    m_le_bFlag2;                    // Flags: res
    // Offset 20
    BYTE                    m_le_bReqServiceId;
    // Offset 21
    BYTE                    m_le_bReqServiceTarget;
    // Offset 22
    BYTE                    m_le_bEplVersion;
    // Offset 23
    tEplSoaPayload          m_Payload;

} PACK_STRUCT tEplSoaFrame;

typedef struct
{
    WORD            m_wEntryType;
    WORD            m_wErrorCode;
    tEplNetTime     m_TimeStamp;
    BYTE            m_abAddInfo[8];

} PACK_STRUCT tEplErrHistoryEntry;

typedef struct
{
    // Offset 18
    BYTE                    m_le_bFlag1;                    // Flags: EN, EC
    BYTE                    m_le_bFlag2;                    // Flags: PR, RS
    BYTE                    m_le_bNmtStatus;                // NMT state
    BYTE                    m_le_bRes1[3];
    QWORD                   m_le_qwStaticError;             // static error bit field
    tEplErrHistoryEntry     m_le_aErrHistoryEntry[14];

} PACK_STRUCT tEplStatusResponse;

typedef struct
{
    // Offset 18
    BYTE                    m_le_bFlag1;                    // Flags: res
    BYTE                    m_le_bFlag2;                    // Flags: PR, RS
    BYTE                    m_le_bNmtStatus;                // NMT state
    BYTE                    m_le_bIdentRespFlags;           // Flags: FW
    BYTE                    m_le_bEplProfileVersion;
    BYTE                    m_le_bRes1;
    DWORD                   m_le_dwFeatureFlags;            // NMT_FeatureFlags_U32
    WORD                    m_le_wMtu;                      // NMT_CycleTiming_REC.AsyncMTU_U16: C_IP_MIN_MTU - C_IP_MAX_MTU
    WORD                    m_le_wPollInSize;               // NMT_CycleTiming_REC.PReqActPayload_U16
    WORD                    m_le_wPollOutSize;              // NMT_CycleTiming_REC.PResActPayload_U16
    DWORD                   m_le_dwResponseTime;            // NMT_CycleTiming_REC.PResMaxLatency_U32
    WORD                    m_le_wRes2;
    DWORD                   m_le_dwDeviceType;              // NMT_DeviceType_U32
    DWORD                   m_le_dwVendorId;                // NMT_IdentityObject_REC.VendorId_U32
    DWORD                   m_le_dwProductCode;             // NMT_IdentityObject_REC.ProductCode_U32
    DWORD                   m_le_dwRevisionNumber;          // NMT_IdentityObject_REC.RevisionNo_U32
    DWORD                   m_le_dwSerialNumber;            // NMT_IdentityObject_REC.SerialNo_U32
    QWORD                   m_le_qwVendorSpecificExt1;
    DWORD                   m_le_dwVerifyConfigurationDate; // CFM_VerifyConfiguration_REC.ConfDate_U32
    DWORD                   m_le_dwVerifyConfigurationTime; // CFM_VerifyConfiguration_REC.ConfTime_U32
    DWORD                   m_le_dwApplicationSwDate;       // PDL_LocVerApplSw_REC.ApplSwDate_U32 on programmable device or date portion of NMT_ManufactSwVers_VS on non-programmable device
    DWORD                   m_le_dwApplicationSwTime;       // PDL_LocVerApplSw_REC.ApplSwTime_U32 on programmable device or time portion of NMT_ManufactSwVers_VS on non-programmable device
    DWORD                   m_le_dwIpAddress;
    DWORD                   m_le_dwSubnetMask;
    DWORD                   m_le_dwDefaultGateway;
    BYTE                    m_le_sHostname[32];
    BYTE                    m_le_abVendorSpecificExt2[48];

} PACK_STRUCT tEplIdentResponse;

typedef struct
{
    // Offset 18
    BYTE                    m_le_bNmtCommandId;
    BYTE                    m_le_bRes1;
    BYTE                    m_le_abNmtCommandData[32];

} PACK_STRUCT tEplNmtCommandService;

typedef struct
{
    // Offset 18
    WORD                    m_le_wReserved;
    DWORD                   m_le_dwSyncStatus;
    DWORD                   m_le_dwLatency;
    DWORD                   m_le_dwSyncNodeNumber;
    DWORD                   m_le_dwSyncDelay;
    DWORD                   m_le_dwPResTimeFirst;
    DWORD                   m_le_dwPResTimeSecond;
    
} PACK_STRUCT tEplSyncResponse;

typedef struct
{
    BYTE                    m_le_bReserved;
    BYTE                    m_le_bTransactionId;
    BYTE                    m_le_bFlags;
    BYTE                    m_le_bCommandId;
    WORD                    m_le_wSegmentSize;
    WORD                    m_le_wReserved;
    BYTE                    m_le_abCommandData[8];  // just reserve a minimum number of bytes as a placeholder

}PACK_STRUCT tEplAsySdoCom;


// asynchronous SDO Sequence Header
typedef struct
{
    BYTE           m_le_bRecSeqNumCon;
    BYTE           m_le_bSendSeqNumCon;
    BYTE           m_le_abReserved[2];
    tEplAsySdoCom  m_le_abSdoSeqPayload;

} PACK_STRUCT tEplAsySdoSeq;

typedef struct
{
    // Offset 18
    BYTE                    m_le_bNmtCommandId;
    BYTE                    m_le_bTargetNodeId;
    BYTE                    m_le_abNmtCommandData[32];

} PACK_STRUCT tEplNmtRequestService;


typedef union
{
    // Offset 18
    tEplStatusResponse      m_StatusResponse;
    tEplIdentResponse       m_IdentResponse;
    tEplNmtCommandService   m_NmtCommandService;
    tEplNmtRequestService   m_NmtRequestService;
    tEplAsySdoSeq           m_SdoSequenceFrame;
    tEplSyncResponse        m_SyncResponse;
    BYTE                    m_le_abPayload[256 /*D_NMT_ASndTxMaxPayload_U16
                                        / D_NMT_ASndRxMaxPayload_U16*/];

} tEplAsndPayload;

typedef struct
{
    // Offset 17
    BYTE                    m_le_bServiceId;
    // Offset 18
    tEplAsndPayload         m_Payload;

} PACK_STRUCT tEplAsndFrame;

typedef union
{
    // Offset 17
    tEplSocFrame            m_Soc;
    tEplPreqFrame           m_Preq;
    tEplPresFrame           m_Pres;
    tEplSoaFrame            m_Soa;
    tEplAsndFrame           m_Asnd;

} tEplFrameData;

typedef struct
{
    // Offset 0
    BYTE                    m_be_abDstMac[6];               // MAC address of the addressed nodes
    // Offset 6
    BYTE                    m_be_abSrcMac[6];               // MAC address of the transmitting node
    // Offset 12
    WORD                    m_be_wEtherType;                // Ethernet message type (big endian)
    // Offset 14
    BYTE                    m_le_bMessageType;              // EPL message type
    // Offset 15
    BYTE                    m_le_bDstNodeId;                // EPL node ID of the addressed nodes
    // Offset 16
    BYTE                    m_le_bSrcNodeId;                // EPL node ID of the transmitting node
    // Offset 17
    tEplFrameData           m_Data;

} PACK_STRUCT tEplFrame;

// un-byte-align structures
#ifdef _MSC_VER
#    pragma pack( pop, packing )
#endif


typedef enum
{
    kEplMsgTypeNonEpl = 0x00,
    kEplMsgTypeSoc    = 0x01,
    kEplMsgTypePreq   = 0x03,
    kEplMsgTypePres   = 0x04,
    kEplMsgTypeSoa    = 0x05,
    kEplMsgTypeAsnd   = 0x06,
    kEplMsgTypeAInv   = 0x0D,
} tEplMsgType;



//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------


#endif  // #ifndef _EPL_FRAME_H_


