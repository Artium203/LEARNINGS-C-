#include <iostream>
#include<string>
#include <sec_api/string_s.h>
#include "Complex.h"
#include "math.h"
using namespace std;

Complex::Complex(int _x, int _y) {
    this->setX(_x);
    this->setY(_y);
    this->setR(sqrt(pow(_x,2)+pow(_y,2)));
    this->setTet(atan2(_y,_x)+180);
}
Complex::Complex(int _x, int _y, double _r, double _tet) {
    this->setX(_x);
    this->setY(_y);
    this->setR(_r);
    this->setTet(_tet);
}
void Complex::add(int _x, int _y, int _x2, int _y2) {

}



