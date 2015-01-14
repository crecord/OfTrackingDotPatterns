OfTrackingDotPatterns
==========

![Screenshot](https://github.com/crecord/OfTrackingDotPatterns/blob/master/screen.jpg)

 

Description
-----------

An [openFrameworks](http://www.openframeworks.cc/) project that gives precise location for four different unique patterns of dots. It was used when there was a camera mounted directly above a space and people were wearing wireless headphones with infrared reflective dots on them.

The features:
 - Ability to recognize and identify four different patterns of dots. 
 - Can tell which is the left and the right side of the pattern.  
 - GUI for adjusting all the important thresholds.
 - Ability to send the resulting position array via OSC (Open Sound Control)


Dependencies 
------------

First you need [openFrameworks](http://www.openframeworks.cc/). 
And the following ofxAddons: 

ofxOpenCv
ofxOsc
ofxXmlSettings
ofxCv
ofxUI

The camera I used was filtered so that it could only see IR and it was surrounded by IR LED Panels, located as close to the lens as possible.  
The dot patterns were pieces of 3M IR reflective tape on panels of matte acrylic.

This was last tested on a 10.9.5 mac with Xcode 5.1.1 and OF 0.8.1