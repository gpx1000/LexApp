/** trilateration.h
 *
 * Author: Aleksei Smirnov <aleksei.smirnov@navigine.ru>
 * Copyright (c) 2014 Navigine. All rights reserved.
 */

#ifndef TRILATERATION_ALGORITHM
#define TRILATERATION_ALGORITHM

#include <string>
#include <math.h>
#include <vector>
#include "beacon.h"

static const int ERROR_NO_SOLUTION_TRILATERATION = 4;
static const int ERROR_IN_TRILATER = 28;

class Trilateration
{
  public:
    void updateMeasurements( std::vector < BeaconMeas >& beaconMeasurements );
    void addDeviceSeen(const BeaconMeas& beaconMeasure);
    void fillLocationBeacons( std::vector <Beacon>& beaconsOnFloor );
    int calculateCoordinates( );
    int getCurrentLocationId() const;
    void setCurrentLocationId( int curLoc );
    static void filterUnknownBeacons( std::vector <BeaconMeas>& beaconMeas, const std::vector<Beacon>& mapBeacons );
    static Trilateration * GetThe();
    float getX() const;
    float getY() const;

  private:
    Trilateration();
    ~Trilateration();
    std::vector <Beacon>     mLocationBeacons;
    std::vector <BeaconMeas> mBeaconMeas;
    std::vector <float>      mXY;
    int                      mCurLocationId;
    int calculateTrilaterationCoordinates( );
    int deleteDuplicateMeasurements( std::vector<BeaconMeas>& BeaconMeasurements );
    void getLinearSystem( std::vector<float> &matrixA, std::vector<float> &b, int dim );
    void solveLinearSystem( std::vector<float> matrixA, std::vector <float> b );
};

bool compareBeaconMeasByName(BeaconMeas first, BeaconMeas second );
struct compareBeaconMeasToBeacon
{
    bool operator()( const Beacon &first, const BeaconMeas &second );
    bool operator()( const BeaconMeas &first, const Beacon &second );
};
std::vector<Beacon>::const_iterator findBeaconForMeas(const std::vector<Beacon>& mapBeacons,
                             const std::string& measureBeaconId );

#endif