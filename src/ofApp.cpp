#include "ofApp.h"

std::string get_date(void)
{
    #define MAX_DATE 12
   time_t now;
   char the_date[MAX_DATE];

   the_date[0] = '\0';

   now = time(NULL);

   if (now != -1)
   {
      // strftime(the_date, MAX_DATE, "%d_%m_%Y_%R", gmtime(&now));
    strftime(the_date, MAX_DATE, "%d%H%M%S", gmtime(&now));
      // "%A, %B %d %Y. The time is %X"; 
   }

   return std::string(the_date);
}

#ifdef TARGET_ANDROID
double log2( double n )
{
    // log(n)/log(2) is log2.
    return log( n ) / log( 2 );
}

//--------------------------------------------------------------
void ofApp::locationChanged(ofxLocation& location) {
    ofLogNotice() << "locationChanged: " << location ;
    gpsStatus = 2;    
    myGPScoord = Geo::Coordinate(location.latitude, location.longitude);
    RAW_GPScoord = Geo::Coordinate(location.latitude, location.longitude);

    float timetocalibrate =  15; // seconds

    if(elapsed_seconds.count() > timetocalibrate){
        // outlier filtering - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // firstiteration is a bool initialized at 1
        if (firstiteration == 1) {                                          // in the first iteration
            
            lastlatitude = location.latitude;
            lastlongitude = location.longitude;
            myLastGPScoord = Geo::Coordinate(lastlatitude, lastlongitude);
            
            firstiteration = 0;
        }
        //(abs(location.latitude - lastlatitude) + abs(location.longitude - lastlongitude))
        if ((abs((Geo::GeoUtils::distanceHaversine(myGPScoord, myLastGPScoord)))*1000.0) > threshold){             // if there is an outlier,
            myGPScoord = Geo::Coordinate(lastlatitude, lastlongitude);
            //location.latitude = lastlatitude;
            //location.longitude = lastlongitude;                               // use last- as distance
        }
        else{
            myLastGPScoord = Geo::Coordinate(location.latitude, location.longitude);
            lastlatitude = location.latitude; 
            lastlongitude = location.longitude;                              // update last-
        }
        
        // if (abs(location.longitude - lastlongitude) > threshold){             // if there is an outlier,
        //     location.longitude = lastlongitude;                               // use last- as distance
        // }
        // else{
        //     lastlongitude = location.longitude;
        // }
    }
}
#endif
// void ofApp::locationChanged(ofxLocation& location) {
//     ofLogNotice() << "locationChanged: " << location ;
//     gpsStatus = 2;
//     myGPScoord = Geo::Coordinate(location.latitude, location.longitude);
//     // soundGPScoord = Geo::Coordinate(55.650592, 12.540861); // car parking in front of FKJ15
//     // sounds are inside the car parking in front of FKJ 15
//     soundGPScoords[0] = Geo::Coordinate(55.650592, 12.540861); // car parking in front of FKJ15
//     // soundGPScoords[1] = Geo::Coordinate(55.650341, 12.540794);
//     // soundGPScoords[2] = Geo::Coordinate(55.650785, 12.541061);
//     // soundGPScoords[3] = Geo::Coordinate(55.650979, 12.540536);
//     // soundGPScoords[4] = Geo::Coordinate(55.650974, 12.541018);
//     // here extra code, if needed, to use myGPScoord
// }
// #endif

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60); //set framerate in order to display coordinates
    ofBackground(255,255,255);
    font.load("verdana.ttf",20);	// adding font file, check to have it in your folder
    earconPlayed = 0;
    // android setup
    #ifdef TARGET_ANDROID
        gpsCtrl.startGPS();
        gpsStatus = 1;
        ofRegisterGPSEvent(this);
        ofxOrientation.setup();
        ofxAccelerometer.setup();

        // soundGPScoord = Geo::Coordinate(55.650592, 12.540861); // car parking in front of FKJ15

        // sounds are inside the car parking in front of FKJ 15
        // we have to make sure that these fixed coordinates are on the circle with tha radius 50 meters from our starting point
        training_soundGPScoords[0] = Geo::Coordinate(55.651020, 12.540936);
        training_soundGPScoords[1] = Geo::Coordinate(55.650341, 12.540794);


        soundGPScoords[0] = Geo::Coordinate(55.650592, 12.540861); // car parking in front of FKJ15

        //soundGPScoords[1] = Geo::Coordinate(55.650341, 12.540794);
        //soundGPScoords[2] = Geo::Coordinate(55.650785, 12.541061);
        //soundGPScoords[3] = Geo::Coordinate(55.650979, 12.540536);
        //soundGPScoords[4] = Geo::Coordinate(55.650974, 12.541018);
        // here extra code, if needed, to use myGPScoord

        // logcat debugging
        ofLogNotice("ofApp") << "distances are computed: ";
        ofLogNotice("ofApp") << "azimuth are computed: ";

    #endif

    for(int i=0; i<totalSoundNumber; i++){
        //setting flags to zero. if sound was played or not.
        soundplayedflags[i] = 1;
    }


    // setup OF sound stream
    int nOutputs = 2;                                   // number of output channels
    int numInputs = 1;                                  // number of input channels
    int ticksPerBuffer = 8;                             // compute the number of libpd ticks per buffer used to
    int bufferSize = ofxPd::blockSize()*ticksPerBuffer; // compute the audio buffer len: tpb * blocksize (always 64)
                                                        // 8 * 64 = buffer len of 512 samples
    int sampleRate = 44100; //
    int nBuffers = 4;                                   // number of buffers that your system will create and swap out.
                                                        // The more buffers, the faster your computer will write information into the buffer,
                                                        // but the more memory used. You should probably use two for each channel that youre using.

    ofSoundStreamSetup(nOutputs, numInputs, this, sampleRate, bufferSize, 3);

    //allocate and initialize variables
    // setup variables used for the buffers
	runnerAz = 0;
    runnerGPS = 0;
    // float azimuthBuffer [azimuthBufferSize] = {0, 0, 0, 0, 0};
	// float GPSbuffer [GPSbufferSize] = {0, 0, 0, 0};
    // setup Pd
    //
    // set 4th arg to true for queued message passing using an internal ringbuffer,
    // this is useful if you need to control where and when the message callbacks
    // happen (ie. within a GUI thread)
    //
    // note: you won't see any message prints until update() is called since
    // the queued messages are processed there, this is normal
    //
    if(!pd.init(2, numInputs, 44100, ticksPerBuffer, false)) {
        OF_EXIT_APP(1);
    }

    // load externals
    headShadow_setup();

    // add the data/pd folder to the search path
    pd.addToSearchPath("pd/abs");

    // audio processing on
    pd.start();

    // open patch
    Patch patch = pd.openPatch("pd/soundenginemain.pd");

    // write header to text file
    dataBuffer.append("timestamps, raw distance, filtered distance, raw azimuth, filtered azimuth, Latitude, Longitude, RAW_LAT, RAW_LONG, Sound_LAT, Sound_LONG, currentOrientation, trialState, trialNumber, earconPlayed \n");

    // get starting time
    chrono_start = std::chrono::system_clock::now();
}

