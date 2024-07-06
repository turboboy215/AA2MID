/*AudioArts (GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
long seqPtrLoc;
long seqOffset;
int i, j;
char outfile[1000000];
int format = 0;
int songNum;
long seqPtrs[4];
long curSpeed;
long nextPtr;
long endPtr;
long bankAmt;

const char Table1Find[15] = { 0x16, 0x00, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19};
const char Table2Find[12] = { 0x1A, 0x22, 0x13, 0x1A, 0x13, 0x73, 0x2C, 0x72, 0x5F, 0x16, 0x00, 0x21 };

/*Alternate code to look for in Carmageddon (early driver?)*/
const char Table1FindCarma[9] = { 0x16, 0x00, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x2A };
const char Table2FindCarma[13] = { 0x1A, 0x77, 0x13, 0x1A, 0x13, 0x23, 0x73, 0x23, 0x72, 0x5F, 0x16, 0x00, 0x21 };

unsigned long seqList[500];
unsigned static char* romData;
int totalSeqs;

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptrs[4], long nextPtr, long endPtr, int speed);
void getSeqList(unsigned long list[], long offset);
void seqs2txt(unsigned long list[]);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("AudioArts (GBC) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: AA2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			if ((rom = fopen(argv[1], "rb")) == NULL)
			{
				printf("ERROR: Unable to open file %s!\n", argv[1]);
				exit(1);
			}
			else
			{
				bank = strtol(argv[2], NULL, 16);
				if (bank != 1)
				{
					bankAmt = bankSize;
				}
				else
				{
					bankAmt = 0;
				}
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);

			/*Try to search the bank for song pattern table loader*/
			for (i = 0; i < bankSize; i++)
			{
				if (!memcmp(&romData[i], Table1Find, 15))
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					if (romData[i + 15] == 0x2A)
					{
						format = 1;
						printf("Detected old table format.\n");
					}
					else if (romData[i + 15] == 0x19)
					{
						format = 2;
						printf("Detected new table format.\n");
					}
					printf("Song table starts at 0x%04x...\n", tableOffset);
					break;
				}
			}

			/*Alternate method for Carmageddon*/
			for (i = 0; i < bankSize; i++)
			{
				if (!memcmp(&romData[i], Table1FindCarma, 9) && format != 1 && format != 2)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					format = 0;
					printf("Detected old (Carmageddon) table format.\n");
					printf("Song table starts at 0x%04x...\n", tableOffset);
					break;
				}
			}

			/*Now try to search the bank for sequence table loader*/
			for (i = 0; i < bankSize; i++)
			{
				if (!memcmp(&romData[i], Table2Find, 12))
				{
					seqPtrLoc = bankAmt + i + 12;
					printf("Found pointer to sequence table at address 0x%04x!\n", seqPtrLoc);
					seqOffset = ReadLE16(&romData[seqPtrLoc - bankAmt]);
					printf("Sequence table starts at 0x%04x...\n", seqOffset);
					getSeqList(seqList, seqOffset);
					break;
				}
			}

			/*Alternate method for Carmageddon*/
			for (i = 0; i < bankSize; i++)
			{
				if (!memcmp(&romData[i], Table2FindCarma, 13) && format != 1 && format != 2)
				{
					seqPtrLoc = bankAmt + i + 13;
					printf("Found pointer to sequence table at address 0x%04x!\n", seqPtrLoc);
					seqOffset = ReadLE16(&romData[seqPtrLoc - bankAmt]);
					printf("Sequence table starts at 0x%04x...\n", seqOffset);
					getSeqList(seqList, seqOffset);
					break;
				}
			}
			if (tableOffset != NULL && seqOffset != NULL)
			{
				songNum = 1;
				i = tableOffset;
				if (format == 1)
				{
					/*Skip first "empty" song*/
					i += 9;

					while ((i + 4) != seqOffset)
					{
						curSpeed = romData[i + 12 - bankAmt];
						printf("Song %i tempo: %i\n", songNum, curSpeed);
						seqPtrs[0] = ReadLE16(&romData[i + 4 - bankAmt]);
						printf("Song %i channel 1: 0x%04x\n", songNum, seqPtrs[0]);
						seqPtrs[1] = ReadLE16(&romData[i + 6 - bankAmt]);
						printf("Song %i channel 2: 0x%04x\n", songNum, seqPtrs[1]);
						seqPtrs[2] = ReadLE16(&romData[i + 8 - bankAmt]);
						printf("Song %i channel 3: 0x%04x\n", songNum, seqPtrs[2]);
						seqPtrs[3] = ReadLE16(&romData[i + 10 - bankAmt]);
						printf("Song %i channel 4: 0x%04x\n", songNum, seqPtrs[3]);
						if (format == 1)
						{
							nextPtr = ReadLE16(&romData[i + 18 - bankAmt]);
						}
						else if (format == 2)
						{
							nextPtr = ReadLE16(&romData[i + 16 - bankAmt]);
						}
						endPtr = seqOffset;
						song2txt(songNum, seqPtrs, nextPtr, endPtr, curSpeed);
						i += 13;
						songNum++;
					}
				}
				else if (format == 2)
				{
				/*Skip first "empty" song*/
				i += 14;

				while (i != seqOffset)
				{

					curSpeed = romData[i - bankAmt];
					printf("Song %i tempo: %i\n", songNum, curSpeed);
					seqPtrs[0] = ReadLE16(&romData[i + 2 - bankAmt]);
					printf("Song %i channel 1: 0x%04x\n", songNum, seqPtrs[0]);
					seqPtrs[1] = ReadLE16(&romData[i + 4 - bankAmt]);
					printf("Song %i channel 2: 0x%04x\n", songNum, seqPtrs[1]);
					seqPtrs[2] = ReadLE16(&romData[i + 6 - bankAmt]);
					printf("Song %i channel 3: 0x%04x\n", songNum, seqPtrs[2]);
					seqPtrs[3] = ReadLE16(&romData[i + 8 - bankAmt]);
					printf("Song %i channel 4: 0x%04x\n", songNum, seqPtrs[3]);
					nextPtr = ReadLE16(&romData[i + 16 - bankAmt]);
					endPtr = seqOffset;
					song2txt(songNum, seqPtrs, nextPtr, endPtr, curSpeed);
					i += 14;
					songNum++;
				}
				}

				/*Carmageddon format - doesn't use channel 3 for music*/
				else if (format == 0)
				{
				while (romData[i - bankAmt] != 0)
				{
					curSpeed = 10;
					seqPtrs[0] = ReadLE16(&romData[i - bankAmt]);
					printf("Song %i channel 1: 0x%04x\n", songNum, seqPtrs[0]);
					seqPtrs[1] = ReadLE16(&romData[i + 2 - bankAmt]);
					printf("Song %i channel 2: 0x%04x\n", songNum, seqPtrs[1]);
					seqPtrs[2] = 0;
					seqPtrs[3] = ReadLE16(&romData[i + 4 - bankAmt]);
					printf("Song %i channel 4: 0x%04x\n", songNum, seqPtrs[3]);
					nextPtr = ReadLE16(&romData[i + 6 - bankAmt]);
					song2txt(songNum, seqPtrs, nextPtr, endPtr, curSpeed);
					i += 6;
					songNum++;
				}
				endPtr = i;
				}
				seqs2txt(seqList);
			}
			else
			{
			printf("ERROR: Magic bytes not found!\n");
			exit(-1);
			}

			printf("The operation was successfully completed!\n");
		}
	}
}

