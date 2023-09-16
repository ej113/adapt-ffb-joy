/*
  Force Feedback Joystick
  Joystick model specific code for handling force feedback data.
  This code is for Microsoft Sidewinder Force Feedback Pro joystick.

  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  Copyright 2013  Saku Kekkonen
  Copyright 2023  Ed Wilkinson

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "ffb-pro.h"
#include "ffb.h"

#include <util/delay.h>
#include "debug.h"

uint8_t FfbproUsbToMidiEffectType(uint8_t usb_effect_type)
{
	const uint8_t usbToMidiEffectType[] = {
		0x12,	// Constant, 
		0x06, 	// Ramp
		0x05, 	// Square
		0x02, 	// Sine
		0x08,	// Triangle
		0x0A,	// SawtoothDown
		0x0B,	// SawtoothUp
		0x0D,	// Spring
		0x0E,	// Damper
		0x0F,	// Inertia
		0x10,	// Friction
		0x01 	// Custom ?
	};
	
	if (usb_effect_type >= sizeof(usbToMidiEffectType))
		return 0;
		
	return usbToMidiEffectType[usb_effect_type];
}

static void FfbproInitPulses(uint8_t count)
{
	while (count--) {
		FfbPulseX1();
		_delay_us10(10);
	}
}

void FfbproEnableInterrupts(void)
{
	const uint8_t startupFfbData_0[] = {
		0xc5, 0x01        // <ProgramChange> 0x01
	};

	const uint8_t startupFfbData_1[] = {
		0xf0,
		0x00, 0x01, 0x0a, 0x01, 0x10, 0x05, 0x6b,  // ???? - reset all?
		0xf7
	};

	const uint8_t startupFfbData_2[] = {
		0xb5, 0x40, 0x7f,  // <ControlChange>(Modify, 0x7f)
		0xa5, 0x72, 0x57,  // offset 0x72 := 0x57
		0xb5, 0x44, 0x7f,
		0xa5, 0x3c, 0x43,
		0xb5, 0x48, 0x7f,
		0xa5, 0x7e, 0x00,
		0xb5, 0x4c, 0x7f,
		0xa5, 0x04, 0x00,
		0xb5, 0x50, 0x7f,
		0xa5, 0x02, 0x00,
		0xb5, 0x54, 0x7f,
		0xa5, 0x02, 0x00,
		0xb5, 0x58, 0x7f,
		0xa5, 0x00, 0x7e,
		0xb5, 0x5c, 0x7f,
		0xa5, 0x3c, 0x00,
		0xb5, 0x60, 0x7f
	};

	const uint8_t startupFfbData_3[] = {
		0xa5, 0x14, 0x65,
		0xb5, 0x64, 0x7f,
		0xa5, 0x7e, 0x6b,
		0xb5, 0x68, 0x7f,
		0xa5, 0x36, 0x00,
		0xb5, 0x6c, 0x7f,
		0xa5, 0x28, 0x00,
		0xb5, 0x70, 0x7f,
		0xa5, 0x66, 0x4c,
		0xb5, 0x74, 0x7f,
		0xa5, 0x7e, 0x01,
	};

    WaitMs(100);

	FfbPulseX1();
    WaitMs(7);

	FfbproInitPulses(4);
	WaitMs(35);

	FfbproInitPulses(3);
	WaitMs(14);

	FfbproInitPulses(2);
	WaitMs(78);

	FfbproInitPulses(2);
    WaitMs(4);

	FfbproInitPulses(3);
	WaitMs(59);

	FfbproInitPulses(2);
	
	// -- START MIDI
	FfbSendData(startupFfbData_0, sizeof(startupFfbData_0));	// Program change
	WaitMs(20);

	FfbSendData(startupFfbData_1, sizeof(startupFfbData_1));	// Init
	WaitMs(57);

	FfbSendData(startupFfbData_2, sizeof(startupFfbData_2));	// Initialize effects data memory
	FfbSendData(startupFfbData_3, sizeof(startupFfbData_3));	// Initialize effects data memory

    FfbproSetAutoCenter(0);
	WaitMs(70);
	}

void FfbproSetAutoCenter(uint8_t enable)
{
	const uint8_t ac_enable[] = {
		0xc5, 0x01
	};
	
	const uint8_t ac_disable[] = {
		0xb5, 0x7c, 0x7f,
		0xa5, 0x7f, 0x00,
		0xc5, 0x06,
	};

	FfbSendData(ac_enable, sizeof(ac_enable));
	if (!enable) {
		WaitMs(70);
		FfbSendData(ac_disable, sizeof(ac_disable));
	}
}

const uint8_t* FfbproGetSysExHeader(uint8_t* hdr_len)
{
	static const uint8_t header[] = {0xf0, 0x00, 0x01, 0x0a, 0x01};
	*hdr_len = sizeof(header);
	return header;
}

// effect operations ---------------------------------------------------------

static void FfbproSendEffectOper(uint8_t effectId, uint8_t operation)
{
	uint8_t midi_cmd[3];
	midi_cmd[0] = 0xB5;
	midi_cmd[1] = operation;
	midi_cmd[2] = effectId;
	FfbSendData(midi_cmd, 3);
}

void FfbproStartEffect(uint8_t effectId)
{
	FfbproSendEffectOper(effectId, 0x20);
}

void FfbproStopEffect(uint8_t effectId)
{
	FfbproSendEffectOper(effectId, 0x30);
}

void FfbproFreeEffect(uint8_t effectId)
{
	FfbproSendEffectOper(effectId, 0x10);
}

// modify operations ---------------------------------------------------------

// Send to MIDI effect data modification to the given address of the given effect
void FfbproSendModify(uint8_t effectId, uint8_t address, uint16_t value)
{
	// Modify + Address
	uint8_t midi_cmd[3];
	midi_cmd[0] = 0xB5;
	midi_cmd[1] = address;
	midi_cmd[2] = effectId;
	FfbSendData(midi_cmd, 3);

	// New value
	midi_cmd[0] = 0xA5;
	midi_cmd[1] = value & 0x7F;
	midi_cmd[2] = (value & 0x7F00) >> 8;
	FfbSendData(midi_cmd, 3);
}

void FfbproModifyDuration(uint8_t effectState, uint16_t* midi_data_param, uint8_t effectId, uint16_t duration)
{
	FfbSetParamMidi_14bit(effectState, midi_data_param, effectId, 
							FFP_MIDI_MODIFY_DURATION, duration);
	//FfbproSendModify(effectId, 0x40, duration);
}

void FfbproSetEnvelope(
	USB_FFBReport_SetEnvelope_Output_Data_t* data,
	volatile TEffectState* effect)
{
	uint8_t eid = data->effectBlockIndex;
	
	uint16_t midi_fadeTime;
	/*
	USB effect data:
		uint8_t	reportId;	// =2
		uint8_t	effectBlockIndex;	// 1..40
		uint8_t attackLevel;
		uint8_t	fadeLevel;
		uint16_t	attackTime;	// ms
		uint16_t	fadeTime;	// ms

	MIDI effect data:
		uint8_t command;	// always 0x23	-- start counting checksum from here
		uint8_t waveForm;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
		uint8_t unknown1;	// Overwrite an allocated effect 
		uint16_t duration;	// unit=2ms
		uint16_t triggerButton;	// Bitwise buttons 1 to 9 from LSB
		uint16_t direction;
		uint8_t gain;
		uint16_t sampleRate;	//default 0x64 0x00 = 100Hz
		uint16_t truncate;		//default 0x10 0x4e = 10000 for full waveform
		uint8_t attackLevel;
		uint16_t	attackTime;
		uint8_t		magnitude;
		uint16_t	fadeTime;
		uint8_t	fadeLevel;
		uint16_t frequency;	// unit=Hz; 1 for constant and ramps
		uint16_t param1;	// Varies by effect type; Constant: positive=7f 00, negative=01 01, Other effects: 01 01
		uint16_t param2;	// Varies by effect type; Constant: 00 00, Other effects 01 01
	*/
	
	if (DoDebug(DEBUG_DETAIL))
		{
		LogTextP(PSTR("Set Envelope:"));
		LogBinaryLf(data, sizeof(USB_FFBReport_SetEnvelope_Output_Data_t));
		LogTextP(PSTR("  id    =")); LogBinaryLf(&eid, sizeof(eid));
		LogTextP(PSTR("  attack=")); LogBinaryLf(&data->attackLevel, sizeof(data->attackLevel));
		LogTextP(PSTR("  fade  =")); LogBinaryLf(&data->fadeLevel, sizeof(data->fadeLevel));
		LogTextP(PSTR("  attackTime=")); LogBinaryLf(&data->attackTime, sizeof(data->attackTime));
		LogTextP(PSTR("  fadeTime  =")); LogBinaryLf(&data->fadeTime, sizeof(data->fadeTime));
		FlushDebugBuffer();
		}
		
	volatile FFP_MIDI_Effect_Basic *midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;

	effect->usb_attackLevel = data->attackLevel;
	effect->usb_fadeLevel = data->fadeLevel;
	effect->usb_fadeTime = data->fadeTime;

	if (data->fadeTime == USB_DURATION_INFINITE) // is this check needed? Only if duration is not INF but fadeTime is INF - can this occur?
		midi_fadeTime = MIDI_DURATION_INFINITE;
	else
		midi_fadeTime = UsbUint16ToMidiUint14_Time(effect->usb_duration - effect->usb_fadeTime); 

	FfbSetParamMidi_14bit(effect->state, &(midi_data->fadeTime), eid, 
							FFP_MIDI_MODIFY_FADETIME, midi_fadeTime);
	FfbSetParamMidi_14bit(effect->state, &(midi_data->attackTime), eid, 
							FFP_MIDI_MODIFY_ATTACKTIME, UsbUint16ToMidiUint14_Time(data->attackTime));
	FfbSetParamMidi_7bit(effect->state, &(midi_data->fadeLevel), eid, 
							FFP_MIDI_MODIFY_FADE, ((data->fadeLevel) >> 1) & 0x7f);
	FfbSetParamMidi_7bit(effect->state, &(midi_data->attackLevel), eid, 
							FFP_MIDI_MODIFY_ATTACK, ((data->attackLevel) >> 1) & 0x7f);
}

