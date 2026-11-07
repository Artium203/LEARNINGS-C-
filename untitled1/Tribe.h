#pragma once
#include "Survivor.h"
#include<iostream>
#include<string>

class Tribe{
private:
    string _tribeName;
    int _maxSurvivors;
    int _numSurvivors;
    Survivor** _survivorsArray;
public:
    Tribe(string n="Tzofim",int max=10);
    void setTribe(string n = "Tzofim", int max = 10);
    ~Tribe();
    void printTribe() const;
    void sortSurivovrsByName();
    void addSurivovrToTribe();
    void removeSurivorFromTribe();
    void removeYoungestSurivorFromTribe();
    int findSurvivorByName(string n);
    void MENU();
};

