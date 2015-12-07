#pragma once

#include "ofMain.h"
#include "ofxGui.h"
//#include "ofxSpatialHash.h"
#include "ofxGeo.h"
#include "ofxMaps.h"
#include "ofxPd.h"

#include <vector>                                   // for the data array
#include <chrono>                                   // timers
#include <sstream>                                  // for the to_string function
#include <time.h>                                   // for the get_date function
#include <string>                                   // for the get_date function
#include <iostream>


// number of sounds
#define totalSoundNumber 1

// Buffersizes of mean Filter
#define azimuthBufferSize 7
#define GPSbufferSize 49

#ifdef TARGET_ANDROID
    #include "ofxAndroid.h"
    #include "ofxAndroidGPS.h"
    #include "ofxAccelerometer.h"
    #include "ofxOrientation.h"
#endif

// load the pd external
extern "C"{
    void headShadow_setup();
}

// a namespace for the Pd types
using namespace pd;
using namespace ofx;


namespace patch_string
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

class ofApp : public ofBaseApp, public PdReceiver, public PdMidiReceiver {

    public:
        void setup();
        void update();
        void draw();
        void close();

        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void mouseEntered(int x, int y);
        void mouseExited(int x, int y);
        void windowResized(int w, int h);
        void dragEvent(ofDragInfo dragInfo);
        void gotMessage(ofMessage msg);

        void touchDown(int x, int y, int id);
        void touchDoubleTap(int x, int y, int id);

        void pause();
        void stop();
        void resume();
        void reloadTextures();

        bool backPressed();
        void okPressed();
        void cancelPressed();

        double computeAzimuth(Geo::Coordinate soundGPScoord);
        float meanAzimuth(float azValue);
        double meanGPS(double GPSvalue);
        std::pair<float, float> coordinates_at_distance_angle(float lat, float lon, float dist, float angle);


        ofTrueTypeFont font;
        ofVec3f accel, normAccel;
        ofVec3f currentOrientation;
        string curr_or[3];
        string messages[3];

        // define pairwise coordinates couples
        Geo::Coordinate myGPScoord;
        Geo::Coordinate myLastGPScoord;
        Geo::Coordinate RAW_GPScoord;

        //Geo::Coordinate soundGPScoord;
        Geo::Coordinate* soundGPScoords = new Geo::Coordinate[totalSoundNumber]; //define an array that consists of soundCoordinates
        Geo::Coordinate n;
        Geo::Coordinate wCoord;
        Geo::Coordinate sCoord;
        Geo::Coordinate nCoord;
        // define variable to store distance
        //double distance;
        double bearingHaversine;
        double bearingHaversineDegree;
        double actualOrientation;
        //double azimuth;
        double actualOrientationDegree;
        double currentOrientationRadians;
        double pitagDist;
        //int bearingHaversine;
        //int actualOrientation; // round the angle to int
        double nX, nY;

        double wLat, wLong, sLat, sLong, nLat, nLong;
        double  b, a, c, beta, gamma, theta;

        double distance[totalSoundNumber];
        double azimuth[totalSoundNumber];

        double soundplayedflags[totalSoundNumber];

		// variables for azimuth mean calculation
		int runnerAz;
		float azimuthBuffer[azimuthBufferSize];
		float intermediateAzimuth[totalSoundNumber];
        float intermediateDistance[totalSoundNumber];
        float lastDistance[totalSoundNumber];
        //const int trialArray[7] = {0, 1, 2, 3, 4, 5, 6};
		
        int runnerGPS;
        float GPSbuffer[GPSbufferSize];

        // gps outlier detection
        bool firstiteration = 1;
        float lastlatitude;
        float lastlongitude;
        float threshold = 30;

        //counter for location triggering
        int soundLocationCounter = 0;

        //Trial statistics
        int trialState = 0;
        int trialNumber = 0;
        int earconPlayed;
        Geo::Coordinate* training_soundGPScoords = new Geo::Coordinate[2];
        int prev_ang;
        int button_pressed = 0;
        int training_num = 0;
        int training_ang[2] = {150, 290};

    #ifdef TARGET_ANDROID
      static const bool isMobileVersion = true;
      void locationChanged(ofxLocation& location);
      ofxAndroidGPS gpsCtrl;
    #else
      static const bool isMobileVersion = false;
    #endif
      int gpsStatus = 0;



      // for pd

    float length_float;


    // define pd functions
    // do something
    void playTone(int pitch);

    // audio callbacks
    void audioReceived(float * input, int bufferSize, int nChannels);
    void audioRequested(float * output, int bufferSize, int nChannels);

    // pd message receiver callbacks
    void print(const std::string& message);

    void receiveBang(const std::string& dest);
    void receiveFloat(const std::string& dest, float value);
    void receiveSymbol(const std::string& dest, const std::string& symbol);
    void receiveList(const std::string& dest, const List& list);
    void receiveMessage(const std::string& dest, const std::string& msg, const List& list);

    // pd midi receiver callbacks
    void receiveNoteOn(const int channel, const int pitch, const int velocity);
    void receiveControlChange(const int channel, const int controller, const int value);
    void receiveProgramChange(const int channel, const int value);
    void receivePitchBend(const int channel, const int value);
    void receiveAftertouch(const int channel, const int value);
    void receivePolyAftertouch(const int channel, const int pitch, const int value);

    void receiveMidiByte(const int port, const int byte);

    ofxPd pd;
    vector<float> scopeArray;
    vector<Patch> instances;

    int midiChan;

    // for the data array
    std::vector<string> dv_timestamps;                 // data vector time stamps
    std::vector<string> dv_fil_distance;
    std::vector<string> dv_raw_distance;
    std::vector<string> dv_fil_azimuth;
    std::vector<string> dv_raw_azimuth;
    std::vector<string> dv_myLat;
    std::vector<string> dv_myLong;
    std::vector<string> dv_myCurrOrr;
    std::vector<string> dv_RAWLat;
    std::vector<string> dv_RAWLong;
    std::vector<string> dv_soundLat;
    std::vector<string> dv_soundLong;

    // Printing trial statistics
    std::vector<string> dv_trialState;
    std::vector<string> dv_trialNumber;
    std::vector<string> dv_earconPlayed;

    // define timers
    std::chrono::time_point<std::chrono::system_clock> chrono_start, chrono_now;
    std::chrono::duration<double> elapsed_seconds;

    // create text file buffer
    ofBuffer dataBuffer;
};
