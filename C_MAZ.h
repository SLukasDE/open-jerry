/*
 * MAZ.h
 *
 *  Created on: 01.06.2011
 *      Author: Sven Lukas
 * Description: Controler des MAZ Desktops
 */

#ifndef CONTROL_MAZ_H_
#define CONTROL_MAZ_H_

#include <string>
#include <utility>
#include <ostream>
#include <time.h>

#include "Model/Config/MC_Root.h"
#include "Model/LocalConfig/MLC_Root.h"
#include "Control/C_SwitchEvent.h"
#include "Control/C_Action.h"
#include "Control/C_DStore.h"
#include "Control/C_HttpGet.h"
#include "Control/C_Unzip.h"
#include "Control/C_ValueLookup.h"


#define CRONJOB_TIMEOUT 9000 // so viele Ticks (1/10 Sekunden) wird die Timer-Funktion aufgerufen, bis
                             // ein Update der Button-Leiste erfolgt. 9000 = 15 Min.
#define UPDATE_TIMEOUT    10 // so viele Ticks (1/10 Sekunden) wird die Timer-Funktion aufgerufen, bis
                             // ein Update der Button-Leiste erfolgt. 10 = 1Sek.
#define SHUTDOWN_TIMEOUT  50 // so viele Ticks (1/10 Sekunden) wird die Timer-Funktion aufgerufen, bis
                             // die Fenster nach vorne geholt werden. 50 = 5 Sek.
#define LOCK_TIMEOUT    3000 // so viele Ticks (1/10 Sekunden) wird die Timer-Funktion aufgerufen, bis
                             // der Desktop gelockt wird. 3000 = 5 Min.

namespace Control {

enum LoginResult {
	lrOK,
	lrLocalOK,
	lrUnknownUser,
	lrFailed,
	lrError
};

class MAZ {
public:
	MAZ();
	~MAZ();

	void run();

	void setKeyTrapSwitch(SwitchEvent* aKeyTrap);
	void setMouseTrapSwitch(SwitchEvent* aMouseTrap);
	void setTrapAction(Action* aTrapAction);
	void setTaskBarSwitch(Switch* aTaskBar);
	void setDesktopAction(Action* aDesktop);
	void setLoginAction(Action* aLoginAction);
	void setDStore(DStore* aDStore);
	void setHttpGetAction(HttpGet* aHttpGet);
	void setUnzip(Unzip* aUnzip);
	void setValueLookup(ValueLookup* aValueLookup);

	void setWeather(const std::string& aWeather);
	const std::string& getWeather() const;

	void setHostname(const std::string& aHostname);
	const std::string& getHostname() const;
	std::string getMbbk() const;

	void setPassword(const std::string& aPassword);
	const std::string& getPassword() const;

	LoginResult testLogin(const std::string &aUser, const std::string &aPassword);
	LoginResult login(const std::string &aUser, const std::string &aPassword);
	Model::Config::User* getCurrentUser() const;

	void setNextUsername(const std::string &aNextUsername);
	const std::string& getNextUsername() const;

	void setNextPassword(const std::string &aNextPassword);
	const std::string& getNextPassword() const;

	void resetTimeVars();
	void substitute(std::ostream& aOut, const std::string& aValue, const Model::Config::Element* aElement) const;
	std::string getValue(const std::string& aKey, const Model::Config::Element* aElement) const;

	void makeHomepage();

	const Model::LocalConfig::Root& getLocalConfig() const;

private:
	void enableKeyTrap(bool);
	void enableMouseTrap(bool);
	void hideTaskBar(bool);
	bool runDesktop();
	bool runLogin();
	AuthResult dStoreAuthenticate(const std::string &aUser, const std::string &aPassword);
	int httpGet(const std::string &aHost, const std::string &aRequest, char** aResponse, unsigned long &aFileLength);
	bool doUnzip(const std::string &zipFile, const std::string &extractPath);

	void updateMazCfg();
	void updateResource();
	void updateWetter();

	void loadConfig();
	void logout();

	std::string getFile(const std::string &aFile) const;
	void mkFile(const std::string &aDstName, const std::string &aSrcName, const Model::Config::Element* aElement) const;
	std::string getMarktnummer() const;
	std::string getTime() const;
	std::string getDateTime() const;

	static std::string makeHash(const std::string &value);
	std::string extractMakroParam(std::string &aParam, const Model::Config::Element* aElement) const;
	void saveLocalConfig();

	SwitchEvent* keyTrap;
	SwitchEvent* mouseTrap;
	Action* trapAction;
	Switch* taskBar;
	Action* desktop;
	Action* loginAction;
	DStore* dStore;
	HttpGet* httpGetAction;
	Unzip* unzip;
	ValueLookup* valueLookup;

	Model::Config::User* currentUser;
	Model::Config::Root* config;
	Model::LocalConfig::Root* localConfig;
	std::string weather;
	std::string hostname;
	std::string password;
	time_t timeValue;

	std::string nextUsername;
	std::string nextPassword;
};

}

#endif
