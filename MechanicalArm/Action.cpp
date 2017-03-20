#include "Dobot.hpp"

list<Point3D> readCmd(std::string filename = "cmd.csv")
{
	ifstream is(filename);
	istream_iterator<Point3D> data(is), end;
	list<Point3D> li(data, end);
	return li;
}

bool grasped = false;

void moveCmd(Dobot &arm, string recv)
{
	//Point3D bias = { 0, 0, -8.0f };
	Point3D pt;
	std::stringstream ss;
	ss << recv;
	ss >> pt;
	MESSAGE_INFO("MOVE TO " << pt);

	//arm.gripperCtrl(true, true);

	Point3D current = arm.getPosition();
	current.z = 60.0f;
	arm.gotoPoint(current, true);
	pt.z = 65.f;
	arm.gotoPoint(pt, true);
}

int parseCommand(Dobot &arm, string recv)
{
	static Point3D bias = { 0, 0, 50.0f };
	if (recv == "home") {
		arm.home();
		arm.gotoPoint(arm.getPosition() + bias, true);
		return 'h';
	}
	else if (recv == "change"){
		grasped = false;
		arm.changeGripper();
		return 'c';
	}
	else if (recv == "up"){
		arm.UP();
		arm.waitForSeconds(1.0f);
		arm.STOP();
		return 'u';
	}
	else if (recv == "down"){
		arm.DOWN();
		arm.waitForSeconds(1.0f);
		arm.STOP();
		return 'd';
	}
	else if (recv == "grasp"){
		arm.grasp(1.0f);
		arm.slient();
		return 'g';
	}
	else if (recv == "quit") {
		return 'q';
	}
	else {
		//if (!grasped){
			arm.gripperCtrl(true, true);
			moveCmd(arm, recv);
			arm.grasp(1.0f);
			arm.slient();
			grasped = true;
		//}
		return 'm';
	}
}

Point3D origin = { 143.8221f, 4.8719f, -21.0000f };
float side = 71.0f;

int test()
{
	Dobot arm(origin);
	arm.connect();	
	if (!arm.isConnected()){
		MessageBox(NULL, TEXT("Please connect DOBOT to the computer!"), TEXT("OK"), MB_OK);
		return -1;
	}
	// wait for cmd
	arm._connectSocket();
	while (true){
		string recv = arm.getSocketData();
		parseCommand(arm, recv);
	}
	arm._closeSocket();
	return 0;
}

int main()
{
	test();
	return 0;
}
