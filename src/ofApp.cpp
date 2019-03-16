#include "ofApp.h"

/*
elimine parametros en save/load gesto

los gestos pueden manejar un numero arbitrario de parametros, pero json y supercollider no se llevan bien
por lo que solo uso los cuatro definidos en en un archivo de texto, si no la comunicacion por osc es dificil
*/

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	ofSetWindowTitle("CBCS");

	vFps.assign(6, 60);

	oscSender.setup("127.0.0.1", 57120);
	oscReceiver.setup(8333);

	pointCloud.allocate(500, 500, 6408, 4);
	pointCloud.begin();
	ofBackground(255);
	pointCloud.end();
	pointCloudMargin.set(0,0);

	//no encuentro como actualizar las opciones del dropdown
	ofBuffer featureBuffer = ofBufferFromFile("features.txt");
	for (auto line : featureBuffer.getLines()) features.push_back(line);
	selFeatures = { features[0], features[1] };
	knn = 1;
	modes = { "mouse", "gesture", "lfo" };

	ofBuffer parameterBuffer = ofBufferFromFile("parameters.txt");
	for (auto line : parameterBuffer.getLines()) {
		sDefaultParameters.push_back(ofSplitString(line, "=")[0]);
		defaultParameters[ofSplitString(line, "=")[0]] = ofToFloat(ofSplitString(line, "=")[1]);
	}

	mode = 0;
	frequency = 20;
	lastEvent = 0;
	display = 0;
	scroll = 0;
	indexGesto = 1;
	activeGesto = 0;
	gLfoIndex = 0;
	gLfoValue = 0;
	go = false;
	PCpressed = false;
	help = false;
	prevPCpressed = false;
	iAnalize = -1;

	guiSetup();
	oscClear(true);
}

//--------------------------------------------------------------
void ofApp::update() {
	oscReceive();
	updateKnn();
	updateGestures();
	
	if (iAnalize > -1) {
		iAnalize++;
		if (iAnalize > 3) {
			analizeFile();
			iAnalize = -1;
		}
	}

	iFps = 0;
	for (int i = 0; i < vFps.size() - 1; i++) {
		vFps[i] = vFps[i + 1];
		iFps += vFps[i];
	}
	vFps[vFps.size() - 1] = ceil(ofGetFrameRate());
	iFps += ofGetFrameRate();
	iFps /= vFps.size();
}

void ofApp::updateKnn() {
	knnGui->setVisible(display == 0);
	knnGui->setEnabled(display == 0);
	knnGui->update();

	playing = false;
	if (ofGetElapsedTimeMillis() - lastEvent > (1000 / (float)frequency)) {
		if (samples.size() > 0 && go) {
			map <string, float> cursor;
			ofVec2f pos;
			switch (mode) {
			case 0:
				if (PCpressed) {
					pos.x = (mouseX - pointCloudMargin.x) / pointCloud.getWidth();
					pos.y = (mouseY - pointCloudMargin.y) / pointCloud.getHeight();
					cursor[selFeatures[0]] = pos.x;
					cursor[selFeatures[1]] = pos.y;
					findNClosest(knn, cursor, selected);
					lastSelected.clear();
					lastSelected = selected;
					curGesto.record(pos);
					playing = true;
				}
				break;
			case 1:
				if (gLfoIndex == 0) {
					pos.x = samples[(int)((samples.size() - 1) * gLfoValue)].getFeatureValue(selFeatures[0]);
					pos.y = samples[(int)((samples.size() - 1) * gLfoValue)].getFeatureValue(selFeatures[1]);
					cursor[selFeatures[0]] = pos.x;
					cursor[selFeatures[1]] = pos.y;
					findNClosest(knn, cursor, selected);
					playing = true;
				}
				else if (gLfoIndex - 1 < gestures.size()) {
					int curIndex = gLfoIndex - 1;
					pos = gestures[curIndex].play(gLfoValue);
					vector <string> gFeatures = gestures[curIndex].getFeatures();
					cursor[gFeatures[0]] = pos.x;
					cursor[gFeatures[1]] = pos.y;
					findNClosest(knn, cursor, selected);
					playing = true;
				}
				break;
			case 2:
				if (fNLfoValues.size() == features.size()) {
					for (int i = 0; i < features.size(); i++) if (fNLfoValues[i] >= 0) cursor[features[i]] = fNLfoValues[i];
					findNClosest(knn, cursor, selected);
					playing = true;
				}
				break;
			}
		}
	}
	if (playing) {
		oscNewPos();
		lastEvent = ofGetElapsedTimeMillis();
	}
}

void ofApp::updateGestures() {
	for (int i = 0; i < gestureGui.size(); i++) {
		gestureGui[i]->setVisible(display == 1);
		gestureGui[i]->setEnabled(display == 1);
		gestureGui[i]->setPosition(i * (gestureGui[i]->getWidth() + 10) + 10 + scroll, 20);
		gestureGui[i]->update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);
	switch (display) {
	case 0:
		drawKnn();
		break;
	case 1:
		drawGestures();
		break;
	}

	if (iAnalize != -1) {
		ofPushStyle();
		ofPushMatrix();
		ofSetColor(255, 200);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		ofSetColor(0);
		ofDrawBitmapString("Analyzing, please wait.", ofGetWidth() * 0.5 - 100, ofGetHeight() * 0.5);
		ofPopMatrix();
		ofPopStyle();
	}
}

void ofApp::drawKnn() {
	ofPushStyle();
	pointCloud.draw(pointCloudMargin.x, pointCloudMargin.y);
	if (go && (PCpressed || mode > 0)) {
		ofSetColor(255, 0, 0);
		for (auto sample : selected) {
			ofVec2f curPos = { samples[sample[0]].getFeatureValue(selFeatures[0]), samples[sample[0]].getFeatureValue(selFeatures[1]) };
			curPos.x *= pointCloud.getWidth();
			curPos.y *= pointCloud.getHeight();
			curPos += pointCloudMargin;
			ofDrawEllipse(curPos, 8, 8);
		}
	}

	if (PCpressed) {
		ofSetColor(255, 0, 0, 50);
		ofDrawEllipse(mouseX, mouseY, 50, 50);
	}

	ofPopStyle();
	if (help) drawHelp();

	ofDrawBitmapString("Samples: " + ofToString(samples.size()), 520, 400);
	ofDrawBitmapString("FPS: " + ofToString(iFps), 520, 420);
	ofDrawBitmapString("Gesture: " + ofToString(activeGesto), 520, 440);
	knnGui->draw();
}

void ofApp::drawGestures() {
	for (auto gui : gestureGui) gui->draw();
}

void ofApp::guiSetup() {
	knnGui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
	knnGui->addLabel("x/y features");
	knnGui->addDropdown("X", features);
	knnGui->getDropdown("X")->setLabel(features[0]);
	knnGui->addDropdown("Y", features);
	knnGui->getDropdown("Y")->setLabel(features[1]);
	knnGui->addSlider("knn", 1, 30, 1);
	knnGui->getSlider("knn")->setPrecision(0);
	knnGui->addLabel("mode");
	knnGui->addDropdown("mode", modes);
	knnGui->getDropdown("mode")->setLabel(modes[0]);
	knnGui->addToggle("go!", false);
	knnGui->addSlider("frequency", 1, 100, 20);
	knnGui->getSlider("frequency")->setPrecision(0);
	knnGui->addTextInput("osc send", "127.0.0.1:57120");
	knnGui->addTextInput("osc receive", "8333");

	knnGui->onToggleEvent(this, &ofApp::trTogInput);
	knnGui->onSliderEvent(this, &ofApp::trSlInput);
	knnGui->onDropdownEvent(this, &ofApp::trDDownInput);
	knnGui->onTextInputEvent(this, &ofApp::trTextInput);

	knnGui->setAutoDraw(false);
	knnGui->setPosition(500, 10);
	knnGui->setTheme(new ofxDatGuiThemeAqua());
	knnGui->setWidth(300, 0.4);
}

