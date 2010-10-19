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
* Description:  BtDeviceDialogWidget class declaration.
*
*/

#ifndef BLUETOOTHTRACE_H
#define BLUETOOTHTRACE_H

#include <e32base.h>
#include "traceconfig.h"

/*
 * Common tracing utility definition to be used by Bluetooth projects.
 * The configuration is loaded from traceconfig.h which shall be private for
 * each individual project.
 * 
 * In this utility, a set of OST-alike tracing macros are defined. 
 * (The purpose is to ease migration to OST in future.)
 * 
 * Individual project can also define new macros based on this utility in
 * its own space.
 * 
 */

#ifdef BLUETOOTHTRACE_ENABLED

#ifdef BLUETOOTHTRACE_MEDIA_OST

/**
 * Convert own macros to OST macros when OST tracing is used.
 * In OST tracing, aTraceName must be a unique identifier in scope of a module. 
 * Thus many OST compiling errors may occur if the same TraceName is used in multiple
 * tracing lines where tracing is miigrated from non-OST to OST. 
 * The fix is to renaming the TraceName:s to be unique.
 */
#include <OpenSystemTrace.h>

#define BOstrace0( aGroupName, aTraceName, aTraceText ) \
    OstTrace0( aGroupName, aTraceName, aTraceText )

#define BOstrace1( aGroupName, aTraceName, aTraceText, aParam ) \
    OstTrace1( aGroupName, aTraceName, aTraceText, aParam )

#define BOstraceData( aGroupName, aTraceName, aTraceText, aPtr, aLength ) \
    OstTraceData( aGroupName, aTraceName, aTraceText, aPtr, aLength )

#define BOstraceExt1( aGroupName, aTraceName, aTraceText, aParam ) \
    OstTraceExt1( aGroupName, aTraceName, aTraceText, aParam )

#define BOstraceExt2( aGroupName, aTraceName, aTraceText, aParam1, aParam2 ) \
    OstTraceExt2( aGroupName, aTraceName, aTraceText, aParam1, aParam2 )

#define BOstraceExt3( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3 ) \
    OstTraceExt3( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3 )

#define BOstraceExt4( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4 ) \
    OstTraceExt4( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4 )

#define BOstraceExt5( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4, aParam5 ) \
    OstTraceExt5( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4, aParam5 )

#define BOstraceFunctionEntry0( aTraceName ) \
    OstTraceFunctionEntry0( aTraceName )

#define BOstraceFunctionEntry1( aTraceName, aInstance ) \
    OstTraceFunctionEntry1( aTraceName, aInstance )

#define BOstraceFunctionEntryExt(aTraceName, aInstance, aArg) \
    OstTraceFunctionEntryExt(aTraceName, aInstance, aArg)

#define BOstraceFunctionExit0( aTraceName ) \ 
    OstTraceFunctionExit0( aTraceName )

#define BOstraceFunctionExit1( aTraceName, aInstance ) \
    OstTraceFunctionExit1( aTraceName, aInstance )

#define BOstraceFunctionExitExt(aTraceName, aInstance, aRetval) \
    OstTraceFunctionExitExt(aTraceName, aInstance, aRetval)
    
#define BOstraceEventStart0( aTraceName, aEventName ) \
    OstTraceEventStart0( aTraceName, aEventName )
    
#define BOstraceEventStart1( aTraceName, aEventName, aParam ) \
    OstTraceEventStart1( aTraceName, aEventName, aParam )

#define BOstraceEventStop( aTraceName, aEventName ) \
    OstTraceEventStop( aTraceName, aEventName )

#define BOstraceState0( aTraceName, aStateName, aNewState ) \
    OstTraceState0( aTraceName, aStateName, aNewState )

#define BOstraceState1( aTraceName, aStateName, aNewState, aInstance ) \
    OstTraceState1( aTraceName, aStateName, aNewState, aInstance )
    
#else  // BLUETOOTHTRACE_MEDIA_OST

#ifdef BLUETOOTHTRACE_MEDIA_FILE
#include <flogger.h>
#else   
#include <e32debug.h>
#endif

/**
 * When tracing compilation with OST is disabled, the TraceName in each OST trace line
 * is ignored, that is, the Trace Names are not checked at compiling time, neither
 * are they written into the specified trace output media.
 */
    
/**
 * Handlers below are used for tolerating overflow of formatting strings.
 * to trucate rather than panic the caller.
 */
NONSHARABLE_CLASS( TBtTraceOflowTruncate8 ) : public TDes8Overflow
    {
public:
    void Overflow( TDes8& /*aDes*/ ) {}
    };

NONSHARABLE_CLASS( TBtTraceOflowTruncate16 ) : public TDes16Overflow
    {
public:
    void Overflow( TDes16& /*aDes*/ ) {}
    };

/**
 * internal tracing implementation, do not use it out of this file.
 */
inline void Trace(const TDesC &trGrp, const TDesC &trTxt)
{
    TPtrC cp(KComponentName);
#ifdef BLUETOOTHTRACE_MEDIA_FILE
    RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, _L("%S%S%S"),&cp,&trGrp,&trTxt);
