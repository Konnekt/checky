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
#include "../plug_defs/lib.h"
#include "checky_main.h"
#include "resource.h"

int __stdcall DllMain(void * hinstDLL, unsigned long fdwReason, void * lpvReserved)
{
        return true;
}

using namespace Checky;

namespace Checky {

	//Tables::oTable data;

	unsigned int cfgGroup;
	unsigned int actNow;

	namespace Data {
		Tables::tColId command;
		Tables::tColId interval;
		Tables::tColId enabled;
	};

	int IStart() {


		int count = ICMessage(IMC_CNT_COUNT);
		for (int i=1; i<count;i++)
			if (GETCNTI(i , CNT_NET)==Checky::net) {
				Item::create(Ctrl->DTgetID(DTCNT , i));
			}
		return 1;
	}

	int IEnd() {
		return 1;
	}

	int ISetCols() {
		/*
		data = Tables::registerTable(Ctrl, "Checky");
		data |= optAutoLoad;
		data |= optAutoSave;
		data->setColumn(Ctrl->getPlugin(), DTArchive::phone, ctypeString | cflagXor, "phone");
		dtIncoming->setFilename("actio_in.dtb");
		dtIncoming->setDirectory(0);
		*/

		Data::command = SetColumn(DTCNT, -1, DT_CT_PCHAR, 0, "Checky/command");
		Data::interval = SetColumn(DTCNT, -1, DT_CT_INT, 120, "Checky/interval");
		Data::enabled = SetColumn(DTCNT, -1, DT_CT_INT, 0, "Checky/enabled");
		
		return 1;
	}

	int IPrepare() {

		/*
		Konnekt::Unique::registerId(Unique::domainNet, Checky::net,"Checky");
		cfgGroup = Konnekt::Unique::registerName(Unique::domainAction, "Checky/cfgGroup");
		actNow = Konnekt::Unique::registerName(Unique::domainAction, "Checky/actNow");
		*/
		cfgGroup = 82000;
		actNow = 82001;

		IconRegister(IML_16 , UIIcon(IT_STATUS , Checky::net , Checky::statusNormal , 0) , Ctrl->hDll() , IDI_ST_NORMAL);
		IconRegister(IML_16 , UIIcon(IT_STATUS , Checky::net , Checky::statusWarning , 0) , Ctrl->hDll() , IDI_ST_WARNING);
		IconRegister(IML_16 , UIIcon(IT_STATUS , Checky::net , Checky::statusCritical , 0) , Ctrl->hDll() , IDI_ST_CRITICAL);
		IconRegister(IML_16 , UIIcon(IT_LOGO , Checky::net , 0 , 0) , Ctrl->hDll() , IDI_ST_NORMAL);


		UIGroupInsert(IMIG_NFO , cfgGroup , 0 , ACTR_INIT , "Checky" , UIIcon(IT_LOGO , Checky::net , 0 , 0));

		UIActionCfgAddPluginInfoBox2(cfgGroup, 
			"<div>Wtyczka uruchamia polecenie co <i>n</i> sekund. Je¿eli polecenie zwróci dowolny kod b³êdu, Checky uzna ¿e wyst¹pi³ problem i nas o tym poinformuje.</div><div>Polecenia uruchamiane s¹ w osobnym w¹tku. Wynik zapisywany jest w polu 'Wiêcej/Opis w³asny'."
				, "Copyright ©2005 <b>Stamina</b>"

				, "res://dll/101.ico", -4);

		UIActionInsert(cfgGroup, 0, -1, ACTT_GROUP, "");

		UIActionInsert(cfgGroup, 0, -1, ACTT_COMMENT | ACTSC_INLINE, "Polecenie", 0, 50);
		UIActionInsert(cfgGroup, IMIB_CNT, -1, ACTT_EDIT | ACTSC_FULLWIDTH, "" AP_TIP "Komenda do wywo³ania", Data::command);
		UIActionInsert(cfgGroup, 0, -1, ACTT_COMMENT | ACTSC_INLINE, "Interwa³", 0, 50);
		UIActionInsert(cfgGroup, IMIB_CNT, -1, ACTT_EDIT | ACTSC_INT, "", Data::interval);

		UIActionInsert(cfgGroup, IMIB_CNT, -1, ACTT_CHECK, "W³¹czony", Data::enabled);

		UIActionInsert(cfgGroup, 0, -1, ACTT_SEPARATOR, "");

		UIActionInsert(cfgGroup, actNow, -1, ACTT_BUTTON, "SprawdŸ" CFGTIP "Najpierw zapisz zmiany!", 0, 100, 25);

		UIActionInsert(cfgGroup, 0, -1, ACTT_GROUPEND, "Komenda");


		UIActionInsert(IMIG_CNT, actNow, 0, ACTR_INIT | ACTSMENU_BOLD, "Odœwie¿", UIIcon(IT_LOGO , Checky::net , 0 , 0));


		//UIActionInsert(IMIG_MSGTB , konnfer::Action::show_recipients , 1 , ACTR_INIT , "Wypisz rozmówców" , Ico::group_show);
		return 1;
	}

/*int ActionCfgProc(sUIActionNotify_base * anBase) {
  sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
  switch (anBase->act.id & ~IMIB_CFG) {
  }
  return 0;
}*/

