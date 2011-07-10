
#include "script/ScriptedLang.h"

#include "ai/Paths.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "graphics/Math.h"
#include "platform/String.h"
#include "io/Logger.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"
#include <winsock.h>

using std::string;

extern SCRIPT_EVENT AS_EVENT[];

namespace script {

namespace {

class NopCommand : public Command {
	
public:
	
	NopCommand() : Command("nop") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		LogDebug << "nop";
		
		return Success;
	}
	
};

class GotoCommand : public Command {
	
	bool sub;
	
public:
	
	GotoCommand(string command, bool _sub = false) : Command(command), sub(_sub) { }
	
	Result execute(Context & context) {
		
		string label = context.getLowercase();
		
		LogDebug << getName() << ' ' << label;
		
		size_t pos = context.skipCommand();
		if(pos != (size_t)-1) {
			LogWarning << "unexpected text after " << getName() << " at " << pos;
		}
		
		if(!context.jumpToLabel(label)) {
			// TODO should be a warning, but some scripts do this
			LogDebug << "error jumping to label \"" << label << '"';
			return AbortError;
		}
		
		return Jumped;
	}
	
};

class AbortCommand : public Command {
	
	Result result;
	
public:
	
	AbortCommand(string command, Result _result) : Command(command), result(_result) { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		LogDebug << getName();
		
		return result;
	}
	
};

class RandomCommand : public Command {
	
public:
	
	RandomCommand() : Command("random") { }
	
	Result execute(Context & context) {
		
		float chance = clamp(context.getFloat(), 0.f, 100.f);
		
		LogDebug << "random " << chance;
		
		float t = rnd() * 100.f;
		if(chance < t) {
			context.skipStatement();
		}
		
		return Jumped;
	}
	
};

class ReturnCommand : public Command {
	
public:
	
	ReturnCommand() : Command("return") { }
	
	Result execute(Context & context) {
		
		LogDebug << getName();
		
		if(!context.returnToCaller()) {
			LogError << "return failed";
			return AbortError;
		}
		
		return Jumped;
	}
	
};

class SetMainEventCommand : public Command {
	
public:
	
	SetMainEventCommand(const string & command) : Command(command, ANY_IO) { }
	
	Result execute(Context & context) {
		
		string event = context.getLowercase();
		
		LogDebug << "setmainevent " << event;
		
		ARX_SCRIPT_SetMainEvent(context.getIO(), event);
		
		return Success;
	}
	
};

class StartStopTimerCommand : public Command {
	
	const bool start;
	
public:
	
	StartStopTimerCommand(const string & command, bool _start) : Command(command), start(_start) { }
	
	Result execute(Context & context) {
		
		string timer = context.getLowercase();
		
		LogDebug << getName() << ' ' << timer;
		
		long t;
		if(timer == "timer1") {
			t = 0;
		} else if(timer == "timer2") {
			t = 1;
		} else if(timer == "timer3") {
			t = 2;
		} else if(timer == "timer4") {
			t = 3;
		} else {
			LogWarning << "invalid timer: " << timer;
			return Failed;
		}
		
		if(start) {
			context.getScript()->timers[t] = ARXTimeUL();
			if(context.getScript()->timers[t] == 0) {
				context.getScript()->timers[t] = 1;
			}
		} else {
			context.getScript()->timers[t] = 0;
		}
		
		return Success;
	}
	
};

class SendEventCommand : public Command {
	
	enum SendTarget {
		SEND_NPC = 1,
		SEND_ITEM = 2,
		SEND_FIX = 4
	};
	DECLARE_FLAGS(SendTarget, SendTargets)
	
public:
	
	SendEventCommand() : Command("sendevent") { }
	
	Result execute(Context & context) {
		
		SendTargets sendto = 0;
		bool radius = false;
		bool zone = false;
		bool group = false;
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			group = (flg & flag('g'));
			sendto |= (flg & flag('f')) ? SEND_FIX : (SendTargets)0;
			sendto |= (flg & flag('i')) ? SEND_ITEM : (SendTargets)0;
			sendto |= (flg & flag('n')) ? SEND_NPC : (SendTargets)0;
			radius = (flg & flag('r'));
			zone = (flg & flag('z'));
			if(!flg || (flg & ~flags("gfinrz"))) {
				LogWarning << "unexpected flags: sendevent " << options;
			}
		}
		if(!sendto) {
			sendto = SEND_NPC;
		}
		
		string groupname;
		if(group) {
			groupname = toLowercase(context.getStringVar(context.getLowercase()));
		}
		