void ofApp::update(){
    ofLogNotice("ofApp") << "Starting update";
    //current Orientation
    curr_or[0] = "orr(x) = " + ofToString(currentOrientation.x,2);
    curr_or[1] = "orr(y) = " + ofToString(currentOrientation.y,2);
    curr_or[2] = "orr(z) = " + ofToString(currentOrientation.z,2);
    //Accelerometer
    // messages[0] = "g(x) = " + ofToString(accel.x,2);
    // messages[1] = "g(y) = " + ofToString(accel.y,2);
    // messages[2] = "g(z) = " + ofToString(accel.z,2);
    // normAccel = accel.getNormalized(); // not needed
    #ifdef TARGET_ANDROID
        accel = ofxAccelerometer.getOrientation();
        currentOrientation = ofxOrientation.getOrientation();
    #endif
    // compute the azimuth for each sound 
    for(int i=0; i<totalSoundNumber; i++){
        intermediateDistance[i] = Geo::GeoUtils::distanceHaversine(soundGPScoords[i],myGPScoord)*1000.0;
        distance[i] = meanGPS(intermediateDistance[i]);
        if(distance[i]<70 && earconPlayed == 0){
            intermediateAzimuth[i] = computeAzimuth(soundGPScoords[i]);  //here we define a local method called computeAzimuth
            azimuth[i] = meanAzimuth(intermediateAzimuth[i]);
            // sending data to PD
            // ofLogNotice("ofApp") << "Send data to PD";
            pd.startMessage();
            pd.addFloat(i+1);                                           // sound index
            pd.addFloat(1);                                // on/off
            pd.addFloat(distance[i]);                                   // distance
            pd.addFloat(azimuth[i]);                                    // azimuth
            pd.finishList("play");
            soundplayedflags[i] = 1;
            // dont play the earcon
            // pd.startMessage();
            // pd.addFloat(2);                                             // sound index
            // pd.addFloat(0);                                             // on/off
            // pd.addFloat(1);                                   // distance
            // pd.addFloat(0);                                    // azimuth
            // pd.finishList("play");
            //lastDistance[i]=distance[i];
            if((distance[i]<7) && (earconPlayed == 0)){
                pd.sendBang("earconbang");
                // turn off target sound 
                pd.startMessage();
                pd.addFloat(i+1);                                             // sound index
                pd.addFloat(0);                                             // on/off
                pd.finishList("play");
                earconPlayed = 1;
                trialState = 0;

            }
        }
        else if(soundplayedflags[i]==1){
            // if((distance[i]-lastDistance[i])>10){
            //     pd.startMessage();
            //     pd.addFloat(i+1);                                           // sound index
            //     pd.addFloat(1);                                // on/off
            //     pd.addFloat(lastDistance[i]);                                   // distance
            //     pd.addFloat(azimuth[i]);                                    // azimuth
            //     pd.finishList("play");
            //     soundplayedflags[i] = 1;
            // }
            // else{
            pd.startMessage();
            pd.addFloat(i+1);                                           // sound index
            pd.addFloat(0);                                             // on/off
            //pd.addFloat(distance[i]);                                   // distance
            //pd.addFloat(azimuth[i]);                                    // azimuth
            pd.finishList("play");
            soundplayedflags[i] = 0;  
            // // dont play the earcon
            // pd.startMessage();
            // pd.addFloat(2);                                             // sound index
            // pd.addFloat(0);                                             // on/off
            // pd.addFloat(1);                                   // distance
            // pd.addFloat(0);                                    // azimuth
            // pd.finishList("play");
            // //}
        }
      }

    
    
    // compute time stamps
    
    // get time now
    chrono_now = std::chrono::system_clock::now();
    // get elapsed seconds
    elapsed_seconds = chrono_now-chrono_start;

    // add data to arrays - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    dv_timestamps.push_back(patch_string::to_string(elapsed_seconds.count()));  // timestamps
    dv_raw_distance.push_back(patch_string::to_string(intermediateDistance[0]));// raw distance
    dv_fil_distance.push_back(patch_string::to_string(distance[0]));            // filtered distance
    dv_raw_azimuth.push_back(patch_string::to_string(intermediateAzimuth[0]));  // raw azimuth
    dv_fil_azimuth.push_back(patch_string::to_string(azimuth[0]));              // filtered azimuth

    dv_myLat.push_back(patch_string::to_string(myGPScoord.getLatitude()));      // Latitude
    dv_myLong.push_back(patch_string::to_string(myGPScoord.getLongitude()));    // Longitude

    dv_RAWLat.push_back(patch_string::to_string(RAW_GPScoord.getLatitude()));      // RAW Latitude
    dv_RAWLong.push_back(patch_string::to_string(RAW_GPScoord.getLongitude()));    // RAW Longitude

    dv_soundLat.push_back(patch_string::to_string(soundGPScoords[0].getLatitude()));      // RAW Latitude
    dv_soundLong.push_back(patch_string::to_string(soundGPScoords[0].getLongitude()));    // RAW Longitude

    dv_myCurrOrr.push_back(patch_string::to_string(currentOrientation.x));    // Current Orientation

    dv_trialState.push_back(patch_string::to_string(trialState));                       // trialState
    dv_trialNumber.push_back(patch_string::to_string(trialNumber));                     // trialNumber
    dv_earconPlayed.push_back(patch_string::to_string(earconPlayed));

    // debugging raw data and mean data
    ofLogNotice("ofApp") << "elapsed_seconds: " << patch_string::to_string(elapsed_seconds.count()) << "," << " rawDist: " << intermediateDistance[0] << "," << " Dist: " << distance[0] << "," << " rawAz: " << intermediateAzimuth[0] << "," << " Az: " << azimuth[0] << "," << "outlierDist: " << lastDistance[0];
    
    // one printing per line
    // ofLogNotice("ofApp") << "elapsed_seconds: " << patch_string::to_string(elapsed_seconds.count());
    // ofLogNotice("ofApp") << "rawDist: " << intermediateDistance[0]; 
    // ofLogNotice("ofApp") << "Dist: " << distance[0];
    // ofLogNotice("ofApp") << "rawAz: " << intermediateAzimuth[0];
    // ofLogNotice("ofApp") << "Az: " << azimuth[0];
}

