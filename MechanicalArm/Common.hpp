#include <string>
#include <vector>
#include <list>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>

#include <thread>

using std::string;
using std::vector;
using std::list;
using std::queue;

using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::istream_iterator;

using std::cout;
using std::endl;


struct Point3D{
	float x, y, z;
	bool operator == (Point3D &other){
		return (x == other.x) && (y == other.y) && (z == other.z);
	}
	Point3D operator +(Point3D &p){
		return{ x + p.x, y + p.y, z + p.z };
	}
	friend istream& operator >> (istream &is, Point3D &p) {
		is >> p.x >> p.y >> p.z;
		return is;
	}
	friend ostream& operator <<(ostream &os, Point3D &p) {
		os << "[" << p.x << ", " << p.y << ", " << p.z << "]";
		return os;
	}
};