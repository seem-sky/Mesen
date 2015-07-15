#pragma once
#include "stdafx.h"
#include "../BlipBuffer/Blip_Buffer.h"
#include "APU.h"
#include "IMemoryHandler.h"
#include "ApuEnvelope.h"

class NoiseChannel : public ApuEnvelope
{
private:	
	const vector<uint16_t> _noisePeriodLookupTable = { { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 } };

	//On power-up, the shift register is loaded with the value 1.
	uint16_t _shiftRegister = 1;
	bool _modeFlag = false;

	bool IsMuted()
	{
		//The mixer receives the current envelope volume except when Bit 0 of the shift register is set, or The length counter is zero
		return (_shiftRegister & 0x01) == 0x01;
	}

public:
	NoiseChannel()
	{
		SetVolume(0.0741);
	}

	void GetMemoryRanges(MemoryRanges &ranges)
	{
		ranges.AddHandler(MemoryType::RAM, MemoryOperation::Write, 0x400C, 0x400F);
	}

	void WriteRAM(uint16_t addr, uint8_t value)
	{
		APU::StaticRun();
		switch(addr & 0x03) {
			case 0:		//400C
				InitializeLengthCounter((value & 0x20) == 0x20);
				InitializeEnvelope(value);
				break;

			case 2:		//400E
				_period = _noisePeriodLookupTable[value & 0x0F];
				break;

			case 3:		//400F
				LoadLengthCounter(value >> 3);

				//The envelope is also restarted.
				ResetEnvelope();
				break;
		}
	}
	
	void Clock()
	{
		uint32_t volume = GetVolume();
		//Feedback is calculated as the exclusive-OR of bit 0 and one other bit: bit 6 if Mode flag is set, otherwise bit 1.
		uint16_t feedback = (_shiftRegister & 0x01) ^ ((_shiftRegister >> (_modeFlag ? 6 : 1)) & 0x01);
		_shiftRegister >>= 1;
		_shiftRegister |= (feedback << 14);

		if(IsMuted()) {
			AddOutput(0);
		} else {
			AddOutput(GetVolume()); 
		}
	}
};