/** beacon.h
 *
 * Author: Aleksei Smirnov <aleksei.smirnov@navigine.ru>
 * Copyright (c) 2014 Navigine. All rights reserved.
 *
 */

#ifndef NAVIGINE_BEACON_CLASS
#define NAVIGINE_BEACON_CLASS

#include <string>
#include <cstring>

static const int TRANSMITTER_POINT_UNUSED = 1;


// Beacon is transmitter emitting the signal, that we can use to obtain pseudoDistances
// and process trilateration navigation algorithms
class Beacon
{
  public:
    Beacon();
    Beacon(const Beacon&);
    ~Beacon() {};

    //specify unique beacon identifier that allows us to distinguish beacon
    // and its measurements
    void setBeaconId(const std::string& beaconId);

    // func allows simultaneously fill the data:
    void fillData(const float x,                     //planar coordinate: x
                  const float y,                     //planar coordinate: y
                  const std::string& id,              //beacon identifier
                  const std::string& name,            //beacon name
                  const std::string& locName);        //name of location

    const std::string& getId() const;
    float getX() const;
    float getY() const;

    // specify x,y, beacon planar coordinates
    void setX( const float x );
    void setY( const float y );

    // specify x,y, beacon planar coordinates
    void setLocationName( const std::string& name);
    void setLocationId( int id );

    int getLocationId() const;

    bool operator==(const Beacon& beacon)const;
    bool operator!=(const Beacon& beacon)const;

  private:
    float mX, mY;

    std::string mId;                 // id of the beacon (e.g. major+minor+uuid)
    std::string mName;               // name of the beacon
    std::string mLocationName;       // name of sublocation

    int         mLocationId;         // sublocation id
};


// IBeacon is a beacon, invented by apple, that has additional fields:
// Major,Minor, UUID identifiers
class IBeacon : public Beacon
{
  public:
    IBeacon();
    IBeacon(const IBeacon&);

    const std::string& getUuid() const;
    void setUuid( const std::string& uuid );

    int getMajor() const;
    int getMinor() const;

    void setMajor( const unsigned int major );
    void setMinor( const unsigned int minor );
  private:
    std::string uuid;
    unsigned int major;
    unsigned int minor;
};


// class that keep measurements from certain transmitter (e.g. beacon)
class BeaconMeas
{
  public:

    BeaconMeas();
    BeaconMeas( Beacon* beacon, float rssi, float distance );
    ~BeaconMeas();

    // copy constructor
    BeaconMeas( const BeaconMeas& );

    float getRssi()const;                            //get RSSI of current measurement
    float getDistance()const;                        //get distance of current measurement
    const std::string& getBeaconId()const;                   //get beacon identifier
    const Beacon* getBeaconPtr()const;                      //get pointer to beacon

    void setRssi( const float );                     //set measurement RSSI
    void setDistance( const float );                 //set measurement distance
    void setBeaconPtr( const Beacon* beaconPtr );     //set pointer to beacon (from which we got meas)
    void setBeaconId( const std::string& beaconId );  //set beacon it (that identifies measurement)

    bool operator<(const BeaconMeas& entry)const;
    bool operator>(const BeaconMeas& entry)const;
    bool operator==(const BeaconMeas& entry)const;
    bool operator!=(const BeaconMeas& entry)const;

  private:
    std::string  mBeaconId;     // id that allow us identify beacon and set mBeaconPtr
    Beacon*      mBeaconPtr;    // pointer to beacon from which we got measurement
    float       mRssi;         // RSSI of the measurement
    float       mDistance;     // Distance to the beacon
};

#endif // NAVIGINE_BEACON_CLASS