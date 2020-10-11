#include "StdAfx.h"
#include "DataProc.h"
#include "RFIDReader.h"

CDataProc::CDataProc(CRFIDReader *pReader)
{
	m_frameNum = 0x00;
	m_framePriority = 0;
	m_frameAnswer = 0;
	m_BusAddr = 0x00;
	m_pasLength = 0;
	mReader = pReader;
}

CDataProc::~CDataProc(void)
{
	
}


bool CDataProc::UnPackMsg(unsigned char* revData,int revLength)
{
	int &pasLength = m_pasLength; //ʹ������,���ı�ԭ�ȴ���
	CRFrame &revFrame = m_revFrame;
	unsigned char *pasBuffer = m_pasBuffer;

	if (revLength + pasLength == 0 )
		return false;

	int nCurPasBufferLen = 0;							//��ǰpasBuffer�е����ݳ���
	int revHead = 0;
	int nextHead = 0;

	//1.���µ�ǰҪ��������ݻ������������յ����ݷ���洢��������
	//----------------------------------------------------------------------------------------------------------------------------
	if (pasLength == 0)	//�����ڹ�ȥ���ݰ�
	{
		for ( revHead = 0; revHead < revLength; ++revHead)
		{
			if (revData[revHead] == FRAME_HEAD)			//��鵽�½��յ��������а���֡ͷ, ��ʾ���µ�֡���ݰ�������
				break;
		}

		if(revHead != revLength) //����ڴ˴��հ��������ҵ�֡ͷ
		{
			memcpy(pasBuffer, revData + revHead, revLength - revHead);//���½��յ�����������ʶ��Ϊ֡ͷ������ݿ�������ǰ�洢������			
			nCurPasBufferLen = revLength - revHead;			//���µ�ǰ�洢����������
		}
		else//�½��յ����ݰ�����֡ͷ����Ϊ���������ݣ�����������
		{
			memset((unsigned char*)&revFrame, 0 , sizeof(revFrame));
			return false;
		}
	}																
	else//��ǰbuffer����������(���ڹ�ȥ��)
	{
		if ( pasLength + revLength > REC_FRAME_MAXBUFF)	//��ǰ������������ʱ��������������
		{//����������ݣ����½���
			for ( revHead = 0; revHead < revLength; ++revHead)
			{
				if (revData[revHead] == FRAME_HEAD)
					break;
			}
			if(revHead != revLength)
			{
				memset(pasBuffer,0,REC_FRAME_MAXBUFF);
				pasLength = 0;
				memcpy(pasBuffer, revData + revHead, revLength - revHead);		
				nCurPasBufferLen = revLength - revHead;
			}
			/*memcpy(pasBuffer+pasLength, revData, REC_FRAME_MAXBUFF - pasLength);
			nCurPasBufferLen = REC_FRAME_MAXBUFF;*/
		}
		else//Ӧ��ִ���������,���½��յ����ݴ���pasBuffer
		{
			memcpy(pasBuffer+pasLength, revData, revLength);
			nCurPasBufferLen = pasLength + revLength;
		}
	}

	/*char dbgbuffer[64];
	sprintf(dbgbuffer, "�����յ����ݣ�%d�ֽڣ�\t��ǰ�洢�������й������ݣ�%d�ֽ�\n", revLength, nCurPasBufferLen);
	OutputDebugString((_bstr_t)dbgbuffer);*/

	//2.����pasBuffer���Ƿ���ڶ��֡, ���ҵ���2��֡ͷ��λ�ã�Ŀ�������û���ҵ��ڶ�֡֡ͷ��˵���洢�������п���ֻ��һ֡������һ֡����ȫ
	//----------------------------------------------------------------------------------------------------------------------------
	for (nextHead = 1; nextHead < nCurPasBufferLen; ++nextHead)			
	{
		if ( pasBuffer[nextHead] == FRAME_HEAD)
			break;
	}

	//3.�������� �������������ݴ�����սṹ�壬��ԭ����pasbuffer�����ݺ��½��յ���revData�е�����һ�����revFrame��
	//----------------------------------------------------------------------------------------------------------------------------
	//cxm BugFix: ����������澭��ת����ʵ���յ������ݳ���
	//��Ϊ����ȡ��:
	//0x56 0x56�ᱻת����0x55
	//0x56 0x57�ᱻת����0x56����ʵ�ʳ��ȹ�Ϊת���󳤶�
	//��� Ҫ��6B��ʱ,��13���ֽ�,����Щ�ֽ�ǡ����0x55,��(0x56,0x56),��ôҪ��26���ֽ�,������13���ֽ�
	//
	int nFixLen = 0;

	unsigned char *pRevData = (unsigned char*)&revFrame;//�趨ָ�� ָ����յĽṹ��
	int tempj = 0;
	//��:�յ�һ����������Ϊ23ʱ,nextHead=23,�˵�tempj��Χ[0-21],ע���ж�[tempj+1]�ķ���,����22���д��?

	// ת������ �������������ݴ�����սṹ��
	for (tempj = 0; tempj < nextHead - 1; ++tempj)
	{
		// 0x56 0x56ת����0x55
		if( pasBuffer[tempj] == FRAME_HEAD_REPLACE && pasBuffer[tempj + 1] == FRAME_HEAD_REPLACE ) 
		{
			++tempj;
			*pRevData ++ = FRAME_HEAD;
		}
		//0x56 0x57ת����0x56
		else if( pasBuffer[tempj] == FRAME_HEAD_REPLACE && pasBuffer[tempj + 1] == FRAME_FRAME_REPLACE ) 
		{
			++tempj;
			*pRevData ++ = FRAME_HEAD_REPLACE;
		}			
		else
			*pRevData ++ = pasBuffer[tempj];

		nFixLen ++;//cxm FixBug:ѭ����һ���������һ��ת��,revFrame ʵ�ʽ��յĳ���
	}
	/*
	�������յ�23���ֽڣ�pasBufferΪ[0-22]
	0 1 2 3 4 ... 21 22 23 24
	A:�����tempjΪ21ʱ,pasBuffer[tempj]Ϊ0x55,pasBuffer[tempj+1] = pasBuffer[22] Ϊ0x55,��ôpasBuffer[22]��ֵ�Ѿ���ת����
	�������� pRevData = pasBuffer[nextHead-1] = pasBuffer[22] ������,pRevData�� ++ 
	B:���tempjΪ21ʱ,pasBuffer[tempj]Ϊ0x45,pasBuffer[tempj+1]Ϊ0x08(������,��֮��������ת��0,ѭ����tempjֵδ��,����21,��һ��ѭ��ǰ
	ֵ��Ϊ22,����������,ѭ��δִ��,��ʱ��*pRevData = pasBuffer[nextHead-1] = pasBuffer[22]���� 
	*/
	//*pRevData = pasBuffer[nextHead - 1];//cmx Buf fix:����������
	if( tempj != nextHead ) //֤������治��0x55,0x55 ����0x55,0x56
	{
		*pRevData = pasBuffer[tempj];
		nFixLen++;
	}

	//4.�������ݻ��������������ݣ������ڶ�֡���,���Ƴ���1֡, �������������ǰ�ƶ�, ����ֱ��ˢ��pasBufferΪ0
	//----------------------------------------------------------------------------------------------------------------------------
	unsigned char moveBuffer[REC_FRAME_MAXBUFF];
	if (nextHead != nCurPasBufferLen)	
	{	
		pasLength = nCurPasBufferLen - nextHead;
		memcpy (moveBuffer, pasBuffer + nextHead , pasLength);
		memset(pasBuffer, 0, nCurPasBufferLen);
		memcpy(pasBuffer, moveBuffer,  pasLength);
	}
	else//ֻ��1֡�����
	{
		if (revFrame.bLength == 0 )	//ԭ����revFrame�еĳ�����bLength��û������
		{
			pasLength = nCurPasBufferLen;//���ݰ���û������,���´˴���ȡ��pasLenth�ĳ���
			return false;
		}

		if (revFrame.bLength != 0 && revFrame.bLength > nFixLen - FRAME_WITHOUTBLENGTH) //cxm FixBug
		{	
			pasLength = nCurPasBufferLen;//���ݰ���û������,���´˴���ȡ��pasLenth�ĳ���
			return false;
		}
		else//3.���ý�����1�����ݰ�
		{
			memset(&revFrame.bCommand + revFrame.bLength + 2, 0, DATA_BUFFER - revFrame.bLength - 2);	//�����Ч��Ϣ����֮���������Ϣ
			memset(pasBuffer, 0, nCurPasBufferLen);
			pasLength = 0;//���������0,��Ϊ ����ֻ��һ֡(��if����),�Ѿ������� 
		}
	}

	//5.�������ϴ���,��ǰ�õ���revFrame��һ�������İ���У���������֡������
	//----------------------------------------------------------------------------------------------------------------------------
	if( CRC16( (unsigned char*)(&revFrame), revFrame.bLength + 5) != 0 )	//�����ʱ�򷵻�Ϊ1����֡
	{
		memset((unsigned char*)&revFrame, 0 , sizeof(CRFrame));
		return false;
	}


	//printData("���յ�����",(unsigned char*)&revFrame,revFrame.bLength + 5);
	return true;
}



