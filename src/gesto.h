#include "ofMain.h"
#ifndef _GESTO
#define _GESTO

class Gesto {
public:
	Gesto();
	void setFeatures(vector<string> features);
	void record(ofVec2f position);
	void setStartTime(float time);
	void setEndTime(float time);
	void setParameters(string fileName, map<string, float> parameters);
	void setDefaultParameters(map<string, float> parameters);
	ofVec2f play(float index);
	ofVec2f play(int index);
	float getDur();
	vector<string> getFeatures();
	map<string, float> getParameters(string fileName);
	map<string, map<string, float>> getParameters();
	void clearParameters();
	int size();

	void clear();
private:
	vector<ofVec2f> _positions;
	vector<string> _features;
	map<string, map<string, float>> _sParameters;
	map<string, float> _defaultParameters;
	float _start, _dur;
};

#endif
