#include "ofMain.h"
#ifndef _SAMPLE
#define _SAMPLE

class Sample {
public:
	Sample();

	void setFileName(string _fileName);
	void setTime(float time);
	void setLength(float length);
	void setFeatureValue(string feature, float value);
	void normalizeFeature(string feature, float min, float max);
	string getFileName();
	float getTime();
	float getLength();
	float getFeatureValue(string feature, bool normalized = true);
	bool hasFeature(string feature);

private:
	string _fileName;
	float _time, _length;
	map <string, float> _mFeatures, _nMFeatures;
};

#endif
