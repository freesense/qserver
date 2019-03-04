
/////////////////////////////////////////////////////////////////////////
//编写人：李开程
//编写时间：2002-12-12
//实现功能：读取拼音库，根据汉字生成其拼音简称
/////////////////////////////////////////////////////////////////////////
#ifndef _INC_WORDSPELL_H
#define _INC_WORDSPELL_H

struct SWordSpell
{
	unsigned short  m_usWord;
	char			m_acSpell[8];
};

class CWordSpell
{
private:
	int				m_iTotalWord;
	SWordSpell*		m_pstWordSpell;//读拼音库放入成员变量m_pstWordSpell内
public:
CWordSpell()
{
	m_iTotalWord=0;
	m_pstWordSpell=NULL;
}

~CWordSpell()
{
	if(m_pstWordSpell!=NULL)
		delete [] m_pstWordSpell;
}

void ReadVoiceLib(char*	pcExecPath)
{
	if(m_pstWordSpell!=NULL)
		return;
	char	acExecPath[256];
	sprintf(acExecPath,"%svoice.lib",pcExecPath);

	FILE* fp = fopen(acExecPath,"rb");
	if(fp == NULL)
		return;
	fseek(fp,0,SEEK_END);
	int nFileLen = (int)ftell(fp);
	if(nFileLen <=0 )
	{
		fclose(fp);
		return;
	}
	m_iTotalWord = nFileLen/sizeof(SWordSpell);

	if (m_pstWordSpell)
	{
		delete m_pstWordSpell;
		m_pstWordSpell = NULL;
	}
	m_pstWordSpell = new SWordSpell[m_iTotalWord];
	if (m_pstWordSpell == NULL )
	{
		fclose(fp);
		return;
	}
	else
	{
		fseek(fp,0,SEEK_SET);
		fread(m_pstWordSpell,sizeof(SWordSpell),m_iTotalWord,fp);
		fclose(fp);
	}
}

//查询
int BinarySearch(unsigned short usValue,SWordSpell* pstWordSpell,int iHigh)
{
	int iMedian,iLow = 0;

 	iHigh--;
	do
	{
		iMedian = (iLow+iHigh)/2;
		if(usValue > pstWordSpell[iMedian].m_usWord)
			iLow = iMedian + 1;
		else
			iHigh = iMedian - 1;
	}while(! (usValue == pstWordSpell[iMedian].m_usWord || iLow > iHigh));
	return (usValue == pstWordSpell[iMedian].m_usWord) ? iMedian:-1;
}

//中文名称转成拼音简称
void WordToSpell(char* pcWord,char* pcSpell, int nSize)
{
	if(m_pstWordSpell == NULL)
		return;
	char			*pcString;
 	int				iResult;
	unsigned short	usInCode;
	SWordSpell		*pstWordSpell = m_pstWordSpell;

 	pcString = (char*)pcWord;
	pcSpell[0] = '\0';
	int				i = 0;
	int				j = 0;
	while( *pcString )
	{
		if(i >= nSize)
			break;
		if((*pcString & 0x80)&&(*(pcString+1) & 0x80))
		{
			usInCode = * (unsigned short*)pcString;
			pcString += 2;
			i += 2;
		}
		else
		{
			usInCode = (unsigned short)*pcString;
			if(usInCode>=97 && usInCode<=122)
				usInCode-=32;
 			pcString++;
			i++;
		}
		iResult = BinarySearch(usInCode,pstWordSpell,m_iTotalWord);
		if(iResult>=0 && iResult<m_iTotalWord)
		{
 			pcSpell[j++] = pstWordSpell[iResult].m_acSpell[0];
			if(j >= 4)
				return;
		}
	}
	pcSpell[3]=' ';
}

};
#endif // End define _REDEFINE_FILE_
