#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxDatGui.h"
#include "ofxSpatialHash.h"
#include "sample.h"
#include "gesto.h"

class ofApp : public ofBaseApp{

	public:
		typedef glm::vec2 Vec2;
		typedef glm::vec3 Vec3;
		
		ofApp();
		
		void setup();
		
		void update();
		void updateKnn();
		void updateGestures();
		
		void draw();
		void drawKnn();
		void drawGestures();

		void guiSetup();
		void trTogInput(ofxDatGuiToggleEvent e);
		void trSlInput(ofxDatGuiSliderEvent e);
		void trDDownInput(ofxDatGuiDropdownEvent e);
		void trTextInput(ofxDatGuiTextInputEvent e);
		void trButtonInput(ofxDatGuiButtonEvent e);

		void updatePoints(string fFeat, string sFeat);
		void drawHelp();

		void findNClosest(int n, map <string, float> pos, vector <vector <float>> &selected);
		void find2Closest(int n, ofVec2f pos, vector <vector <float>> &selected);
		void normalizeFeatures();
		void clearAll();

		void addGesture(Gesto curGesto);
		
		void analizeFile();
		void loadFile();
		void saveFile(bool norm);

		//osc send
		void oscNewFile(string path);
		void oscNewPos(string path, float timeMs, float length);
		void oscNewPos();
		void oscClear(bool init = false);
		void oscParameters(int gIndex);

		//osc receive
		void oscReceive();

		void keyPressed(int key);
		void keyReleased(int key);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);

		void exit();
		
		vector <Sample> samples;
		vector <Gesto> gestures;
		
		Gesto curGesto;
		int indexGesto;
		int activeGesto;
		vector<int> gestureName;
		vector <ofxDatGui*> gestureGui;
		int gLfoIndex;
		float gLfoValue;
		map<string, float> defaultParameters;
		vector<string> sDefaultParameters;
		vector<float> fNLfoValues;
		vector<string> modes;
		int mode;

		vector <string> paths;
		vector <vector <float>> selected, lastSelected;
		map <string, float[2]> minMax;
		vector <string> features;
		vector <string> selFeatures;

		ofVec2f pointCloudMargin;
		ofFbo pointCloud;
		bool PCpressed, prevPCpressed;

		vector<int> vFps;
		float lastEvent, frequency;
		bool go, playing, help;
		int knn, display, scroll, iAnalize, iFps;
		
		ofxDatGui *knnGui;

		ofxOscSender oscSender;
		ofxOscReceiver oscReceiver;

		//spatial hash
		vector<ofVec2f> points;
		ofx::KDTree<ofVec2f> hash;
		ofx::KDTree<ofVec2f>::SearchResults searchResults;
};
