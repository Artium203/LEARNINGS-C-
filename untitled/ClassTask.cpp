#include <iostream>
#include<string>
#include <sec_api/string_s.h>
#define ID_SIZE 10
using namespace std;

class Car {
private:
    char _licenseNumber[ID_SIZE + 1];
    double _price;
    int _year;
    string _model;
    bool isValidLicenseNumber(const char ln[]);
    bool isValidModel(string model);
    bool isValidPrice(double price){return (price>500.99);}
    bool isValidYear(int year) { return (year >= 1920); }

public:
    int getYear() { return this->_year; }
    double getPrice(){return this->_price;}
    const char* getLicenseNumber() { return this->_licenseNumber; }
    string getModel() { return this->_model; }

    void setYear(int y = 1996);
    void setLicenseNumber(const char* ln = "653-55-932");
    void setModel(string m = "Unknown");
    void setPrice(double p = 2800.69);
    void setCar(const char* ln = "653-55-932", string m = "Unknown", int y = 1996,double p = 2800.69);
    void printCar();

    Car();
    Car(const char* ln,string m,int y,double p);
    ~Car();
};
bool Car::isValidLicenseNumber(const char ln[]) {
    for (int i = 0; i < strlen(ln); i++)
        if (!(ln[i] >= '0' && ln[i] <= '9' || ln[i]=='-'))
            return false;
    return true;
}

bool Car::isValidModel(string model) {
    for (int i = 0; i < model.length(); i++)
        if (!(model[i] >= 'a' && model[i] <= 'z' || model[i] >= 'A' && model[i] <= 'Z' ||
            model[i] == ' ' || model[i] == '-'))
            return false;
    return true;
}
void Car::setYear(int y) {
    if (this->isValidYear(y) == false) {
        cout << "Year is out of range, updated to 1996" << endl;
        this->_year = 1996;
        return;
    }
    this->_year = y;
    cout << "Year is updated to " << y << endl;
}
void Car::setLicenseNumber(const char* ln) {
    if (!(this->isValidLicenseNumber(ln))) {
        cout << "Id is incorrect, update to 11111111" << endl;
        strcpy_s(this->_licenseNumber, strlen("11111111") + 1, "11111111");
        return;
    }
    strcpy_s(this->_licenseNumber, strlen(ln) + 1, ln);
    cout << "License Number is updated to " << ln << endl;
}
void Car::setModel(string m) {
    // if (!(this->isValidFullName(m))) {
    //     cout << "Full name is oncorrect, updated to Unknown" << endl;
    //     this->_model = "Unknown";
    //     return;
    // }
    cout << "Model is updated to " << m << endl;
    this->_model = m;
}
void Car::setPrice(double p) {
    cout << "Price is updated to " << p << endl;
    this->_price = p;
}

void Car::setCar(const char* ln, string m, int y,double p) {
    this->setYear(y);
    this->setModel(m);
    this->setLicenseNumber(ln);
    this->setPrice(p);
}
void Car::printCar() {
    cout << "Car details : model - " << this->getModel() << ", License Number - " << this->getLicenseNumber() <<
        ", Year - " << this->getYear() << ", price - " << this->getPrice() << endl;
}
Car::Car() {
    this->setCar();
    cout << "Person is constructed at address : " << this << endl;
}
Car::Car(const char* ln, string m, int y, double p) {
    this->setCar(ln, m, y, p);
    cout << "Person is constructed at adress : " << this << endl;
}
Car::~Car() {
    cout << "Car is destroyed from address : " << this <<" and his number was - " << this->getLicenseNumber()<< endl;
}
int main() {
    Car c1;//calling to default constructor
    c1.setCar();
    c1.printCar();
    c1.setCar("555-555-555", "Moshe Levi", 30,5402.64);
    c1.printCar();
    Car c2("444-444-444", "Ronna Cohen", 40,3249.54);
    c2.printCar();
    return 0;
}