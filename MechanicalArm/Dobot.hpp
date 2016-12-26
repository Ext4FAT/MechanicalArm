#include "./DobotDll/DobotDll.h"
#include "Common.hpp"
#include "Macro.h"

#pragma comment(lib, "./DobotDll/DobotDll.lib")

#include <windows.h>
#pragma comment(lib, "ws2_32.lib")  

typedef uint32_t uint;

#define _JOG_ 1
#define _COORDINATE_ 0
#define _JOGMODE_  _JOG_

#define _STOP_ 0
#define _J1P_ 1
#define _J1N_ 2
#define _J2P_ 3
#define _J2N_ 4
#define _J3P_ 5
#define _J3N_ 6
#define _J4P_ 7
#define _J4N_ 8


class Dobot{
public:
	Dobot(Point3D origin) :Origin_(origin)
	{}
public:
	void connect(void);
	void init(void);
//private:
	void close(void);
	void home(void);
	void _home(void);
	void _moveToOrigin(void);
	void grasp(float seconds);
	void UP();
	void DOWN();
	void STOP();
	void _jog(int i);	// "i" respresent which JOG's index, 0 is stop
	void changeGripper();
	void waitForSeconds(float seconds, bool waitEnd = false);
	void gotoPoint(Point3D pt, bool waitEnd);
	void _gotoPoint(float x, float y, float z, float r, bool waitEnd = false);
	void gripperCtrl(bool grip, bool waitEnd = false);
	void _gripperCtrl(bool enable, bool grip, bool waitEnd = false);
	void slient(bool waitEnd = false); // Because the pump is too noisy, if you cann't stand£¬ please keep silent 
	Point3D getPosition();
//Socket
	int _connectSocket();
	void _closeSocket();
	string getSocketData();

private:
	//void LaserCtrl(bool isOn, bool waitEnd = false);
	//void suctionCupCtrl(bool suck, bool waitEnd = false);
//Status
public:
	bool isConnected();
	bool isStatic();
	~Dobot();
private:
	bool ConnectStatus_ = false;
	bool Gripped_ = false;
	uint JOGMode_ = 1; // 0: Joint   1: Coordinate
	Point3D Cur_Pos_;
	Point3D Origin_;
	//bool GripperCtrlEnabled_ = true;
	SOCKET  Server_; // server socket  
};







