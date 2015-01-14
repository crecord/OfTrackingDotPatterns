#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"
#include "ofxCv.h"
#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 1234

class Glow : public ofxCv::RectFollower {
protected:
	ofColor color;
	ofVec2f smooth;
	float startedDying;
	ofPolyline all;
    ofVec2f cur; 
public:
	Glow()
    :startedDying(0) {
	}
	void setup(const cv::Rect& track);
	void update(const cv::Rect& track);
	void kill();
	void draw();
    ofVec2f giveCord(const cv::Rect& track);
    
};

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void exit();
        void guiEvent(ofxUIEventArgs &e);
    
    ofVideoPlayer videoPlayer;

        ofxUICanvas *gui;


        ofxCvColorImage			colorImg;

        ofxCvGrayscaleImage 	grayImage;
        ofxCvGrayscaleImage 	blurredImg;
        ofxCvGrayscaleImage 	blurredImg2;
        ofxCvGrayscaleImage 	referenceBlobID;
        // this is for drawing out the blobs
        ofPath path;
        
        ofPixels thePix;
        ofxCvContourFinder 	contourFinderBig;
        ofxCvContourFinder 	contourFinderLittle;
        
        ofPoint theCenters;
        ofFbo blobIdRender;
    ofFbo trackInfo;
    ofFbo continuousTrack; 
    
    
    int blurLevel; 
    
    float blobSize;
    
    struct headphone {
        int ID;
        ofPoint left;
        ofPoint right;
    };
    
    // this is the result!
    vector<headphone> headphones;
    
		int 				threshold;
		bool				bLearnBakground;
        int                 imageWidth;
        int                 imageHeight;
        ofImage myImage;
    
    // gui variables
    float bigBlobPxMin;
    float bigBlobPxMax;
    float littleBlobPxMin;
    float littleBlobPxMax;
    float defineGap;
    float blurResolution;
    
    bool bNewFram;
    vector<int> pattern;
    
    bool whichAxis;
    bool howManyInGroup;
    bool whatOrder;
    bool whatIsMyPattern;
    bool whatGaps;
    bool whatSize;
    bool isColinear; 
    
    float lastVideoPosition;
    
    float bright;
    float contrast;
    
    bool drawTrackInfo;
    bool drawConinuousTrack;
    int persistance;
    int maxDistance; 
    

    ofxCv::ContourFinder contourFinder;
	ofxCv::RectTrackerFollower<Glow> tracker;
    
    
    ofxOscSender sender;
};