void ofApp::trTogInput(ofxDatGuiToggleEvent e) {
	if (e.target->getName() == "go!") go = e.checked;
}

void ofApp::trSlInput(ofxDatGuiSliderEvent e) {
	if (e.target->getName() == "knn") knn = (int)e.value;
	if (e.target->getName() == "frequency") frequency = e.value;
}

void ofApp::trDDownInput(ofxDatGuiDropdownEvent e) {
	if (e.target->getName() == "X") selFeatures[0] = e.target->getLabel();
	else if (e.target->getName() == "Y") selFeatures[1] = e.target->getLabel();
	else if (e.target->getName() == "mode") {
		for (int i = 0; i < modes.size(); i++) {
			if (modes[i] == e.target->getLabel()) {
				mode = i;
				break;
			}
		}
	}
	updateFbo(selFeatures[0], selFeatures[1]);
}

void ofApp::trTextInput(ofxDatGuiTextInputEvent e) {
	if (e.target->getName() == "osc send") {
		vector<string> split = ofSplitString(e.text, ":");
		if (split.size() == 2) {
			if (ofSplitString(split[0], ".").size() == 4) oscSender.setup(split[0], ofToInt(split[1]));
		}
	}
	else if (e.target->getName() == "osc receive") {
		oscReceiver.setup(ofToInt(e.text));
	}
}

void ofApp::trButtonInput(ofxDatGuiButtonEvent e) {
	int index = ofToInt(ofSplitString(e.target->getLabel(), ":")[0]);

	for (int i = 0; i < gestureName.size(); i++) {
		if (gestureName[i] == index) {
			gestureGui.erase(gestureGui.begin() + i);
			gestures.erase(gestures.begin() + i);
			gestureName.erase(gestureName.begin() + i);
			break;
		}
	}
}

bool indexSort(vector<float> a, vector<float> b) {
	return (a[0] > b[0]);
}

bool distanceSort(vector<float> a, vector<float> b) {
	return (a[1] < b[1]);
}

void ofApp::findNClosest(int n, map <string, float> pos, vector <vector <float>> &selected) {
	selected.clear();
	//investigar optimizacion de busqueda -> cuadrantes?
	for (int i = 0; i < samples.size(); i++) {
		vector <float> curSample = { (float)i ,0 };
		for (auto key : pos) {
			float curDis;
			if (samples[i].hasFeature(key.first)) {
				curDis = key.second - samples[i].getFeatureValue(key.first);
				curDis *= curDis;
			}
			else curDis = 1;
			curSample[1] += curDis;
		}
		selected.push_back(curSample);
	}
	sort(selected.begin(), selected.end(), distanceSort);
	selected.resize(n);
}

void ofApp::normalizeFeatures() {
	minMax.clear();

	for (auto sample : samples) {
		for (auto key : features) {
			if (minMax.find(key) == minMax.end()) {
				minMax[key][0] = sample.getFeatureValue(key, false);
				minMax[key][1] = sample.getFeatureValue(key, false);
			}
			else {
				if (minMax[key][0] < sample.getFeatureValue(key, false)) minMax[key][0] = sample.getFeatureValue(key, false);
				if (minMax[key][1] > sample.getFeatureValue(key, false)) minMax[key][1] = sample.getFeatureValue(key, false);
			}
		}
	}

	for (auto & sample : samples) {
		for (auto key : minMax) {
			sample.normalizeFeature(key.first, key.second[0], key.second[1]);
		}
	}
}

void ofApp::clearAll() {
	minMax.clear();
	samples.clear();
	gestures.clear();
	curGesto.clear();
	gestureGui.clear();
	features = { "flatness", "rms", "centroid", "rollOff", "bandWidth" };
	go = false;
	mode = 0;
	activeGesto = 0;
	knnGui->getToggle("go!")->setChecked(false);
	knnGui->getToggle("lfo")->setChecked(false);
}

