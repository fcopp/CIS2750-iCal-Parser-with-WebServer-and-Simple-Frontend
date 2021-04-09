//Finnian Copp
//0928913

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CalendarParser.h"

bool compareFloats(float numOne, float numTwo);
void cleanUp(Calendar* cal,Event* event, Alarm* alarm, FILE* file);
ICalErrorCode parseDT(char* line, DateTime** dt);
ICalErrorCode validEventsList(List* events);
ICalErrorCode validAlarmsList(List* events);
bool summaryFind(const void* first,const void* second);
bool validDT(DateTime dt);
//ICalErrorCode validPropertiesList(List* events);
char* getSubString(char* str, int i1, int i2);
bool numberCheck(char* strOne);
int strcmpCaseInsen(char* strOne1, char* strTwo2);
#endif