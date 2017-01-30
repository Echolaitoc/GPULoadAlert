#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxSMTP.h"

// reference: http://eliang.blogspot.de/2011/05/getting-nvidia-gpu-usage-in-c.html
// magic numbers, do not change them
#define NVAPI_MAX_PHYSICAL_GPUS   64
#define NVAPI_MAX_USAGES_PER_GPU  34

class ofApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	void exit();
	void onSMTPDelivery(ofx::SMTP::Message::SharedPtr& message);
	void onSMTPException(const ofx::SMTP::ErrorArgs& evt);
	void onSSLClientVerificationError(Poco::Net::VerificationErrorArgs& args);
	void onSSLPrivateKeyPassphraseRequired(std::string& passphrase);

private:
	// function pointer types
	typedef int *(*NvAPI_QueryInterface_t)(unsigned int offset);
	typedef int(*NvAPI_Initialize_t)();
	typedef int(*NvAPI_EnumPhysicalGPUs_t)(int **handles, int *count);
	typedef int(*NvAPI_GPU_GetUsages_t)(int *handle, unsigned int *usages);

	void nvidiaSetup();
	void guiSetup();
	void mailSetup();
	void alertLowGPU();
	void resetWaitTimer();
	string getDateString();
	string getTimeString();
	string getDoubleDigitString(int number);

	// nvapi.dll internal function pointers
	NvAPI_QueryInterface_t NvAPI_QueryInterface;
	NvAPI_Initialize_t NvAPI_Initialize;
	NvAPI_EnumPhysicalGPUs_t NvAPI_EnumPhysicalGPUs;
	NvAPI_GPU_GetUsages_t NvAPI_GPU_GetUsages;

	ofx::SMTP::Client smtp;
	ofSoundPlayer alertSound;
	ofxPanel gui;
	ofxToggle monitor;
	ofxIntSlider gpuIndex;
	ofxIntSlider waitTime;
	ofxIntSlider minimumLoad;
	ofxToggle useMail;
	ofxToggle useSound;
	ofxLabel gpuLoad;
	HMODULE hmod;
	string stopTime;
	string email;
	float waitTimer;
	float timestamp;
	int gpuCount;
	int *gpuHandles[NVAPI_MAX_PHYSICAL_GPUS];
	unsigned int gpuUsages[NVAPI_MAX_USAGES_PER_GPU];
	bool loadOk;
	bool gpuLow;
	bool debug;
	bool mailOk;
	bool isMonitoring;
};