void FfbproSetCondition(
	USB_FFBReport_SetCondition_Output_Data_t* data,
	volatile TEffectState* effect)
{
	uint8_t eid = data->effectBlockIndex;
	volatile FFP_MIDI_Effect_Basic *common_midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;

	/*
	USB effect data:
		uint8_t	effectBlockIndex;	// 1..40
		uint8_t	parameterBlockOffset;	// bits: 0..3=parameterBlockOffset, 4..5=instance1, 6..7=instance2
		int8_t cpOffset;	// -128..127
		uint8_t	positiveCoefficient;	// 0..255

	MIDI effect data:
		uint16_t coeffAxis0;
		uint16_t coeffAxis1;
		uint16_t offsetAxis0; // not in friction
		uint16_t offsetAxis1; // not in friction
	*/
	
	if (DoDebug(DEBUG_DETAIL))
		{
		LogTextP(PSTR("Set Condition:"));
		LogBinaryLf(data, sizeof(USB_FFBReport_SetCondition_Output_Data_t));
		LogTextP(PSTR("  id   =")); LogBinaryLf(&eid, sizeof(eid));
		LogTextP(PSTR("  block =")); LogBinaryLf(&data->parameterBlockOffset, sizeof(data->parameterBlockOffset));
		LogTextP(PSTR("  offset=")); LogBinaryLf(&data->cpOffset, sizeof(data->cpOffset));
		LogTextP(PSTR("  coeff+=")); LogBinaryLf(&data->positiveCoefficient, sizeof(data->positiveCoefficient));
		FlushDebugBuffer();
		}

	switch (common_midi_data->waveForm) {
		case 0x0d:	// spring (midi: 0x0d)
		case 0x0e:	// damper (midi: 0x0e)
		case 0x0f:	// inertia (midi: 0x0f)
		{
			volatile FFP_MIDI_Effect_Spring_Inertia_Damper *midi_data =
				(FFP_MIDI_Effect_Spring_Inertia_Damper *)&effect->data;
			
			uint16_t midi_offsetAxis1;
			
			if (data->parameterBlockOffset == 0) {
				FfbSetParamMidi_14bit(effect->state, &(midi_data->coeffAxis0), eid, 
										FFP_MIDI_MODIFY_COEFFAXIS0, UsbInt8ToMidiInt14(data->positiveCoefficient));
				FfbSetParamMidi_14bit(effect->state, &(midi_data->offsetAxis0), eid, 
										FFP_MIDI_MODIFY_OFFSETAXIS0, UsbInt8ToMidiInt14(data->cpOffset));
			} else {
				FfbSetParamMidi_14bit(effect->state, &(midi_data->coeffAxis1), eid, 
										FFP_MIDI_MODIFY_COEFFAXIS1, UsbInt8ToMidiInt14(data->positiveCoefficient));
				if (data->cpOffset == 0x80)
					midi_offsetAxis1 = 0x007f;
				else
					midi_offsetAxis1 = UsbInt8ToMidiInt14(-data->cpOffset);
				FfbSetParamMidi_14bit(effect->state, &(midi_data->offsetAxis1), eid, 
										FFP_MIDI_MODIFY_OFFSETAXIS1, midi_offsetAxis1);			
			}

		}
		break;
		
		case 0x10:	// friction (midi: 0x10)
		{
			volatile FFP_MIDI_Effect_Friction *midi_data =
					(FFP_MIDI_Effect_Friction *)&effect->data;

			if (data->parameterBlockOffset == 0)
				FfbSetParamMidi_14bit(effect->state, &(midi_data->coeffAxis0), eid, 
										FFP_MIDI_MODIFY_COEFFAXIS0, UsbInt8ToMidiInt14(data->positiveCoefficient));
			else
				FfbSetParamMidi_14bit(effect->state, &(midi_data->coeffAxis1), eid, 
										FFP_MIDI_MODIFY_COEFFAXIS1, UsbInt8ToMidiInt14(data->positiveCoefficient));
		}
		break;
		
		default:
			break;
	}
}

