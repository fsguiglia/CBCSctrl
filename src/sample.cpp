#include "sample.h"

Sample::Sample() {
}

void Sample::setFileName(string fileName) {
	_fileName = fileName;
}

void Sample::setTime(float time) {
	_time = time;
}

void Sample::setLength(float length) {
	_length = length;
}

void Sample::setFeatureValue(string feature, float value) {
	_mFeatures[feature] = value;
	_nMFeatures[feature] = value;
}

void Sample::normalizeFeature(string feature, float min, float max) {
	if (_nMFeatures.find(feature) != _nMFeatures.end()) {
		if (min != max) {
			_nMFeatures[feature] = (_mFeatures[feature] - min) / (max - min);
			_nMFeatures[feature] = abs(_nMFeatures[feature]);
			if (_nMFeatures[feature] < 0) _nMFeatures[feature] = 0;
			if (_nMFeatures[feature] > 1) _nMFeatures[feature] = 1;
		}
	}
}

string Sample::getFileName() {
	return _fileName;
}

float Sample::getTime() {
	return _time;
}

float Sample::getLength() {
	return _length;
}

float Sample::getFeatureValue(string feature, bool normalized) {
	float curValue = -1;
	if (normalized) curValue = _nMFeatures[feature];
	else curValue = _mFeatures[feature];
	return curValue;
}

bool Sample::hasFeature(string feature) {
	bool bFeature = false;
	if (_mFeatures.find(feature) != _mFeatures.end()) bFeature = true;
	return bFeature;
}
