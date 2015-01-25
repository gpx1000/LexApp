# LexApp
Giving Through Glass application
Classroom Champions nonprofit volunteer project.

This is an application to help Lex, a blind athlete, paraolympian.  It uses Google Glass' array of sensors to facilitate independant navigation and sight via sensory substitution.

The project owes the SSD (Sensory Substitution Device) inspired portion to seeingwithsound.com and code from http://www.seeingwithsound.com/hificode_OpenCV.cpp.
The version in this project is heavily optimized over the version presented in the hificode_OpenCV.cpp link above.

The second part of the project is indoor navigation.  Main algorithms are from https://github.com/Navigine/Indoor-navigation-algorithms.
The intent of this portion is to allow for glass to use trilateration from known location broadcasting BLE beacons.
By doing trilateration, a cartesian XY coordinate is determined by probing the strength of RF single from broadcast beacons or RFFI.
From this information, RSSI + distance measured from bluetooth client, we create an area of probability that the listening device is located by using a minimum of three beacons.
Beacon location information is pulled from Google Maps as placed in markers and physically in the correctly corresponding lat/long indoor location.
When the XY coordinate is determined, it then will attempt to play auditory cues for direction of left and right by a sound wave out of stereo headphones.
Location information can be further refined by adding a pedometer calculation based upon compass, gyro, and accellerometer as part of future work.
Obstical avoidance in the future will also be aided by optical flow for obstical avoidance and localization can be further aided by SLAM.
