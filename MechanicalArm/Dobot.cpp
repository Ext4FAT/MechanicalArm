#include "Dobot.hpp"
#include <thread>
#include <sstream>
#include <time.h>
#include <sys/timeb.h>
#pragma comment(lib, "user32")

VOID CALLBACK showPosition(	HWND hwnd, // handle of window for timer messages
							UINT uMsg, // WM_TIMER message
							UINT idEvent, // timer identifier
							DWORD dwTime // current system time	
							){
		Pose p;
		GetPose(&p);
		Point3D p3d = { p.x, p.y, p.z };
		MESSAGE_COUT("Current", p3d);
	}

VOID CALLBACK Task(HWND hwnd, // handle of window for timer messages
	UINT uMsg, // WM_TIMER message
	UINT idEvent, // timer identifier
	DWORD dwTime // current system time
	){
	PeriodicTask();
}



string Dobot::getSocketData()
{
	const int BUF_SIZE = 64;
	SOCKET  sClient;    //客户端套接字  
	char    buf[BUF_SIZE]; //接收数据缓冲区  
	int     retVal;     //返回值  
 
	sockaddr_in addrClient;
	int addrClientlen = sizeof(addrClient);
	sClient = accept(Server_, (sockaddr FAR*)&addrClient, &addrClientlen);
	if (INVALID_SOCKET == sClient)
	{
		MESSAGE_ERROR("accept failed!");
		_closeSocket();
		return "F_ACCEPT";
	}
	
	while (true){
		ZeroMemory(buf, BUF_SIZE);
		retVal = recv(sClient, buf, BUF_SIZE, 0);
		if (SOCKET_ERROR == retVal)
		{
			MESSAGE_ERROR("recv failed!");
			closesocket(sClient);    
			_closeSocket();
			return ("F_RECV");
		}
		if (buf[0] == 0)
			break;
		return string(buf);
	}
	return "";
}

int Dobot::_connectSocket()
{
	WSADATA     wsd;   
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		MESSAGE_ERROR("WSAStartup failed!");
		return -1;
	}
	Server_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == Server_)
	{
		MESSAGE_ERROR("socket failed!");
		WSACleanup();  
		return -2;
	}
	
	SOCKADDR_IN	addrServ;  
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(11229);
	addrServ.sin_addr.s_addr = INADDR_ANY;

	int	retVal;
	retVal = bind(Server_, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		MESSAGE_ERROR("bind failed!");
		_closeSocket();
		return -3;
	}

	retVal = listen(Server_, 1);
	if (SOCKET_ERROR == retVal)
	{
		MESSAGE_ERROR("listen failed!");
		_closeSocket();
		return -4;
	}
	return 0;
}

void Dobot::_closeSocket()
{
	closesocket(Server_); 
	WSACleanup();   
}

void Dobot::connect()
{
	if (!ConnectStatus_) {
		if (ConnectDobot(0, 115200) != DobotConnect_NoError) {
			MESSAGE_COUT("ERROR", "Cannot connect Dobot!");
			return;
		}
		ConnectStatus_ = true;
		SetTimer(0, 1, 10, Task);
		//SetTimer(0, 2, 250, showPosition);
		init();
		MESSAGE_INFO("Connected!");
	}
	else {
		ConnectStatus_ = false;
		KillTimer(0, 1);
		//KillTimer(0, 2);
		DisconnectDobot();
		MESSAGE_INFO("Disconnect!");
	}
}

void Dobot::init()
{
	// Command timeout
	SetCmdTimeout(3000);
	// Clear old commands and set the queued command running
	SetQueuedCmdClear();
	SetQueuedCmdStartExec();
	
	//// Device SN
	//char deviceSN[64];
	//GetDeviceSN(deviceSN, sizeof(deviceSN));
	//MESSAGE_INFO("Device SN :" << deviceSN);
	//
	//// Device Name
	//char deviceName[64];
	//GetDeviceName(deviceName, sizeof(deviceName));
	//MESSAGE_INFO("Device Name:" << deviceName);
	//
	//// Device version information
	//uint8_t majorVersion, minorVersion, revision;
	//GetDeviceVersion(&majorVersion, &minorVersion, &revision);
	//MESSAGE_INFO("Device information:" << majorVersion << "." << minorVersion << "." << revision);

	// Set the end effector parameters
	EndEffectorParams endEffectorParams;
	memset(&endEffectorParams, 0, sizeof(EndEffectorParams));
	endEffectorParams.xBias = 59.7f;
	SetEndEffectorParams(&endEffectorParams, false, NULL);
	
	// 1. Set the JOG parameters
	JOGJointParams jogJointParams;
	for (uint32_t i = 0; i < 4; i++) {
		jogJointParams.velocity[i] = 200;
		jogJointParams.acceleration[i] = 200;
	}
	SetJOGJointParams(&jogJointParams, false, NULL);
	
	JOGCoordinateParams jogCoordinateParams;
	for (uint32_t i = 0; i < 4; i++) {
		jogCoordinateParams.velocity[i] = 200;
		jogCoordinateParams.acceleration[i] = 200;
	}
	SetJOGCoordinateParams(&jogCoordinateParams, false, NULL);
	
	JOGCommonParams jogCommonParams;
	jogCommonParams.velocityRatio = 50;
	jogCommonParams.accelerationRatio = 50;
	SetJOGCommonParams(&jogCommonParams, false, NULL);
	
	// 2. Set the PTP parameters
	PTPJointParams ptpJointParams;
	for (uint32_t i = 0; i < 4; i++) {
		ptpJointParams.velocity[i] = 200;
		ptpJointParams.acceleration[i] = 200;
	}
	SetPTPJointParams(&ptpJointParams, false, NULL);
	
	PTPCoordinateParams ptpCoordinateParams;
	ptpCoordinateParams.xyzVelocity = 200;
	ptpCoordinateParams.xyzAcceleration = 200;
	ptpCoordinateParams.rVelocity = 200;
	ptpCoordinateParams.rAcceleration = 200;
	SetPTPCoordinateParams(&ptpCoordinateParams, false, NULL);
	
	PTPJumpParams ptpJumpParams;
	ptpJumpParams.jumpHeight = 10;
	ptpJumpParams.zLimit = 150;
	SetPTPJumpParams(&ptpJumpParams, false, NULL);
}

