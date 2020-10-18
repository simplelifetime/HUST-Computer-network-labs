#include "Global.h"
#include "SRRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver() : expectSequenceNumberRcvd(0)
{
	seqsize = 8;
	winsizeN = 4;
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1; //忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
	{
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < 8; i++)
		tempPacketrcv[i] = 0;
}

SRRdtReceiver::~SRRdtReceiver()
{
}

bool SRRdtReceiver::isinwindow(int seqNum)
{
	if ((expectSequenceNumberRcvd + winsizeN) % seqsize > expectSequenceNumberRcvd)
		return (seqNum >= expectSequenceNumberRcvd) && (seqNum < (expectSequenceNumberRcvd + winsizeN) % seqsize);
	else if ((expectSequenceNumberRcvd + winsizeN) % seqsize < expectSequenceNumberRcvd)
		return (seqNum >= expectSequenceNumberRcvd) || (seqNum < (expectSequenceNumberRcvd + winsizeN) % seqsize);
	else
	{
		return false;
	}
}

void SRRdtReceiver::receive(const Packet &packet)
{
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum)
	{
		if (this->expectSequenceNumberRcvd == packet.seqnum)
		{
			pUtils->printPacket("接收方正确收到发送方的报文", packet);
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			tempPacketrcv[this->expectSequenceNumberRcvd] = 0;
			this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % seqsize;
			for (int i = this->expectSequenceNumberRcvd; i != (this->expectSequenceNumberRcvd + winsizeN - 1) % seqsize; i = (i + 1) % seqsize)
			{
				if (tempPacketrcv[i] == 1)
				{
					memcpy(msg.data, temppacket[i].payload, sizeof(temppacket[i].payload));
					pns->delivertoAppLayer(RECEIVER, msg);
					tempPacketrcv[i] = 0;
					this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % seqsize;
				}
				else
					break;
			}
		}
		else if(isinwindow(packet.seqnum))
		{
			pUtils->printPacket("接收方收到发送方的错序报文", packet);
			temppacket[packet.seqnum] = packet;
			tempPacketrcv[packet.seqnum] = 1;
		}
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt); //调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
	}
	else
	{
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
	}
}
