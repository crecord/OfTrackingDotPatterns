#include "testApp.h"
#include <cmath> 
typedef std::vector<ofPoint> point_vec;
typedef std::vector<point_vec>  vvec ;

using namespace ofxCv;
using namespace cv;

const float dyingTime = 1;

void Glow::setup(const cv::Rect& track) {
	color.setHsb(ofRandom(0, 255), 255, 255);
	cur = toOf(track).getCenter();
	smooth = cur;
}

ofVec2f Glow::giveCord(const cv::Rect& track){
    return cur; 
}

void Glow::update(const cv::Rect& track) {
	cur = toOf(track).getCenter();
	smooth.interpolate(cur, .5);
	all.addVertex(smooth);
}

void Glow::kill() {
	float curTime = ofGetElapsedTimef();
	if(startedDying == 0) {
		startedDying = curTime;
	} else if(curTime - startedDying > dyingTime) {
		dead = true;
	}
}

void Glow::draw() {
	ofPushStyle();
	float size = 16;
	ofSetColor(255);
	if(startedDying) {
		ofSetColor(ofColor::red);
		size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
	}
	ofNoFill();
	ofCircle(cur, size);
	ofSetColor(color);
	all.draw();
	ofSetColor(255);
	ofDrawBitmapString(ofToString(label), cur);
	ofPopStyle();
}

//--------------------------------------------------------------
void testApp::setup(){
    //ofSetVerticalSync(true);
	ofBackground(0);
    //videoPlayer.setUseTexture(false);
    //videoPlayer.loadMovie("OverheadTestImg.jpg");
    
    //videoPlayer.play();
    //videoPlayer.setLoopState(OF_LOOP_NORMAL);
    myImage.loadImage("OverheadTestImg2.jpg");
    //imageHeight = videoPlayer.height;
    //imageWidth = videoPlayer.width;
    imageWidth =myImage.width;
    imageHeight = myImage.height;
    //original Image
    colorImg.allocate(imageWidth,imageHeight);
    //Grey Scaled Image
	grayImage.allocate(imageWidth,imageHeight);
    //Image to blur
    blurredImg.allocate(imageWidth,imageHeight);
    //smaller Image to blurr and blobdetect.
    // the blur
    blurResolution =2; 

	threshold = 100;
    blurLevel =9;
    bigBlobPxMin = 7;
    bigBlobPxMax = 2700;
    littleBlobPxMin= 0;
    littleBlobPxMax= 40;
    defineGap = 8;
    persistance = 5;
    maxDistance = 100;
    
    
    gui = new ofxUICanvas();
    gui->setPosition(imageWidth/2, 0);
    gui->addSlider("Blur Level",1,15,blurLevel);
    gui->addSlider("BW Threshold", 0, 100, threshold);
    gui->addSlider("Define the Gap", 0, 25, defineGap);
    gui->addRangeSlider("Size of Big Blob", 0, 3000, &bigBlobPxMin, &bigBlobPxMax);
    gui->addRangeSlider("Size of Small Blob", 0, 255, &littleBlobPxMin, &littleBlobPxMax);
    gui->addSlider("resolution of Blur Level", 1, 7, blurResolution);
    gui->addSlider("brightness", 0, 2, bright);
    gui->addSlider("contrast", 0, 1, contrast);
    gui->addSlider("persistance", 0, 150, persistance);
    gui->addSlider("maximumDistance", 0, 100, maxDistance);
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);    
    gui->loadSettings("settings.xml");
    
    
    bNewFram =false;
    
    whichAxis=false;
    howManyInGroup=false;
    whatOrder=false;
    whatIsMyPattern=false;
    
    videoPlayer.setPosition(.0357995);
    trackInfo.allocate(imageWidth,imageHeight);
    continuousTrack.allocate(imageWidth,imageHeight);
    
    
    contourFinder.setMinAreaRadius(1);
	contourFinder.setMaxAreaRadius(100);
	contourFinder.setThreshold(15);
	tracker.setPersistence(persistance);
	// an object can move up to 50 pixels per frame
	tracker.setMaximumDistance(maxDistance);
    sender.setup(HOST, PORT);
	
}