void Dobot::gotoPoint(Point3D pt, bool waitEnd)
{
	_gotoPoint(pt.x, pt.y, pt.z, 0.0f, waitEnd);
}

void Dobot::_gotoPoint(float x, float y, float z, float r, bool waitEnd)
{
	PTPCmd ptpCmd = { JOGMode_, x, y, z, r };
	// Send the command. If failed, just resend the command
	uint64_t queuedCmdIndex;
	do {
		int result = SetPTPCmd(&ptpCmd, true, &queuedCmdIndex);
		if (result == DobotCommunicate_NoError) {
			break;
		}
	} while (1);
	// Check whether the command is finished
	do {
		if (waitEnd == false) {
			break;
		}
		uint64_t currentIndex;
		int result = GetQueuedCmdCurrentIndex(&currentIndex);
		if (result == DobotCommunicate_NoError &&
			currentIndex >= queuedCmdIndex) {
			break;
		}
	} while (1);
}

void Dobot::home()
{
	Point3D cur = getPosition();
	cur.z = 100.0f;
	gotoPoint(cur, true);
	_home();
	cur = getPosition();
	Point3D bias = { 0.0f, 0.0f, 100.0f };
	gotoPoint(cur + bias, true);
}

void Dobot::_home()
{
	// Send the command. If failed, just resend the command
	uint64_t queuedCmdIndex;
	do {
		HOMECmd homeCmd;
		int result = SetHOMECmd(&homeCmd, true, &queuedCmdIndex);
		if (result == DobotCommunicate_NoError) {
			break;
		}
	} while (1);
	// Check whether the command is finished
	do {
		bool waitEnd = true;
		if (waitEnd == false) {
			break;
		}
		uint64_t currentIndex;
		int result = GetQueuedCmdCurrentIndex(&currentIndex);
		if (result == DobotCommunicate_NoError &&
			currentIndex >= queuedCmdIndex) {
			break;
		}
	} while (1);
}

void Dobot::_moveToOrigin()
{
	gotoPoint(Origin_, true);
}

void Dobot::DOWN()
{
	_jog(_J3N_);
}

void Dobot::UP()
{
	_jog(_J3P_);
}

void Dobot::STOP()
{
	_jog(_STOP_);
}

void Dobot::_jog(int i)
{
	JOGCmd jogCmd;
	jogCmd.isJoint = _JOGMODE_;
	jogCmd.cmd = i;
	//SetJOGCmd(&jogCmd, true, NULL);
	SetJOGCmd(&jogCmd, false, NULL);
}

void Dobot::gripperCtrl(bool grip, bool waitEnd)
{
	_gripperCtrl(true, grip, waitEnd);
	Gripped_ = grip;
}

void Dobot::slient(bool waitEnd)
{
	_gripperCtrl(false, false, waitEnd);
}

void Dobot::changeGripper()
{
	gripperCtrl(!Gripped_, true);
	waitForSeconds(0.7f);
	slient();
}

void Dobot::_gripperCtrl(bool enable, bool grip, bool waitEnd)
{
	// Send the command. If failed, just resend the command
	uint64_t queuedCmdIndex;
	do {
		int result = SetEndEffectorGripper(enable, grip, true, &queuedCmdIndex);
		if (result == DobotCommunicate_NoError) {
			break;
		}
	} while (1);
	// Check whether the command is finished
	do {
		if (waitEnd == false) {
			break;
		}
		uint64_t currentIndex;
		int result = GetQueuedCmdCurrentIndex(&currentIndex);
		if (result == DobotCommunicate_NoError &&
			currentIndex >= queuedCmdIndex) {
			break;
		}
	} while (1);
}

void Dobot::waitForSeconds(float seconds, bool waitEnd)
{
	// Send the command. If failed, just resend the command
	uint64_t queuedCmdIndex;
	do {
		WAITCmd waitCmd;
		waitCmd.timeout = (uint32_t)(seconds * 1000);
		int result = SetWAITCmd(&waitCmd, true, &queuedCmdIndex);
		if (result == DobotCommunicate_NoError) {
			break;
		}
	} while (1);
	// Check whether the command is finished
	do {
		if (waitEnd == false) {
			break;
		}
		uint64_t currentIndex;
		int result = GetQueuedCmdCurrentIndex(&currentIndex);
		if (result == DobotCommunicate_NoError &&
			currentIndex >= queuedCmdIndex) {
			break;
		}
	} while (1);
}

bool Dobot::isConnected()
{
	return ConnectStatus_;
}

bool Dobot::isStatic()
{
	Point3D tmp = getPosition();
	return tmp == Cur_Pos_;
}

void Dobot::close()
{
	DisconnectDobot();
	ConnectStatus_ = false;
}

Dobot::~Dobot()
{
	close();
}

Point3D Dobot::getPosition()
{
	Pose p;
	GetPose(&p);
	return { p.x, p.y, p.z };
}

void Dobot::grasp(float seconds)
{
	waitForSeconds(seconds, true);
	gripperCtrl(false, true);
	waitForSeconds(seconds, true);
	Point3D bias = { 0, 0, 60.0f };
	gotoPoint(getPosition() + bias, true);
}