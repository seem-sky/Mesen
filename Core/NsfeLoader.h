#pragma once
#include "stdafx.h"
#include "RomData.h"
#include <algorithm>
#include "NsfLoader.h"

class NsfeLoader : public NsfLoader
{
private:
	void Read(uint8_t* &data, uint8_t& dest)
	{
		dest = data[0];
		data++;
	}

	void Read(uint8_t* &data, uint16_t& dest)
	{
		dest = data[0] | (data[1] << 8);
		data += 2;
	}

	void Read(uint8_t* &data, uint32_t& dest)
	{
		dest = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
		data += 4;
	}

	void Read(uint8_t* &data, int32_t& dest)
	{
		dest = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
		data += 4;
	}

	void Read(uint8_t* &data, char* dest, size_t len)
	{
		memcpy(dest, data, len);
		data += len;
	}

	vector<string> ReadStrings(uint8_t* &data, uint8_t* chunkEnd)
	{
		vector<string> strings;
		stringstream ss;
		while(data < chunkEnd) {
			if(data[0] == 0) {
				//end of string
				strings.push_back(ss.str());
				ss = stringstream();
			} else {
				ss << (char)data[0];
			}
			data++;
		}

		//truncate all strings to 255 characters + null
		for(size_t i = 0; i < strings.size(); i++) {
			strings[i] = strings[i].substr(0, std::min((int)strings[i].size(), 255));
		}

		return strings;
	}

	string ReadFourCC(uint8_t* &data)
	{
		stringstream ss;
		for(int i = 0; i < 4; i++) {
			ss << (char)data[i];
		}
		data += 4;
		return ss.str();
	}

	bool ReadChunk(uint8_t* &data, uint8_t* dataEnd, RomData& romData)
	{
		if(data + 4 > dataEnd) {
			return false;
		}

		NsfHeader& header = romData.NsfHeader;

		uint32_t length;
		Read(data, length);

		uint8_t* chunkEnd = data + 4 + length;

		if(chunkEnd > dataEnd) {
			return false;
		}

		string fourCC = ReadFourCC(data);
		if(fourCC.compare("INFO") == 0) {
			Read(data, header.LoadAddress);
			Read(data, header.InitAddress);
			Read(data, header.PlayAddress);
			Read(data, header.Flags);
			Read(data, header.SoundChips);
			Read(data, header.TotalSongs);
			Read(data, header.StartingSong);

			header.PlaySpeedNtsc = 16639;
			header.PlaySpeedPal = 19997;

			//Adjust to match NSF spec
			header.StartingSong++;
		} else if(fourCC.compare("DATA") == 0) {
			//Pad start of file to make the first block start at a multiple of 4k
			romData.PrgRom.insert(romData.PrgRom.end(), header.LoadAddress % 4096, 0);

			romData.PrgRom.insert(romData.PrgRom.end(), (uint8_t*)data, data+length);

			//Pad out the last block to be a multiple of 4k
			if(romData.PrgRom.size() % 4096 != 0) {
				romData.PrgRom.insert(romData.PrgRom.end(), 4096 - (romData.PrgRom.size() % 4096), 0);
			}
		} else if(fourCC.compare("NEND") == 0) {
			//End of file
			romData.Error = false;
			return false;
		} else if(fourCC.compare("BANK") == 0) {
			memset(header.BankSetup, 0, sizeof(header.BankSetup));
			Read(data, (char*)header.BankSetup, std::min(8, (int)length));
		} else if(fourCC.compare("plst") == 0) {
			//not supported
		} else if(fourCC.compare("time") == 0) {
			int i = 0;
			while(data < chunkEnd) {
				Read(data, header.TrackLength[i]);
				i++;
			}
		} else if(fourCC.compare("fade") == 0) {
			int i = 0;
			while(data < chunkEnd) {
				Read(data, header.TrackFade[i]);
				i++;
			}
		} else if(fourCC.compare("tlbl") == 0) {
			vector<string> trackNames = ReadStrings(data, chunkEnd);
			stringstream ss;
			for(string &trackName : trackNames) {
				ss << trackName;
				ss << "[!|!]";
			}
			strcpy_s(header.TrackName, ss.str().c_str());

		} else if(fourCC.compare("auth") == 0) {
			vector<string> infoStrings = ReadStrings(data, chunkEnd);

			if(infoStrings.size() > 0) {
				strcpy_s(header.SongName, infoStrings[0].c_str());
				if(infoStrings.size() > 1) {
					strcpy_s(header.ArtistName, infoStrings[1].c_str());
					if(infoStrings.size() > 2) {
						strcpy_s(header.CopyrightHolder, infoStrings[2].c_str());
						if(infoStrings.size() > 3) {
							strcpy_s(header.RipperName, infoStrings[3].c_str());
						}
					}
				}
			}
		} else if(fourCC.compare("text") == 0) {
			//not supported
		} else {
			if(fourCC[0] >= 'A' && fourCC[0] <= 'Z') {
				//unknown required block, can't read file
				return false;
			}
		}

		data = chunkEnd;

		return true;
	}

public:
	RomData LoadRom(vector<uint8_t>& romFile)
	{
		RomData romData;
		NsfHeader &header = romData.NsfHeader;

		InitHeader(header);

		uint8_t* data = romFile.data() + 4;
		uint8_t* endOfData = romFile.data() + romFile.size();

		memset(header.SongName, 0, sizeof(header.SongName));
		memset(header.ArtistName, 0, sizeof(header.ArtistName));
		memset(header.CopyrightHolder, 0, sizeof(header.CopyrightHolder));

		//Will be set to false when we read NEND block
		romData.Error = true;
		while(ReadChunk(data, endOfData, romData)) {
			//Read all chunks
		}

		InitializeFromHeader(romData);

		return romData;
	}
};