void ofApp::addGesture(Gesto curGesto) {
	ofxDatGui *curGui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
	curGui->addLabel("x: " + curGesto.getFeatures()[0]);
	curGui->addLabel("y: " + curGesto.getFeatures()[1]);
	curGui->addLabel(ofToString(indexGesto) + ": " + ofToString(curGesto.size()) + " samples");
	curGui->addLabel(ofToString(round(curGesto.getDur() * 100) / 100) + " segundos");
	curGui->addButton(ofToString(indexGesto) + ": clear");
	curGui->onButtonEvent(this, &ofApp::trButtonInput);
	curGui->setAutoDraw(false);
	curGui->setPosition(0, 0);
	curGui->setTheme(new ofxDatGuiThemeAqua());
	curGui->setWidth(100, 0.4);
	curGui->setAutoDraw(false);

	curGesto.setDefaultParameters(defaultParameters);
	gestures.push_back(curGesto);
	gestureGui.push_back(curGui);
	gestureName.push_back(indexGesto);
	indexGesto += 1;
}

void ofApp::updateFbo(string fFeat, string sFeat) {
	int fboWidth = pointCloud.getWidth();
	
	pointCloud.begin();
	ofBackground(255);
	for (auto sample : samples) {
		ofSetColor(0, 0, 255);
		ofVec2f curPos = { sample.getFeatureValue(fFeat), sample.getFeatureValue(sFeat) };
		curPos *= fboWidth;
		ofDrawEllipse(curPos, 5, 5);
	}
	pointCloud.end();
}

void ofApp::drawHelp() {
	ofPushMatrix();
	ofTranslate(pointCloudMargin);
	ofPushStyle();
	
	ofSetColor(255, 200);
	ofDrawRectangle(0, 0, pointCloud.getWidth(), pointCloud.getHeight());
	
	vector<string> hPar = {
		"a: analize audio file",
		"C: clear samples",
		"h: toggle help display",
		"g: record last gesture",
		"l: load json feature file",
		"n: normalize sample features",
		"s: save samples to json",
		"S: save normalized samples to json (normalize first)",
		"X: delete selected samples"
	};

	ofSetColor(0);
	ofTranslate(20, 0);
	for (auto text : hPar) {
		ofTranslate(0, 20);
		ofDrawBitmapString(text, 0, 0);
	}
	ofPopStyle();
	ofPopMatrix();
}

void ofApp::analizeFile() {
	ofFileDialogResult openFileResult = ofSystemLoadDialog("Select audio file");
	if (openFileResult.bSuccess) {
		string command = ofFilePath::getAbsolutePath(ofToDataPath("")) + "featureExtractor.py -s " + openFileResult.getPath();
		system(command.c_str());
	}
}

void ofApp::loadFile() {
	string curPath;
	ofFileDialogResult openFileResult = ofSystemLoadDialog("Select feature file");
	if (openFileResult.bSuccess) {
		ofJson curJson = ofLoadJson(openFileResult.getPath());
		bool hasGestures = curJson["gestures"].size() > 0;
	
		for (int i = 0; i < features.size(); i++) {
			if (features[i] == curJson["xFeature"]) {
				selFeatures[0] = features[i];
				knnGui->getDropdown("X")->setLabel(features[i]);
			}
			if (features[i] == curJson["yFeature"]) {
				selFeatures[1] = features[i];
				knnGui->getDropdown("Y")->setLabel(features[i]);
			}
		}

		for (auto sample : curJson["samples"]) {
			Sample curSample;

			curSample.setFileName(sample["file"]);
			if (curSample.getFileName() != curPath || curPath == "empty") {
				curPath = curSample.getFileName();
				paths.push_back(curPath);
				oscNewFile(curPath);
			}

			curSample.setTime(sample["time"]);
			curSample.setLength(sample["length"]);
			
			for (auto feature : features) {
				if (sample.find(feature) != sample.end()) curSample.setFeatureValue(feature, sample[feature]);
			}
			
			samples.push_back(curSample);
			
		}
		if (hasGestures) {
			gestures.clear();
			curGesto.clear();
			gestureGui.clear();

			for (auto gesture : curJson["gestures"]) {
				Gesto curGesto;
				vector <string> features = { gesture["fFeature"], gesture["sFeature"]};
				curGesto.setFeatures(features);
				curGesto.setStartTime(0);
				curGesto.setEndTime(gesture["length"]);
				for (auto value : gesture["values"]) {
					ofVec2f curPos;
					curPos.x = value["x"];
					curPos.y = value["y"];
					curGesto.record(curPos);
				}
				addGesture(curGesto);
			}
		}
	}
}