//--------------------------------------------------------------
void ofApp::close(){
#ifdef TARGET_ANDROID
    gpsCtrl.stopGPS(); // stop GPS when application closed
#endif
}

//--------------------------------------------------------------
void ofApp::draw(){
    // draw the background colors:
    float heightDiv = ofGetHeight() / 3.0f;
    float widthDiv = ofGetWidth() / 2.0f;
    ofSetHexColor(0xffffff); // white
    ofDrawRectangle(0,0,ofGetWidth(),heightDiv); 
    ofSetHexColor(0xeeeeee); // lite grey
    ofDrawRectangle(0,heightDiv,ofGetWidth(),heightDiv); 
    ofSetHexColor(0xdddddd); // grey
    ofDrawRectangle(0,heightDiv*2,ofGetWidth(),heightDiv);
    
    if(button_pressed == 1){
        ofSetHexColor(0x00ff00); //green
        ofDrawRectangle(widthDiv,0,ofGetWidth(),heightDiv);
    }
    else{
        ofSetHexColor(0xcc362e); // red
        ofDrawRectangle(widthDiv,0,ofGetWidth(),heightDiv);
    }

    


    string myReport = "";
    string myDistance = "";
    string myAngle = "";
    string sndLocation = "";
    string myBearing = "";
    string myAngleToNorth = "";
    string myAngleToNorthRadians = "";
    string debVar = "";
    string myPit = "";
    string myBdeg = "";

    switch (gpsStatus){
        case 0:
            myReport = "Desktop version - GPS not present";
            break;
        case 1:
            myReport = "Mobile version - GPS offline";
            break;
        case 2:
            //myReport = "M.V. GPS coord: " + ofToString(myGPScoord);
            //sndLocation = "M.V. Sound coord: " + ofToString(soundGPScoords[0]);
            //myDistance = "distance: " + ofToString(distance[0]) + " m";
            //myAngle = "Theta Radians: " + ofToString(azimuth[0]); //+ " \nTheta Degree: " + ofToString(round(actualOrientationDegree));

            //myBearing = "Alfa: " + ofToString(bearingHaversine);
            //myBdeg = "Alfa degree: " + ofToString(bearingHaversineDegree);
            //myAngleToNorth = "Angle to North: " + ofToString(round(currentOrientation.x)) + " degrees";
            //myAngleToNorthRadians = "Angle Radians to North: " + ofToString(currentOrientationRadians);

            // debugging variable
            // debVar = "distance[1]: " + ofToString(distance[1]) + "\nThetaR[1]: " + ofToString(azimuth[1]) + "\ndistance[2]: " + ofToString(distance[2]) + "\nThetaR[2]: " + ofToString(azimuth[2]) + "\ndistance[3]: " + ofToString(distance[3]) + "\nThetaR[3]: " + ofToString(azimuth[3]) + "\ndistance[4]: " + ofToString(distance[4]) + "\nThetaR[4]: " + ofToString(azimuth[4]); //+ "\ndistance[5]: " + ofToString(distance[5]) + "\nThetaR[5]: " + ofToString(azimuth[5]);
            //debVar = "intermediateDis: " + ofToString(intermediateDistance[0]) + "\ndistance: " + ofToString(distance[0]) + "\nintermediateAzimuth: " + ofToString(intermediateAzimuth[0]) + "\nazimuth: " + ofToString(azimuth[0]) + "\nmyLastGPScoord: " + ofToString(myLastGPScoord) + "\nsoundLocationCounter:" + ofToString(soundLocationCounter) + "\ntrialState: " + ofToString(trialState) + "\ntrialNumber: " + ofToString(trialNumber) + "\nearconPlayed: " + ofToString(earconPlayed); 
            debVar = "trialState: " + ofToString(trialState) + "\ntrialNumber: " + ofToString(trialNumber) + "\nearconPlayed: " + ofToString(earconPlayed);

            break;
    }
    ofSetHexColor(000000);


	// print with no font
   	// ofDrawBitmapString(myReport, 100, 100);
   	// ofDrawBitmapString(sndLocation, 100, 125);
   	// ofDrawBitmapString(myAngleToNorth, 100, 150);
   	// ofDrawBitmapString(myDistance, 100, 175);
   	// ofDrawBitmapString(myAngle, 100, 200);
   	// // debugging
   	// ofDrawBitmapString(debVar, 100, 350);

//    ofDrawBitmapString(myPit,100,225);
//    ofDrawBitmapString(myBearing, 100, 300);
//    ofDrawBitmapString(myBdeg, 100,325);

	// print with Font
	//font.drawString(myReport,100,100);
	// font.drawString(sndLocation,100,125);
	// font.drawString(myAngleToNorth,100,150);
	// font.drawString(myDistance,100,175);
	// font.drawString(myAngle,100,200);
	font.drawString(debVar,100,100);
    font.drawString("TAP TWICE TO START TRAINING TRIAL",100,heightDiv+50);
    font.drawString("TAP TWICE TO START NEXT TEST TRIAL!",100,heightDiv*2+50);
}

