#include "MyDate.h"
#include <iostream>
#include<string>
#include <sec_api/string_s.h>
using namespace std;

void MyDate::setYear(int year) {
    if (year>0) {
        this->_year = year;
    }else {
        return;
    }
}

void MyDate::setMyDate(int _year, int _mounth, int day, string comment) {
    this->setYear(_year);
    this->setMounth(_mounth);
    this->setDay(day);
    this->setComment(comment);
}