void FfbproSetPeriodic(
	USB_FFBReport_SetPeriodic_Output_Data_t* data,
	volatile TEffectState* effect)
{
	uint8_t eid = data->effectBlockIndex;

	/*
	USB effect data:
		uint8_t	reportId;	// =4
		uint8_t	effectBlockIndex;	// 1..40
		uint8_t magnitude;
		int8_t	offset;
		uint8_t	phase;	// 0..255 (=0..359, exp-2)
		uint16_t	period;	// 0..32767 ms

	MIDI effect data:

		Offset values other than zero do not work and thus it is ignored on FFP
	*/
	
	if (DoDebug(DEBUG_DETAIL))
		{
		LogTextP(PSTR("Set Periodic:"));
		LogBinaryLf(data, sizeof(USB_FFBReport_SetPeriodic_Output_Data_t));
		LogTextP(PSTR("  id=")); LogBinaryLf(&eid, sizeof(eid));
		LogTextP(PSTR("  magnitude=")); LogBinaryLf(&data->magnitude, sizeof(data->magnitude));
		LogTextP(PSTR("  offset   =")); LogBinaryLf(&data->offset, sizeof(data->offset));
		LogTextP(PSTR("  phase    =")); LogBinaryLf(&data->phase, sizeof(data->phase));
		LogTextP(PSTR("  period   =")); LogBinaryLf(&data->period, sizeof(data->period));
		FlushDebugBuffer();
		}
	
	volatile FFP_MIDI_Effect_Basic *midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;

	uint16_t midi_param1 = 0x007f, midi_param2 = 0x0101, midi_frequency = 0x0001;
	
	effect->usb_magnitude = data->magnitude;

	// Calculate frequency (in MIDI it is in units of Hz and can have value from 1 to 169Hz)
	if (data->period <= 5)
		midi_frequency = 0x0129; //169Hz
	else if (data->period < 1000)
		midi_frequency = UsbUint16ToMidiUint14(1000 / data->period);
	
	FfbSetParamMidi_14bit(effect->state, &(midi_data->frequency), eid, 
							FFP_MIDI_MODIFY_FREQUENCY, midi_frequency);
							
	// Check phase if relevant (+90 phase for sine makes it a cosine)
	if (midi_data->waveForm == 2 || midi_data->waveForm == 3) // sine
	{
		if (data->phase >= 32 && data->phase <= 224) {
			midi_data->waveForm = 3;	// cosine. Can't be modified
		} else {
			midi_data->waveForm = 2;	// sine. Can't be modified
		}

		// Calculate min-max from magnitude and offset
		uint8_t magnitude = data->magnitude / 2;
		
		FfbSetParamMidi_14bit(effect->state, &(midi_data->param1), eid, 
								FFP_MIDI_MODIFY_PARAM1, UsbInt8ToMidiInt14(data->offset / 2 + magnitude)); // max	
		FfbSetParamMidi_14bit(effect->state, &(midi_data->param2), eid, 
								FFP_MIDI_MODIFY_PARAM2, UsbInt8ToMidiInt14(data->offset / 2 - magnitude)); // min											
	} else {
		midi_data->param1 = midi_param1; //never again changed
		midi_data->param2 = midi_param2; //never again changed
	}
	

}