		string event = context.getLowercase();
		
		string zonename;
		if(zone) {
			zonename = toLowercase(context.getStringVar(context.getLowercase()));
		}
		
		float rad = 0.f;
		if(radius) {
			rad = context.getFloat();
		}
		
		string target;
		if(!group && !zone && !radius) {
			target = context.getLowercase();
			
			// work around broken scripts
			for(size_t i = 0; i < SM_MAXCMD; i++) {
				if(!strcasecmp(target, AS_EVENT[i].name.c_str() + 3)) { // TODO(case-sensitive) strcasecmp
					std::swap(target, event);
					break;
				}
			}
		}
		
		string param = context.getWord();
		
		LogDebug << "sendevent " << options << " g=\"" << groupname << "\" e=\"" << event << "\" r=" << rad << " t=\"" << target << "\" p=\"" << param << '"';
		
		INTERACTIVE_OBJ * oes = EVENT_SENDER;
		EVENT_SENDER = context.getIO();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(radius) { // SEND EVENT TO ALL OBJECTS IN A RADIUS
			
			for(long l = 0 ; l < inter.nbmax ; l++) {
				
				if(!inter.iobj[l] || inter.iobj[l] == io || (inter.iobj[l]->ioflags & (IO_CAMERA|IO_MARKER))) {
					continue;
				}
				
				if(group && !IsIOGroup(inter.iobj[l], groupname)) {
					continue;
				}
				
				if(((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
				   || ((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
				   || ((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM))) {
					
					Vec3f _pos, _pos2;
					GetItemWorldPosition(inter.iobj[l], &_pos);
					GetItemWorldPosition(io, &_pos2);
					
					if(distSqr(_pos, _pos2) <= square(rad)) {
						io->stat_sent++;
						Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, param, event);
					}
				}
			}
			
		} else if(zone) { // SEND EVENT TO ALL OBJECTS IN A ZONE
			
			ARX_PATH * ap = ARX_PATH_GetAddressByName(zonename);
			if(!ap) {
				LogWarning << "unknown zone: " << zonename;
				EVENT_SENDER = oes;
				return Failed;
			}
			
			for(long l = 0; l < inter.nbmax; l++) {
				
				if(!inter.iobj[l] || (inter.iobj[l]->ioflags & (IO_CAMERA|IO_MARKER))) {
					continue;
				}
				
				if(group && !IsIOGroup(inter.iobj[l], groupname)) {
					continue;
				}
				
				if(((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
				   || ((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
				   || ((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM))) {
					
					Vec3f _pos;
					GetItemWorldPosition(inter.iobj[l], &_pos);
					
					if(ARX_PATH_IsPosInZone(ap, _pos.x, _pos.y, _pos.z)) {
						io->stat_sent++;
						Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, param, event);
					}
				}
			}
			
		} else if(group) { // sends an event to all members of a group
			
			for(long l = 0; l < inter.nbmax; l++) {
				
				if(!inter.iobj[l] || inter.iobj[l] == io || !IsIOGroup(inter.iobj[l], groupname)) {
					continue;
				}
				
				io->stat_sent++;
				Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, param, event);
			}
			
		} else { // single object event
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(io);
			}
			
			if(!ValidIONum(t)) {
				LogWarning << "invalid target: " << target;
				EVENT_SENDER = oes;
				return Failed;
			}
			
			io->stat_sent++;
			Stack_SendIOScriptEvent(inter.iobj[t], SM_NULL, param, event);
		}
		
		EVENT_SENDER = oes;
		
		return Success;
	}
	
};

}

void setupScriptedLang() {
	
	ScriptEvent::registerCommand(new NopCommand);
	ScriptEvent::registerCommand(new GotoCommand("goto"));
	ScriptEvent::registerCommand(new GotoCommand("gosub", true));
	ScriptEvent::registerCommand(new AbortCommand("accept", Command::AbortAccept));
	ScriptEvent::registerCommand(new AbortCommand("refuse", Command::AbortRefuse));
	ScriptEvent::registerCommand(new RandomCommand);
	ScriptEvent::registerCommand(new ReturnCommand);
	ScriptEvent::registerCommand(new SetMainEventCommand("setstatus"));
	ScriptEvent::registerCommand(new SetMainEventCommand("setmainevent"));
	ScriptEvent::registerCommand(new StartStopTimerCommand("starttimer", true));
	ScriptEvent::registerCommand(new StartStopTimerCommand("stoptimer", false));
	ScriptEvent::registerCommand(new SendEventCommand);
	
}

} // namespace script