void ofApp::saveFile(bool norm) {
	
	string dialog;
	if (norm) dialog = "Save feature file";
	else dialog = "Save normalized feature file";
	ofFileDialogResult saveFileResult = ofSystemSaveDialog("untitled.json", dialog);
	
	if (saveFileResult.bSuccess) {
		ofJson jOut;
		string path = saveFileResult.getPath();
		
		jOut["xFeature"] = selFeatures[0];
		jOut["yFeature"] = selFeatures[1];

		ofJson jSamples;
		for (auto sample : samples) {
			ofJson curSample;
			curSample["file"] = sample.getFileName();
			for (auto feature : features) curSample[feature] = sample.getFeatureValue(feature, norm);
			curSample["length"] = sample.getLength();
			curSample["time"] = sample.getTime();
			for (auto feature : features) curSample[feature] = sample.getFeatureValue(feature, norm);
			curSample["length"] = sample.getLength();
			curSample["time"] = sample.getTime();
			jSamples.push_back(curSample);
		}

		ofJson jGestures;
		for (auto gesture : gestures) {
			ofJson curGesto;
			curGesto["fFeature"] = gesture.getFeatures()[0];
			curGesto["sFeature"] = gesture.getFeatures()[1];
			curGesto["length"] = gesture.getDur();
			for (int i = 0; i < gesture.size(); i++) {
				ofVec2f pos = gesture.play(i);
				ofJson jPos;
				jPos["x"] = pos.x;
				jPos["y"] = pos.y;
				curGesto["values"].push_back(jPos);
			}
			for (auto map : gesture.getParameters()) {
				ofJson fileParameters;
				fileParameters["file"] = map.first;
				for (auto key : map.second) fileParameters[key.first] = key.second;
				curGesto["parameters"].push_back(fileParameters);
			}
			jGestures.push_back(curGesto);
		}

		jOut["samples"] = jSamples;
		jOut["gestures"] = jGestures;
	
		if(ofSavePrettyJson(path, jOut)) cout << "saved to " << path << endl;
		else cout << "oops" << endl;
	}
}

void ofApp::oscNewFile(string path) {
	ofxOscMessage m;
	m.setAddress("/newFile");
	m.addStringArg(path);
	oscSender.sendMessage(m);
}

void ofApp::oscNewPos(string path, float timeMs, float length) {
	ofxOscMessage m;
	m.setAddress("/newGrain");
	m.addStringArg(path);
	m.addFloatArg(timeMs);
	m.addFloatArg(length);
	oscSender.sendMessage(m);
}

void ofApp::oscNewPos() {
	ofxOscMessage m;
	m.setAddress("/newGrain");
	m.addInt32Arg(selected.size());
	for (auto sample : selected) {
		m.addStringArg(samples[sample[0]].getFileName());
		m.addFloatArg(samples[sample[0]].getTime());
		m.addFloatArg(samples[sample[0]].getLength());
	}
	oscSender.sendMessage(m);
}

void ofApp::oscClear(bool init) {
	ofxOscMessage m;
	m.setAddress("/clear");
	m.addBoolArg(init);
	oscSender.sendMessage(m);
}