void FfbproSetConstantForce(
	USB_FFBReport_SetConstantForce_Output_Data_t* data,
	volatile TEffectState* effect)
{
	uint8_t eid = data->effectBlockIndex;
	/*
	USB data:
		uint8_t	reportId;	// =5
		uint8_t	effectBlockIndex;	// 1..40
		int16_t magnitude;	// -255..255

	MIDI effect data:
		uint8_t command;	// always 0x23	-- start counting checksum from here
		uint8_t waveForm;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
		uint8_t unknown1;	// Overwrite an allocated effect 
		uint16_t duration;	// unit=2ms
		uint16_t triggerButton;	// Bitwise buttons 1 to 9 from LSB
		uint16_t direction;
		uint8_t gain;
		uint16_t sampleRate;	//default 0x64 0x00 = 100Hz
		uint16_t truncate;		//default 0x10 0x4e = 10000 for full waveform
		uint8_t attackLevel;
		uint16_t	attackTime;
		uint8_t		magnitude;
		uint16_t	fadeTime;
		uint8_t	fadeLevel;
		uint16_t frequency;	// unit=Hz; 1 for constant and ramps
		uint16_t param1;	// Varies by effect type; Constant: positive=7f 00, negative=01 01, Other effects: 01 01
		uint16_t param2;	// Varies by effect type; Constant: 00 00, Other effects 01 01

	*/
	
	if (DoDebug(DEBUG_DETAIL))
		{
		LogTextP(PSTR("Set Constant Force:"));
		LogBinaryLf(data, sizeof(USB_FFBReport_SetConstantForce_Output_Data_t));
		LogTextP(PSTR("  id=")); LogBinaryLf(&eid, sizeof(eid));
		LogTextP(PSTR("  magnitude=")); LogBinaryLf(&data->magnitude, sizeof(data->magnitude));
		FlushDebugBuffer();
		}
	
	volatile FFP_MIDI_Effect_Basic *midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;

	effect->usb_magnitude = data->magnitude;

	uint8_t midi_magnitude;
	uint16_t midi_param1;

	if (data->magnitude >= 0) {
		midi_magnitude = (data->magnitude >> 1) & 0x7f;
		midi_param1 = 0x007f;
	} else {
		midi_magnitude = (( -(data->magnitude + 1)) >> 1) & 0x7f;
		midi_param1 = 0x0101;
	}
	
	FfbSetParamMidi_7bit(effect->state, &(midi_data->magnitude), eid, 
						FFP_MIDI_MODIFY_MAGNITUDE, midi_magnitude);
	FfbSetParamMidi_14bit(effect->state, &(midi_data->param1), eid, 
						FFP_MIDI_MODIFY_PARAM1, midi_param1);						
	midi_data->param2 = 0x0000; // never again modified 
}

