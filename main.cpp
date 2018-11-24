#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DrvFR_Conn.h"

using namespace DriverFR;

int errorCode = 0;
static DrvFR* drv;
//---------------------------------------------------------------------
int OpenPaymentDocument()
{
	return 0;
}
//---------------------------------------------------------------------
int AddWareString(DrvFR* drvFr, TCheckType checkType, double quantity, double price, char* stringForPrinting, int taxType, int paymentItemSign, int paymentTypeSign)
{
	drvFr->Price = price;
	drvFr->Quantity = quantity;
	drvFr->Summ1 = drvFr->Price * drvFr->Quantity;
	drvFr->Summ1Enabled = 0x00;
	drvFr->CheckType = (int)checkType;
	drvFr->TaxType = taxType;
	drvFr->TaxValue = 0;
	drvFr->TaxValueEnabled = 0x00;
	drvFr->DiscountOnCheck = 0x00;
	drvFr->PaymentItemSign = paymentItemSign;
	drvFr->PaymentTypeSign = paymentTypeSign;
	strncpy(drvFr->StringForPrinting, stringForPrinting, 40);

	return drvFr->FNOperation();
}
//---------------------------------------------------------------------
int ClosePaymentDocument(DrvFR* drvFr, double Summ1 = 0, double Summ2 = 0, double Summ3 = 0, double Summ4 = 0, double Summ5 = 0, double Summ6 = 0, double Summ7 = 0, double Summ8 = 0, double Summ9 = 0, double Summ10 = 0, double Summ11 = 0, double Summ12 = 0, double Summ13 = 0, double Summ14 = 0, double Summ15 = 0, double Summ16 = 0)
{
	drvFr->Summ1 = Summ1;
	drvFr->Summ2 = Summ2;
	drvFr->Summ3 = Summ3;
	drvFr->Summ4 = Summ4;
	drvFr->Summ5 = Summ5;
	drvFr->Summ6 = Summ6;
	drvFr->Summ7 = Summ7;
	drvFr->Summ8 = Summ8;
	drvFr->Summ9 = Summ9;
	drvFr->Summ10 = Summ10;
	drvFr->Summ11 = Summ11;
	drvFr->Summ12 = Summ12;
	drvFr->Summ13 = Summ13;
	drvFr->Summ14 = Summ14;
	drvFr->Summ15 = Summ15;
	drvFr->Summ16 = Summ16;

	drvFr->TaxValue1 = 0;
	drvFr->TaxValue2 = 0;
	drvFr->TaxValue3 = 0;
	drvFr->TaxValue4 = 0;
	drvFr->TaxValue5 = 0;
	drvFr->TaxValue6 = 0;

	drvFr->RoundingSumm = 0;
	drvFr->TaxType = (int)TDocumentTaxType::Tax6;

	strncpy(drvFr->StringForPrinting, "", 40);

	return drvFr->FNCloseCheckEx();
}
//---------------------------------------------------------------------
void CheckDevice(DrvFR* drv)
{
	drv->GetECRStatus();
	int dataReceiveCount = 0;
	while (!(((drv->ECRMode == (int)TECRMode::OpenWorkspace) || (drv->ECRMode == (int)TECRMode::WorkMode))))
	{
		if (drv->ECRMode == (int)TECRMode::DataInProgress)
		{
			if (errorCode != 0x10)
				printf("���������� ������ ������. ������� ...\n");
			errorCode = 0x10;
			if (dataReceiveCount++ > 5)
			{
				printf("��������� �������� ������ �� ����������!\n");
				drv->Password = 0;
				drv->InterruptDataStream();
				drv->PrintErrorDescription();
				dataReceiveCount = 0;
				drv->Password = 30;
			}
		}
		else if (drv->ECRMode == (int)TECRMode::OpenWorkspace24h)
		{
			if (errorCode != 0x11)
				printf("�������� �����, 24 ���� ���������. ������� �����.\n");
			errorCode = 0x11;
			drv->FNCloseSession();
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::Blocked_WrongPasswordTaxInspector)
		{
			if (errorCode != 0x12)
				printf("������ ���������� ���������� �� ���������. ��������� ������ ��������������.\n");
			errorCode = 0x12;
			drv->Password = 30;
		}
		else if ((drv->ECRMode == (int)TECRMode::WorkWithFNDocument) || (drv->ECRMode == (int)TECRMode::WorkFNDocumentDone))
		{
			if (errorCode != 0x13)
				printf("������ � ���������� ���������� ....\n");
			errorCode = 0x13;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::PrintingEKLZReport)
		{
			if (errorCode != 0x14)
				printf("������ ������ ���� ....\n");
			errorCode = 0x14;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::PrintingFNReport)
		{
			if (errorCode != 0x15)
				printf("������ ����������� ������ ....\n");
			errorCode = 0x15;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::DigitPointChangeRight)
		{
			if (errorCode != 0x16)
				printf("���������� ��������� ��������� ���������� �����. \n");
			errorCode = 0x16;
			drv->PointPosition = true;
			drv->SetPointPosition();
		}
		else if (drv->ECRMode == (int)TECRMode::WaitingDateConfirm)
		{
			if (errorCode != 0x17)
				printf("�������� ������������� ����� ����. \n");
			errorCode = 0x17;
			int day = 24;
			int month = 10;
			int year = 2018;
			drv->Date.tm_mday = day;
			drv->Date.tm_mon = month - 1;
			drv->Date.tm_year = year - 1900;
			mktime(&drv->Date);
			time_t rawtime;
			struct tm * loctime = localtime(&rawtime);
			time(&rawtime);
			memcpy(&drv->Date, loctime, sizeof(drv->Date));
			drv->ConfirmDate();
		}
		else if (drv->ECRMode == (int)TECRMode::CloseWorkspace)
		{
			if (errorCode != 0x18)
				printf("����� �������. ������� �����.\n");
			errorCode = 0x18;
			drv->FNOpenSession();
		}
		else if (drv->ECRMode == (int)TECRMode::PrintFNDocument)
		{
			if (errorCode != 0x19)
				printf("������ ���� ....\n");
			errorCode = 0x19;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::OpenedDocument)
		{
			break;
		}
		else
			printf("�� ���������� ����� ����������. ��������� \"������� �����\" ��� \"����� �������\" Debug: (%d) %s\n", drv->ECRMode, drv->ECRModeDescription);
		usleep(1000000);
		drv->GetECRStatus();
	}

	while (drv->ECRAdvancedMode != (int)TECRAdvancedMode::DeviceOK)
	{
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::NoPapper)
		{
			if (errorCode != 0x1A)
				printf("����������� ������. ������� ...\n");
			errorCode = 0x1A;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::PrintingFNDocument)
		{
			if (errorCode != 0x1B)
				printf("���������� ���������� ��������. ������� ���������� ...\n");
			errorCode = 0x1B;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::PrintingFNDocument)
		{
			if (errorCode != 0x1C)
				printf("���������� ��������. ������� ���������� ...\n");
			errorCode = 0x1C;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::NoPapperError)
		{
			if (errorCode != 0x1D)
				printf("��������! ����������� ������!\n");
			errorCode = 0x1D;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::WaitPrintContinue)
		{
			if (errorCode != 0x1E)
				printf("���������� ������ ...\n");
			errorCode = 0x1E;
			drv->ContinuePrinting();
		}
		usleep(1000000);
		drv->GetECRStatus();
	}
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

