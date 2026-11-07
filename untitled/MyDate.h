#pragma once
#include <iostream>
#include<string>
#include <sec_api/string_s.h>
using namespace std;



class MyDate {

private:
    int _year;
    int _mounth;
    int _day;
    string _comment;
public:
    MyDate();
    MyDate(int _year = 2012,int _mounth=1,int day =1);
    void setMyDate(int _year = 2012,int _mounth=1,int day =1,string comment);
    ~MyDate();
    int getYear()const{return this->_year;}
    int getMounth()const{return this->_mounth;}
    int getDay()const{return this->_day;}
    string getComment()const{return this->_comment;}
    void setComment(string comment);
    void setYear(int year);
    void setMounth(int mounth);
    void setDay(int day);
    void setDate(int year,int mounth,int day);
    void inc();
    void init();
    void isBefore(MyDate date);
    void changeComment(char** newCom, int max=50);
    void print()const;
};
