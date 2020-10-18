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

void TCPRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum && isinwindow(ackPkt.acknum))
	{
		this->base = (ackPkt.acknum + 1) % this->seqsize;
		pUtils->printPacket("发送方正确收到确认", ackPkt);
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
			pUtils->printPacket("已收到3个冗余ack，重传分组", this->Allpacket[this->base]);
			this->redundack = 0;
		}
	}
	else
	{
		pUtils->printPacket("收到的包损坏", ackPkt);
	}
}

void TCPRdtSender::timeoutHandler(int seqNum)
{
	if (this->base == this->nextseqnum) //窗口为空的特殊情况
		return;
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
		pns->sendToNetworkLayer(RECEIVER, this->Allpacket[this->base]);
		pUtils->printPacket("重发超时分组", this->Allpacket[this->base]);
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