//--------------------------------------------------------------
void testApp::update(){
    
	//ofBackground(100,100,100);
    //videoPlayer.update();
    //bNewFram = videoPlayer.isFrameNew();
	
    
   // if(bNewFram){
        
        
        
        int blurWidth=imageWidth/blurResolution;
        int blurHeight = imageHeight/blurResolution;
        blurredImg2.allocate(blurWidth, blurHeight);
        // this is the resized image, inwhich each blob is a consistent unique greyscale value
        // this will serve as the image that is referenced when determining what larger blob the smaller blobs belong to
        referenceBlobID.allocate(imageWidth,imageHeight);

        //this offbo render space, in which we do the unique grey scale value creation via ofPath
        //I use this so that I don't have to draw it to the screen.
        blobIdRender.allocate(blurWidth, blurHeight );
        // this stores the pixels from the render space.
        thePix.allocate(blurWidth, blurHeight, OF_IMAGE_COLOR_ALPHA);
        
        
        // load in image cvImage type
        //colorImg.setFromPixels(videoPlayer.getPixels(),imageWidth,imageHeight);
        colorImg.setFromPixels( myImage.getPixels(),imageWidth,imageHeight);
        //change color space
        grayImage = colorImg;
        // make it just black and white
        
        grayImage.brightnessContrast(bright,contrast);
        grayImage.contrastStretch();
		grayImage.threshold(threshold);
    
        // prepare image to blurred (this is for doing the large scale blob dectection)
        blurredImg = grayImage;
        // make it smaller so blurring the img is more efficient
        blurredImg2.scaleIntoMe(blurredImg );
        blurredImg2.blur(blurLevel);
    
        // parameters are minimum amount of pixels in blob, maximum, max # of blobs
        contourFinderBig.findContours(blurredImg2, bigBlobPxMin, bigBlobPxMax, 10, true);
    
        // This is to find each individual dot.
        contourFinderLittle.findContours(grayImage, littleBlobPxMin, littleBlobPxMax, 25,true,false);

   // }
  
}

