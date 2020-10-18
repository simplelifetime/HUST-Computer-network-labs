// StopWait.cpp : 定义控制台应用程序的入口点。
//


#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "SRRdtSender.h"
#include "SRRdtReceiver.h"


int main(int argc, char* argv[])
{
	RdtSender *ps = new SRRdtSender();
	RdtReceiver * pr = new SRRdtReceiver();
	pns->setRunMode(0);  //VERBOS模式
//	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("D:\\StopWait\\input.txt");
	pns->setOutputFile("D:\\StopWait\\output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

