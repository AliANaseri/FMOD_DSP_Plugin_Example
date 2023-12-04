/*************************************************************************************************

* Author:
* Ali A. Naseri
* 
	Important note : Read the LICENSE file

	This example has been created solely for demonstrating an issue with dynamic allocated variables.
	You may want to use fmod's provided examples as learning resources. 
* 
* HOW TO BUILD?

* Set Solution Configuration as Debug and Solution Platform as x64
* Add FMODPLUGIN to your environment variables (default location is C:\Program Files\FMOD SoundSystem\FMOD Studio x.xx.xx\plugins) 
* Note that if you try to build this app without elevated access the build will fail.
* You may want to remove the Post-Build Event if you want to manualy copy built files into your fmod studio directory.
* Your build may fail for the very first time due to file non existance at destination. You may want to manually copy the built files to your fmod studio plugin folder for the first time.

*************************************************************************************************/ 

#include "pch.h"
#include "fmod.hpp"
#include "fmod_dsp.h"
#include <cstdio>
#include <cmath>
#include <time.h>
#include <cstdint>

#define DspExample_USEPROCESSCALLBACK

extern "C" {
	F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

/*----------------------------
	Add your Constant values here
----------------------------*/

enum
{
	/*----------------------------
	 Add your parameters here
	 don't forget to put "= 0" at the end of first one
	----------------------------*/
	DSP_EXAMPLE_RPM, 
	DSP_EXAMPLE_NumberOfParameters
};

// These conversions may come in handy
#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= MinGain) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? MinGain : 20.0f * log10f((float)__linval__))

float Map16bitAudioToFloatRange(int16_t Number) 
{
	return -1.f + static_cast<float>(Number + 32768) / 65535.f * 2.f;
}

FMOD_RESULT F_CALLBACK DspExampleCreateDSP(FMOD_DSP_STATE* dsp);
FMOD_RESULT F_CALLBACK DspExampleReleaseDSP(FMOD_DSP_STATE* dsp);
FMOD_RESULT F_CALLBACK DspExampleResetDSP(FMOD_DSP_STATE* dsp);
#ifndef DspExample_USEPROCESSCALLBACK
	FMOD_RESULT F_CALLBACK DspExampleRead(FMOD_DSP_STATE* dsp,float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels);
#else
	FMOD_RESULT F_CALLBACK DspExampleProcess(FMOD_DSP_STATE* dsp, unsigned int length, const FMOD_DSP_BUFFER_ARRAY* inbufferarray, FMOD_DSP_BUFFER_ARRAY* outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
#endif
FMOD_RESULT F_CALLBACK DspExampleSetFloat(FMOD_DSP_STATE* dsp, int index, float value);
FMOD_RESULT F_CALLBACK DspExampleSetInt(FMOD_DSP_STATE* dsp, int index, int value);
FMOD_RESULT F_CALLBACK DspExampleSetBool(FMOD_DSP_STATE* dsp, int index, FMOD_BOOL value);
FMOD_RESULT F_CALLBACK DspExampleSetData(FMOD_DSP_STATE* dsp, int index, void* data, unsigned int length);
FMOD_RESULT F_CALLBACK DspExampleGetFloat(FMOD_DSP_STATE* dsp, int index, float* value, char* valuestr);
FMOD_RESULT F_CALLBACK DspExampleGetInt(FMOD_DSP_STATE* dsp, int index, int* value, char* valuestr);
FMOD_RESULT F_CALLBACK DspExampleGetBool(FMOD_DSP_STATE* dsp, int index, FMOD_BOOL* value, char* valuestr);
FMOD_RESULT F_CALLBACK DspExampleGetData(FMOD_DSP_STATE* dsp, int index, void** value, unsigned int* length, char* valuestr);

/*----------------------------
	define your parameter descriptions here
----------------------------*/
static FMOD_DSP_PARAMETER_DESC p_RPM;

FMOD_DSP_PARAMETER_DESC* DspParameters[DSP_EXAMPLE_NumberOfParameters] =
{
	/*----------------------------
		All the other parameter descriptions here.
	----------------------------*/
	&p_RPM,
};

FMOD_DSP_DESCRIPTION DspExampleDesc =
{
	FMOD_PLUGIN_SDK_VERSION,
	"DSP Example",   // name
	0x00010000,     // plug-in version
	0,              // number of input buffers to process
	1,              // number of output buffers to process
	DspExampleCreateDSP,
	DspExampleReleaseDSP,
	DspExampleResetDSP,
#ifndef DspExample_USEPROCESSCALLBACK
	DspExampleRead,
#else
	0,
#endif
#ifdef DspExample_USEPROCESSCALLBACK
	DspExampleProcess,
#else
	0,
#endif
	0,
	DSP_EXAMPLE_NumberOfParameters,
	DspParameters,
	DspExampleSetFloat,
	DspExampleSetInt,
	DspExampleSetBool, //WaveGenSetBool,
	DspExampleSetData, //WaveGenSetData,
	DspExampleGetFloat,
	DspExampleGetInt,
	DspExampleGetBool, //WaveGenGetBool,
	DspExampleGetData, //WaveGenGetData,
	0,
	0,                                      // userdata
	0,                                      // Register
	0,                                      // Deregister
	0                                       // Mix
};

extern "C"
{

	F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
	{
		/*----------------------------
		   Initialize all the main dsp plugin parameters here.
		   Usually knobs and buttons.
		----------------------------*/
		FMOD_DSP_INIT_PARAMDESC_FLOAT(p_RPM, "RPM", "RPM", "Rotating speed of engine a number between 0 and 1", 0, 1, 0);
		return &DspExampleDesc;
	}

}

/*----------------------------
	Class declarations then related definitions here
----------------------------*/
class StateClass
{
public:
	StateClass();
	~StateClass();

	void reset(FMOD_DSP_STATE* dsp);

	//WaveFileStructure WaveFile;
	void GenerateSound(float* Oubuffer, unsigned int length, int channels, FMOD_DSP_STATE* dsp);

	void SetRPM(float NewRPM) { /*if (FakeRPM) { *FakeRPM = NewRPM; }*/ EngineRPM = NewRPM; }
	float GetRPM() { return EngineRPM; }

private:
	float EngineRPM;

};

StateClass::StateClass()
{
	srand(time(NULL)); //for generating random seed for later works
	EngineRPM = 0.f;
}

StateClass::~StateClass()
{
	
}

void StateClass::reset(FMOD_DSP_STATE* dsp)
{

}

void StateClass::GenerateSound(float* Outbuffer, unsigned int Length, int Channels, FMOD_DSP_STATE* dsp)
{
	//FMOD_DSP_LOG(dsp, FMOD_DEBUG_LEVEL_WARNING, "Generate Sound", "");

	for (unsigned int i = 0; i < Length; i++)
	{
		Outbuffer[i] = 0.f;
	}

	FMOD_DSP_LOG(dsp, FMOD_DEBUG_LEVEL_WARNING, "RPM  :  ", "%f", EngineRPM);

}

/*----------------------------
	FMOD Callback definitions here
----------------------------*/

FMOD_RESULT F_CALLBACK DspExampleCreateDSP(FMOD_DSP_STATE* dsp)
{
	StateClass* State = (StateClass*)FMOD_DSP_ALLOC(dsp, sizeof(StateClass));
	if (!State)
	{
		return FMOD_ERR_MEMORY;
	}
	dsp->plugindata = (StateClass*)FMOD_DSP_ALLOC(dsp, sizeof(StateClass));
	if (!dsp->plugindata)
	{
		return FMOD_ERR_MEMORY;
	}
	dsp->plugindata = State;
	//FMOD_DSP_LOG(dsp, FMOD_DEBUG_LEVEL_WARNING, "FMOD CREATE", "");
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DspExampleReleaseDSP(FMOD_DSP_STATE* dsp)
{
	//FMOD_DSP_LOG(dsp, FMOD_DEBUG_LEVEL_WARNING, "FMOD RELEASE", "");
	StateClass* state = (StateClass*)dsp->plugindata;
	FMOD_DSP_FREE(dsp, state);
	
	return FMOD_OK;
}
#ifdef DspExample_USEPROCESSCALLBACK
FMOD_RESULT F_CALLBACK DspExampleProcess(FMOD_DSP_STATE* dsp, unsigned int length, const FMOD_DSP_BUFFER_ARRAY* inbufferarray, FMOD_DSP_BUFFER_ARRAY* outbufferarray, FMOD_BOOL /*inputsidle*/, FMOD_DSP_PROCESS_OPERATION op)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	if (op == FMOD_DSP_PROCESS_QUERY)
	{//
		if (outbufferarray)
		{
			outbufferarray->buffernumchannels[0] = 1;
		}
		return FMOD_OK;
	}
	state->GenerateSound(outbufferarray->buffers[0], length, inbufferarray->buffernumchannels[0], dsp);
	return FMOD_OK;
}
#else
FMOD_RESULT F_CALLBACK DspExampleRead(FMOD_DSP_STATE* dsp, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels)
{
	return FMOD_OK
}
#endif

FMOD_RESULT F_CALLBACK DspExampleResetDSP(FMOD_DSP_STATE* dsp)
{
	//FMOD_DSP_LOG(dsp, FMOD_DEBUG_LEVEL_WARNING, "FMOD RESET", "");
	StateClass* state = (StateClass*)dsp->plugindata;
	state->reset(dsp);
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK DspExampleSetFloat(FMOD_DSP_STATE* dsp, int index, float value)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	
	switch (index)
	{
	case DSP_EXAMPLE_RPM:
		state->SetRPM(value);

		return FMOD_OK;
	}
	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleGetFloat(FMOD_DSP_STATE* dsp, int index, float* value, char* valuestr)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	switch(index)
	{
	case DSP_EXAMPLE_RPM:
		*value = state->GetRPM();
		return FMOD_OK;
	}
	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleSetInt(FMOD_DSP_STATE* dsp, int index, int value)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	//switch on parameter index with FMOD_OK at the end

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleGetInt(FMOD_DSP_STATE* dsp, int index, int* value, char* valuestr)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	//switch on parameter index with FMOD_OK at the end

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleSetBool(FMOD_DSP_STATE* dsp, int index, FMOD_BOOL value)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	//switch on parameter index with FMOD_OK at the end
	return FMOD_OK;
	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleGetBool(FMOD_DSP_STATE* dsp, int index, FMOD_BOOL* value, char* valuestr)
{
	StateClass* state = (StateClass*)dsp->plugindata;
	//switch on parameter index with FMOD_OK at the end
	return FMOD_OK;
	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleSetData(FMOD_DSP_STATE* dsp, int index, void* data, unsigned int length)
{
	
	StateClass* state = (StateClass*)dsp->plugindata;

	return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK DspExampleGetData(FMOD_DSP_STATE* dsp, int index, void** value, unsigned int* length, char* valuestr)
{
	StateClass* state = (StateClass*)dsp->plugindata;

	return FMOD_ERR_INVALID_PARAM;
}
