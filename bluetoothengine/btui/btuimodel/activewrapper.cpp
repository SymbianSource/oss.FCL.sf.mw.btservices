/*
* ============================================================================
*  Name        : btuimsettings_p.cpp
*  Part of     : BluetoothUI / bluetoothuimodel       *** Info from the SWAD
*  Description : Implements the data source settings class used by BluetoothUiDataModel.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.1
*/

#include "activewrapper.h"

/*!
    Constructor.
 */
ActiveWrapper::ActiveWrapper( int id, int priority, QObject *parent )
:   QObject( parent ){
    d = new ActiveWrapperPrivate( this, id, priority );
}

/*!
    Destructor.
 */
ActiveWrapper::~ActiveWrapper(){
    delete d;
}

int ActiveWrapper::getRequestId(){
    return d->mRequestId;
}

void ActiveWrapper::setRequestId( int reqId ){
    d->mRequestId = reqId;
}


void ActiveWrapper::setActive() {
    d->SetActive();
}

void ActiveWrapper::cancel(){
    d->Cancel();
}

bool ActiveWrapper::isActive(){
    return (bool) d->IsActive();
}

void ActiveWrapper::emitRequestCompleted( int status, int id ) {
    emit requestCompleted( status, id );
}

void ActiveWrapper::emitCancelRequest( int id ) {
    emit cancelRequest( id );
}

inline void ActiveWrapper::emitError( int status, int id ) {
    emit error( status, id );
}

/*!
    Constructor.
 */
ActiveWrapperPrivate::ActiveWrapperPrivate( 
    ActiveWrapper *parent, int id, int priority )
:   CActive( (TInt) priority ),
    q( parent ),
    mRequestId( id ) {
    CActiveScheduler::Add( this );
}

/*!
    Destructor.
 */
ActiveWrapperPrivate::~ActiveWrapperPrivate() {
    Cancel();
}

/*!
    \reimp
    Called by the active scheduler when the request has been cancelled.
 */
void ActiveWrapperPrivate::RunL() {
    q->emitRequestCompleted( iStatus.Int(), mRequestId );
}

/*!
    \reimp
    Called by Cancel() when the request is outstanding.
 */
void ActiveWrapperPrivate::DoCancel() {
     q->emitCancelRequest( mRequestId );
}

/*!
    \reimp
    Called by the active scheduler when an error in RunL has occurred.
 */
TInt ActiveWrapperPrivate::RunError( TInt error ) {
    q->emitRequestCompleted( error, mRequestId );
    return KErrNone;
}