int main(void)
{
	DrvFR* drv = new DrvFR(30, 0, 0, 1000, 0, TConnectionType::ctSocket, 7778, "192.168.254.111", true);
	if (drv->Connect() > 0)
	{
		/*
		strcpy(drv->StringForPrinting, "123456");
		drv->UseReceiptRibbon = 1;
		drv->PrintString();
		strcpy(drv->StringForPrinting, "Hello world!");
		drv->UseReceiptRibbon = 1;
		drv->FontType = 1;
		drv->PrintString();
		drv->CutType = 1;
		drv->CutCheck();

		printf("-----------------**------------------\n");
		fflush(stdout);

		CheckDevice(drv);
		AddWareString(drv, TCheckType::Sale, 1, 30, "Hello world !!! Hi piple!!!", TTaxType::NoNds, TPaymentItemSign::Service, TPaymentTypeSign::Payment);
		CheckDevice(drv);
		ClosePaymentDocument(drv, 30);
		*/
		struct tm* now;
		time_t rawtime;
		bool error = true;
		int counter = 70;
		while (1)
		{
			time(&rawtime);
			now = localtime(&rawtime);
			if (drv->CheckConnection() != 1)
			{
				if (!error)
					printf ("[%s] Connection error!\n", asctime(now));
				error = true;
				fflush(stdout);
				drv->Disconnect();
				continue;
			}
			if (error)
				printf ("[%s] Connection OK!\nDevice: %s\n", asctime(now), drv->UDescription);
			error = false;
			CheckDevice(drv);
			if (counter++ > 60)
			{
				counter = 0;
				AddWareString(drv, TCheckType::Sale, 1, 5, "�������� �����", TTaxType::NoNds, TPaymentItemSign::Service, TPaymentTypeSign::Payment);
				CheckDevice(drv);
				ClosePaymentDocument(drv, 30);
			}
			sleep(10);
		}
	}
	drv->Disconnect();

	return 1;
}