/*Convert the song data to TXT*/
void song2txt(int songNum, long ptrs[4], long nextPtr, long endPtr, int speed)
{
	long curPos = 0;
	int index = 0;
	int curSeq = 0;
	int curChan = 0;
	long pattern[2];
	signed int transpose = 0;
	int endSong = 0;

	sprintf(outfile, "song%d.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%d.txt!\n", songNum);
		exit(2);
	}
	else
	{
		fprintf(txt, "Song speed: %i\n\n", speed);
		for (curChan = 0; curChan < 4; curChan++)
		{
			fprintf(txt, "Channel %i:\n", curChan + 1);
			if (ptrs[curChan] == 0)
			{
				fprintf(txt, "(Empty channel)\n");
			}
			else
			{
				index = ptrs[curChan] - bankAmt;
				if (curChan == 0)
				{
					endSong = 0;
					while ((index + bankAmt) != ptrs[1] && (index + bankAmt) != ptrs[2] && (index + bankAmt) != ptrs[3] && (index + bankAmt) != nextPtr && (index + bankAmt) != endPtr && index < bankSize && endSong == 0)
					{
						pattern[0] = romData[index];
						pattern[1] = romData[index + 1];
						transpose = (signed char)pattern[0];
						curSeq = pattern[1];
						if (curSeq == 255)
						{
							endSong = 1;
						}
						fprintf(txt, "Sequence %i, transpose %i\n", curSeq, transpose);
						index += 2;
					}
				}
				else if (curChan == 1)
				{
					endSong = 0;
					while ((index + bankAmt) != ptrs[0] && (index + bankAmt) != ptrs[2] && (index + bankAmt) != ptrs[3] && (index + bankAmt) != nextPtr && (index + bankAmt) != endPtr && index < bankSize && endSong == 0)
					{
						pattern[0] = romData[index];
						pattern[1] = romData[index + 1];
						transpose = (signed char)pattern[0];
						curSeq = pattern[1];
						if (curSeq == 255)
						{
							endSong = 1;
						}
						else
						{
							fprintf(txt, "Sequence %i, transpose %i\n", curSeq, transpose);
							index += 2;
						}

					}
				}
				else if (curChan == 2)
				{
					endSong = 0;
					while ((index + bankAmt) != ptrs[0] && (index + bankAmt) != ptrs[1] && (index + bankAmt) != ptrs[3] && (index + bankAmt) != nextPtr && (index + bankAmt) != endPtr && index < bankSize && endSong == 0)
					{
						pattern[0] = romData[index];
						pattern[1] = romData[index + 1];
						transpose = (signed char)pattern[0];
						curSeq = pattern[1];
						if (curSeq == 255)
						{
							endSong = 1;
						}
						else
						{
							fprintf(txt, "Sequence %i, transpose %i\n", curSeq, transpose);
							index += 2;
						}
					}
				}
				else if (curChan == 3)
				{
					endSong = 0;
					while ((index + bankAmt) != ptrs[0] && (index + bankAmt) != ptrs[1] && (index + bankAmt) != ptrs[2] && (index + bankAmt) != nextPtr && (index + bankAmt) != endPtr && index < bankSize && endSong == 0)
					{
						pattern[0] = romData[index];
						pattern[1] = romData[index + 1];
						transpose = (signed char)pattern[0];
						curSeq = pattern[1];
						if (curSeq == 255)
						{
							endSong = 1;
						}
						else
						{
							fprintf(txt, "Sequence %i, transpose %i\n", curSeq, transpose);
							index += 2;
						}
					}
				}
				fprintf(txt, "\n");
			}
		}
		fclose(txt);
	}
}

