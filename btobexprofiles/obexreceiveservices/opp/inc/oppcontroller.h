/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*     Class handles Bluetooth OPP reception.
*
*
*/


#ifndef OPPCONTROLLER_H
#define OPPCONTROLLER_H

//  INCLUDES


#include <e32base.h>
#include <obexutilsmessagehandler.h>
#include <obexutilsuilayer.h>
#include "obexutilspropertynotifier.h"
#include <SrcsInterface.h>
#include "btengdevman.h"

#include <obexutilspropertynotifier.h>
#include <obexutilsglobalprogressdialog.h>

#include <AiwServiceHandler.h> // The AIW service handler

_LIT( KUniqueTransportName, "RFCOMM" );
const TInt KBtStartReserveChannel   = 9;
const TInt KBtEndReserveChannel     = 30;

/**
*  CBtListenActive
*  Class to implement IrObex permanent listen
*/
class COPPController : public CSrcsInterface, public MObexServerNotify, 
                       public MObexUtilsPropertyNotifyHandler,
                       public MGlobalProgressCallback, public MGlobalNoteCallback, 
                       public MBTEngDevManObserver     
    {
public:
    static COPPController* NewL();
    ~COPPController();   

private: // from CSrcsInterface
    TBool IsOBEXActive();
    void SetMediaType(TSrcsMediaType aMediaType);
    TInt SetObexServer(CObexServer* aServer);

private: // from MObexServerNotify
    void ErrorIndication(TInt aError);
    void TransportUpIndication();
    void TransportDownIndication();
    TInt ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo);
    void ObexDisconnectIndication(const TDesC8& aInfo);
    CObexBufObject* PutRequestIndication();
    TInt PutPacketIndication();
    TInt PutCompleteIndication();
    CObexBufObject* GetRequestIndication(CObexBaseObject* aRequiredObject);
    TInt GetPacketIndication();
    TInt GetCompleteIndication();
    TInt SetPathIndication(const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo);
    void AbortIndication();
    
private: // from MObexUtilsPropertyNotifyHandler
    void HandleNotifyL(TMemoryPropertyCheckType aCheckType);
    
private: // from MGlobalProgressCallback
    void HandleGlobalProgressDialogL(TInt aSoftkey); 
    
private: // from MGlobalNoteCallback
    void HandleGlobalNoteDialogL(TInt aSoftkey);
    
private: // from MBTEngDevManObserver
    void HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* aDeviceArray);
    
private:
    COPPController();
    void ConstructL();
    
    void CancelTransfer();
    void HandlePutRequestL();
    TInt HandlePutCompleteIndication();
    void HandleError(TBool aAbort);
    
    TBool CheckCapacityL();
    void LaunchReceivingIndicatorL();
    inline TBool ReceivingIndicatorActive() const { return (iProgressDialog || iWaitDialog); }
    void UpdateReceivingIndicator();
    void CloseReceivingIndicator(TBool aResetDisplayedState = ETrue);
    TInt GetDriveWithMaximumFreeSpaceL();    

private:
    enum TObexTransferState
        {
        ETransferIdle,
        ETransferPut,
        ETransferPutDiskError,
        ETransferPutInitError,
        ETransferPutCancel,
        };

private:
    CObexServer*                iObexServer;
    TObexTransferState          iObexTransferState;
    CObexBufObject*             iObexObject;
    TInt                        iDrive;
    TBool                       iListening;
    CObexUtilsPropertyNotifier* iLowMemoryActiveCDrive;
    CObexUtilsPropertyNotifier* iLowMemoryActiveMMC;
    TMsvId                      iMsvIdParent;
    TMsvId                      iMsvIdAttach;
    TFileName                   iFullPathFilename;
    TFileName                   iDefaultFolder;
    TFileName                   iPreviousDefaultFolder;
    TFileName                   iCenRepFolder;
    RFs                         iFs;
    RFile                       iFile;
    CBufFlat*                   iBuf;
    TBool                       iLengthHeaderReceived;
    TSrcsMediaType              iMediaType;
    TInt                        iTotalSizeByte;
    TFileName                   iReceivingFileName;
    CGlobalProgressDialog*      iProgressDialog;
    CGlobalDialog*              iWaitDialog;
    TBool                       iNoteDisplayed;
    CBTEngDevMan*               iDevMan;
    CBTDeviceArray*             iResultArray;
    TBTDeviceName               iRemoteDeviceName;
    };

#endif      // OPPCONTROLLER_H
            
// End of File
