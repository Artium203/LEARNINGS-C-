#include <iostream>
#include<string>
#include <sec_api/string_s.h>
using namespace std;

class Survivor {
public:
    static const int MAX_NAME_SIZE = 21;
    enum _FamilyStatus { SINGLE, MARRIED, IN_A_RELATIONSHIP };
    void setName( char* name );
    void setAge( int age );
    void setStatus( _FamilyStatus status );

    char* getName();
    int getAge() const;
    _FamilyStatus getStatus() const;

    void init( char* name, int age, _FamilyStatus status );
    void print();
private:
    char	_name[MAX_NAME_SIZE];
    int _age;
    _FamilyStatus	_status;
};
void Survivor::setName( char* name )
{
    if( strlen( name ) >= MAX_NAME_SIZE )
    {
        cout << "The name must contain " << ( MAX_NAME_SIZE - 1 ) << " letters or less." << endl;
        return;
    }

    strcpy( _name, name );
}
void Survivor::setAge( int age )
{
    _age = age;
}
void Survivor::setStatus( Survivor::_FamilyStatus status )
{
    _status = status;
}
char* Survivor::getName()
{
    return _name;
}
int Survivor::getAge() const
{
    return _age;
}
Survivor:: _FamilyStatus Survivor::getStatus() const
{
    return _status;
}
void Survivor::init( char* name, int age, _FamilyStatus status )
{
    setName( name );
    setAge( age );
    setStatus( status );
}
void Survivor::print()
{
    cout << "name: " << _name << endl << "age: " << _age << endl << "status: " << _status << endl;
}
class Tribe
{
public:
    void init(int maxSurvivors);

    bool addSurvivor(Survivor& newSurvivor);

    /*inline*/ Survivor** getAllSurvivors();
    /*inline*/ int getMaxSurvivors() const;
    /*inline*/ int getCurrentNumOfSurvivors() const;

    void print() const;
    void free();

private:
    Survivor**	m_allSurvivors;
    int			m_maxSurvivors;
    int			m_currentNumOfSurvivors;
};
void Tribe::init( int maxSurvivors )
{
    m_maxSurvivors = maxSurvivors;
    m_currentNumOfSurvivors = 0;
    m_allSurvivors = new Survivor*[m_maxSurvivors];
}
bool Tribe::addSurvivor( Survivor& newSurvivor )
{
    if( m_currentNumOfSurvivors < m_maxSurvivors )
    {
        m_allSurvivors[m_currentNumOfSurvivors++] = &newSurvivor;
        return true;
    }

    return false;
}
Survivor** Tribe::getAllSurvivors()
{
    return m_allSurvivors;
}
int Tribe::getCurrentNumOfSurvivors() const
{
    return m_currentNumOfSurvivors;
}
int Tribe::getMaxSurvivors() const
{
    return m_maxSurvivors;
}
void Tribe::print() const
{
    for( int i = 0; i < m_currentNumOfSurvivors; ++i )
        m_allSurvivors[i]->print();
}
void Tribe::free()
{
    for( int i = 0; i < m_currentNumOfSurvivors; ++i )
        delete m_allSurvivors[i];

    delete[] m_allSurvivors;
}

void readTribe( Tribe& t );
void readSurvivor( Survivor& s );

void main()
{
    Tribe t1, t2;

    t1.init( 20 );
    t2.init( 20 );

    cout << "Enter data for first tribe:\n";
    readTribe( t1 );

    cout << "Enter data for second tribe:\n";
    readTribe( t2 );

    t1.print();
    t2.print();

    int currentSurvivors = t1.getCurrentNumOfSurvivors();
    for( int i = 0; i < currentSurvivors; i++ )
        delete t1.getAllSurvivors()[i];

    currentSurvivors = t2.getCurrentNumOfSurvivors();
    for( int i = 0; i < currentSurvivors; i++ )
        delete t2.getAllSurvivors()[i];

    t1.free();
    t2.free();

    system( "pause" );
}

void readSurvivor( Survivor& s )
{
    char name[Survivor::MAX_NAME_SIZE];
    cin.ignore(); // clean the buffer
    cout << "Enter your name: ";
    cin.getline( name, Survivor::MAX_NAME_SIZE );

    int age;
    cout << "Enter you age:  ";
    cin >> age;

    int statusNumber;
    cout << "Enter your status: \n 1.Single \n 2.Married \n 3.In A Relationship \n";
    cin >> statusNumber;

    s.init( name, age, ( Survivor::_FamilyStatus )statusNumber );
}

void readTribe( Tribe& t )
{
    bool toContinue = true;
    int indicator;

    int maxSurvivors = t.getMaxSurvivors();
    for( int i = 0; ( i < maxSurvivors ) && toContinue; i++ )
    {
        Survivor* s = new Survivor;
        readSurvivor( *s );
        t.addSurvivor( *s );

        cout << "Continue? Press 1 for yes 2 for no \n";
        cin >> indicator;

        if( indicator == 2 )
            toContinue = false;

    }
}