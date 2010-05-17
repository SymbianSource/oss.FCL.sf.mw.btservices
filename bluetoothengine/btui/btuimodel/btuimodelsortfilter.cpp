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

#include <btuimodelsortfilter.h>
#include <btdevicemodel.h>

/*!
    Constructor.
 */
BtuiModelSortFilter::BtuiModelSortFilter( QObject *parent )
    : QSortFilterProxyModel( parent )
{
    setDynamicSortFilter( true );
}

/*!
    Destructor.
 */
BtuiModelSortFilter::~BtuiModelSortFilter()
{
}

/*!
    Replace current filter values for filtering on major device class with
    the specified. This Model will not reset itself for performance reason, 
    the caller shall call reset after all filters have been set.
 */
void BtuiModelSortFilter::setDeviceMajorFilter( int filter, FilterMode mode )
{
    mFilters.clear();
    addDeviceMajorFilter( filter, mode );
}

/*!
    Add the specified filter value for filtering on major device class 
    if the specified filter doesn't exist when this function is called.
    This Model will not reset itself for performance reason, 
    the caller shall call reset after all filters have been set.
 */
void BtuiModelSortFilter::addDeviceMajorFilter( int filter, FilterMode mode )
{
    FilterItem f(filter, mode);
    if ( mFilters.indexOf(f) == -1 ) {
        mFilters.append( f );
    }
}

/*!
    Clear the specified filter value for filtering on major device class from this model.
    This Model will not reset itself for performance reason, 
    the caller shall call reset after all filters have been set.
 */
void BtuiModelSortFilter::clearDeviceMajorFilter( int filter, FilterMode mode )
{
    FilterItem f(filter, mode);
    int i = mFilters.indexOf(f);
    if ( i > -1 ) {
        mFilters.removeAt( i );
    }
}

/*!
    clear all filters for filtering on major device class.
    This Model will not reset itself for performance reason, 
    the caller shall call reset after all filters have been set.
 */
void BtuiModelSortFilter::clearDeviceMajorFilters()
{
    // model reset is needed if there are filters :
    bool shReset = ( mFilters.size() > 0 );

    if ( shReset ) {
        reset();
    }
    mFilters.clear();
}

/*!
    return true if the specified filter exists; return false otherwise.
 */
bool BtuiModelSortFilter::hasFilter( int filter, FilterMode mode )
{
    FilterItem f(filter, mode);
    return mFilters.indexOf(f) > -1 ;
}

/*!
    \reimp
 */
bool BtuiModelSortFilter::filterAcceptsRow(
    int sourceRow, const QModelIndex &sourceParent) const 
{
    bool accepted (false );
    // Get the device name from the model
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    // the row shall pass all filters:
    for (int i = 0; i < mFilters.size(); i++ ) {
        if ( mFilters.at(i).mFilter == BtDeviceModel::NullProperty ) {
            accepted = true;    // There is no filter, we accept all
        }
        else {
            int majorProperty = 
                    sourceModel()->data(index, BtDeviceModel::MajorPropertyRole).toInt();
            switch (mFilters.at(i).mMode) {
                case ExactMatch:
                    // Accept if the match is spot-on
                    accepted = majorProperty == mFilters.at(i).mFilter ;
                    break;
                case AtLeastMatch:
                    // accept if it matches all specified filters:
                    accepted = ( mFilters.at(i).mFilter == 
                        ( majorProperty & mFilters.at(i).mFilter ) );
                    break;
                case RoughMatch:
                    // Accept if it matches one of specified filters:
                    accepted = (majorProperty & mFilters.at(i).mFilter) != 0;
                    break;
                case Exclusive:
                    // Accept if this is not one that we want to filter out
                    accepted = (majorProperty & mFilters.at(i).mFilter) == 0;
                    break;
                default:
                    accepted = false;
            }
        }
    }
    if (accepted) {
        // emit signal to inform a row has been accepted by fitler,
        // currently this is only needed by search view
        emit const_cast<BtuiModelSortFilter*>(this)->deviceAcceptedByFilter( sourceRow );
    }
    return accepted;
}

/*!
    \reimp
 */
bool BtuiModelSortFilter::lessThan(
        const QModelIndex &left, const QModelIndex &right) const
{
    Q_UNUSED( left );
    Q_UNUSED( right );
    if (sortRole() == BtDeviceModel::NameAliasRole ||
        sortRole() == BtDeviceModel::LastUsedTimeRole ||
        sortRole() == BtDeviceModel::RssiRole) {
        // base class provides sorting for these types already:
        return QSortFilterProxyModel::lessThan(left, right);
    }
    // no custom sort supported yet.
    return true;
}
