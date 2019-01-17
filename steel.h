#ifndef STEEL_H
#define STEEL_H

#include <iostream>
#include <string>

using namespace std;


class steel
{
public:
    string Name;
	int length;
    float (*ElasticModulus)[2];
    steel(float em[][2],int num,string name);
    ~steel();
};



#endif // STEEL_H