void FfbproSetRampForce(
	USB_FFBReport_SetRampForce_Output_Data_t* data,
	volatile TEffectState* effect)
{
	uint8_t eid = data->effectBlockIndex;
	if (DoDebug(DEBUG_DETAIL))
		{
		uint8_t eid = data->effectBlockIndex;
		LogTextP(PSTR("Set Ramp Force:"));
		LogBinaryLf(data, sizeof(USB_FFBReport_SetRampForce_Output_Data_t));
		LogTextP(PSTR("  id=")); LogBinaryLf(&eid, sizeof(eid));
		LogTextP(PSTR("  start=")); LogBinaryLf(&data->start, sizeof(data->start));
		LogTextP(PSTR("  end  =")); LogBinaryLf(&data->end, sizeof(data->end));
		FlushDebugBuffer();
		}

	// FFP supports only ramp up from MIN to MAX and ramp down from MAX to MIN?
	/*
	USB effect data:
		uint8_t	reportId;	// =6
		uint8_t	effectBlockIndex;	// 1..40
		int8_t start;
		int8_t	end;
	*/
	
	volatile FFP_MIDI_Effect_Basic *midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;

	uint16_t midi_param1;

	if (data->start < 0)
		midi_param1 = 0x0100 | (-(data->start+1));
	else
		midi_param1 = data->start;
	
	FfbSetParamMidi_14bit(effect->state, &(midi_data->param1), eid, 
						FFP_MIDI_MODIFY_PARAM1, midi_param1);	

	FfbSetParamMidi_14bit(effect->state, &(midi_data->param2), eid, 
						FFP_MIDI_MODIFY_PARAM2, UsbInt8ToMidiInt14(data->end));
}

