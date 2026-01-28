#include <StrUtils.h>
#include <Windows.h>
#include <ctime>

void CreateRandomString(char *pszDest, int nLength)
{
	static const char c_szAlphaNum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < nLength; ++i)
		pszDest[i] = c_szAlphaNum[rand() % (sizeof(c_szAlphaNum) - 1)];

	pszDest[nLength] = '\0';
}

void generateRandomHWID(char hwid[64]) { 
    srand(time(0));
    memset(hwid, ' ', 17);
     
    hwid[17] = 'A' + rand() % 26;  
    hwid[18] = 'A' + rand() % 26;  
    hwid[19] = '-';
    hwid[20] = 'A' + rand() % 26;  
    hwid[21] = 'A' + rand() % 26;  
     
    for (int i = 22; i < 30; i++) {
        hwid[i] = '0' + rand() % 10;
    }
     
    hwid[30] = '\0';
}