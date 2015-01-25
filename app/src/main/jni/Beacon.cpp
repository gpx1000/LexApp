/** beacon.cpp
 *
 * Author: Aleksei Smirnov <aleksei.smirnov@navigine.ru>
 * Copyright (c) 2014 Navigine. All rights reserved.
 *
 */

#include "Beacon.h"
#include <android/log.h>

#define  LOG_TAG    "Beacon"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

Beacon::Beacon()
: mX(0.)
, mY(0.)
, mLocationId(0)
{
}

//copy constructor
Beacon::Beacon(const Beacon& beacon)
: mX            (beacon.mX)
, mY            (beacon.mY)
, mId           (beacon.mId)
, mName         (beacon.mName)
, mLocationName (beacon.mLocationName)
, mLocationId   (beacon.mLocationId)
{
}

void Beacon::setBeaconId(const std::string& beaconId)
{
  mId = beaconId;
}

void Beacon::fillData(const float x, const float y, const std::string& id,
        const std::string& name, const std::string& locName)
{
  mX = x;
  mY = y;
  mId = id;
  mName = name;
  mLocationName = locName;
}

const std::string& Beacon::getId () const
{
  return mId;
}

float Beacon::getX() const
{
  return mX;
}

float Beacon::getY( ) const
{
  return mY;
}

void Beacon::setX( const float x )
{
  mX = x;
}

void Beacon::setY( const float y )
{
  mY = y;
}

void Beacon::setLocationName( const std::string& name )
{
  mLocationName += name;
}

void Beacon::setLocationId( const int sublocId )
{
  mLocationId = sublocId;
}

int Beacon::getLocationId() const
{
  return mLocationId;
}

bool Beacon::operator==(const Beacon& entry) const
{
  return getId() == entry.getId();
}

bool Beacon::operator!=(const Beacon& entry) const
{
  return !(*this == entry);
}

IBeacon::IBeacon()
: Beacon()
, major(0)
, minor(0)
{
}

//copy constructor
IBeacon::IBeacon(const IBeacon& iBeacon)
: Beacon(iBeacon)
, uuid(iBeacon.uuid)
, major(iBeacon.major)
, minor(iBeacon.minor)
{
}

const std::string& IBeacon::getUuid () const
{
  return uuid;
}

int IBeacon::getMajor() const
{
  return major;
}

int IBeacon::getMinor() const
{
  return minor;
}

void IBeacon::setMajor( const unsigned int major )
{
  this->major = major;
}

void IBeacon::setMinor( const unsigned int minor )
{
  this->minor = minor;
}

void IBeacon::setUuid( const std::string& uuid )
{
  this->uuid = uuid;
}

// constructor
BeaconMeas::BeaconMeas()
: mBeaconPtr (0)
, mRssi      (TRANSMITTER_POINT_UNUSED)
{
}

//constructor
BeaconMeas::BeaconMeas( Beacon* beacon, float rssi, float distance )
: mBeaconPtr(beacon)
, mRssi(rssi)
, mDistance(distance)
, mBeaconId(beacon->getId())
{
}

//copy constructor
BeaconMeas::BeaconMeas(const BeaconMeas& beaconMeas)
: mBeaconId  (beaconMeas.mBeaconId)
, mBeaconPtr (beaconMeas.mBeaconPtr)
, mRssi      (beaconMeas.mRssi)
, mDistance  (beaconMeas.mDistance)
{
}

BeaconMeas::~BeaconMeas()
{
  mBeaconPtr = 0;
}

float BeaconMeas::getRssi()const
{
  return mRssi;
}

float BeaconMeas::getDistance()const
{
  return mDistance;
}

void BeaconMeas::setRssi(const float rssi)
{
  mRssi = rssi;
}

void BeaconMeas::setDistance(const float distance)
{
  mDistance = distance;
}

bool BeaconMeas::operator<(const BeaconMeas& entry)const
{
  return mDistance < entry.mDistance;
}

bool BeaconMeas::operator>(const BeaconMeas& entry)const
{
  return entry < *this;
}

bool BeaconMeas::operator==(const BeaconMeas& entry)const
{
  if ( getBeaconId().empty() || entry.getBeaconId().empty())
    LOGD("ERROR: id is empty");

  return getBeaconId() == entry.getBeaconId();
}

bool BeaconMeas::operator!=(const BeaconMeas& entry)const
{
  return !(*this == entry);
}

// specify Ptr that correspond to beacon from which we got *this measurement
void BeaconMeas::setBeaconPtr( const Beacon* beaconPtr )
{
  mBeaconPtr = const_cast<Beacon*> (beaconPtr);
}

const Beacon* BeaconMeas::getBeaconPtr() const
{
  if (mBeaconPtr == NULL)
    LOGD( "beaconId = %s : mBeaconPtr == NULL\n", mBeaconId.c_str() );
    //throw exception here

  return mBeaconPtr;
}

void BeaconMeas::setBeaconId( const std::string& beaconId )
{
  mBeaconId = beaconId;
}

const std::string& BeaconMeas::getBeaconId() const
{
  return mBeaconId;
}