	ActionProc(sUIActionNotify_base * anBase) {
		sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
	//	if ((anBase->act.id & IMIB_) == IMIB_CFG) return ActionCfgProc(anBase); 
		bool isChecky = (GETCNTI(anBase->act.cnt , CNT_NET) == Checky::net);
		if (anBase->act.id == cfgGroup && anBase->code == ACTN_CREATE) {
			// Pewne rzeczy wypada³oby ukryæ
			// Lepiej, ¿eby u¿ytkownik nie zmienia³ UID'a...
			UIActionSetStatus(anBase->act , isChecky?0:-1 , ACTS_HIDDEN);
		} else if (anBase->act.id == actNow) {
			if (anBase->act.parent == cfgGroup) {
				if (anBase->code == ACTN_ACTION) {
					Item::runCommand((tCntId)anBase->act.cnt, UIActionCfgGetValue(sUIAction(anBase->act.parent, IMIB_CNT | Data::command, anBase->act.cnt), 0, 0));
				}
			} else {
				if (anBase->code == ACTN_ACTION) {
					Item::runCommand((tCntId)anBase->act.cnt, "");
				} else if (anBase->code == ACTN_CREATE) {
					UIActionSetStatus(anBase->act, isChecky ? 0 : -1, ACTS_HIDDEN);
				} else if (anBase->code == ACTN_DEFAULT) {
					return isChecky;
				}
			}
		}
		return 0;
	}

} // namespace


int __stdcall IMessageProc(sIMessage_base * msgBase) {
    sIMessage_2params * msg = (msgBase->s_size>=sizeof(sIMessage_2params))?static_cast<sIMessage_2params*>(msgBase):0;
    switch (msgBase->id) {
	case IM_PLUG_NET:        return Checky::net; 
	case IM_PLUG_TYPE:       return IMT_CONTACT | IMT_NETUID | IMT_NET; 
    case IM_PLUG_VERSION:    return 0; 
    case IM_PLUG_SDKVERSION: return KONNEKT_SDK_V;  
    case IM_PLUG_SIG:        return (int)"CHECKY"; 
    case IM_PLUG_CORE_V:     return (int)"W98"; 
    case IM_PLUG_UI_V:       return 0; 
    case IM_PLUG_NAME:       return (int)"Checky"; 
    case IM_PLUG_NETNAME:    return (int)"Checky"; 
    case IM_PLUG_INIT:       Plug_Init(msg->p1,msg->p2);return 1;
    case IM_PLUG_DEINIT:     Plug_Deinit(msg->p1,msg->p2);return 1;
	case IM_PLUG_PRIORITY:	 return PLUGP_LOW - 5;

	case IM_SETCOLS:		 return Checky::ISetCols();

    case IM_UI_PREPARE:      return Checky::IPrepare();
    case IM_START:           return Checky::IStart();
    case IM_END:             return Checky::IEnd();

    case IM_UIACTION:        return Checky::ActionProc((sUIActionNotify_base*)msg->p1);

	case IM_PLUG_UPDATE: 
		if (msg->p1 && msg->p1 < VERSION_TO_NUM(1,3,0,0)) {
			int c = ICMessage(IMC_CNT_COUNT);
			for (int i = 0; i < c; i++) {
				if (GETCNTI(i, CNT_NET) == 80 && GETCNTC(i, Data::command)[0] != 0) {
					IMLOG("Sieæ kontaktu %d zamieniona na 82", i);
					SETCNTI(i, CNT_NET, Checky::net);
				}
			}
		}
		return 0;

	// Obs³uga dodawania/usuwania kontaktów -----------------
	case IM_CNT_ADD:
		// Mo¿e jakaœ wtyczka albo skrypt chc¹ dodaæ kontakt?
		if (GETCNTI(msg->p1 , CNT_NET)==Checky::net) {
			Item::update(msg->p1);
		}
		break;
	case IM_CNT_REMOVE:
		if (GETCNTI(msg->p1 , CNT_NET)==Checky::net) {
			Item::remove(msg->p1);
		}
		break;
	case IM_CNT_CHANGED:{
		sIMessage_CntChanged * cc = static_cast<sIMessage_CntChanged*>(msgBase);
		if (!cc->_changed.net && !cc->_changed.uid && GETCNTI(cc->_cntID , CNT_NET)==Checky::net) {
			Item::update(cc->_cntID);
			break;
		}
		// Je¿eli zosta³ zmieniony kontakt, który by³ w konferencjach
		// najpierw trzeba usun¹æ jego obiekt
		if (cc->_oldNet==Checky::net) {
			Item::remove(cc->_cntID);
		}
		// Je¿eli aktualnie nadal jest obiektem konferencji... tworzymy go na nowo...
		if (GETCNTI(cc->_cntID , CNT_NET) == Checky::net) {
			Item::update(cc->_cntID);
		}
		break;}

	case IM_CNT_DOWNLOAD:
		if (GETCNTI(msg->p1 , CNT_NET)==Checky::net) {
			UIActionCall(&sUIActionNotify_2params(sUIAction(cfgGroup, actNow, msg->p1), ACTN_ACTION, 0, 0));
		} else {
			Ctrl->setError(IMERROR_NORESULT);
		}
		break;

	case kSound::DOREGISTER:
		kSound::SoundRegister(Ctrl, "checkyFailed", "Checky wykry³ b³¹d", kSound::flags::contacts);
		kSound::SoundRegister(Ctrl, "checkyCritical", "Checky wykry³ b³¹d krytyczny", kSound::flags::contacts);
		return 0;


    default:
        if (Ctrl) Ctrl->setError(IMERROR_NORESULT);
        return 0;

 }
 return 0;
}

