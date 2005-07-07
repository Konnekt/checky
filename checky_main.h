#pragma once


namespace Checky {

	const unsigned int net = 82;

	const unsigned int criticalReturn = 10;

	enum {
		operNone,
		operEq,
		operNotEq,
		operGt,
		operLt,

	};

	namespace Data {
		extern Tables::tColId command;
		extern Tables::tColId interval;
		extern Tables::tColId enabled;
	};

	enum {
		statusNormal = ST_OFFLINE,
		statusWarning = ST_AWAY,
		statusCritical = ST_ONLINE,
	};



	class Item: private Stamina::Timer {
	public:
		Item(tCntId cnt);

		static void create(tCntId cnt) {
			update(cnt);
		}
		static void update(tCntId cnt);
		static void remove(tCntId cnt);
		static void runCommand(tCntId cnt, const std::string& command);
	private:

		tCntId _cnt;

		void setTimer();
		void run(bool user, const std::string& command="", bool inNewThread = true);
		void update();
		void timerProc(__int64 nanotime);
		static std::string getField(const std::string& txt, const std::string& field, const std::string& def);

	};

};