bool CDataProc::MsgProcess(unsigned char* revData,int revLength)
{
	if (mReader && revLength > 0)
	{
		//mReader->SendRFIDAck(revLength);
	}
	
	if( UnPackMsg(revData,revLength) )
	{
		if( mReader != NULL )
		{	
			mReader->SetMsgResult(m_revFrame);
			return true;
		}
	}

	return false;
}

unsigned short CDataProc::CountCRC16(unsigned char dataMsg, unsigned short crc)
{
	return (crc << 8) ^ CRCTable [ ( crc >> 8 ) ^ dataMsg ];
}

unsigned short CDataProc::CRC16(unsigned char* pMsg, int msgLength)
{
	unsigned short CRCValue = 0xffff;
	unsigned char *pTemp = pMsg;
	for (int tempi = 1 ;tempi < msgLength+1; ++tempi) //cxm fixed
	{
		CRCValue = CountCRC16( *(pTemp + tempi), CRCValue);
	}
	return CRCValue;
}

int CDataProc::PackMsg(unsigned char pSendCommand, 
						unsigned char *pSendData, 
						int dataLength,
						unsigned char *pForSend,
						int *pSendLen)
{
	++m_frameNum ;//֡���к����� ��ʼΪ0,ÿ��һ��+1
	if (m_frameNum >= 16)
	{
		m_frameNum = 1;// 16ʱ��Ϊ0
	}

	CRFrame* sendFrame = (CRFrame*)pForSend;
	//���ݴ��

	sendFrame->bFrame = FRAME_HEAD;					//֡ͷ ����Ϊ 55H
	sendFrame->bAddr = m_BusAddr;//cxm fixed
	sendFrame->bAddrInfo = m_framePriority + m_frameAnswer + m_frameNum;	//�����ֽ� ����:���ȼ�, �ظ�֡, �ظ����к�
	sendFrame->bLength = dataLength + 1;				//������Ϊ ���ݳ��� + ָ��(1�ֽ�)
	sendFrame->bCommand = pSendCommand;				//����ָ��
	memcpy(sendFrame->bData, pSendData, dataLength);

	unsigned short CRCValue = 0;					//CRCУ���㷨, ������֡β2�ֽ�
	CRCValue = CRC16( (unsigned char*)sendFrame, dataLength + 4);//֡ͷ��Ҫ,������Ϊ3
	sendFrame->bData[dataLength] = (char)((CRCValue >> 8) & 0xff);
	sendFrame->bData[dataLength + 1] = (char)(CRCValue & 0xff);

	//֡����ת��
	unsigned char pForSendBuffer[SENDFRAME_MAXBUF];
	unsigned char *pBuffer = pForSendBuffer + 1;			//������֡ͷ
	unsigned char *pFrameData = pForSend;
	int forSendLength = dataLength + 7;						//֡ͷΪ 1�ֽ�, ������Ϊ 3�ֽ�, ָ��Ϊ 1�ֽ�, CRC16Ϊ 2�ֽ�  7 = 1+3+1+2;

	pForSendBuffer[0] = FRAME_HEAD;
	for (int tempi = 1 ; tempi < dataLength + 7; ++tempi)
	{
		if ( pFrameData[tempi] == FRAME_HEAD)				//֡�����г��� 0x55 �����, ��0x56 + 0x56 2���ֽڴ���
		{
			*pBuffer++ = FRAME_HEAD_REPLACE;
			*pBuffer++ = FRAME_HEAD_REPLACE;
			++forSendLength;
		}
		else
		{
			if ( pFrameData[tempi] == FRAME_HEAD_REPLACE)	//���� 0x56 �����, ��0x56 + 0x57 2�ֽڴ���
			{
				*pBuffer++ = FRAME_HEAD_REPLACE;
				*pBuffer++ = FRAME_FRAME_REPLACE;
				++forSendLength;
			}
			else
			{
				*pBuffer++ = pFrameData[tempi] ;
			}
		}
	}

	memcpy(pForSend, pForSendBuffer, forSendLength);
	*pSendLen = forSendLength;

	return m_frameNum;
}

bool CDataProc::SendMsg(unsigned char pSendCommand,
						unsigned char *pSendData,
						int dataLength)
{
	if( !mReader->SendData(pSendData,dataLength) )
	{
		return false;
	}
	return true;
}