//--------------------------------------------------------------
void testApp::draw(){
    
    //videoPlayer.draw(0,0);
    //videoPlayer.setPosition(<#float pct#>);
    //videoPlayer.getPosition();
    
    if(whichAxis){
        ofDrawBitmapString("which axis", 0, imageHeight/2 +12);
    }
    if(howManyInGroup){
        ofDrawBitmapString("how Many In Group", 0, imageHeight/2+12);
    }
    if(whatOrder){
        ofDrawBitmapString("what Order", 0, imageHeight/2+12);
    }
    if(whatIsMyPattern){
        ofDrawBitmapString("what Is My Pattern", 0, imageHeight/2+12);
    }
    if(whatGaps){
        ofDrawBitmapString("what Gaps", 0, imageHeight/2+12);
    }
    if(whatSize){
        ofDrawBitmapString("what Size", 0, imageHeight/2+12);
    }
    if (isColinear){
        ofDrawBitmapString("is Colinear", 0, imageHeight/2+12);
    }
    
    //if(bNewFram){
    //----------------------(Creating the Reference Image for ID larger blobs)--------------------------
    
    // we enter the FBO render space, rather than drawing to the screen.
    blobIdRender.begin();
    // use this as the background, Could just be a black rect, but this already the correct dimensions
    blurredImg2.draw(0,0);
    // store the greyscale ID values in an array for later reference.
    int size = contourFinderBig.nBlobs;
    int greyScaleIds[size];
    // draw each blob at a different greyscale value
    // working in a 0-255 space
    
    int incrementBy = 255/contourFinderBig.nBlobs;
    int counter =0;
    
    for (int i = 0; i < contourFinderBig.nBlobs; i++){
        ofPath path;
        for ( int y=0; y < contourFinderBig.blobs[i].pts.size(); y++){
            if(y==0){
                path.newSubPath();
                path.moveTo(contourFinderBig.blobs[i].pts[y]);
            }else{
                path.lineTo(contourFinderBig.blobs[i].pts[y]);
            }
        }
        path.close();
        path.simplify();
        // unique greyscale values
        counter+=incrementBy;
        int value = counter;
        greyScaleIds[i]= value; 
        ofColor colorGrey = value;
        path.setFillColor(colorGrey);
        path.draw();
    }
    // exit alternative render space
    blobIdRender.end();
        trackInfo.begin();
        ofBackground(0);

    // store pixels from renderspace in "thePix"
    blobIdRender.readToPixels(thePix,0);
    // very important: convert from OF_IMAGE_COLOR_ALPHA into greyscale
    thePix.setImageType(OF_IMAGE_GRAYSCALE);
    // I thought I had to do this pointer stuff, but turns out I don't.
    //unsigned char *pixels = thePix.getPixels();
    //blurredImg2.setFromPixels(pixels,imageWidth/5, imageHeight/5);
    // store pixels in a CV image
    blurredImg2.setFromPixels(thePix);
    //resize back to the original size and store final variable.
    referenceBlobID.scaleIntoMe(blurredImg2);
    referenceBlobID.draw(0,0, imageWidth/2, imageHeight/2);
    // now this referenceBlobID is ready to be referenced when identifying the smaller blobs.
   
    //----------------------------(Identify each dot and sort based off reference image) ----------------------------------------------------
    
    // two dimensional array to store and organize points
    std::vector<std::vector<ofPoint> > numbers(contourFinderBig.nBlobs);
    std::vector<ofPoint> tempVector;
    // go through each little headphone dot
    unsigned char * pixels = referenceBlobID.getPixels();
    for (int i = 0; i < contourFinderLittle.nBlobs; i++){
        int x= contourFinderLittle.blobs[i].boundingRect.getCenter().x;
        int y = contourFinderLittle.blobs[i].boundingRect.getCenter().y;
        ofFill();
        ofEllipse(x/2,y/2,3,3);
        int area = contourFinderLittle.blobs[i].area;
        blobSize = sqrt(area)/M_PI;
        theCenters = contourFinderLittle.blobs[i].centroid; 
        // get the greyscale value in the reference image
        int brightness = referenceBlobID.getPixels()[x*y];
        int value = pixels[y * imageWidth + x];
        // check which greyscale value it matches up with and store it in an array accordingly
        for (int j=0; j < contourFinderBig.nBlobs; j++){
            int currentCheck = greyScaleIds[j];
            //ofLog()<<currentCheck;
            if(value == currentCheck){
                // get whatever variables are already in there
                tempVector = numbers[j];
                // add the new value
                tempVector.push_back(theCenters);
                // now replace with the updated vector
                numbers[j].swap(tempVector);
                // you have found what you are looking for and 
                break;
            }
        }
    }
        if (howManyInGroup){
            for(int i=0; i < numbers.size();i++ ){
                if(numbers[i].size()>0){
                    ofDrawBitmapString(ofToString(numbers[i].size()), numbers[i][0].x/2,numbers[i][0].y/2);
                }
            }
            
        }
    
    

    
    
    // discard any points which are in the blob but not colinear. 
    tempVector.clear();
    //vector<vector <int> > theSlopes;
    //std::vector<int> tempVector2;
    //ofLog() << endl;
    
    // find the formula for the line.
    
    for(int j =0; j< numbers.size(); j++){
        int count = numbers[j].size();
        float sumX = 0;
        float sumY =0;
        float sumX2 =0;
        float sumXY =0;
        for(int l=0; l<numbers[j].size(); l++){
            sumX += numbers[j][l].x;
            sumX2 += std::sqrt(numbers[j][l].x);
            sumY += numbers[j][l].y;
            sumXY += numbers[j][l].x * numbers[j][l].y;
        }
        float Xmean = sumX/count;
        float Ymean = sumY/count;
        float slope = (sumXY - sumX * Ymean) / (sumX2 - sumX * Xmean);
        float Yint = Ymean - slope * Xmean;
        
        int spacingCount = 0; 
        for (int l=0; l<numbers[j].size(); l++){
            // actual - predicition
            float actual = numbers[j][l].y;
            float prediction = slope * numbers[j][l].x + Yint;
            spacingCount+= 15;
            if(isColinear){
                ofDrawBitmapString(ofToString(abs(actual - prediction)),numbers[j][l].x/2,numbers[j][l].y/2-spacingCount);
                ofLine(numbers[j][l].x,numbers[j][l].y, numbers[j][l].x, prediction);
            }
            //ofLog() << std::abs(actual - prediction);
        }
        //ofLog() << endl;
    }
    
    
    
      //----------------------------(make HeadPhoneID) ----------------------------------------------------
    
    //sort out each list so the values go from left to right or from top to bottom.
    //(depending on whether the greater variation is in the x or y axis)
   
    if(numbers.size()>=3){
    for ( int i = 0; i < numbers.size(); i++ )
    {
        int smallX =100000;
        int largeX =0;
        int smallY =100000;
        int largeY =0;
        // 0 = X is best, 1 = Y is best
        // in the case that they are both the same, use x.
        bool bestVal = 0;
        // first, find which axis has the most variation.
        
        for ( int j = 0; j < numbers[i].size(); j++ )
        {
            ofPoint temps = numbers[i][j];
            if(temps.x < smallX){
                smallX=temps.x;
            }
            if (temps.x > largeX){
                largeX = temps.x;
            }

            if(temps.y < smallY){
                smallY=temps.y;
            }
            if (temps.x > largeX){
                largeY = temps.y;
            }
            
        }
        
        
        if( (largeY-smallY) > (largeX- smallX) ){
            bestVal =1;
        }
        if ( bestVal == 1 ){
            if(whichAxis){
                ofDrawBitmapString("y is Best",largeX/2,largeY/2);
            }
            // bubble sort based off the y values
            for ( int j = 0; j < numbers[i].size(); j++ ){
                
                for ( int l = 0; l < numbers[i].size() -1 ; l++ ){
                     if (numbers[i][l].y > numbers [i][l+1].y ){
                         
                         ofPoint temp = numbers [i][l+1];
                         numbers [i][l+1] = numbers [i][l];
                         numbers [i][l] = temp;
                     }
                 }
            }
        }
        else{
            if(whichAxis){
                ofDrawBitmapString("x is Best", largeX/2, largeY/2);
            }
            for ( int j = 0; j < numbers[i].size(); j++ ){
                for ( int l = 0; l < numbers[i].size()-1 ; l++ ){
                    if (numbers[i][l].x  > numbers [i][l+1].x ){
                        ofPoint temp = numbers [i][l+1];
                        numbers [i][l+1] = numbers [i][l];
                        numbers [i][l] = temp;
                    }
                }
            }
        }
        if(whatOrder){
            for(int j=0; j< numbers[i].size(); j++){
                ofDrawBitmapString(ofToString(j),numbers[i][j].x/2, numbers[i][j].y/2 );
            }
        }
    }
    

    // next code the dots and gaps to understand which pattern (aka which headphone is associated with which blob)
    

    
    //
    int patterns [4][6] = {{1,1,0,0,0,1},{1,1,1,0,0,1},{1,1,0,1,0,1},{1,1,1,1,0,1}};
    
    //.036611
    //.0357995
    
    for ( int i = 0; i < numbers.size(); i++ )
    {
        //std::cout << endl;
        
        if ( numbers[i].size()>1){
            //ofLog()<< "numbers_size: "<< numbers[i].size();
            string fullPatern;
            string gapSizes;
            string circleSizes;
            
        for ( int j = 0; j < numbers[i].size()-1; j++ )
        {
            //std::cout<< "(" << numbers[i][j].x << "," << numbers[i][j].y << ")" ;
            ofVec2f p1( numbers[i][j].x,numbers[i][j].y );
            ofVec2f p2( numbers[i][j+1].x,numbers[i][j+1].y );
            float diff = p1.distance(p2);
            gapSizes+= ofToString(diff);
            gapSizes+=" ";
            pattern.push_back(1);
            
            // should make this adjustable based off of the circum. of the blobs
            //int divideBy = defineGap;
        
            //ofLog()<< "diff: "<< diff;
            
            int gaps = (int)round(diff/defineGap);
            if(gaps > 1){
            for (int v=0; v < gaps-1; v++){
                // something here causes a crash when you adjust the threshold!
                pattern.push_back(0);
              }
           }
        }

        //if(){
            pattern.push_back(1);
            
            //fullPatern += ofToString(1);
        //}
            circleSizes = "size: "+ ofToString(blobSize);
            
            
            
        // make sure "1,1" comes first so that you can compare to the patterns.
        // if it doesn't come first, flip pattern and numbers[i]
        if ( pattern[0] == 1 && pattern[1] ==1){
            // do nothing
        }
        else {
            std::reverse(pattern.begin(), pattern.end());
            std::reverse(numbers[i].begin(), numbers[i].end());
        }
            
            if(whatIsMyPattern){
                
                for(int r=0; r<pattern.size(); r++){
                    fullPatern += ofToString(pattern[r]); 
                }
                ofDrawBitmapString(fullPatern,numbers[i][0].x/2,numbers[i][0].y/2);
            }
            if(whatGaps){
                ofDrawBitmapString(gapSizes,numbers[i][0].x/2,numbers[i][0].y/2);
            }
            if(whatSize){
                ofDrawBitmapString(circleSizes,numbers[i][0].x/2,numbers[i][0].y/2);
            }
        
        for (int z=0; z< pattern.size(); z++){
            //std::cout<< pattern[z];
        }
        // check which pattern it is equal too
            if(pattern.size() >= 6){
        for (int j=0; j< 4; j++){
            bool match = true;
            for(int l=0; l< 6 ; l++){
                //ofLog() << "tamplate: "<< patterns[j][l];
                //ofLog() << "current: " << pattern[l];
                //std::cout << pattern[l];
                if ( patterns[j][l] != pattern[l]){
                    match = false;
                    break;
                }
            }
            //ofDrawBitmapString(ofToString(j),numbers[i][0].x/2+30,numbers[i][0].y/2+30);
            //std::cout << endl;
            //ofDrawBitmapString("yat",numbers[i][0].x/2+30,numbers[i][0].y/2+30);
            if(match == true){
                
                headphone newHD;
                newHD.ID = j;
                newHD.left = numbers[i][0];
                newHD.right = numbers[i][numbers[i].size()-1];
                ofFill();
                ofEllipse(newHD.left.x/2,newHD.left.y/2, 3, 3);
                ofEllipse(newHD.right.x/2, newHD.right.y/2, 3, 3);
                stringstream ss;
                ss << j;
                string idNum = ss.str();
                ofDrawBitmapString(idNum,numbers[i][0].x/2+30,numbers[i][0].y/2+30);
                idNum += "somthin"; 
                
                
                headphones.push_back(newHD);
                break;
            }
            else{
                //ofDrawBitmapString("no",numbers[i][0].x/2+20,numbers[i][0].y/2+20);
            }
        }
            
            //ofLog() << "siu";
            //pattern.clear();
            
        }
        
        //ofLog() << "piu";
        pattern.clear();
    }
    }
    }
    ofDrawBitmapString(ofToString(numbers.size()), 0, imageHeight/2+25);
    ofDrawBitmapString(ofToString(videoPlayer.getPosition()), 0, imageHeight/2+35);
    numbers.clear();
    trackInfo.end();
   // }
    //!!!!lastVideoPosition = videoPlayer.getPosition();
    
    continuousTrack.begin();
    
    //ofBackground(0);
    //blurredImg2.draw(0,0);
    ofFill();
    ofSetColor(0);
    ofRect(0, 0, imageWidth, imageHeight);
    
    for(int i=0; i< headphones.size(); i++){
        int ID = headphones[i].ID;
        ofSetColor(255); 
        ofFill();
        ofxOscMessage sendLeft;
        sendLeft.setAddress("/"+ ofToString(ID) + "/leftPos");
        sendLeft.addIntArg(headphones[i].left.x);
        sendLeft.addIntArg(headphones[i].left.y);
        sender.sendMessage(sendLeft);
        
        ofxOscMessage sendRight;
        sendRight.setAddress("/"+ ofToString(ID) + "/rightPos");
        sendRight.addIntArg(headphones[i].right.x);
        sendRight.addIntArg(headphones[i].right.y);
        sender.sendMessage(sendRight);
        

        ofEllipse(headphones[i].left, 10, 10);
        ofEllipse(headphones[i].right, 10, 10);
     
    }
     
    ofSetColor(255);
    
    
    continuousTrack.end();
    
    if(drawTrackInfo){
        
        trackInfo.draw(0,0);
        //trackInfo.unbind();
    }
    else if (drawConinuousTrack){
        continuousTrack.draw(0,0,imageWidth/2,imageHeight/2);
        /*
        ofPixels ImgToTrack;
        ImgToTrack.allocate(imageWidth,imageHeight,OF_IMAGE_GRAYSCALE);
        continuousTrack.readToPixels(ImgToTrack);
        ofImage trackImg;
        trackImg.setFromPixels(ImgToTrack);
        //contourFinder.findContours(continuousTrack);
        blur(trackImg, 10);
		contourFinder.findContours(trackImg);
		tracker.track(contourFinder.getBoundingRects());
        contourFinder.draw();

        vector<Glow>& followers = tracker.getFollowers();
        for(int i = 0; i < followers.size(); i++) {
            followers[i].draw();
            //ofVec2f hjhd = toOf(followers[i]).getCenter();
            //ofVec2f hjhd;
            //hjhd = followers[i].cur();
        }
        */
        
        //continuousTrack.unbind();
    }
    headphones.clear();
    trackInfo.unbind();
}