void ofApp::oscParameters(int gIndex) {
	//solo mando los valores de los keys en el archivo por defecto, ver comentario al principio
	if (gIndex > 0 && gIndex - 1 < gestures.size()) {
		for (auto file : gestures[gIndex - 1].getParameters()) {
			ofxOscMessage m;
			m.setAddress("/parameters");
			m.addStringArg(file.first);
			for (auto parameter : sDefaultParameters) {
				if(file.second.find(parameter) != file.second.end()) m.addFloatArg(file.second[parameter]);
			}
			oscSender.sendMessage(m);
		}
	}
}

void ofApp::oscReceive() {
	if (oscReceiver.hasWaitingMessages()) {
		int curKnn = knn;
		float curFrequency = frequency;

		while (oscReceiver.hasWaitingMessages()) {
			ofxOscMessage m;
			oscReceiver.getNextMessage(m);

			if (m.getAddress() == "/frequency") {
				frequency = m.getArgAsFloat(0);
				if (curFrequency != frequency) {
					frequency = curFrequency;
					knnGui->getSlider("frequency")->setValue(frequency);
				}
			}

			else if (m.getAddress() == "/gesture") {
				if (m.getNumArgs() == 2) {
					gLfoIndex = m.getArgAsInt(0);
					gLfoValue = m.getArgAsFloat(1);
				}
			}

			else if (m.getAddress() == "/curGesture") {
				if (m.getNumArgs() == 1) {
					gLfoIndex = activeGesto;
					gLfoValue = m.getArgAsFloat(0);
				}
			}

			else if (m.getAddress() == "/lfo") {
				fNLfoValues.clear();
				if (m.getNumArgs() == 2) {
					fNLfoValues.assign(features.size(), -1);
					for (int i = 0; i < features.size(); i++) {
						if (features[i] == selFeatures[0]) fNLfoValues[i] = m.getArgAsFloat(0);
						else if (features[i] == selFeatures[1]) fNLfoValues[i] = m.getArgAsFloat(1);
					}
				}
				else {
					for (int i = 0; i < features.size(); i++) {
						if (i < m.getNumArgs()) fNLfoValues.push_back(m.getArgAsFloat(i));
						else fNLfoValues.push_back(-1);
					}
				}
			}

			else if (m.getAddress() == "/mode") {
				int curMode = m.getArgAsInt(0);
				if (curMode < modes.size()) mode = curMode;
				knnGui->getDropdown("mode")->setLabel(modes[mode]);
				if (mode == 1) oscParameters(activeGesto);
			}

			else if (m.getAddress() == "/knn") {
				int curKnn = m.getArgAsInt(0);
				if (curKnn > 0 && knn <= 30) {
					knn = curKnn;
					knnGui->getSlider("knn")->setValue(knn);
				}

				if (curKnn != knn) {
					knn = curKnn;
					knnGui->getSlider("knn")->setValue(knn);
				}
			}

			else if (m.getAddress() == "/active") {
				bool curGo = m.getArgAsBool(0);
				go = curGo;
				knnGui->getToggle("go!")->setChecked(curGo);
			}

			else if (m.getAddress() == "/setDefaultParameters") {
				if (m.getNumArgs() % 2 == 0) {
					map<string, float> parameters;
					for (int i = 0; i < m.getNumArgs() * 0.5; i += 2) {
						if (m.getArgType(i) == OFXOSC_TYPE_STRING && m.getArgType(i + 1) == OFXOSC_TYPE_FLOAT) parameters[m.getArgAsString(i)] = m.getArgAsFloat(i + 1);
					}
					for (auto gesture : gestures) gesture.setDefaultParameters(parameters);
				}
			}

			else if (m.getAddress() == "/setParameters") {
				if (m.getNumArgs() > 2 && m.getNumArgs() % 2 == 0) {
					int gesture = m.getArgAsInt(0) - 1;
					if (gesture >= 0 && gesture < gestures.size()) {
						string file = paths[m.getArgAsInt(1)];
						map<string, float> parameters;
						for (int i = 0; i < (m.getNumArgs() - 2) * 0.5; i += 2) {
							if(m.getArgType(i) == OFXOSC_TYPE_STRING && m.getArgType(i + 1) == OFXOSC_TYPE_FLOAT) parameters[m.getArgAsString(i)] = m.getArgAsFloat(i + 1);
						}
						gestures[gesture].setParameters(file, parameters);
					}
				}
			}

			else if (m.getAddress() == "/setSimpleParameters") {
				//numGesture, source, p1, p2, ..., pn
				if (m.getNumArgs() == sDefaultParameters.size() + 2) {
					int gesture = m.getArgAsInt(0) - 1;
					string file = m.getArgAsString(1);
					if (gesture >= 0 && gesture < gestures.size() && gesture < gestures.size()) {
						map<string, float> curParameters;
						for (int i = 0; i < sDefaultParameters.size(); i++) curParameters[sDefaultParameters[i]] = m.getArgAsFloat(i + 2);
						gestures[gesture].setParameters(file, curParameters);
					}
				}
			}

			else if (m.getAddress() == "/gestureDial") {
				if (m.getNumArgs() == 1) {
					int dial = m.getArgAsInt(0);
					activeGesto += dial;
					if (activeGesto < 0) activeGesto = gestures.size();
					if (activeGesto > gestures.size()) activeGesto = 0;
					oscParameters(activeGesto);
				}
			}

			else if (m.getAddress() == "/setGesture") {
				if (m.getNumArgs() == 1) {
					int index = m.getArgAsInt(0);
					if (index > 0 && index < gestures.size()) activeGesto = index;
					oscParameters(activeGesto);
				}
			}
		}
	}
}