//--------------------------------------------------------------
//method to compute the Azimuth - maybe we should make it private since it is only used internally
double ofApp::computeAzimuth(Geo::Coordinate soundGPScoord){
    //new algorithm to get actualOrientation (theta) using asin
    //new position of W - it is our projected position
    wLat = myGPScoord.getLatitude() - myGPScoord.getLatitude();
    wLong = myGPScoord.getLongitude() - myGPScoord.getLongitude();
    wCoord = Geo::Coordinate(wLat,wLong);

    // new position of S - it is sound projected prosition
    sLat = soundGPScoord.getLatitude() - myGPScoord.getLatitude();
    sLong = soundGPScoord.getLongitude() - myGPScoord.getLongitude();
    sCoord = Geo::Coordinate(sLat,sLong);

    // new position of N - new point to compute beta
    nLat = wLat;
    nLong = abs(sLong);
    nCoord = Geo::Coordinate(nLat,nLong);

    //angles we need
    b = Geo::GeoUtils::distanceHaversine(nCoord,sCoord);
    a = Geo::GeoUtils::distanceHaversine(nCoord,wCoord);
    c = Geo::GeoUtils::distanceHaversine(wCoord,sCoord);

    //compute gamma
    gamma = asin(a/c);

    //compute beta
    if(sLat > wLat && sLong > wLong)
        beta = 90 - gamma;
    else if(sLat < wLat && sLong > wLong)
        beta = 180 - gamma;
    else if(sLat < wLat && sLong < wLong)
        beta = 270 - gamma;
    else if(sLat > wLat && sLong < wLong)
        beta = 360 - gamma;

    //compute theta
    if (beta > 180)
        if (abs(beta - currentOrientation.x) > 180)
            theta = (abs(beta - currentOrientation.x)) - 360;
        else
            theta = beta - currentOrientation.x;
    else
        if (abs(beta - currentOrientation.x) > 180)
            theta = abs((abs(beta - currentOrientation.x)) - 360);
        else
            theta = beta - currentOrientation.x;


    actualOrientation = round((theta * (PI/180))*1000) / 1000; //radians
    actualOrientationDegree = theta ; //degree

    return actualOrientation;
}

