
#include "hashtable.h"

unsigned long from_blizzard::cryptTable[0x500];
bool from_blizzard::bInitialized = false;

void from_blizzard::prepareCryptTable()
{
	unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;
	for (index1 = 0; index1 < 0x100; index1++)
	{
		for (index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
		{
			unsigned long temp1, temp2;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			temp1 = (seed & 0xFFFF) << 0x10;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			temp2 = (seed & 0xFFFF);
			cryptTable[index2] = (temp1 | temp2);
		}
	}
	bInitialized = true;
}

unsigned long from_blizzard::HashString(unsigned long dwHashType, const char *lpsz, unsigned int len)
{
	if (!bInitialized)
		prepareCryptTable();
	
	unsigned char *key  = (unsigned char *)lpsz;
	unsigned long seed1 = 0x7FED7FED;
	unsigned long seed2 = 0xEEEEEEEE;
	int ch;

	if (len == 0)
	{
		while (*key != 0)
		{
			ch = toupper(*key++);
			seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}
	}
	else
	{
		for (unsigned int i = 0; i < len; i++)
		{
			ch = toupper(*key++);
			seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}
	}
	return seed1;
}
