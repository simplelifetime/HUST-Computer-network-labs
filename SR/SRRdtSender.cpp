#include "Global.h"
#include "SRRdtSender.h"

void SRRdtSender::INIT()
{
	for (int i = 0; i < 8; i++)
		rcvstatus[i] = 0;
}

SRRdtSender::SRRdtSender() : expectSequenceNumberSend(0), waitingState(false)
{
	base = 0;
	nextseqnum = 0;
	winsizeN = 4;
	seqsize = 8;
	Packet temp;
	INIT();
}

SRRdtSender::~SRRdtSender()
{
}

bool SRRdtSender::isinwindow(int seqNum)
{
	if ((base + winsizeN) % seqsize > base)
		return (seqNum >= base) && (seqNum < (base + winsizeN) % seqsize);
	else if ((base + winsizeN) % seqsize < base)
		return (seqNum >= base) || (seqNum < (base + winsizeN) % seqsize);
	else
	{
		return false;
	}
}
bool SRRdtSender::getWaitingState()
{
	return (base + winsizeN) % seqsize == nextseqnum;
}

bool SRRdtSender::send(const Message &message)
{
	this->waitingState = getWaitingState();
	if (this->waitingState)
	{ //发送方处于等待确认状态
		return false;
	}
	this->Allpacket[nextseqnum].acknum = -1; //忽略该字段
	this->Allpacket[nextseqnum].seqnum = nextseqnum;
	this->Allpacket[nextseqnum].checksum = 0;
	memcpy(this->Allpacket[nextseqnum].payload, message.data, sizeof(message.data));
	this->Allpacket[nextseqnum].checksum = pUtils->calculateCheckSum(this->Allpacket[nextseqnum]);
	pUtils->printPacket("发送方发送报文", this->Allpacket[nextseqnum]);
	printSlideWindow();
	pns->startTimer(SENDER, Configuration::TIME_OUT, this->nextseqnum);
	pns->sendToNetworkLayer(RECEIVER, this->Allpacket[nextseqnum]); //调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方

	this->nextseqnum = (this->nextseqnum + 1) % this->seqsize;
	printSlideWindow();
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum)
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (this->base == ackPkt.acknum)
		{
			this->rcvstatus[this->base] = 0;
			this->base = (this->base + 1) % seqsize;
			int tmp = this->base;
			for (int i = tmp; i != (tmp + this->winsizeN - 1) % seqsize; i = (i + 1) % seqsize)
			{
				if (this->rcvstatus[i] == 1)
				{
					this->rcvstatus[i] = 0;
					this->base = (this->base + 1) % seqsize;
				}
				else
				{
					break;
				}
			}
		}
		else if(isinwindow(ackPkt.acknum))
		{
			rcvstatus[ackPkt.acknum] = 1;
		}
		else
		{
			pUtils->printPacket("收到的包失序", ackPkt);
		}
		
	}
	else
	{
		pUtils->printPacket("收到的包损坏", ackPkt);
	}
	printSlideWindow();
	return;
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	pns->sendToNetworkLayer(RECEIVER, this->Allpacket[seqNum]);
	pUtils->printPacket("重发超时分组", this->Allpacket[seqNum]);
}

void SRRdtSender::printSlideWindow()
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