void getSeqList(unsigned long list[], long offset)
{
	int cnt = 0;
	unsigned long curValue;
	unsigned long curValue2;
	long newOffset = offset;
	long offset2 = offset - bankAmt;

	for (cnt = 0; cnt < 500; cnt++)
	{
		curValue = (ReadLE16(&romData[newOffset - bankAmt])) - bankAmt;
		curValue2 = (ReadLE16(&romData[newOffset - bankAmt]));
		if (curValue2 >= bankAmt && curValue2 < (bankAmt * 2))
		{
			list[cnt] = curValue;
			newOffset += 2;
		}
		else
		{
			totalSeqs = cnt;
			break;
		}
	}
}

void seqs2txt(unsigned long list[])
{
	int songEnd = 0;
	sprintf(outfile, "seqs.txt");
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file seqs.txt!\n");
		exit(2);
	}
	else
	{
		unsigned char command[3];
		int seqNum = 0;
		int curPos = 0;
		int nextPos = 0;
		int curNote = 0;
		int k = 0;
		int seqsLeft = totalSeqs;
		curPos = list[0];
		for (seqsLeft = totalSeqs; seqsLeft >= 0; seqsLeft--)
		{
			while (curPos < bankSize)
			{
				for (k = 0; k < totalSeqs; k++)
				{
					if (curPos == list[k])
					{
						if (k != 0)
						{
							fprintf(txt, "\n");
						}
						fprintf(txt, "Sequence %i:\n", k);
					}
				}
				command[0] = romData[curPos];
				command[1] = romData[curPos + 1];
				command[2] = romData[curPos + 2];
				if (command[0] == 0x00)
				{
					fprintf(txt, "End of sequence\n");
					curPos++;
				}
				else if (command[1] == 0xFD && command[0] != 0xFF)
				{
					fprintf(txt, "Repeat note, length: %i\n", command[0]);
					curPos += 2;
				}
				else if (command[1] == 0xFE || command[1] == 0xFF)
				{
					fprintf(txt, "Rest: %i\n", command[0]);
					curPos += 2;
				}
				else if (command[0] == 0xFF)
				{
					fprintf(txt, "Loop back to point: 0x%04X\n", ReadLE16(&romData[curPos+1]));
					curPos += 3;
				}
				else
				{
					fprintf(txt, "Note: %02X, length: %i, instrument: %i\n", command[1], command[0], command[2]);
					curPos += 3;
				}
			}

		}
		fclose(txt);
	}
}