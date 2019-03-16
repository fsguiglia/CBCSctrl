#include "gesto.h"

Gesto::Gesto() {
	_start = 0;
	_dur = 0;
	_features = { "empty", "empty" };
}

void Gesto::setFeatures(vector<string> features) {
	_features = features;
}

void Gesto::record(ofVec2f position) {
	_positions.push_back(position);
}

void Gesto::setStartTime(float time) {
	_start = time;
}

void Gesto::setEndTime(float time) {
	_dur = time - _start;
}

void Gesto::setParameters(string fileName, map<string, float> parameters) {
	_sParameters[fileName] = parameters;
}

void Gesto::setDefaultParameters(map<string, float> parameters) {
	_defaultParameters = parameters;
}

ofVec2f Gesto::play(float index) {
	int iIndex = index * (_positions.size() - 1);
	return _positions[iIndex];
}

ofVec2f Gesto::play(int index) {
	int _index = 0;
	if (index < _positions.size()) _index = index;
	return _positions[index];
}

float Gesto::getDur() {
	return _dur;
}

vector<string> Gesto::getFeatures() {
	return _features;
}

map<string, float> Gesto::getParameters(string fileName) {
	map <string, float> curParameters;
	if (_sParameters.find(fileName) != _sParameters.end()) curParameters = _sParameters[fileName];
	else curParameters = _defaultParameters;
	return curParameters;
}

map<string, map<string, float>> Gesto::getParameters() {
	return _sParameters;
}

void Gesto::clearParameters() {
	_sParameters.clear();
}

int Gesto::size() {
	return _positions.size();
}

void Gesto::clear() {
	_positions.clear();
	_features.clear();
}
