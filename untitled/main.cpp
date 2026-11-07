#include<iostream>
#include<string>
#include <sec_api/string_s.h>

#define ID_SIZE 9

using namespace std;

class Person {
private:
	char _id[ID_SIZE + 1];
	string _fullName;
	int _age;

	bool isValidId(const char id[]);
	bool isValidFullName(string fullName);
	bool isValidAge(int age) { return (age >= 0 && age <= 120); }

public:
	//getters:
	int getAge() { return this->_age; }
	const char* getId() { return this->_id; }
	string getFullName() { return this->_fullName; }
	//setters:
	void setAge(int a = 18);
	void setId(const char* id = "123123123");
	void setFullName(string fn = "Unknown");
	void setPerson(const char* id = "123123123", string fn = "Unknown", int a = 18);
	void printPerson();

	Person();
	Person(const char* id, string n, int a);
	~Person();
};

bool Person::isValidId(const char id[]) {
	for (int i = 0; i < strlen(id); i++)
		if (!(id[i] >= '0' && id[i] <= '9'))
			return false;
	return true;
}

bool Person::isValidFullName(string fullName) {
	for (int i = 0; i < fullName.length(); i++)
		if (!(fullName[i] >= 'a' && fullName[i] <= 'z' || fullName[i] >= 'A' && fullName[i] <= 'Z' ||
			fullName[i] == ' ' || fullName[i] == '-'))
			return false;
	return true;
}
void Person::setAge(int a) {
	if (this->isValidAge(a) == false) {
		cout << "Age is out of range, updated to 18" << endl;
		this->_age = 18;
		return;
	}
	this->_age = a;
	cout << "Age is updated to " << a << endl;
}
void Person::setId(const char* id) {
	if (!(this->isValidId(id))) {
		cout << "Id is incorrect, update to 111111111" << endl;
		strcpy_s(this->_id, strlen("111111111") + 1, "111111111");
		return;
	}
	strcpy_s(this->_id, strlen(id) + 1, id);
	cout << "Id is updated to " << id << endl;
}
void Person::setFullName(string fn) {
	if (!(this->isValidFullName(fn))) {
		cout << "Full name is oncorrect, updated to Unknown" << endl;
		this->_fullName = "Unknown";
		return;
	}
	cout << "Full name is updated to " << fn << endl;
	this->_fullName = fn;
}
void Person::setPerson(const char* id, string fn, int a) {
	this->setAge(a);
	this->setFullName(fn);
	this->setId(id);
}
void Person::printPerson() {
	cout << "Person details : name - " << this->getFullName() << ", id - " << this->getId() <<
		", age - " << this->getAge() << endl;
}
Person::Person() {
	this->setPerson();
	cout << "Person is constructed at address : " << this << endl;
}
Person::Person(const char* id, string n, int a) {
	this->setPerson(id, n, a);
	cout << "Person is constructed at adress : " << this << endl;
}
Person::~Person() {
	cout << "Person is destroyed from address : " << this << endl;
}

// void main() {
// 	Person p1;//calling to default constructor
// 	p1.setPerson();
// 	p1.printPerson();
// 	p1.setPerson("555555555", "Moshe Levi", 30);
// 	p1.printPerson();
// 	Person p2("444444444", "Ronna Cohen", 40);
// 	p2.printPerson();
//
// }
/*
Exercise:

Build class Car which contains: model (string), year (int), price (double), licenseNumber (char [10])
//Dont 4get to implement getters & setters as usual
//printCar
//Any 3 constructors
//Destructor that says which license number if deleted from the pc memory

*/