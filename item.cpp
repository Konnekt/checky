/*
 *  Checky
 *  
 *  Please READ /License.txt FIRST! 
 * 
 *  Copyright (C)2005 Rafa³ Lindemann, Stamina
 *
 *  $Id: $
 */

#include "stdafx.h"
#include "checky_main.h"

namespace Checky {

	Stamina::Thread mainThread;

	typedef boost::shared_ptr< Item > oItem;
	std::map <tCntId, oItem> itemMap;

	Item::Item(tCntId cnt) {
		this->_cnt = cnt;
	}

	void Item::update(tCntId cnt) {
		if (itemMap.find(cnt) == itemMap.end()) {
			itemMap.insert(std::make_pair( cnt, new Item(cnt) ));
		}
		itemMap[cnt]->update();
	}
	void Item::remove(tCntId cnt) {
		itemMap.erase(cnt);
	}
	void Item::runCommand(tCntId cnt, const std::string& command) {
		if (itemMap.find(cnt) != itemMap.end()) {
			itemMap[cnt]->run(true, command);
		}
	}

	std::string Item::getField(const std::string& txt, const std::string& field, const std::string& def) {
		Stamina::RegEx reg;
		reg.match(("/<konnekt:"+field+">(.+?)</konnekt:"+field+">/s").c_str(), txt.c_str());
		if (!reg.isMatched()) {
			return def;
		}
        return reg[1];
	}

	void Item::run(bool user, const std::string& command, bool inNewThread) {
		this->stop();

		if (inNewThread) {
			inNewThread = false;
			Stamina::threadRun(boost::bind(&Item::run, this, user, command, inNewThread));
			return;
		}

		//char * cmd = strdup( command.empty() ? command : GETCNTC(this->_cnt, Data::command));
		std::string cmd = command.empty() ? GETCNTC(this->_cnt, Data::command) : command;

		IMDEBUG(DBG_FUNC, "Spawning %s", cmd.c_str());

		//int ret = _spawnv(_P_WAIT, argv[0], argv);
		PROCESS_INFORMATION proc;
		STARTUPINFO defSI;
		memset(&defSI, 0, sizeof(defSI));
		defSI.cb = sizeof(sizeof(defSI));
		defSI.dwFlags = STARTF_USESTDHANDLES;
		const int buffSize = 250;
		HANDLE readPipe, writePipe;

		SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
        saAttr.bInheritHandle = TRUE; 
        saAttr.lpSecurityDescriptor = NULL; 

		if (!CreatePipe(&readPipe, &writePipe, &saAttr, buffSize)) {
			IMDEBUG(DBG_ERROR, "Could not create pipe!");
			return;
		}
		defSI.hStdOutput = writePipe;
		defSI.hStdError = writePipe;

		if (!CreateProcess(0, (char*)cmd.c_str(), 0, 0, 1, CREATE_NO_WINDOW, 0, 0, &defSI, &proc)) {
			MessageBox(0, "Wprowadzona komenda jest niepoprawna!", "Checky", MB_ICONERROR);
			return;
		}
		CStdString out;
		CloseHandle(writePipe);

		while (1) {
			char buff [buffSize+1];
			buff[0] = 0;
			DWORD read=0;
			bool success = ReadFile(readPipe, buff, buffSize, &read, 0);
			int err = GetLastError();
			if (!success) {
				break;
			}
			buff[read]=0;
			out += buff;
		}

		WaitForSingleObject(proc.hProcess, INFINITE);
		DWORD ret = 0;
		GetExitCodeProcess(proc.hProcess, &ret);

		CloseHandle(proc.hProcess);
		CloseHandle(proc.hThread);
		CloseHandle(readPipe);

		out.Replace("\r\n", "\n");
		out.Replace("\n", "\r\n");
	
		if (user && !out.empty()) {
			std::string txt;
			txt += out;
			MessageBox(0, txt.c_str(), cmd.c_str(), MB_ICONINFORMATION);
		} else {
			SETCNTC(this->_cnt, CNT_DESCRIPTION, out.c_str());
		}

		CStdString fieldInfo = getField(out, "info", ret ? "!!!Problem!!!" : "OK");
		CStdString fieldNotify = getField(out, "notify", (ret ? "Pojawi³ siê problem!" : "Ju¿ wszystko dobrze."));

		int fieldIcon = Stamina::chtoint(getField(out, "icon", "").c_str());

		IMDEBUG(DBG_FUNC, "Returned %d (errno=%d) %s", ret, errno, out.c_str());

//		free(line);

		if (ret == -1) {
		}
		
		int status = statusNormal;
		if (ret >= criticalReturn) {
			status = statusCritical;
		} else if (ret > 0) {
			status = statusWarning;
		}

		if ((GETCNTI(this->_cnt, CNT_STATUS) & CNTM_STATUS) != status || ret >= criticalReturn) {
			if (status == statusWarning) {
				kSound::SoundPlay(Ctrl, "checkyFailed", this->_cnt);
			} else if (status == statusCritical) {
				kSound::SoundPlay(Ctrl, "checkyCritical", this->_cnt);
			}
			if (IMessage(IM_PLUG_NET, KNotify::net)) { // je¿eli jest KNotify...
				std::string body = fieldNotify;
				body += " (";
				body += GETCNTC(this->_cnt, CNT_DISPLAY);
				body += ")";
				KNotify::sIMessage_notify sin (body.c_str() , UIIcon(IT_STATUS, Checky::net, status, 0), ret ? KNotify::sIMessage_notify::tInform : KNotify::sIMessage_notify::tError, (status == statusNormal ? 1 : 0));
				Ctrl->IMessage(&sin);
			}
			
		}
		SETCNTI(this->_cnt, CNT_STATUS_ICON, fieldIcon);
		CntSetStatus(this->_cnt, status, fieldInfo);
		ICMessage(IMI_REFRESH_CNT, this->_cnt);
		this->setTimer();
	}

	void Item::setTimer() {
		if (!mainThread.isCurrent()) {
			Stamina::threadInvoke(mainThread
				, boost::bind(&Item::setTimer, this), false);
			return;
		}

		if (GETCNTI(this->_cnt, Data::enabled) == 0) {
			return;
		}

		this->start( GETCNTI(this->_cnt, Data::interval) * 1000 );
	}

	void Item::update() {
		this->stop();

		this->setTimer();

	}
	void Item::timerProc(__int64 nanotime) {
		run(false);
	}

};