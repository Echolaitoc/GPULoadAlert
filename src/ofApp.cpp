#include "ofApp.h"

void ofApp::setup(){
	debug = true;
#ifdef NDEBUG
	debug = false;
#endif
	gpuLow = false;
	nvidiaSetup();
	guiSetup();
	mailSetup();

	timestamp = ofGetElapsedTimef(); 
	resetWaitTimer();

	alertSound.load("beep.mp3");

	if (!debug)
	{
		FreeConsole();
	}
	else
	{
		ofSetLogLevel(ofLogLevel::OF_LOG_VERBOSE);
	}

	ofSetWindowTitle("GPU%");
}

void ofApp::update()
{
	if (!loadOk)
	{
		return;
	}
	(*NvAPI_GPU_GetUsages)(gpuHandles[gpuIndex], gpuUsages);
	int usage = gpuUsages[3];
	gpuLoad = ofToString(usage) + "%";

	if (monitor != isMonitoring)
	{
		resetWaitTimer();
	}
	isMonitoring = monitor;

	float now = ofGetElapsedTimef();
	float delta = now - timestamp;
	timestamp = now;

	if (usage < minimumLoad && monitor)
	{
		if (!gpuLow)
		{
			stopTime = getTimeString();
			if (debug)
			{
				ofLogVerbose("ofApp::update") << "[" << stopTime << "] GPU is now low";
			}
		}
		gpuLow = true;
		waitTimer -= delta;
		if (waitTimer < 0)
		{
			bool reset = periodical;
			monitor = reset;
			resetWaitTimer();
			alertLowGPU();
		}
	}
	else
	{
		if (gpuLow && debug && monitor)
		{
			ofLogVerbose("ofApp::update") << "[" << getTimeString() << "] GPU is busy";
		}
		gpuLow = false;
		resetWaitTimer();
	}
}

void ofApp::draw(){
	if (!loadOk)
	{
		ofDrawBitmapString("Couldn't load nvapi.dll / nvapi64.dll", 10, 20);
		return;
	}
	if (monitor)
	{
		ofBackgroundGradient(ofColor(115, 210, 115), ofColor(165, 210, 165));
	}
	else
	{
		ofBackgroundGradient(ofColor(150, 85, 85), ofColor(150, 115, 115));
	}
	gui.draw();
}

void ofApp::exit()
{
	ofLogVerbose("ofApp::exit") << "[" << getTimeString() << "] app shutting down";
	if (mailOk)
	{
		ofRemoveListener(smtp.events.onSMTPDelivery, this, &ofApp::onSMTPDelivery);
		ofRemoveListener(smtp.events.onSMTPException, this, &ofApp::onSMTPException);
	}
}

void ofApp::onSMTPDelivery(ofx::SMTP::Message::SharedPtr& message)
{
	ofLogVerbose("ofApp::onSMTPDelivery") << "Message Sent: " << message->getSubject();
}


void ofApp::onSMTPException(const ofx::SMTP::ErrorArgs& evt)
{
	ofLogError("ofApp::onSMTPException") << evt.getError().displayText();

	if (evt.getMessage())
	{
		ofLogError("ofApp::onSMTPException") << evt.getMessage()->getSubject();
	}
}

void ofApp::onSSLPrivateKeyPassphraseRequired(std::string& passphrase)
{
	//passphrase = ofSystemTextBoxDialog("Enter the Private Key Passphrase", "");
}

void ofApp::onSSLClientVerificationError(Poco::Net::VerificationErrorArgs& args)
{
	ofLogVerbose("ofApp::onClientVerificationError") << std::endl << ofToString(args);
}

