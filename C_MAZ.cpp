/*
 * MAZ.cpp
 *
 *  Created on: 24.05.2011
 *      Author: A0CA1E4
 */


#include "stdafx.h"
#include "windows.h"

#include "Control/C_MAZ.h"

#include "Model/Config/MC_Load.h"
#include "Model/Config/MC_Save.h"

#include "Model/LocalConfig/MLC_Load.h"
#include "Model/LocalConfig/MLC_Save.h"

#include <fstream>
#include <ostream>
#include <sstream>
#include "Protok.h"
#include "MD5/md5.h"
#include "base64.h"
//#include "EDP/edp.h"
#include <stdlib.h>
#include <time.h>

#define MAZ_RISNET_DE "maz.eil.risnet.de"

namespace Control {

MAZ::MAZ()
: config(new Model::Config::Root()),
  keyTrap(NULL),
  mouseTrap(NULL),
  trapAction(NULL),
  taskBar(NULL),
  desktop(NULL),
  loginAction(NULL),
  dStore(NULL),
  httpGetAction(NULL),
  unzip(NULL),
  valueLookup(NULL),
  localConfig(new Model::LocalConfig::Root()),
  currentUser(NULL)
{
	resetTimeVars();
}

MAZ::~MAZ() {
	delete config;
	delete localConfig;
}

void MAZ::run() {
	bool running = true;

	printfInfo("%s: Lade Konfiguration\r\n", logTimeStr());
	loadConfig();

	enableKeyTrap(localConfig->getKeyHook());
	hideTaskBar(true);
	
	printfInfo("%s: Starte Update von maz.cfg\r\n", logTimeStr());
	updateMazCfg();
	printfInfo("%s: Starte Update von ressoure.zip\r\n", logTimeStr());
	updateResource();
	printfInfo("%s: Hohle Wetter\r\n", logTimeStr());
	updateWetter();

	printfInfo("%s: Erneutes Lade der Konfiguration\r\n", logTimeStr());
	loadConfig();

	printfInfo("%s: Starte Hauptschleife\r\n", logTimeStr());
	while(running) {
		while(running && getCurrentUser() == NULL) {
			printfInfo("%s: Run Login\r\n", logTimeStr());
			running = runLogin();
		}
		if(running) {
			printfInfo("%s: MAZ Client gestartet\r\n", logTimeStr());
			enableMouseTrap(true);
			running = runDesktop();
			enableMouseTrap(false);
		}
		printfInfo("%s: Logout\r\n", logTimeStr());
		logout();
	}

	printfInfo("%s: Setze Windows-Manipulationen zurueck\r\n", logTimeStr());
	enableKeyTrap(false);
	hideTaskBar(false);

	setKeyTrapSwitch(NULL);
	setMouseTrapSwitch(NULL);
	setTrapAction(NULL);
	setTaskBarSwitch(NULL);
	setDesktopAction(NULL);
	setLoginAction(NULL);
	setDStore(NULL);
	setHttpGetAction(NULL);
	setUnzip(NULL);
	printfInfo("%s: Beende MAZ\r\n", logTimeStr());
}

void MAZ::setKeyTrapSwitch(SwitchEvent* aKeyTrap) {
	if(keyTrap && keyTrap != aKeyTrap) {
		keyTrap->setHandler(NULL);
	}
	keyTrap = aKeyTrap;
	if(keyTrap) {
		keyTrap->setHandler(trapAction);
	}
}

void MAZ::setMouseTrapSwitch(SwitchEvent* aMouseTrap) {
	if(mouseTrap && mouseTrap != aMouseTrap) {
		mouseTrap->setHandler(NULL);
	}
	mouseTrap = aMouseTrap;
	if(mouseTrap) {
		mouseTrap->setHandler(trapAction);
	}
}

void MAZ::setTrapAction(Action* aTrapAction) {
	if(trapAction && trapAction != aTrapAction) {
		if(keyTrap) {
			keyTrap->setHandler(NULL);
		}
		if(mouseTrap) {
			mouseTrap->setHandler(NULL);
		}
	}
	trapAction = aTrapAction;
	if(keyTrap) {
		keyTrap->setHandler(trapAction);
	}
	if(mouseTrap) {
		mouseTrap->setHandler(trapAction);
	}
}

void MAZ::setTaskBarSwitch(Switch* aTaskBar) {
	taskBar = aTaskBar;
}

void MAZ::setDesktopAction(Action* aDesktop) {
	desktop = aDesktop;
}

void MAZ::setLoginAction(Action* aLoginAction) {
	loginAction = aLoginAction;
}

void MAZ::setDStore(DStore* aDStore) {
	dStore = aDStore;
}

void MAZ::setHttpGetAction(HttpGet* aHttpGet) {
	httpGetAction = aHttpGet;
}

void MAZ::setUnzip(Unzip* aUnzip) {
	unzip = aUnzip;
}

void MAZ::setValueLookup(ValueLookup* aValueLookup) {
	valueLookup = aValueLookup;
}

void MAZ::setWeather(const std::string& aWeather) {
	weather = aWeather;
}

const std::string& MAZ::getWeather() const {
	return weather;
}

void MAZ::setHostname(const std::string& aHostname) {
	hostname = aHostname;
}

const std::string& MAZ::getHostname() const {
	if(localConfig->getHostname() != "") {
		return localConfig->getHostname();
	}
	return hostname;
}

std::string MAZ::getMbbk() const {
	std::string aMBBK;

	aMBBK = getHostname();

	// entferne die vorderen Zeichen, die nicht 0-9 sind
	while(!aMBBK.empty()) {
		if(aMBBK.at(0) >= '0' && aMBBK.at(0) <= '9') {
			break;
		}
		aMBBK = aMBBK.substr(1, aMBBK.size()-1);
	}

	// pruefe, ob nach den Zahlen noch andere Zeichen kommen. Wenn ja, dann abschneiden.
	for(std::string::size_type i=0; i<aMBBK.size(); ++i) {
		if(aMBBK.at(i) < '0' || aMBBK.at(i) > '9') {
			aMBBK = aMBBK.substr(0, i);
			break;
		}
	}
	return aMBBK;
}

std::string MAZ::getMarktnummer() const {
	std::string result;
	std::string tmp;

	tmp = MAZ::getValue("REGION", NULL);
	while(result.size() + tmp.size() < 2) {
		result += "0";
	}
	result += tmp;

	tmp = MAZ::getValue("BEREICH", NULL);
	while(result.size() + tmp.size() < 4) {
		result += "0";
	}
	result += tmp;

	tmp = MAZ::getValue("ZAEHLNUMMER", NULL);
	while(result.size() + tmp.size() < 8) {
		result += "0";
	}
	result += tmp;

	return result;
}

std::string MAZ::getTime() const {
	std::stringstream strStream;

	strStream << (long) timeValue;
	return strStream.str();
}

std::string MAZ::getDateTime() const {
	struct tm*  tmValue;
    time_t      localTimeValue;
    char buffer [64];

    localTimeValue = timeValue;
    if(localTimeValue == -1 || (tmValue = localtime(&localTimeValue)) == NULL) {
    	return "";
    }

    strftime(buffer, 64, "%Y%m%d%H%M%S", tmValue);
    return buffer;
}

void MAZ::setPassword(const std::string& aPassword) {
	password = aPassword;
}

const std::string& MAZ::getPassword() const {
	return password;
}


LoginResult MAZ::testLogin(const std::string &aUserID, const std::string &aPassword) {
	Model::LocalConfig::User* aLocalUser;
	Model::Config::User* aUser;

	printfDebug("%s: Test-Login\r\n", logTimeStr());

	aUser = config->getUser(aUserID);
	if(aUser == NULL) {
		/* User nicht registriert */
		printfDebug("%s: User ist in der maz.cfg nicht vorhanden\r\n", logTimeStr());
		return lrUnknownUser;
	}

	aLocalUser = localConfig->getUser(aUserID);
	switch(dStoreAuthenticate(aUserID, aPassword)) {
	case arOK:
		printfDebug("%s: DStore-Authentifizierung erfolgreich\r\n", logTimeStr());
		if(aLocalUser == NULL) {
			printfDebug("%s: Benutzer wurde der lokalen Konfiguration hinzugefurgt\r\n", logTimeStr());
			aLocalUser = localConfig->addUser(aUserID);
		}
		aLocalUser->setHash(makeHash(aPassword));
		saveLocalConfig();
		/* Authentifizierung erfolgreich */
		return lrOK;
	case arError:
		printfError("%s: Es ist ein Fehler in der DStore-Authentifizierung aufgetreten\r\n", logTimeStr());
		if(aLocalUser != NULL
		&& aLocalUser->getHash() == makeHash(aPassword)) {
			printfDebug("%s: Lokale Authentifizierung erfolgreich\r\n", logTimeStr());
			/* Authentifizierung lokal erfolgreich */
			return lrLocalOK;
		}
	case arFailed:
	default:
		printfDebug("%s: DStore-Authentifizierung fehlgeschlagen\r\n", logTimeStr());
		break;
	}

	/* Authentifizierung fehlgeschlagen */
	return lrFailed;
}

LoginResult MAZ::login(const std::string &aUserID, const std::string &aPassword) {
	LoginResult aResult;

	printfDebug("%s: Login von UserID \"%s\" ...\r\n", logTimeStr(), aUserID.c_str());
	if(currentUser) {
		printfError("%s: Es ist bereits ein User eingeloggt\r\n", logTimeStr(), aUserID.c_str());
		return lrError;
	}

	aResult = testLogin(aUserID, aPassword);

	if(aResult == lrOK || aResult == lrLocalOK) {
		currentUser = config->getUser(aUserID);
		if(currentUser == NULL) {
			/* User nicht registriert */
			printfDebug("%s: User ist in der maz.cfg nicht vorhanden\r\n", logTimeStr());
			return lrUnknownUser;
		}
		setPassword(aPassword);
	}

	return aResult;
}

Model::Config::User* MAZ::getCurrentUser() const {
	return currentUser;
}

void MAZ::setNextUsername(const std::string &aNextUsername) {
	nextUsername = aNextUsername;
}

const std::string& MAZ::getNextUsername() const {
	return nextUsername;
}

void MAZ::setNextPassword(const std::string &aNextPassword) {
	nextPassword = aNextPassword;
}

const std::string& MAZ::getNextPassword() const {
	return nextPassword;
}

const Model::LocalConfig::Root& MAZ::getLocalConfig() const {
	return *localConfig;
}

void MAZ::makeHomepage() {
	std::ifstream iStream("C:\\MAZ\\desktop\\res\\template.html");

	printfDebug("%s: Erzeuge Homepage\r\n", logTimeStr());

	// existiert kein Template?
	if(!iStream.good()) {
		printfDebug("%s: Datei \"C:\\MAZ\\desktop\\res\\template.html\" konnte nicht geoeffnet werden.\r\n", logTimeStr());
		// existiert denn bereits eine Homepage?
		std::ifstream iStream2("C:\\MAZ\\desktop\\homepage.html");
		if(iStream2.good()) {
			printfDebug("%s: Datei \"C:\\MAZ\\desktop\\homepage.html\" existiert bereits und wird beibehalten.\r\n", logTimeStr());
			// Ja?, dann fertig.
			return;
		}
	}


	std::ofstream oStream("C:\\MAZ\\desktop\\homepage.html");
	
	// es existiert kein Template und noch keine Homepage?
	if(!iStream.good()) {
		// Nein? Dann erzeuge eins.
		std::stringstream sStream;

		printfDebug("%s: Datei \"C:\\MAZ\\desktop\\homepage.html\" wird mit Standardinhalt gefuellt.\r\n", logTimeStr());

		sStream << "<html>" << std::endl;
		sStream << "<body>" << std::endl;
		sStream << "hello</br>" << std::endl;
		sStream << "USER:     ${USER}</br>" << std::endl;
		sStream << "ROLE:     ${ROLE}</br>" << std::endl;
		sStream << "CFG_TS:   ${CFG_TS}</br>" << std::endl;
		sStream << "RES_TS:   ${RES_TS}</br>" << std::endl;
		sStream << "URI:      ${URI}</br>" << std::endl;
		sStream << "WETTER:   ${WETTER}</br>" << std::endl;
		sStream << "HOSTNAME: ${HOSTNAME}</br>" << std::endl;
		sStream << "COMPILE_DATE:  ${COMPILE_DATE}</br>" << std::endl;
		sStream << "VERSION:  ${VERSION}</br>" << std::endl;
		sStream << "</body>" << std::endl;
		sStream << "</html>" << std::endl;
		substitute(oStream, sStream.str(), NULL);
	}
	else {
		printfDebug("%s: Datei \"C:\\MAZ\\desktop\\res\\template.html\" wird zu \"homepage.html\" substituiert.\r\n", logTimeStr());
		while(!iStream.eof() && iStream.good()) {
			std::string aStr;
			getline(iStream, aStr);
			substitute(oStream, aStr, NULL);
			oStream << std::endl;
		}
		iStream.close();
	}

	oStream.close();
}
std::string MAZ::getFile(const std::string &aFile) const {
	std::stringstream aStrStream;
	std::string aStrRes;
	std::ifstream iStream(aFile.c_str(), std::ios::in | std::ios::binary);

	if(!iStream) {
		return "";
	}
 
	while(iStream) { // in will be false when eof is reached
		char aChar;
		iStream.get(aChar);
		if(iStream) {
			aStrStream << aChar;
		}
	}

	iStream.close();
	aStrRes = aStrStream.str();
	return aStrRes;
}

void MAZ::mkFile(const std::string &aDstName, const std::string &aSrcName, const Model::Config::Element* aElement) const {
	std::fstream aSrcFStream;
	std::fstream aDstFStream;
  
	// Source und Destination oeffnen
	aDstFStream.open(aDstName.c_str(), std::ios::out | std::ios::trunc);
	aSrcFStream.open(aSrcName.c_str(), std::ios::in);

	if(aDstFStream.good()) {
		while(!aSrcFStream.eof() && aSrcFStream.good()) {
			std::string aStr;
			getline(aSrcFStream, aStr);
			substitute(aDstFStream, aStr, aElement);
			aDstFStream << std::endl;
		}
	}

	// Source und Destination schliessen
	if(aDstFStream.is_open()) {
		aDstFStream.close();
	}
	if(aSrcFStream.is_open()) {
		aSrcFStream.close();
	}
}

void MAZ::resetTimeVars() {
	timeValue = time(NULL);	
}

void MAZ::substitute(std::ostream &aOut, const std::string& aValue, const Model::Config::Element* aElement) const {
	for(std::string::const_iterator aIter = aValue.begin(); aIter != aValue.end(); ++aIter) {
		if(*aIter != '$') {
			aOut << *aIter;
			continue;
		}

		std::string::const_iterator aIter2;
		aIter2 = aIter;

		++aIter2;
		if(aIter2 == aValue.end() || *aIter2 != '{') {
			aOut << *aIter;
			continue;
		}

		++aIter2;
		aIter = aIter2;
		int aDepth = 1;

		while(aIter != aValue.end()) {
			if(*aIter == '}') {
				--aDepth;
			}
			if(*aIter == '{') {
				++aDepth;
			}
			if(aDepth <= 0) {
				break;
			}
			++aIter;
		}

		aOut << getValue(std::string(aIter2, aIter), aElement);

		if(aIter == aValue.end()) {
			break;
		}
	}
}

std::string MAZ::extractMakroParam(std::string &aParam, const Model::Config::Element* aElement) const {
	std::stringstream aStrStream;
	std::string aTmpStr;

	// fuehrende Leerzeichen entfernen.
	while(!aParam.empty() && aParam.at(0) == ' ') {
		aParam = aParam.substr(1, aParam.size()-1);
	}

	while(!aParam.empty() && aParam.at(0) != ' ') {
		aTmpStr += aParam.at(0);
		aParam = aParam.substr(1, aParam.size()-1);
	}
	substitute(aStrStream, aTmpStr, aElement);
	return aStrStream.str();
}

std::string MAZ::getValue(const std::string& aKey, const Model::Config::Element* aElement) const {
	if(valueLookup) {
		std::string aValue;

		aValue = (*valueLookup)(aKey, aElement);
		if(aValue != "") {
			return aValue;
		}
	}

	if(aKey == "USER") {
		if(getCurrentUser()) {
			return getCurrentUser()->getName();
		}
		return "";
	}

	if(aKey == "ROLE") {
		if(getCurrentUser()) {
			return getCurrentUser()->getRole();
		}
		return "";
	}

	if(aKey == "PASSWORD") {
		if(getCurrentUser()) {
			return getPassword();
		}
		return "";
	}

	if(aKey == "CFG_TS") {
		return config->getTimestamp();
	}

	if(aKey == "RES_TS") {
		return localConfig->getTimestamp();
	}

	if(aKey == "MAZ_SERVER") {
		return localConfig->getMazServer();
	}

	if(aKey == "WETTER") {
		std::stringstream aStringStream;
		substitute(aStringStream, getWeather(), NULL);
		return aStringStream.str();
	}

	if(aKey == "HOSTNAME") {
		return getHostname();
	}

	if(aKey == "MBBK") {
		return getMbbk();
	}

	if(aKey == "MARKTNUMMER") {
		return getMarktnummer();
	}

	if(aKey == "TIME") {
		return getTime();
	}

	if(aKey == "DATETIME") {
		return getDateTime();
	}

	if(aKey == "EDP_AUTO_PWD") {
		std::string aMbbk;
		std::string aUser;
		std::string aResult;

		aMbbk = getMbbk();

		if(getCurrentUser() == NULL) {
			return "";
		}
		aUser = getCurrentUser()->getName();

		if(aMbbk.length() != 7 || aUser.length() < 3 || aUser.length() > 4) {
			return "";
		}
		
		aResult = aUser;
		aResult.insert(2, aMbbk.substr(4, 2));
		aResult += aMbbk.substr(6, 1);
		
		while(aResult.length() < 8) {
			aResult += "9";
		}

		return aResult;
	}


	if(aKey == "SPACE") {
		return " ";
	}

	if(aKey == "COMPILE_DATE") {
		return __DATE__;
	}

	if(aKey == "VERSION") {
		return "1.4";
	}

	if(aKey.size() >= 4 && aKey.substr(0, 4) == "MD5 ") {
		std::stringstream aStrStream;

		substitute(aStrStream, aKey.substr(4, aKey.size()-4), aElement);
		return md5(aStrStream.str());
	}

	if(aKey.size() >= 7 && aKey.substr(0, 7) == "BASE64 ") {
		std::stringstream aStrStream;

		substitute(aStrStream, aKey.substr(7, aKey.size()-7), aElement);
		return base64(aStrStream.str());
	}

	if(aKey.size() >= 7 && aKey.substr(0, 7) == "SUBSTR ") {
		std::string aParam;
		std::string aStr; /* aFileName; */
		int aPos;
		int aLen;
  
		aParam = aKey.substr(7, aKey.size()-7);
		printfDebug("%s: Aufruf von Makro SUBSTR\r\n", logTimeStr());

		// 1. Parameter (String) auslesen
		aStr = extractMakroParam(aParam, aElement);
		printfDebug("%s: 1. Parameter (Str): %s\r\n", logTimeStr(), aStr.c_str());

		// 2. Parameter (Position) auslesen
		aPos = atoi(extractMakroParam(aParam, aElement).c_str());
		printfDebug("%s: 2. Parameter (pos)     : %d\r\n", logTimeStr(), aPos);
		if(aPos < 0) {
			aPos = 0;
		}

		// 3. Parameter (Length) auslesen
		aLen = atoi(extractMakroParam(aParam, aElement).c_str());
		printfDebug("%s: 3. Parameter (len)     : %d\r\n", logTimeStr(), aLen);
		if(aLen > (int)(aStr.size() - aPos)) {
			aLen = aStr.size() - aPos;
		}

		return aStr.substr(aPos, aLen);
	}

	if(aKey.size() >= 5 && aKey.substr(0, 5) == "FILE ") {
		std::stringstream aStrStream;
		std::string aStrArg;
		std::string aStrRes;

		aStrArg = aKey.substr(5, aKey.size()-5);
		substitute(aStrStream, aStrArg, aElement);
		aStrArg = aStrStream.str();

		aStrRes = getFile(aStrArg);
		return aStrRes;
	}

	if(aKey.size() >= 8 && aKey.substr(0, 8) == "MK_FILE ") {
		std::string aParam;
		std::string aDstName; /* aFileName; */
		std::string aSrcName;
  
		aParam = aKey.substr(8, aKey.size()-8);
		printfDebug("%s: Aufruf von Makro MK_FILE\r\n", logTimeStr());

		// 1. Parameter (Destination Filename) auslesen
		aDstName = extractMakroParam(aParam, aElement);
		printfDebug("%s: 1. Parameter (Destination): %s\r\n", logTimeStr(), aDstName.c_str());

		// 2. Parameter (Source Filename) auslesen
		aSrcName = extractMakroParam(aParam, aElement);
		printfDebug("%s: 2. Parameter (Source)     : %s\r\n", logTimeStr(), aSrcName.c_str());

		mkFile(aDstName, aSrcName, aElement);

		return "";
	}
/*
	if(aKey.size() >= 8 && aKey.substr(0, 8) == "EDPHTML ") {
		std::string aParam;
		std::string aFileName;
		std::string aMBBK;
		std::string aUser;
		std::string aTarget;
		std::string aHost;
		std::string aURLPostfix;

		aParam = aKey.substr(8, aKey.size()-8);
		printfDebug("%s: Aufruf von Makro EDPHTML\r\n", logTimeStr());
		// 1. Parameter (Filename) auslesen
		aFileName = extractMakroParam(aParam, aElement);
		printfDebug("%s: 1. Parameter (filename) : %s\r\n", logTimeStr(), aFileName.c_str());

		// 2. Parameter (MBBK) auslesen
		aMBBK = extractMakroParam(aParam, aElement);
		printfDebug("%s: 2. Parameter (MBBK)     : %s\r\n", logTimeStr(), aMBBK.c_str());

		// 3. Parameter (User) auslesen
		aUser = extractMakroParam(aParam, aElement);
		printfDebug("%s: 3. Parameter (user)     : %s\r\n", logTimeStr(), aUser.c_str());

		// 4. Parameter (Target) auslesen
		aTarget = extractMakroParam(aParam, aElement);
		printfDebug("%s: 4. Parameter (target)   : %s\r\n", logTimeStr(), aTarget.c_str());

		// 5. Parameter (Host) auslesen
		aHost = extractMakroParam(aParam, aElement);
		printfDebug("%s: 5. Parameter (host)     : %s\r\n", logTimeStr(), aHost.c_str());

		// 6. Parameter (URL-Postfix) auslesen
		aURLPostfix = extractMakroParam(aParam, aElement);
		printfDebug("%s: 6. Parameter (URL-PF.)  : %s\r\n", logTimeStr(), aURLPostfix.c_str());

		if(edpHtml(aFileName, aMBBK, aUser, aTarget, aHost, aURLPostfix) == false) {
//		if(edpHtml(aFileName, aMBBK, aUser, aTarget, aHost) == 0) {
			printfError("%s: Es ist ein Fehler beim Aufruf des EDPHTML-Makros aufgetreten\r\n", logTimeStr());
		}
		return "";
	}
*/
	if(aKey == "TO_CRONJOB") {
		std::string aStr;

		aStr = 	config->getValue(aKey);
		if(aStr == "") {
			std::stringstream aStrStream;

			aStrStream << CRONJOB_TIMEOUT;
			aStr = aStrStream.str();
		}
		return aStr;
	}

	if(aKey == "TO_UPDATE") {
		std::string aStr;

		aStr = 	config->getValue(aKey);
		if(aStr == "") {
			std::stringstream aStrStream;

			aStrStream << UPDATE_TIMEOUT;
			aStr = aStrStream.str();
		}
		return aStr;
	}

	if(aKey == "TO_SHUTDWN") {
		std::string aStr;

		aStr = 	config->getValue(aKey);
		if(aStr == "") {
			std::stringstream aStrStream;

			aStrStream << SHUTDOWN_TIMEOUT;
			aStr = aStrStream.str();
		}
		return aStr;
	}

	if(aKey == "TO_LOCK") {
		std::string aStr;

		aStr = 	config->getValue(aKey);
		if(aStr == "") {
			std::stringstream aStrStream;

			aStrStream << LOCK_TIMEOUT;
			aStr = aStrStream.str();
		}
		return aStr;
	}

	return config->getValue(aKey);
}

void MAZ::enableKeyTrap(bool doEnable) {
	if(keyTrap == NULL) {
		return;
	}
	if(doEnable) {
		keyTrap->enable();
	}
	else {
		keyTrap->disable();
	}
}

void MAZ::enableMouseTrap(bool doEnable) {
	if(mouseTrap == NULL) {
		return;
	}
	if(doEnable) {
		mouseTrap->enable();
	}
	else {
		mouseTrap->disable();
	}
}

void MAZ::hideTaskBar(bool doHide) {
	if(taskBar == NULL) {
		return;
	}
	if(doHide) {
		taskBar->disable();
	}
	else {
		taskBar->enable();
	}
}

bool MAZ::runDesktop() {
	if(desktop) {
		return (*desktop)();
	}
	return true;
}

bool MAZ::runLogin() {
	if(loginAction) {
		return (*loginAction)();
	}
	return false;
};

void MAZ::updateMazCfg() {
	int aResult;
	std::string aMAZServerHostname;
	std::string aRequest;
	char* aResponse = NULL;
	unsigned long aFileLength = 0;

	if(localConfig->getMazServer() == "") {
		aMAZServerHostname = MAZ_RISNET_DE;
		printfDebug("%s: verwende Default-Server: %s\r\n", logTimeStr(), aMAZServerHostname.c_str());
	}
	else {
		aMAZServerHostname = localConfig->getMazServer();
		printfDebug("%s: verwende konfigurierten Server: %s\r\n", logTimeStr(), aMAZServerHostname.c_str());
	}

	aRequest = "configuration/maz/" + getMbbk();
	if(config->getTimestamp() != "") {
		aRequest = aRequest + "?timestamp=" + config->getTimestamp();
	}
	printfDebug("%s: Request: %s\r\n", logTimeStr(), aRequest.c_str());

	aResult = httpGet(aMAZServerHostname, aRequest, &aResponse, aFileLength);
	if(aResult == 200) {
		if(aFileLength > 0) {
			printfDebug("%s: Server liefert eine neue Konfiguration.\r\n", logTimeStr());
			std::ofstream aFile("C:\\MAZ\\desktop\\maz.cfg");
			aFile.write(aResponse, aFileLength);
			aFile.close();
		}
		else {
			printfDebug("%s: Die Konfiguration ist bereits aktuell\r\n", logTimeStr());
		}
	}
	else {
		printfDebug("%s: Server liefert Fehlercode %d\r\n", logTimeStr(), aResult);
	}

	if(aResponse) {
		delete[] aResponse;
	}
}

void MAZ::updateResource() {
	int aResult;
	std::string aMAZServerHostname;
	std::string aRequest;
	char* aResponse = NULL;
	unsigned long aFileLength = 0;
	
	// Download von resource.zip
	if(localConfig->getMazServer() == "") {
		aMAZServerHostname = MAZ_RISNET_DE;
		printfDebug("%s: verwende Default-Server: %s\r\n", logTimeStr(), aMAZServerHostname.c_str());
	}
	else {
		aMAZServerHostname = localConfig->getMazServer();
		printfDebug("%s: verwende konfigurierten Server: %s\r\n", logTimeStr(), aMAZServerHostname.c_str());
	}

	aRequest = "configuration/resource/" +  getMbbk();
	if(localConfig->getTimestamp() != "") {
		aRequest = aRequest + "?timestamp=" + localConfig->getTimestamp();
	}
	printfDebug("%s: Request: %s\r\n", logTimeStr(), aRequest.c_str());

	aResult = httpGet(aMAZServerHostname, aRequest, &aResponse, aFileLength);
	if(aResult == 200) {
		if(aFileLength > 15) {
			std::ofstream aFile("C:\\MAZ\\desktop\\resource.zip", std::ios::out | std::ios::binary);
			
			printfDebug("%s: Server liefert neue Ressourcen.\r\n", logTimeStr());

			localConfig->setTimestamp(std::string(aResponse, 0, 14));
			saveLocalConfig();
			printfDebug("%s: Timestamp der Resourcen: %s\r\n", logTimeStr(), localConfig->getTimestamp().c_str());

			aFile.write(&aResponse[15], aFileLength-15);
			aFile.close();
		}
		else if(aFileLength > 0) {
			printfError("%s: Die Antwort vom Server ist nicht korrekt\r\n", logTimeStr());
		}
		else {
			printfDebug("%s: Die Ressourcen sind bereits aktuell\r\n", logTimeStr());
		}
	}
	else {
		printfDebug("%s: Server liefert Fehlercode %d\r\n", logTimeStr(), aResult);
	}
	if(aResponse) {
		delete[] aResponse;
	}

	// Entpacken
	if(doUnzip("C:\\MAZ\\desktop\\resource.zip", "C:\\MAZ\\desktop\\res") == false) {
		printfWarn("%s: Fehler beim Entpacken aufgetreten.\r\n", logTimeStr());
		return;
	}
	printfDebug("%s: ZIP Datei \"C:\\MAZ\\desktop\\resource.zip\" wurde erfolgreich entpackt.\r\n", logTimeStr());
}

void MAZ::updateWetter() {
	std::stringstream sStream;
	int aResult;
	std::string aMAZServerHostname;
	std::string aRequest;
	char* aResponse = NULL;
	unsigned long aFileLength = 0;
	
	// Download von weather.html
	if(localConfig->getMazServer() == "") {
		aMAZServerHostname = MAZ_RISNET_DE;
		printfDebug("%s: verwende Default-Server: %s\r\n", logTimeStr(), aMAZServerHostname.c_str());
	}
	else {
		aMAZServerHostname = localConfig->getMazServer();
		printfDebug("%s: verwende konfigurierten Server: %s\r\n", logTimeStr(), aMAZServerHostname.c_str());
	}

	aRequest = "configuration/weather/" + getMbbk();
//	if(config->getTimestamp() != "") {
//		aRequest = aRequest + "?timestamp=" + config->getTimestamp();
//	}
	printfDebug("%s: Request: %s\r\n", logTimeStr(), aRequest.c_str());

	aResult = httpGet(aMAZServerHostname, aRequest, &aResponse, aFileLength);
	if(aResult == 200) {
		if(aFileLength > 0) {
			printfDebug("%s: Server liefert neue Wetterinformationen.\r\n", logTimeStr());
			std::ofstream aFile("C:\\MAZ\\desktop\\weather.html");
			aFile.write(aResponse, aFileLength);
			aFile.close();
		}
		else {
			printfDebug("%s: Die Wetterinformationen sind bereits aktuell\r\n", logTimeStr());
		}
	}
	else {
		printfDebug("%s: Server liefert Fehlercode %d\r\n", logTimeStr(), aResult);
	}
	if(aResponse) {
		delete[] aResponse;
	}


	// weather.html verarbeiten
	std::ifstream iStream("C:\\MAZ\\desktop\\weather.html");
	if(!iStream.good()) {
		printfDebug("%s: Datei \"C:\\MAZ\\desktop\\weather.html\" wurde nicht gefunden. Es wird ein Standardinhalt verwendet.\r\n", logTimeStr());

		sStream << "<div>Heute kein Wetter</div>" << std::endl;
	}
	else {
		printfDebug("%s: Datei \"C:\\MAZ\\desktop\\weather.html\" wird geladen.\r\n", logTimeStr());
		while(!iStream.eof() && iStream.good()) {
			std::string aStr;
			getline(iStream, aStr);
			sStream << aStr << std::endl;
		}
		iStream.close();
	}
	setWeather(sStream.str());
}

AuthResult MAZ::dStoreAuthenticate(const std::string &aUser, const std::string &aPassword) {
	if(dStore == NULL) {
		printfDebug("%s: dStore-Objekt ist nicht gesetzt.\r\n", logTimeStr());
		return arError;
	}

	return (*dStore)(aUser, aPassword, localConfig->getDStoreIPv4());
}

int MAZ::httpGet(const std::string &aHost, const std::string &aRequest, char** aResponse, unsigned long &aFileLength) {
	int aResult;
	int aTry=1;
	const int TRY_MAX = 3;

	if(httpGetAction == NULL) {
		*aResponse = NULL;
		aFileLength = 0;
		return 404;
	}

	for(; aTry <= TRY_MAX; ++aTry) {
		printfDebug("%s: %d. Versuch...\r\n", logTimeStr(), aTry);

		if(aTry > 1) {
			int aSleepTime;

			// warten...
			aSleepTime = rand() % (10000 * (aTry-1));
			printfDebug("%s: Warte %d ms.\r\n", logTimeStr(), aSleepTime);
			Sleep(aSleepTime);
			printfDebug("%s: done.\r\n", logTimeStr());
		}

		aResult = (*httpGetAction)(aHost, aRequest, aResponse, aFileLength);
		printfDebug("%s: Get-Action liefert: %d\r\n", logTimeStr(), aResult);

		// -2 == unbekannter Servername (DNS...)
		if(aResult == 200 ||aResult == 404 || aResult == -2) {
			return aResult;
		}

		if(*aResponse) {
			delete[] *aResponse;
			*aResponse = NULL;
		}
	}

	printfDebug("%s: Gebe nach %d Versuchen auf!\r\n", logTimeStr(), TRY_MAX);
	return -2;
}

bool MAZ::doUnzip(const std::string &zipFile, const std::string &extractPath) {
	if(unzip == NULL) {
		return false;
	}

	return (*unzip)(zipFile, extractPath);
}

void MAZ::loadConfig() {
	/* Konfiguration darf nur neu erstellt werden, wenn kein User der bisherigen Konfiguration referenziert wird */
	if(currentUser == NULL) {
		/* Lade maz.cfg */
		if(config) {
			delete config;
		}
		config = Model::Config::Load("C:\\MAZ\\desktop\\maz.cfg");
		if(config == NULL) {
			config = new Model::Config::Root();
		}
	}

	/* Lade mazlocal.cfg */
	if(localConfig) {
		delete localConfig;
	}
	localConfig = Model::LocalConfig::Load("C:\\MAZ\\desktop\\mazlocal.cfg");
	if(localConfig == NULL) {
		localConfig = new Model::LocalConfig::Root();
	}

}

void MAZ::logout() {
	currentUser = NULL;
	setPassword("");
}



std::string MAZ::makeHash(const std::string &value) {
	std::string back;
	/* fill hash with seed */
	int hash[16] = {
		0x13, 0xE8, 0xFB, 0x44,
		0x2B, 0xE9, 0x8D, 0xC4,
		0x51, 0x0B, 0xCF, 0x45,
		0xE1, 0xEF, 0xFB, 0x1A
	};

	for(unsigned int i=0; i<value.size(); ++i) {
		int c;
		c = value.at(i);

		hash[0] += c;

		for(unsigned int j=0; j<16; ++j) {
			hash[j]
			= (hash[j]
			+ hash[(j+1) % 16] * hash[(j+1) % 16]
			+ hash[(j+2) % 16] * 2) %0x100;
		}
	}
	
	for(unsigned int j=0; j<16; ++j) {
		int c;

		c = (hash[j] >> 4) & 0x0F;
		if(c >= 0 && c < 10) {
			back += (char) ('0' + c);
		}
		else if(c >= 10 && c < 16) {
			back += (char) ('A' - 10 + c);
		}
		else {
			back += 'X';
		}

		c = (hash[j] >> 0) & 0x0F;
		if(c >= 0 && c < 10) {
			back += (char) ('0' + c);
		}
		else if(c >= 10 && c < 16) {
			back += (char) ('A' - 10 + c);
		}
		else {
			back += 'X';
		}

	}

	return back;
}

void MAZ::saveLocalConfig() {
	std::ofstream file("C:\\MAZ\\desktop\\mazlocal.cfg");
	std::ostream *oStream;

	oStream = &file;
	Model::LocalConfig::Save(*oStream, *localConfig);

	file.close();
}

}