void testApp::exit()
{
    ofLog()<<"popsters"; 
	gui->saveSettings("settings.xml");
    delete gui;
     
}

void testApp::guiEvent(ofxUIEventArgs &e)

{
    if(e.getName() == "Blur Level")
    {
        ofxUISlider *slider = e.getSlider();
        //ofBackground(slider->getScaledValue());
        blurLevel = (int) slider->getScaledValue();
    
    }
    else if(e.getName() == "BW Threshold"){
        ofxUISlider *slider = e.getSlider();
        threshold = (int) slider->getScaledValue();
    }
    else if (e.getName() == "Define the Gap"){
        ofxUISlider *slider = e.getSlider();
        defineGap = slider->getScaledValue();
    }
    else if(e.getName() == "resolution of Blur Level"){
        ofxUISlider *slider = e.getSlider();
        blurResolution = slider->getScaledValue();
    }
    else if(e.getName() == "brightness"){
        ofxUISlider *slider = e.getSlider();
        bright = slider->getScaledValue();
    }
    else if(e.getName() == "contrast"){
        ofxUISlider *slider = e.getSlider();
        contrast = slider->getScaledValue();
    }
    else if (e.getName()== "persistance"){
        //ofxUISlider *slider = e.getSlider();
        //persistance = slider->getScaledValue();
        //tracker.setPersistence(persistance);
        
    }
    else if(e.getName()=="maximum distance"){
        //ofxUISlider *slider = e.getSlider();
        //maxDistance = slider->getScaledValue();
        // an object can move up to 50 pixels per frame
        //tracker.setMaximumDistance(maxDistance);
    }
 
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){
    ofLog()<<key;
    float currentPos = videoPlayer.getPosition();
    float lastPos = currentPos - .000788;
	switch (key){
		case 's':
			gui->saveSettings("settings.xml");
            break;
        case 'n':
            lastVideoPosition =videoPlayer.getPosition();
            videoPlayer.nextFrame();
            ofLog()<< "the Position: "<< videoPlayer.getPosition();
            break;
        case 'm':
            videoPlayer.setPosition(lastPos);
            break;
        case 'q':
            whatSize = false;
            whatGaps =false;
            howManyInGroup=false;
            whatOrder=false;
            whatIsMyPattern=false;
            whichAxis =true;
            ofLog()<<"a";
            break;
        case 'w':
            whatSize = false;
            whatGaps =false;
            whichAxis=false;
            
            whatOrder=false;
            whatIsMyPattern=false;
            
            howManyInGroup =true;
            ofLog()<<"2";
            break;
        case 'e':
            whatSize = false;
            whatGaps =false;
            whichAxis=false;
            howManyInGroup=false;
            isColinear=false;
            whatIsMyPattern=false;
            whatOrder =true;
            ofLog()<<"3";
            break;
        case 'r':
            whatSize = false;
            whatGaps =false;
            whichAxis=false;
            howManyInGroup=false;
            whatOrder=false;
            isColinear=false;
            whatIsMyPattern =true;
            ofLog()<<"4";
            break;
        case 't':
            whatSize = false;
            whatGaps =false;
            whichAxis=false;
            howManyInGroup=false;
            whatOrder=false;
            whatIsMyPattern =false;
            isColinear=false;
            whatGaps =true;
            break;
        case'y':
            whichAxis=false;
            howManyInGroup=false;
            whatOrder=false;
            whatIsMyPattern =false;
            whatSize = false;
            whatGaps =false;
            isColinear=false;
            whatSize = true;
            break;
        case'u':
            whichAxis=false;
            howManyInGroup=false;
            whatOrder=false;
            whatIsMyPattern =false;
            whatSize = false;
            whatGaps =false;
            isColinear=false;
            isColinear =true;
            break;
        case 'z':
            drawConinuousTrack =false;
            drawTrackInfo =true;
            break;
        case 'x':
            drawTrackInfo =false;
            drawConinuousTrack = true;
            break;
    }
    
    /*

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
	}
     */
    
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