//--------------------------------------------------------------
// method to smooth the azimuth signal using mean calculation
// first is the current azimuth value
float ofApp::meanAzimuth(float azValue){
	float out = 0;
    int polarity = 0;
    int polarityChanged =0;
    int higherThanThree = 0;
    int negative=0;
    int positive =0;
	runnerAz = runnerAz%azimuthBufferSize;
	azimuthBuffer[runnerAz] = azValue;
    //check if polarity changed within the buffer and if higher than 3.0
    for(int i=0; i<azimuthBufferSize; i++){
        if(abs(azimuthBuffer[i])>=3.0)
            higherThanThree=1;
        if(azimuthBuffer[i]<0)
            negative = 1;
        else 
            positive = 1;
    }
    if(negative == 1 && positive == 1){
        ofLogNotice("ofApp") << "Polarity has chenged";
        polarityChanged = 1;
    }
       
    // range from 0 to 3.0 and -3.0
    if(polarityChanged==1 && higherThanThree==1){
        ofLogNotice("ofApp") << "Buffer has changed polarity and values higher than 3" ;
        for(int i=0; i<azimuthBufferSize; i++){
            out = abs(azimuthBuffer[i])+out;
            if(azimuthBuffer[i]<0)
                polarity++;
        }
        runnerAz++;
        out=out/azimuthBufferSize;
        if (polarity > azimuthBufferSize/2)
            return -out;
        else 
            return out;
    }
    else{
        ofLogNotice("ofApp") << "No polarity changed in buffer" ;
    	for(int i=0; i<azimuthBufferSize; i++){
	   	   out = azimuthBuffer[i]+out;
    	}
	runnerAz++;
	out = out/azimuthBufferSize;
    return out;
    }
    
}

