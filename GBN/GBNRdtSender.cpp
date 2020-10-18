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
	{ //发送方处于等待确认状态
		std::cout << "窗口已满" << std::endl;
		return false;
	}
	this->Allpacket[nextseqnum].acknum = -1; //忽略该字段
	this->Allpacket[nextseqnum].seqnum = nextseqnum;
	this->Allpacket[nextseqnum].checksum = 0;
	memcpy(this->Allpacket[nextseqnum].payload, message.data, sizeof(message.data));
	this->Allpacket[nextseqnum].checksum = pUtils->calculateCheckSum(this->Allpacket[nextseqnum]);
	pUtils->printPacket("发送方发送报文", this->Allpacket[nextseqnum]);
	printSlideWindow();
	if (base == nextseqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base); //启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->Allpacket[nextseqnum]);   //调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方

	this->nextseqnum = (this->nextseqnum + 1) % this->seqsize;
	printSlideWindow();
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum)
	{
		this->base = (ackPkt.acknum + 1) % this->seqsize;
		pUtils->printPacket("发送方正确收到确认", ackPkt);
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
		pUtils->printPacket("收到的包损坏", ackPkt);
	}
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
	if (this->base == this->nextseqnum) //窗口为空的特殊情况
		return;
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
		for (int i = this->base; i != this->nextseqnum; i = (i + 1) % this->seqsize)
		{
			pns->sendToNetworkLayer(RECEIVER, this->Allpacket[i]);
			pUtils->printPacket("重发超时分组", this->Allpacket[i]);
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