#else
    RDebug::Print( _L("%S%S%S"),&cp,&trGrp,&trTxt );
#endif
}

/*
 * trace with no parameters
 */
#define BOstrace0( aGroupName, aTraceName, aTraceText ) \
{\
    Trace( aGroupName, aTraceText ); \
}

/*
 * trace with one 32-bit parameter
 */
#define BOstrace1( aGroupName, aTraceName, aTraceText, aParam ) \
{\
    TBuf<200> buf; TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(aTraceText, &overflow, aParam); \
    Trace( aGroupName, buf ); \
}

/*
 * trace with more than 32 bits of data. Not supported
 */
#define BOstraceData( aGroupName, aTraceName, aTraceText, aPtr, aLength ) 

/*
 * trace with one parameter that is not 32-bit integer
 */
#define BOstraceExt1( aGroupName, aTraceName, aTraceText, aParam ) \
    BOstrace1( aGroupName, aTraceName, aTraceText, aParam )

/*
 * trace with two parameters.
 */
#define BOstraceExt2( aGroupName, aTraceName, aTraceText, aParam1, aParam2 ) \
{\
    TBuf<200> buf; TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(aTraceText, &overflow, aParam1, aParam2); \
    Trace( aGroupName, buf ); \
}

/*
 * trace with three parameters.
 */
#define BOstraceExt3( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3 ) \
{\
    TBuf<200> buf; TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(aTraceText, &overflow, aParam1, aParam2, aParam3); \
    Trace( aGroupName, buf ); \
}

/*
 * trace with four parameters
 */    
#define BOstraceExt4( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4 ) \
{\
    TBuf<200> buf; TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(aTraceText, &overflow, aParam1, aParam2, aParam3, aParam4); \
    Trace( aGroupName, buf ); \
}

/*
 * trace with five parameters
 */   
#define BOstraceExt5( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4, aParam5 ) \
{\
    TBuf<200> buf; TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(aTraceText, &overflow, aParam1, aParam2, aParam3, aParam4, aParam5); \
    Trace( aGroupName, buf ); \
}

/*
 * Function entry trace without extra parameters. 
 * The trace is mapped to TRACE_API group.
 */
#define BOstraceFunctionEntry0( aTraceName ) \
{\
    TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
    TBuf<200> buf; buf.Copy( func.Left(150) ); buf.Insert(0, _L(">> ") );\
    Trace( _L("[ API ]"), buf ); \
}

/*
 * Function entry trace with a parameter representing the instance identifier, e.g.
 * "this" pointer.
 * The trace is mapped to TRACE_API group.
 */
#define BOstraceFunctionEntry1( aTraceName, aInstance ) \
{\
    TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
    TBuf<200> buf; buf.Copy( func.Left(150) ); buf.Insert(0, _L(">> ") ); \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L(" 0x%X(%d)"), &overflow, aInstance, aInstance); \
    Trace( _L("[ API ]"), buf ); \
}

/*
 * Function entry trace, which traces function parameters.
 * The trace is mapped to TRACE_API group. 
 */
#define BOstraceFunctionEntryExt(aTraceName, aInstance, aArg) \
{ \
    TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
    TBuf<200> buf; buf.Copy( func.Left(150) );  buf.Insert(0, _L(">> ")); \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L(" 0x%X(%d) arg %d"), &overflow, aInstance, aInstance, aArg); \
    Trace( _L("[ API ]"), buf ); \
}
/*
 * Function exit trace without extra parameters. 
 * The trace is mapped to TRACE_API group.
 */
#define BOstraceFunctionExit0( aTraceName ) \
{\
    TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
    TBuf<200> buf; buf.Copy( func.Left(150) );  buf.Insert(0, _L("<< ")); \
    Trace( _L("[ API ]"), buf ); \
}

/*
 * Function exit trace with a parameter representing the instance identifier
 * for example "this" pointer.
 * The trace is mapped to TRACE_API group.
 */
#define BOstraceFunctionExit1( aTraceName, aInstance ) \
{\
    TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
    TBuf<200> buf; buf.Copy( func.Left(150) );  buf.Insert(0, _L("<< ")); \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L(" 0x%X(%d)"), &overflow, aInstance, aInstance); \
    Trace( _L("[ API ]"), buf ); \
}

/*
 * Function exit trace with parameters representing the instance identifier, 
 * for example "this" pointer, and return value.
 * The trace is mapped to TRACE_API group.
 */
#define BOstraceFunctionExitExt(aTraceName, aInstance, aRetval) \
{\
    TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
    TBuf<200> buf; buf.Copy( func.Left(150) );  buf.Insert(0, _L("<< ")); \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L(" 0x%X(%d) ret %d"), &overflow, aInstance, aInstance, aRetval); \
    Trace( _L("[ API ]"), buf ); \
}

/*
 * Performance measurement event start trace without extra parameters. 
 * The trace is mapped to TRACE_PERFORMANCE group.
 */
#define BOstraceEventStart0( aTraceName, aEventName ) \
{\
    TBuf<200> buf(_L("[Start] ")); buf.Append( aEventName ); \
    Trace( _L("[PFMAN]"), buf ); \
}

/*
 * Performance measurement event start trace with single 32-bit parameter. 
 * The trace is mapped to TRACE_PERFORMANCE group.
 */
#define BOstraceEventStart1( aTraceName, aEventName, aParam ) \
{\
    TPtrC evt(aEventName); TBuf<200> buf; \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L("[Start] %S 0x%X(%d)"), &overflow, &evt, aParam, aParam ); \
    Trace( _L("[PFMAN]"), buf ); \
}

/*
 * Performance measurement event end trace. 
 * The trace is mapped to TRACE_PERFORMANCE group.
 */
#define BOstraceEventStop( aTraceName, aEventName ) \
{\
    TBuf<200> buf(_L("[Stop] ")); buf.Append( aEventName ); \
    Trace( _L("[PFMAN]"), buf ); \
}

/*
 * State transition event.
 * The trace is mapped to TRACE_STATE group.
 */
#define BOstraceState0( aTraceName, aStateName, aNewState ) \
{\
    TPtrC evt(aStateName); TBuf<200> buf; \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L("%S 0x%X(%d)"), &overflow, &evt, aNewState, aNewState ); \
    Trace( _L("[STATE]"), buf ); \
}

/*
 * State transition event with instance identifier.
 * The trace is mapped to TRACE_STATE group.
 */
#define BOstraceState1( aTraceName, aStateName, aNewState, aInstance ) \
{\
    TPtrC evt(aStateName); TBuf<200> buf; \
    TBtTraceOflowTruncate16 overflow; \
    buf.AppendFormat(_L("%S 0x%X(%d) instance=0x%X(%d)"), &overflow, &evt, aNewState, aNewState, aInstance, aInstance ); \
    Trace( _L("[STATE]"), buf ); \
}

#endif // BLUETOOTHTRACE_MEDIA_OST

// Extended tracing macros facilitating domain specific tracing needs:

/*
 * A block of source code merely for tracing purpose.
 */
#define BtTraceBlock( exp ) {exp}

/*
 * trace macro for BT device address printing with an additional trace text. 
 * aParam must be TBTDevAddr type.
 */
#define BtTraceBtAddr1( aGroupName, aTraceName, aTraceText, aParam ) \
{ \
    TBuf<12> buf; \
    aParam.GetReadable( buf ); TPtrC p2(buf);\
    BOstraceExt2( aGroupName, aTraceName, "%S%S", aTraceText, &p2 ); \
}

/*
 * trace macro for BT device address printing with no additional trace text. 
 * aParam must be TBTDevAddr type.
 */
#define BtTraceBtAddr0( aGroupName, aTraceName, aParam ) \
{ \
    TBuf<12> buf; aParam.GetReadable( buf ); TPtrC p(buf); \
    BOstraceExt1( aGroupName, aTraceName, _L("%S"), &p ); \
}

#else // BLUETOOTHTRACE_ENABLED

#define BOstrace0( aGroupName, aTraceName, aTraceText )
#define BOstrace1( aGroupName, aTraceName, aTraceText, aParam )
#define BOstraceData( aGroupName, aTraceName, aTraceText, aPtr, aLength )
#define BOstraceExt1( aGroupName, aTraceName, aTraceText, aParam )
#define BOstraceExt2( aGroupName, aTraceName, aTraceText, aParam1, aParam2 )
#define BOstraceExt3( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3 )
#define BOstraceExt4( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4 )
#define BOstraceExt5( aGroupName, aTraceName, aTraceText, aParam1, aParam2, aParam3, aParam4, aParam5 )

#define BOstraceFunctionEntry0( aTraceName )
#define BOstraceFunctionEntry1( aTraceName, aInstance )
#define BOstraceFunctionEntryExt(aTraceName, aInstance, aArg)
#define BOstraceFunctionExit0( aTraceName )
#define BOstraceFunctionExit1( aTraceName, aInstance )
#define BOstraceEventStart0( aTraceName, aEventName )
#define BOstraceEventStart1( aTraceName, aEventName, aParam )
#define BOstraceFunctionExitExt(aTraceName, aInstance, aRetval)
#define BOstraceEventStop( aTraceName, aEventName )
#define BOstraceState0( aTraceName, aStateName, aNewState )
#define BOstraceState1( aTraceName, aStateName, aNewState, aInstance )

#define BtTraceBlock( exp )
#define BtTraceBtAddr1( aGroupName, aTraceName, aTraceText, aParam )
#define BtTraceBtAddr0( aGroupName, aTraceName, aParam )

#endif // BLUETOOTHTRACE_ENABLED

/*
 * Additional general purpose definition, a hook for defining a friend class
 * for unit testing to get access to class internals.
 */
#ifndef BTUNITTEST
#define BTUNITTESTHOOK
#endif

#endif // BLUETOOTHTRACE_H
