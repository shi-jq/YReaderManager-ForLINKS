#ifndef READCARD_FOR_6C_THREAD_H
#define READCARD_FOR_6C_THREAD_H
#include <QThread>
#include <map>
#include "Lock.h"
#include "readerdllbase.h"

class CLock;
class CMyEvent;
class CardFor6CInfo;
class ReadCardFor6CThread: public QThread
{
	Q_OBJECT
public:
	enum
	{
		READ_FOR_MAKETAGUPLOAD = 0,//������ǩ
		READ_FOR_YTAGREAD = 1,//������ǩ
	};
	ReadCardFor6CThread(ReaderDllBase* pReaderDllBase,QObject *parent);
	~ReadCardFor6CThread(void);
	
	void SetReader(ReaderDllBase* pReaderDllBase);
	bool StartRead(int nReadType);
	bool StopRead();
	void clearCard();
	void GetCardList(std::map<QString,CardFor6CInfo*>& cardList);
	
	void run();
private:
	bool AnalysisCard(
		unsigned char nTagType,
		unsigned int pId,
		unsigned char pBit, 
		int nParam1,
		int nParam2,
		int nParam3,
		int nAntenna);

	QString GetTagStateStr(unsigned char nTagType,int nTagState);
	QString GetNormalTagState(int nTagState); //��ͨ��ǩ
	QString GetTemperatureTagState(int nTagState);//�¶ȱ�ǩ
	QString GetInvigoratTagState(int nTagState);//������ǩ
	QString GetCurrentTagState(int nTagState);//������ǩ

	QString GetTagDescribe(int nTagType, int nParam1, int nParam2,int nParam3);
	QString GetNarmalTagDescribe(int nParam1, int nParam2,int nParam3);//��ͨ��ǩ
	QString GetTemperatureTagDescribe(int nParam1, int nParam2,int nParam3);//�¶ȱ�ǩ
	QString GetInvigoratTagDescribe(int nParam1, int nParam2,int nParam3);//������ǩ
	QString GetCurrentTagDescribe(int nParam1, int nParam2,int nParam3);//������ǩ

	QString GetTagRSSI(int nTagType,int nParam3);
	
	CardFor6CInfo* GetCard(const QString& szCard);
	CardFor6CInfo* FindCard(const QString& szCard);
	CardFor6CInfo* CreateCard(const QString& szCard);
private:
	ReaderDllBase* mReaderDllBase;
	bool mIsStop;			
	CMyEvent mExitEvent;
	int mReadingType;

	CLock mCardLock;
	std::map<QString,CardFor6CInfo*>  mCardList;
};

class CardFor6CInfo
{
public:	
	CardFor6CInfo(){
		m_nAntenna1Count = 0;
		m_nAntenna2Count = 0;
		m_nAntenna3Count = 0;
		m_nAntenna4Count = 0;
	}
	~CardFor6CInfo(){}
public:
	QString m_szCardID;
	QString m_szTagState;
	QString m_szRssi;
	QString m_otherInfo;
	QString m_szCurReadTime;	
	unsigned int m_nAntenna1Count;
	unsigned int m_nAntenna2Count;
	unsigned int m_nAntenna3Count;
	unsigned int m_nAntenna4Count;
};

#endif // READCARD_FOR_6C_THREAD_H