int FfbproSetEffect(
	USB_FFBReport_SetEffect_Output_Data_t *data,
	volatile TEffectState* effect
)
{
	uint8_t eid = data->effectBlockIndex;

	/*
	USB effect data:
		uint8_t	reportId;	// =1
		uint8_t	effectBlockIndex;	// 1..40
		uint8_t	effectType;	// 1..12 (effect usages: 26,27,30,31,32,33,34,40,41,42,43,28)
		uint16_t	duration; // 0..32767 ms
		uint16_t	triggerRepeatInterval; // 0..32767 ms
		uint16_t	samplePeriod;	// 0..32767 ms
		uint8_t	gain;	// 0..255	 (physical 0..10000)
		uint8_t	triggerButton;	// button ID (0..8)
		uint8_t	enableAxis; // bits: 0=X, 1=Y, 2=DirectionEnable
		uint8_t	directionX;	// angle (0=0 .. 180=0..360deg)
		uint8_t	directionY;	// angle (0=0 .. 180=0..360deg)
	*/

	volatile FFP_MIDI_Effect_Basic *midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;
	uint8_t midi_data_len = sizeof(FFP_MIDI_Effect_Basic); 	// default MIDI data size
	bool is_periodic = false;
	
	// Data applying to all effects
	uint16_t buttonBits = 0;
	if (data->triggerButton != USB_TRIGGERBUTTON_NULL)		
		buttonBits = (1 << data->triggerButton);
	//Buttons 1-9 from LSB
	FfbSetParamMidi_14bit(effect->state, &(midi_data->triggerButton), eid, 
							FFP_MIDI_MODIFY_TRIGGERBUTTON, (buttonBits & 0x7F) + ( (buttonBits & 0x0180) << 1 ));	

	// Fill in the effect type specific data
	switch (data->effectType)
	{
		case USB_EFFECT_SQUARE:
		case USB_EFFECT_SINE:
		case USB_EFFECT_TRIANGLE:
		case USB_EFFECT_SAWTOOTHDOWN:
		case USB_EFFECT_SAWTOOTHUP:
			is_periodic = true;
		case USB_EFFECT_CONSTANT:
		case USB_EFFECT_RAMP:
		{
			/*
			MIDI effect data:
				uint8_t command;	// always 0x23	-- start counting checksum from here
				uint8_t waveForm;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
				uint8_t unknown1;	// Overwrite an allocated effect 
				uint16_t duration;	// unit=2ms
				uint16_t triggerButton;	// Bitwise buttons 1 to 9 from LSB
				uint16_t direction;
				uint8_t gain;
				uint16_t sampleRate;	//default 0x64 0x00 = 100Hz
				uint16_t truncate;		//default 0x10 0x4e = 10000 for full waveform
				uint8_t attackLevel;
				uint16_t	attackTime;
				uint8_t		magnitude;
				uint16_t	fadeTime;
				uint8_t	fadeLevel;
				uint16_t frequency;	// unit=Hz; 1 for constant and ramps
				uint16_t param1;	// Varies by effect type; Constant: positive=7f 00, negative=01 01, Other effects: 01 01
				uint16_t param2;	// Varies by effect type; Constant: 00 00, Other effects 01 01

			*/
			// Effect Gain
			FfbSetParamMidi_7bit(effect->state, &(midi_data->gain), eid, 
								FFP_MIDI_MODIFY_GAIN, (data->gain >> 1) & 0x7f);			
			
			// Convert direction
			uint16_t usbdir = data->directionX;
			usbdir = usbdir * 2;
			FfbSetParamMidi_14bit(effect->state, &(midi_data->direction), eid, 
								FFP_MIDI_MODIFY_DIRECTION, (usbdir & 0x7F) + ( (usbdir & 0x0180) << 1 ));
			
			// Recalculate fadeTime for MIDI since change to duration changes the fadeTime too
			uint16_t midi_fadeTime;
			if (data->duration == USB_DURATION_INFINITE) {
				midi_fadeTime = MIDI_DURATION_INFINITE;
			} else {
				if (effect->usb_fadeTime == USB_DURATION_INFINITE) {
					midi_fadeTime = MIDI_DURATION_INFINITE;
				} else {
					if (effect->usb_duration > effect->usb_fadeTime) {
						// add some safety and special case handling
						midi_fadeTime = UsbUint16ToMidiUint14_Time(effect->usb_duration - effect->usb_fadeTime);
					} else {
						midi_fadeTime = midi_data->duration;
					}
				}
			}			
			FfbSetParamMidi_14bit(effect->state, &(midi_data->fadeTime), eid, 
								FFP_MIDI_MODIFY_FADETIME, midi_fadeTime);
						
		}
		break;
	
		case USB_EFFECT_SPRING:
		case USB_EFFECT_DAMPER:
		case USB_EFFECT_INERTIA:
		{
			/*
			MIDI effect data:
				uint8_t command;	// always 0x23	-- start counting checksum from here
				uint8_t waveForm;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
				uint8_t unknown1;	// ? always 0x7F
				uint16_t duration;	// unit=2ms
				uint16_t triggerButton;	// Bitwise buttons 1 to 9 from LSB
				uint16_t coeffAxis0;
				uint16_t coeffAxis1;
				uint16_t offsetAxis0;
				uint16_t offsetAxis1;
			*/
//			volatile FFP_MIDI_Effect_Spring_Inertia_Damper *midi_data = (FFP_MIDI_Effect_Spring_Inertia_Damper *) &gEffectStates[eid].data;
			midi_data_len = sizeof(FFP_MIDI_Effect_Spring_Inertia_Damper);

		}
		break;
		
		case USB_EFFECT_FRICTION:
		{
			/*
			MIDI effect data:
				uint8_t command;	// always 0x23	-- start counting checksum from here
				uint8_t waveForm;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
				uint8_t unknown1;	// ? always 0x7F
				uint16_t duration;	// unit=2ms
				uint16_t triggerButton;	// Bitwise buttons 1 to 9 from LSB
				uint16_t coeffAxis0;
				uint16_t coeffAxis1;
			*/
//			volatile FFP_MIDI_Effect_Friction *midi_data = (FFP_MIDI_Effect_Friction *) &gEffectStates[eid].data;
			midi_data_len = sizeof(FFP_MIDI_Effect_Friction);

		}
		break;
		
		case USB_EFFECT_CUSTOM:	// custom (midi: ? does FFP support custom forces?)
		{
		}
		break;
		
		default:
		break;
	}
	
	return midi_data_len;
}

