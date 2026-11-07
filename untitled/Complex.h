//
// Created by artem on 7/14/2024.
//

#pragma once
#include <iostream>
#include<string>
#include <sec_api/string_s.h>
#include "math.h"
using namespace std;



class Complex {
private:
    int _x;
    int _y;

public:
    Complex(int _x,int _y);
    Complex();
    Complex(const Complex& other);
    int getX(){return _x;}
    int getY(){return _y;}
    void setX(int x) {_x = x;}
    void setY(int y) {_y = y;}
    void add(int _x,int _y,int _x2,int _y2);
    void sub(int _x,int _y,int number);
    void print();
    void setX();
};

