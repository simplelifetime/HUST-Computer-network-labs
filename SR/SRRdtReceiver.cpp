#include "Global.h"
#include "SRRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver() : expectSequenceNumberRcvd(0)
{
	seqsize = 8;
	winsizeN = 4;
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1; //���Ը��ֶ�
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
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum)
	{
		if (this->expectSequenceNumberRcvd == packet.seqnum)
		{
			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
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
			pUtils->printPacket("���շ��յ����ͷ��Ĵ�����", packet);
			temppacket[packet.seqnum] = packet;
			tempPacketrcv[packet.seqnum] = 1;
		}
		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt); //����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
	}
	else
	{
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
	}
}