void ofApp::nvidiaSetup()
{
	loadOk = true;
	HMODULE hmod = LoadLibraryA("nvapi.dll");
	if (hmod == NULL)
	{
		HMODULE hmod = LoadLibraryA("nvapi64.dll");
		if (hmod == NULL)
		{
			ofLogError("ofApp::nvidiaSetup") << "Couldn't load nvapi.dll or nvapi64.dll";
			loadOk = false;
			ofSetWindowShape(320, 30);
			return;
		}
	}

	NvAPI_QueryInterface = NULL;
	NvAPI_Initialize = NULL;
	NvAPI_EnumPhysicalGPUs = NULL;
	NvAPI_GPU_GetUsages = NULL;

	// nvapi_QueryInterface is a function used to retrieve other internal functions in nvapi.dll
	NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(hmod, "nvapi_QueryInterface");

	// some useful internal functions that aren't exported by nvapi.dll
	NvAPI_Initialize = (NvAPI_Initialize_t)(*NvAPI_QueryInterface)(0x0150E828);
	NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t)(*NvAPI_QueryInterface)(0xE5AC921F);
	NvAPI_GPU_GetUsages = (NvAPI_GPU_GetUsages_t)(*NvAPI_QueryInterface)(0x189A1FDF);

	if (NvAPI_Initialize == NULL || NvAPI_EnumPhysicalGPUs == NULL ||
		NvAPI_EnumPhysicalGPUs == NULL || NvAPI_GPU_GetUsages == NULL)
	{
		ofLogError("ofApp::nvidiaSetup") << "Couldn't get functions in nvapi.dll / nvapi64.dll";
		loadOk = false;
		ofSetWindowShape(320, 30);
		return;
	}

	// initialize NvAPI library, call it once before calling any other NvAPI functions
	(*NvAPI_Initialize)();

	gpuCount = 0;

	// gpuUsages[0] must be this value, otherwise NvAPI_GPU_GetUsages won't work
	gpuUsages[0] = (NVAPI_MAX_USAGES_PER_GPU * 4) | 0x10000;

	(*NvAPI_EnumPhysicalGPUs)(gpuHandles, &gpuCount);
}

void ofApp::guiSetup()
{
	gui.setup("", "settings.xml", 10, 10);
	gui.add(monitor.setup("Monitor GPU", false));
	gui.add(periodical.setup("Periodical Alert", true));
	gui.add(gpuIndex.setup("Which GPU No.", gpuCount - 1, 0, gpuCount - 1));
	gui.add(useMail.setup("Use Mail", false));
	gui.add(useSound.setup("Use Alert Sound", true));
	gui.add(waitTime.setup("Wait Time (Minutes)", 1, 0, 15));
	gui.add(minimumLoad.setup("Minimum GPU Load", 80, 0, 100));
	gui.add(gpuLoad.setup("Current GPU Load", "0%"));
	gui.loadFromFile("settings.xml");
	ofSetWindowShape(gui.getWidth() + 20, gui.getHeight() + 20);
}

void ofApp::mailSetup()
{
	ofSSLManager::registerClientEvents(this);
	string file = "smtp.xml";
	mailOk = false;
	if (ofFile::doesFileExist(file))
	{
		ofx::SMTP::Settings sslSettings = ofx::SMTP::Settings::loadFromXML(file);
		email = sslSettings.getCredentials().getUsername();
		smtp.setup(sslSettings);
		ofAddListener(smtp.events.onSMTPDelivery, this, &ofApp::onSMTPDelivery);
		ofAddListener(smtp.events.onSMTPException, this, &ofApp::onSMTPException);
		mailOk = true;
	}

	if (!mailOk)
	{
		ofLogError("ofApp::mailSetup") << "[" << getTimeString() << "] mail setup from smtp.xml failed";
	}
}

void ofApp::alertLowGPU()
{
	if (debug)
	{
		ofLogVerbose("ofApp::alertLowGPU") << "[" << getTimeString() << "] sending alert";
	}
	if (alertSound.isLoaded() && useSound)
	{
		alertSound.play();
	}
	if (mailOk && useMail)
	{
		if (debug)
		{
			ofLogVerbose("ofApp::alertLowGPU") << "[" << getTimeString() << "] sending mail (only in release)";
		}
		else
		{
			ofx::SMTP::Message::SharedPtr message = ofx::SMTP::Message::makeShared();
			message->setSender(Poco::Net::MailMessage::encodeWord(email, "UTF-8"));
			message->addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, email));
			message->setSubject(Poco::Net::MailMessage::encodeWord("GPU Alert " + getDateString() + " " + getTimeString(), "UTF-8"));
			string content = "GPU Alert\n";
			int load = minimumLoad;
			content += "GPU load is below threshold of " + ofToString(load) + "% since " + stopTime;
			message->addContent(new Poco::Net::StringPartSource(content));
			smtp.send(message);
		}
	}
}

void ofApp::resetWaitTimer()
{
	if (debug)
	{
		waitTimer = waitTime;
	}
	else
	{
		waitTimer = waitTime * 60;
	}
}

string ofApp::getDateString()
{
	return to_string(ofGetYear()) + "-" + getDoubleDigitString(ofGetMonth()) + "-" + getDoubleDigitString(ofGetDay());
}

string ofApp::getTimeString()
{
	return getDoubleDigitString(ofGetHours()) + ":" + getDoubleDigitString(ofGetMinutes()) + ":" + getDoubleDigitString(ofGetSeconds());
}

string ofApp::getDoubleDigitString(int number)
{
	string numString = to_string(number);
	return (numString.length() > 1 ? numString : "0" + numString);
}