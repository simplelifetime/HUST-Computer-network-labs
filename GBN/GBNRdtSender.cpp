#include "Global.h"
#include "GBNRdtSender.h"

GBNRdtSender::GBNRdtSender() : expectSequenceNumberSend(0), waitingState(false)
{
	base = 0;
	nextseqnum = 0;
	winsizeN = 4;
	seqsize = 8;
	Packet temp;
}

GBNRdtSender::~GBNRdtSender()
{
}

bool GBNRdtSender::getWaitingState()
{
	return (base + winsizeN) % seqsize == nextseqnum % seqsize;
}

bool GBNRdtSender::send(const Message &message)
{
	this->waitingState = getWaitingState();
	if (this->waitingState)
	{ //���ͷ����ڵȴ�ȷ��״̬
		std::cout << "��������" << std::endl;
		return false;
	}
	this->Allpacket[nextseqnum].acknum = -1; //���Ը��ֶ�
	this->Allpacket[nextseqnum].seqnum = nextseqnum;
	this->Allpacket[nextseqnum].checksum = 0;
	memcpy(this->Allpacket[nextseqnum].payload, message.data, sizeof(message.data));
	this->Allpacket[nextseqnum].checksum = pUtils->calculateCheckSum(this->Allpacket[nextseqnum]);
	pUtils->printPacket("���ͷ����ͱ���", this->Allpacket[nextseqnum]);
	printSlideWindow();
	if (base == nextseqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base); //�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->Allpacket[nextseqnum]);   //����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�

	this->nextseqnum = (this->nextseqnum + 1) % this->seqsize;
	printSlideWindow();
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum)
	{
		this->base = (ackPkt.acknum + 1) % this->seqsize;
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		printSlideWindow();
		if (base == nextseqnum)
			pns->stopTimer(SENDER, this->base);
		else
		{
			pns->stopTimer(SENDER, this->base);
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
		}
	}
	else
	{
		pUtils->printPacket("�յ��İ���", ackPkt);
	}
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
	if (this->base == this->nextseqnum) //����Ϊ�յ��������
		return;
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
		for (int i = this->base; i != this->nextseqnum; i = (i + 1) % this->seqsize)
		{
			pns->sendToNetworkLayer(RECEIVER, this->Allpacket[i]);
			pUtils->printPacket("�ط���ʱ����", this->Allpacket[i]);
		}
	}
}

void GBNRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "(";
		if (i == this->nextseqnum)
			std::cout << "[" << i <<"]";
		else
		{
			std::cout << i;
		}
		if (i == (base + this->winsizeN - 1) % seqsize)
			std::cout << ")";
		std::cout << "  ";
	}
	std::cout << std::endl;
}