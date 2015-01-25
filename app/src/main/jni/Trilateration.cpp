/** trilateration.cpp
 *
 * Author: Aleksei Smirnov <aleksei.smirnov@navigine.ru>
 * Copyright (c) 2014 Navigine. All rights reserved.
 *
 */
#include <algorithm>
#include <android/log.h>

#define  LOG_TAG    "Trilateration"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include "trilateration.h"

Trilateration::Trilateration()
: mCurLocationId(0)
{
}

Trilateration::~Trilateration()
{
}

Trilateration * Trilateration::GetThe()
{
    static Trilateration The;
    return &The;
}

void Trilateration::updateMeasurements( std::vector < BeaconMeas >& beaconMeas )
{
  mBeaconMeas = beaconMeas;
}

void Trilateration::fillLocationBeacons( std::vector <Beacon>& beaconsOnFloor )
{
  mLocationBeacons = beaconsOnFloor;
}

int Trilateration::calculateCoordinates()
{
  int errorCode = deleteDuplicateMeasurements( mBeaconMeas );
  if (errorCode) return errorCode;
  std::sort( mBeaconMeas.begin(), mBeaconMeas.end() );
  filterUnknownBeacons (mBeaconMeas, mLocationBeacons);

  if (mBeaconMeas.size() < 3)
  {
    LOGE("The number of visible beacon = %d \n", static_cast<int>(mBeaconMeas.size()));
    return ERROR_NO_SOLUTION_TRILATERATION;
  }

  errorCode = calculateTrilaterationCoordinates();
  if (errorCode) return errorCode;

  return 0;
}

int Trilateration::getCurrentLocationId() const
{
  return mCurLocationId;
}

void Trilateration::setCurrentLocationId( int curSubloc )
{
  mCurLocationId = curSubloc;
}

void Trilateration::filterUnknownBeacons( std::vector <BeaconMeas>& beaconMeas, const std::vector<Beacon>& mapBeacons )
{
    std::vector<BeaconMeas> done(beaconMeas.size());
    std::vector<BeaconMeas>::iterator it;
    it = std::set_intersection(beaconMeas.begin(), beaconMeas.end(), mapBeacons.begin(), mapBeacons.end(), done.begin(),
        compareBeaconMeasToBeacon());
    done.resize(it - done.begin());
    beaconMeas = done;
}

void Trilateration::addDeviceSeen(const BeaconMeas& beaconMeasure)
{

}

float Trilateration::getX() const
{
  return mXY.at(0);
}

float Trilateration::getY() const
{
  return mXY.at(1);
}

int Trilateration::calculateTrilaterationCoordinates()
{
  float normalizeCoefficient = 0.0f;
  //take revert values, because lower distance then bigger weight
  std::vector<BeaconMeas>::const_iterator citer = mBeaconMeas.begin();
  for (; citer != mBeaconMeas.end(); ++citer)
    normalizeCoefficient += 1.0f / fabs( citer->getDistance() );

  std::vector<float> weight( mBeaconMeas.size(), 0.0 );

  for (citer = mBeaconMeas.begin(); citer != mBeaconMeas.end(); ++citer)
  {
    if (citer->getBeaconPtr() == 0)
    {
      LOGE( "ERROR: BeaconMes it =%s : Beacon ptr == NULL\n", citer->getBeaconId().c_str() );
      return ERROR_IN_TRILATER;
    }
    // calculate probability of being at beacons x,y coordinates
    weight[ citer - mBeaconMeas.begin() ] += 1.0f / (fabs( citer->getDistance() *
      normalizeCoefficient ));

    float beaconX = 0, beaconY = 0;
    beaconX = citer->getBeaconPtr()->getX();
    beaconY = citer->getBeaconPtr()->getY();

    //find final coordinates according to probability
    mXY[ 0 ] += weight[ citer - mBeaconMeas.begin() ] * beaconX;
    mXY[ 1 ] += weight[ citer - mBeaconMeas.begin() ] * beaconY;
  }
  return 0;
}

int Trilateration::deleteDuplicateMeasurements( std::vector<BeaconMeas>& beaconMeas )
{
    std::vector<BeaconMeas>::iterator it = beaconMeas.begin();
    it = std::unique( beaconMeas.begin(), beaconMeas.end(), compareBeaconMeasByName );
    beaconMeas.resize( std::distance(beaconMeas.begin(), it));

    if (beaconMeas.size() < 3)
    {
        LOGE( "less then 3 visible beacons in measurements It's not enough for trilateration" );
        return ERROR_NO_SOLUTION_TRILATERATION;
    }
    return 0;
}

void Trilateration::getLinearSystem( std::vector<float> &matrixA, std::vector<float> &b, int dim )
{
  float firstBeaconDistance = powf(mBeaconMeas.begin()->getDistance(), 2);
  float normaFirstBeacon = powf(mBeaconMeas.begin()->getBeaconPtr()->getX(), 2)
    + powf(mBeaconMeas.begin()->getBeaconPtr()->getY(), 2);

  std::vector<BeaconMeas>::const_iterator iter = mBeaconMeas.begin() + 1;
  for(; iter != mBeaconMeas.end(); iter++)
  {
    int i = iter - mBeaconMeas.begin();
    matrixA[ i * dim ] = 2 * (iter->getBeaconPtr()->getX() - mBeaconMeas.begin()->getBeaconPtr()->getX());
    matrixA[ i * dim  + 1 ] = 2 * ( iter->getBeaconPtr()->getY() - mBeaconMeas.begin()->getBeaconPtr()->getY());

    float norma = powf(iter->getBeaconPtr()->getX(), 2) + powf(iter->getBeaconPtr()->getY(), 2);
    b[ i ] = firstBeaconDistance - powf(iter->getDistance(), 2) - normaFirstBeacon + norma;
  }
}

void Trilateration::solveLinearSystem( std::vector<float> matrixA, std::vector <float> b )
{
  int nOfEquations = b.size();
  int dim = matrixA.size() / nOfEquations;

  std::vector <float> xy(dim, 0.0f);
  std::vector <float> aTransposeA(dim * dim, 0.0f);
  std::vector <float> revertMatrix(dim * dim, 0.0f);
  std::vector <float> matrix2xN(dim * nOfEquations, 0.0f);

  for (int row = 0; row < dim; row++)
  {
    for (int col = 0; col < dim; col++)
    {
      for ( int inner = 0; inner < nOfEquations; inner++ )
      {
        aTransposeA[ row * dim + col ] +=
          matrixA[ inner * dim + row ] * matrixA[ inner * dim + col ];
      }
    }
  }

  float det = aTransposeA[ 0 ] * aTransposeA[ 3 ] -
    aTransposeA[ 2 ] * aTransposeA[ 1 ];

  revertMatrix[0] =  aTransposeA[3] / det;
  revertMatrix[1] = -aTransposeA[1] / det;
  revertMatrix[2] = -aTransposeA[2] / det;
  revertMatrix[3] =  aTransposeA[0] / det;

  for ( int row = 0; row < dim; row++ )
  {
    for ( int col = 0; col < nOfEquations; col++ )
    {
      for ( int inner = 0; inner < dim; inner++ )
      {
        matrix2xN[ row * nOfEquations + col ] +=
          revertMatrix[ row * dim + inner] * matrixA[ col * dim + inner ];
      }
    }
  }

  //Multiply matrix2xN on B vector
  for ( int col = 0; col < dim; col++ )
  {
    for ( int inner = 0; inner < nOfEquations; inner++ )
    {
      xy[col] += matrix2xN[col * nOfEquations + inner] * b[inner];
    }
  }
}

bool compareBeaconMeasToBeacon::operator()(const Beacon &first, const BeaconMeas &second)
{
    return first.getId() == second.getBeaconId();
}
bool compareBeaconMeasToBeacon::operator()(const BeaconMeas &first, const Beacon &second)
{
    return first.getBeaconId() == second.getId();
}

bool compareBeaconMeasByName( BeaconMeas first, BeaconMeas second )
{
  return first.getBeaconId() == second.getBeaconId();
}

std::vector<Beacon>::const_iterator findBeaconForMeas( const std::vector<Beacon>& mapBeacons,
                                                       const std::string& measureBeaconId )
{
  std::vector<Beacon>::const_iterator it = mapBeacons.begin();
  for (; it != mapBeacons.end(); ++it)
  {
    if (it->getId() == measureBeaconId)
      return it;
  }
  return it;
}