//--------------------------------------------------------------
double ofApp::meanGPS(double GPSvalue){
    double out = 0;
    runnerGPS = runnerGPS%GPSbufferSize;
    GPSbuffer[runnerGPS] = GPSvalue;
    for(int i=0; i<GPSbufferSize; i++){
        out = GPSbuffer[i]+out;
    }
    runnerGPS++;
    out = out/GPSbufferSize;
    return out;
}

//--------------------------------------------------------------
std::pair<float, float> ofApp::coordinates_at_distance_angle(float lat, float lon, float dist, float angle){
    /*
        Input:
        lat: origin latitude ()
        lon: origin longitude ()
        dist: distance from the origin point to the new one (kilometers)
        angle: angle from the origin point to the new one (radians)
        Output:
        newlat: new latitude
        newlon: new longitude 

        std::pair<float, float> newcoord = coordinates_at_distance_angle(55.639177, 12.524355, 0.05, PI);
        new_latitude = newcoord.first;
        new_longitude = newcoord.second;
    */

    float pi180 = PI / 180;
    float earth_radius = 6378.137;
    float c = dist / earth_radius;
    float rlat = lat * pi180;
    float rlong = lon * pi180;
    float x = sin(angle) * c;
    float y = cos(angle) * c;
    float Y = (asin(cos(c)*sin(rlat) + (cos(rlat)*(sin(c)/c))*y))/pi180;
    float X = (rlong + atan2(x*(sin(c)),(cos(rlat)*cos(c)*c) - (sin(rlat)*sin(c))*y ) )/pi180;
    return std::make_pair(Y,X);
    // return 0;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void ofApp::touchDown(int x, int y, int id){
    // dv_timestamps.push_back(patch_string::to_string(elapsed_seconds.count()));  // timestamps
    // dv_raw_distance.push_back(patch_string::to_string(intermediateDistance[1]));                   // distance
    // dv_fil_distance.push_back(patch_string::to_string(distance[1]));                   // distance
    // dv_raw_azimuth.push_back(patch_string::to_string(intermediateAzimuth[1]));                     // azimuth
    // dv_fil_azimuth.push_back(patch_string::to_string(azimuth[1]));                     // azimuth

    ofLogNotice("ofApp") << "touchDown: ";    

    // for(std::vector<int>::size_type i = 0; i != dv_timestamps.size(); i++) {
    //     dataBuffer.append(dv_timestamps[i] + "," + dv_raw_distance[i] + "," + dv_fil_distance[i] + "," + dv_raw_azimuth[i] + "," + dv_fil_azimuth[i] +  "," + dv_myLat[i] + "," + dv_myLong[i] + "," + dv_RAWLat[i] + "," + dv_RAWLong[i] + "," + dv_soundLat[i] + "," + dv_soundLong[i] + "," + dv_myCurrOrr[i] + "," + dv_trialState[i] + "," + dv_trialNumber[i] + "," + dv_earconPlayed[i] + "\n"); 
    // }

    // ofFile file;
    // file.open(ofToDataPath("") + get_date() + "_datafile.txt\n", ofFile::Append, false);
    // file.writeFromBuffer(dataBuffer);
}