void FfbproCreateNewEffect(
	USB_FFBReport_CreateNewEffect_Feature_Data_t* inData,
	volatile TEffectState* effect)
{
	/*
	USB effect data:
		uint8_t		reportId;	// =1
		uint8_t	effectType;	// Enum (1..12): ET 26,27,30,31,32,33,34,40,41,42,43,28
		uint16_t	byteCount;	// 0..511	- only valid with Custom Force
	*/

	// Set defaults to the effect data

	volatile FFP_MIDI_Effect_Basic *midi_data = (volatile FFP_MIDI_Effect_Basic *)&effect->data;

	midi_data->magnitude = 0x7f;
	midi_data->frequency = 0x0001;
	midi_data->attackLevel = 0x00;
	midi_data->attackTime = 0x0000;
	midi_data->fadeLevel = 0x00;
	midi_data->fadeTime = 0x0000;
	midi_data->gain = 0x7F;
	
	// Constants
	midi_data->command = 0x23;
	midi_data->unknown1 = 0x7F;
	midi_data->triggerButton = 0x0000;	
	midi_data->sampleRate = 0x0064;
	midi_data->truncate = 0x4E10;
	if (inData->effectType == 0x01)	// constant
		midi_data->param2 = 0x0000;
	else
		midi_data->param2 = 0x0101;
}

