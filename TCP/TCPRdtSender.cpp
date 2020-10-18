#include "Global.h"
#include "TCPRdtSender.h"

TCPRdtSender::TCPRdtSender() : expectSequenceNumberSend(0), waitingState(false)
{
	base = 0;
	nextseqnum = 0;
	winsizeN = 4;
	seqsize = 8;
	Packet temp;
	redundack = 0;
}

TCPRdtSender::~TCPRdtSender()
{
}

bool TCPRdtSender::getWaitingState()
{
	return (this->base + winsizeN) % seqsize == nextseqnum % seqsize;
}

bool TCPRdtSender::isinwindow(int seqNum)
{
	if ((this->base + this->winsizeN) % this->seqsize > this->base)
		return (seqNum >= this->base) && (seqNum < (this->base + this->winsizeN) % this->seqsize);
	else if ((this->base + this->winsizeN) % this->seqsize < this->base)
		return (seqNum >= this->base) || (seqNum < (this->base + this->winsizeN) % this->seqsize);
	else
	{
		return false;
	}
}

bool TCPRdtSender::send(const Message &message)
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

void TCPRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum && isinwindow(ackPkt.acknum))
	{
		this->base = (ackPkt.acknum + 1) % this->seqsize;
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		printSlideWindow();
		pns->stopTimer(SENDER, this->base);
		if (this->base != this->nextseqnum)
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
		}
		this->redundack = 0;
	}
	else if (!isinwindow(ackPkt.acknum))
	{
		this->redundack = this->redundack + 1;
		if(this->redundack == 3)
		{
			pns->stopTimer(SENDER, this->base);
			pns->sendToNetworkLayer(RECEIVER, this->Allpacket[this->base]);
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
			pUtils->printPacket("���յ�3������ack���ش�����", this->Allpacket[this->base]);
			this->redundack = 0;
		}
	}
	else
	{
		pUtils->printPacket("�յ��İ���", ackPkt);
	}
}

void TCPRdtSender::timeoutHandler(int seqNum)
{
	if (this->base == this->nextseqnum) //����Ϊ�յ��������
		return;
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
		pns->sendToNetworkLayer(RECEIVER, this->Allpacket[this->base]);
		pUtils->printPacket("�ط���ʱ����", this->Allpacket[this->base]);
	}
}

void TCPRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "(";

		if (i == this->nextseqnum)
			std::cout << "[" << i << "]";
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