//--------------------------------------------------------------
void ofApp::touchDoubleTap(int x, int y, int id){
    float heightStep = ofGetHeight() / 3.0f;
    float widthDiv = ofGetWidth() / 2.0f;
    int new_ang;
    int n_rand;
    // TRAINING BUTTON
    if (y >= heightStep && y <= heightStep*2){
        trialState = 1;
        trialNumber++;
        earconPlayed = 0;
        // soundGPScoords[0] = training_soundGPScoords[0];
        std::pair<float, float> training_coord = coordinates_at_distance_angle(myGPScoord.getLatitude(), myGPScoord.getLongitude(), 0.048, (training_ang[training_num]*PI)/180);
        soundGPScoords[0]= Geo::Coordinate(training_coord.first, training_coord.second);
        training_num++;
        training_num = training_num%2;
    }
    // TESTING BUTTON
    else if(y >= heightStep*2){
        trialState = 2;
        trialNumber++;
        earconPlayed = 0;
        srand (time(NULL));
        new_ang = rand() % 361;
        while (new_ang == prev_ang){
            new_ang = rand() % 361;
        }
        prev_ang = new_ang;
        std::pair<float, float> newcoord = coordinates_at_distance_angle(myGPScoord.getLatitude(), myGPScoord.getLongitude(), 0.048, (new_ang*PI)/180);
        soundGPScoords[0]= Geo::Coordinate(newcoord.first, newcoord.second);
        // soundLocationCounter++;
        // soundLocationCounter%=6;
    }
    else if(y <= heightStep && x >= widthDiv){
        for(std::vector<int>::size_type i = 0; i != dv_timestamps.size(); i++) {
            dataBuffer.append(dv_timestamps[i] + "," + dv_raw_distance[i] + "," + dv_fil_distance[i] + "," + dv_raw_azimuth[i] + "," + dv_fil_azimuth[i] +  "," + dv_myLat[i] + "," + dv_myLong[i] + "," + dv_RAWLat[i] + "," + dv_RAWLong[i] + "," + dv_soundLat[i] + "," + dv_soundLong[i] + "," + dv_myCurrOrr[i] + "," + dv_trialState[i] + "," + dv_trialNumber[i] + "," + dv_earconPlayed[i] + "\n"); 
        }

        ofFile file;
        file.open(ofToDataPath("") + get_date() + "_HRTF.txt\n", ofFile::Append, false);
        file.writeFromBuffer(dataBuffer);
        button_pressed = 1;
    }
}

//--------------------------------------------------------------
bool ofApp::backPressed(){
    return false;
}

//--------------------------------------------------------------
void ofApp::playTone(int pitch) {
    pd << StartMessage() << "pitch" << pitch << FinishList("tone") << Bang("tone");
}

//--------------------------------------------------------------
void ofApp::audioReceived(float * input, int bufferSize, int nChannels) {
    pd.audioIn(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::audioRequested(float * output, int bufferSize, int nChannels) {
    pd.audioOut(output, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::print(const std::string& message) {
    cout << message << endl;
}

//--------------------------------------------------------------
void ofApp::receiveBang(const std::string& dest) {
    cout << "OF: bang " << dest << endl;
}

void ofApp::receiveFloat(const std::string& dest, float value) {
    cout << "OF: float " << dest << ": " << value << endl;
}

void ofApp::receiveSymbol(const std::string& dest, const std::string& symbol) {
    cout << "OF: symbol " << dest << ": " << symbol << endl;
}

void ofApp::receiveList(const std::string& dest, const List& list) {
    cout << "OF: list " << dest << ": ";

    // step through the list
    for(int i = 0; i < list.len(); ++i) {
        if(list.isFloat(i))
            cout << list.getFloat(i) << " ";
        else if(list.isSymbol(i))
            cout << list.getSymbol(i) << " ";
    }

    // you can also use the built in toString function or simply stream it out
    // cout << list.toString();
    // cout << list;

    // print an OSC-style type string
    cout << list.types() << endl;
}

void ofApp::receiveMessage(const std::string& dest, const std::string& msg, const List& list) {
    cout << "OF: message " << dest << ": " << msg << " " << list.toString() << list.types() << endl;
}

//--------------------------------------------------------------
void ofApp::receiveNoteOn(const int channel, const int pitch, const int velocity) {
    cout << "OF MIDI: note on: " << channel << " " << pitch << " " << velocity << endl;
}

void ofApp::receiveControlChange(const int channel, const int controller, const int value) {
    cout << "OF MIDI: control change: " << channel << " " << controller << " " << value << endl;
}

// note: pgm nums are 1-128 to match pd
void ofApp::receiveProgramChange(const int channel, const int value) {
    cout << "OF MIDI: program change: " << channel << " " << value << endl;
}

void ofApp::receivePitchBend(const int channel, const int value) {
    cout << "OF MIDI: pitch bend: " << channel << " " << value << endl;
}

void ofApp::receiveAftertouch(const int channel, const int value) {
    cout << "OF MIDI: aftertouch: " << channel << " " << value << endl;
}

void ofApp::receivePolyAftertouch(const int channel, const int pitch, const int value) {
    cout << "OF MIDI: poly aftertouch: " << channel << " " << pitch << " " << value << endl;
}

// note: pd adds +2 to the port num, so sending to port 3 in pd to [midiout],
//       shows up at port 1 in ofxPd
void ofApp::receiveMidiByte(const int port, const int byte) {
    cout << "OF MIDI: midi byte: " << port << " " << byte << endl;
}