void ofApp::keyPressed(int key) {
	if (display == 1) {
		switch (key) {
		case OF_KEY_LEFT:
			scroll -= 5;
			break;
		case OF_KEY_RIGHT:
			scroll += 5;
			if (scroll > 0) scroll = 0;
			break;
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch (key) {
	case 'a':
	case 'A':
		iAnalize = 0;
		break;
	case 'C':
		clearAll();
		oscClear();
		updateFbo(selFeatures[0], selFeatures[1]);
		break;
	case 'g':
	case 'G':
		addGesture(curGesto);
		break;
	case 'h':
	case 'H':
		help = !help;
		break;
	case 'l':
	case 'L':
		loadFile();
		updateFbo(selFeatures[0], selFeatures[1]);
		break;
	case 'n':
	case 'N':
		normalizeFeatures();
		updateFbo(selFeatures[0], selFeatures[1]);
		break;
	case 's':
		saveFile(false);
		break;
	case 'S':
		saveFile(true);
		break;
	case 'X':
		if (samples.size() > selected.size() && (PCpressed && mode == 0)) {
			for (auto sample : selected) {
				if (sample[0] < samples.size()) {
					samples.erase(samples.begin() + sample[0]);
				}
			}
			updateFbo(selFeatures[0], selFeatures[1]);
			selected.clear();
		}
		break;
	case OF_KEY_TAB:
		display += 1;
		if (display > 1) display = 0;
		break;
	}
}

void ofApp::exit() {
	oscClear();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	PCpressed = false;
	if (display == 0) {
		if (x > pointCloudMargin.x && x < pointCloudMargin.x + pointCloud.getWidth()) {
			if (y > pointCloudMargin.y && y < pointCloudMargin.y + pointCloud.getHeight()) {
				PCpressed = true;
			}
		}
	}

	if (!prevPCpressed) curGesto.setStartTime((float)ofGetElapsedTimeMillis() * 0.001);

	if (PCpressed && !prevPCpressed) {
		curGesto.clear();
		curGesto.setFeatures(selFeatures);
		prevPCpressed = true;
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	if (prevPCpressed) curGesto.setEndTime((float)ofGetElapsedTimeMillis() * 0.001);
	PCpressed = false;
	prevPCpressed = false;
}
