#ifndef COMMON_VST24_H
#define COMMON_VST24_H

#include <aeffect.h>
#include <aeffectx.h>
#include "common/types.h"


namespace Airwave {


//using AudioMasterProc = intptr (VSTCALLBACK*)(AEffect*, i32, i32, intptr, void*, float);
//using VstPluginMainProc = AEffect* (VSTCALLBACK*)(AudioMasterProc);

typedef	intptr_t (VSTCALLBACK *AudioMasterProc)(AEffect*, i32, i32, intptr_t, void*,
		float);

typedef AEffect* (VSTCALLBACK *VstPluginMainProc)(AudioMasterProc);


static const char* const kDispatchEvents[] = {
	"effOpen",
	"effClose",
	"effSetProgram",
	"effGetProgram",
	"effSetProgramName",
	"effGetProgramName",
	"effGetParamLabel",
	"effGetParamDisplay",
	"effGetParamName",
	"effGetVu",
	"effSetSampleRate",
	"effSetBlockSize",
	"effMainsChanged",
	"effEditGetRect",
	"effEditOpen",
	"effEditClose",
	"effEditDraw",
	"effEditMouse",
	"effEditKey",
	"effEditIdle",
	"effEditTop",
	"effEditSleep",
	"effIdentify",
	"effGetChunk",
	"effSetChunk",
	"effProcessEvents",
	"effCanBeAutomated",
	"effString2Parameter",
	"effGetNumProgramCategories",
	"effGetProgramNameIndexed",
	"effCopyProgram",
	"effConnectInput",
	"effConnectOutput",
	"effGetInputProperties",
	"effGetOutputProperties",
	"effGetPlugCategory",
	"effGetCurrentPosition",
	"effGetDestinationBuffer",
	"effOfflineNotify",
	"effOfflinePrepare",
	"effOfflineRun",
	"effProcessVarIo",
	"effSetSpeakerArrangement",
	"effSetBlockSizeAndSampleRate",
	"effSetBypass",
	"effGetEffectName",
	"effGetErrorText",
	"effGetVendorString",
	"effGetProductString",
	"effGetVendorVersion",
	"effVendorSpecific",
	"effCanDo",
	"effGetTailSize",
	"effIdle",
	"effGetIcon",
	"effSetViewPosition",
	"effGetParameterProperties",
	"effKeysRequired",
	"effGetVstVersion",
	"effEditKeyDown",
	"effEditKeyUp",
	"effSetEditKnobMode",
	"effGetMidiProgramName",
	"effGetCurrentMidiProgram",
	"effGetMidiProgramCategory",
	"effHasMidiProgramsChanged",
	"effGetMidiKeyName",
	"effBeginSetProgram",
	"effEndSetProgram",
	"effGetSpeakerArrangement",
	"effShellGetNextPlugin",
	"effStartProcess",
	"effStopProcess",
	"effSetTotalSampleToProcess",
	"effSetPanLaw",
	"effBeginLoadBank",
	"effBeginLoadProgram",
	"effSetProcessPrecision",
	"effGetNumMidiInputChannels",
	"effGetNumMidiOutputChannels"
};


static const char* const kAudioMasterEvents[] = {
	"audioMasterAutomate",
	"audioMasterVersion",
	"audioMasterCurrentId",
	"audioMasterIdle",
	"audioMasterPinConnected",
	"",
	"audioMasterWantMidi",
	"audioMasterGetTime",
	"audioMasterProcessEvents",
	"audioMasterSetTime",
	"audioMasterTempoAt",
	"audioMasterGetNumAutomatableParameters",
	"audioMasterGetParameterQuantization",
	"audioMasterIOChanged",
	"audioMasterNeedIdle",
	"audioMasterSizeWindow",
	"audioMasterGetSampleRate",
	"audioMasterGetBlockSize",
	"audioMasterGetInputLatency",
	"audioMasterGetOutputLatency",
	"audioMasterGetPreviousPlug",
	"audioMasterGetNextPlug",
	"audioMasterWillReplaceOrAccumulate",
	"audioMasterGetCurrentProcessLevel",
	"audioMasterGetAutomationState",
	"audioMasterOfflineStart",
	"audioMasterOfflineRead",
	"audioMasterOfflineWrite",
	"audioMasterOfflineGetCurrentPass",
	"audioMasterOfflineGetCurrentMetaPass",
	"audioMasterSetOutputSampleRate",
	"audioMasterGetOutputSpeakerArrangement",
	"audioMasterGetVendorString",
	"audioMasterGetProductString",
	"audioMasterGetVendorVersion",
	"audioMasterVendorSpecific",
	"audioMasterSetIcon",
	"audioMasterCanDo",
	"audioMasterGetLanguage",
	"audioMasterOpenWindow",
	"audioMasterCloseWindow",
	"audioMasterGetDirectory",
	"audioMasterUpdateDisplay",
	"audioMasterBeginEdit",
	"audioMasterEndEdit",
	"audioMasterOpenFileSelector",
	"audioMasterCloseFileSelector",
	"audioMasterEditFile",
	"audioMasterGetChunkFile",
	"audioMasterGetInputSpeakerArrangement"
};


} // namespace Airwave


#endif // COMMON_VST24_H
