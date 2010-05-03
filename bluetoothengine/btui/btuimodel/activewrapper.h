/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#ifndef ACTIVEWRAPPER_H
#define ACTIVEWRAPPER_H

#include <QObject>
#include <e32base.h>

class ActiveWrapperPrivate;

/*!
    \class ActiveWrapper
    \brief Helper class for using active objects in Qt classes.

    ActiveWrapper is a helper class for using active objects 
    from any Qt class. It wraps a CActive-derived class in an interface
    that uses Qt's signal-slot mechanism for communicating status changes 
    of the active object.

    \\sa bluetoothuimodel
 */
class ActiveWrapper : public QObject
{
    Q_OBJECT
    friend class ActiveWrapperPrivate;

public:
    explicit ActiveWrapper( int id, 
                int priority = CActive::EPriorityStandard, QObject *parent = NULL );
    ~ActiveWrapper();

    int getRequestId();
    void setRequestId(int reqId);
    void setActive();
    void cancel();
    bool isActive();
    
signals:
    void requestCompleted( int status, int id );
    void cancelRequest( int id );
    void error( int status, int id );

private:
    void emitRequestCompleted( int status, int id );
    void emitCancelRequest( int id );
    void emitError( int status, int id );

private:
    ActiveWrapperPrivate *d;

};


/*!
    \class ActiveWrapperPrivate
    \brief Private class of ActiveWrapper, for using active objects in Qt classes.

    ActiveWrapperPrivate is a helper class for using active objects 
    from any Qt class. It derives from CActive and allows other classes to use 
    it for passing to asynchronous function calls through its RequestStatus method.

    \\sa bluetoothuimodel
 */
class ActiveWrapperPrivate : public CActive
{
    friend class ActiveWrapper;

public:
    explicit ActiveWrapperPrivate( ActiveWrapper *parent, int id, int priority );
    ~ActiveWrapperPrivate();

private:
    virtual void RunL();
    virtual void DoCancel();
    virtual TInt RunError( TInt error );

private:
    ActiveWrapper *q;
    int mRequestId;
};

#endif // ACTIVEWRAPPER_H
