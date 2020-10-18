#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include<vector>
class SRRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// ��һ��������� 
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	int base;				//�����
	int nextseqnum;			//��һ�����
	int winsizeN;			//���ڳ���
	int seqsize;			//���г���
	int rcvstatus[8];
	// Packet packetWaitingAck[winsizeN];		//�ѷ��Ͳ��ȴ�Ack�����ݰ�
	Packet Allpacket[8];

public:

	bool getWaitingState();
	bool send(const Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����
	void printSlideWindow();
	void INIT();
	bool isinwindow(int seqNum);

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif