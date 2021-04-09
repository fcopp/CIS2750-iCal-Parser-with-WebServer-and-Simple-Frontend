//Finnian Copp
//0928913

#include "functions.h"
#include "CalendarParser.h"
#include <stdio.h>
#include <stdlib.h>

bool validDT(DateTime dt){
	if(strlen(dt.date) != 8 || strlen(dt.time) != 6){
		return false;
	}
	return true;
}

ICalErrorCode validEventsList(List* events){

	ListIterator iter = createIterator(events);

	for(int i = 0; i < getLength(events); i++){
		Event* event = (Event*)nextElement(&iter);

		if(event == NULL || event->properties == NULL || event->alarms == NULL){
			return INV_EVENT;
		}

		if(strcmp(event->UID,"") == 0 || strlen(event->UID) > 999){
			return INV_EVENT;
		}
		if(!validDT(event->creationDateTime) || !validDT(event->startDateTime)){
	 		return INV_EVENT;
		}


//properties first
		ListIterator iterProp = createIterator(event->properties);
		char* evtPropStrings[] = {"CLASS","CREATED","DESCRIPTION","GEO","LAST-MODIFIED","LOCATION","ORGANIZER","PRIORITY","SEQ","STATUS","SUMMARY","TRANSP","URL","RECURRENCE-ID"};
		int eventFlags[14] = {0};
		int dtendDurationSet = 0;
		int dtstampSet = 0;
		int dtstartSet = 0;




		for(int c = 0; c<getLength(event->properties); c++){
			
			Property* prop = (Property*)nextElement(&iterProp);

			if(strcmp(prop->propName,"") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr,"") == 0){
				return INV_EVENT;
			}

			bool cont = false;
			for(int it = 0; it < 14; it++){
				if(strcmpCaseInsen(evtPropStrings[it],prop->propName) == 0){
					if(eventFlags[it] == 1){
						return INV_EVENT;
					}
					eventFlags[it]++;
					cont = true;
					break;
				}
			}
			if(cont){
				continue;
			}


			if(strcmpCaseInsen(prop->propName,"DTSTAMP") == 0){
				if(dtstampSet == 1){
					return INV_EVENT;
				}
				dtstampSet = 1;
				continue;
			}

			if(strcmpCaseInsen(prop->propName,"UID") == 0){
				return INV_EVENT;
			}
			
			if(strcmpCaseInsen(prop->propName,"DTSTART") == 0){
				if(dtstartSet == 1){
					return INV_EVENT;
				}
				dtstartSet = 1;
				continue;
			}

			if(strcmpCaseInsen(prop->propName,"DTEND") == 0){
				if(dtendDurationSet == 1){
					return INV_EVENT;
				}
				dtendDurationSet = 1;
				continue;
			}

			if(strcmpCaseInsen(prop->propName,"DURATION") == 0){
				if(dtendDurationSet == 1){
					return INV_EVENT;
				}
				dtendDurationSet = 1;
				continue;
			}

			char* allowedEventProps[] = {"attach","attendee","categories","comment",
                  						"contact","exdate","REQUEST-STATUS","RELATED-TO","resources","rdate","rrule"};

            bool inAllowed = false;
            for(int p = 0; p < 11; p++){
            	if(strcmpCaseInsen(allowedEventProps[p],prop->propName) == 0){
            		inAllowed = true;
            		break;
            	}
            }
            if(inAllowed){
            	continue;
            }else{
            	return INV_EVENT;
            }

		}

		if(event->alarms == NULL){
			return INV_EVENT;
		}

		ICalErrorCode alarmsCode = validAlarmsList(event->alarms);
		if (alarmsCode != OK){
			return alarmsCode;
		}
	}

	return 0;
}
ICalErrorCode validAlarmsList(List* alarms){

	ListIterator iter = createIterator(alarms);

	for(int i = 0; i < getLength(alarms); i++){
		Alarm* alarm = (Alarm*)nextElement(&iter);

		if(alarm->properties == NULL){
			return INV_ALARM;
		}

		if(strcmpCaseInsen(alarm->action,"audio") != 0){
			return INV_ALARM;
		}
		if(alarm->trigger == NULL){
			return INV_ALARM;
		}
		if(strcmp(alarm->trigger,"") == 0){
			return INV_ALARM;
		}

		int repSet = 0;
		int durSet = 0;
		int attachSet = 0;
		ListIterator iterProp = createIterator(alarm->properties);
		for(int c = 0; c<getLength(alarm->properties); c++){
			
			Property* prop = (Property*)nextElement(&iterProp);

			if(strcmp(prop->propName,"") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr,"") == 0){
				return INV_ALARM;
			}

			if(strcmpCaseInsen("DURATION",prop->propName) == 0){
				if(durSet == 1){
					return INV_ALARM;
				}
				durSet = 1;
				continue;
			}

			if(strcmpCaseInsen("REPEAT",prop->propName) == 0){
				if(repSet == 1){
					return INV_ALARM;
				}
				repSet = 1;
				continue;
			}

			if(strcmpCaseInsen("ATTACH",prop->propName) == 0){
				if(attachSet == 1){
					return INV_ALARM;
				}
				attachSet = 1;
				continue;
			}

			if(strcmpCaseInsen("TRIGGER",prop->propName) == 0){
				return INV_ALARM;
			}

			if(strcmpCaseInsen("ACTION",prop->propName) == 0){
				return INV_ALARM;
			}

			return INV_ALARM;

		}

		if(repSet != durSet){
			return INV_ALARM;
		}


	}

	return 0;
}

char* getSubString(char* str, int i1, int i2){
	
	if(str == NULL || i2 <= i1){
		return NULL;
	}

	int length = i2 - i1;

	char text[10000] = "";
	//char* str2 = malloc(sizeof(char) * (length + 1));
	//strcpy(str2,"");

	for(int c = 0; c < length; c++){
		text[c] = str[i1 + c];
	}

	char* str2 = malloc(sizeof(char) * (length + 1));
	if(str2 == NULL){
		return NULL;
	}
	strcpy(str2,text);

	return str2;

}

bool summaryFind(const void* first,const void* second){
	Property* one = (Property*)first;
	Property* two = (Property*)second;

	if(strcmpCaseInsen(one->propName,two->propName) == 0){
		return true;
	}else{
		return false;
	}
}

bool compareFloats(float numOne, float numTwo){
	float difference = numOne-numTwo;
	if (difference < 0.001 && difference > -0.001){
		return true;
	}
	return false;	
}

void cleanUp(Calendar* cal,Event* event, Alarm* alarm, FILE* file){

	deleteCalendar(cal);
	deleteEvent(event);
	deleteAlarm(alarm);
	fclose(file);

	return;

}

int strcmpCaseInsen(char* strOne1, char* strTwo2){

	char* strOne = malloc(sizeof(char) * (strlen(strOne1) + 1));
	if(strOne == NULL){
		return -1;
	}
	char* strTwo = malloc(sizeof(char) * (strlen(strTwo2) + 1));
	if(strTwo == NULL){
		return -1;
	}

	strcpy(strOne,strOne1);
	strcpy(strTwo,strTwo2);

	for(int i = 0; i < strlen(strOne); i++){

		if (strOne[i] > 64 && strOne[i] < 91){
			strOne[i] += 32;
		}
	}
	for(int i = 0; i < strlen(strTwo); i++){
		if (strTwo[i] > 64 && strTwo[i] < 91){
			strTwo[i] += 32;
		}
	}
	
	int ret = strcmp(strOne,strTwo);
	free(strOne);
	free(strTwo);

	return ret;

}

ICalErrorCode parseDT(char* line,DateTime** dt){

	int len = strlen(line);

	char* line2 = malloc(sizeof(char) * (len + 1));
	strcpy(line2,line);


	DateTime* temp = malloc(sizeof(DateTime));
	if(temp == NULL){
		return OTHER_ERROR;
	}


	if(len == 16){
		if(line2[15] == 'Z'){
			char* strDate = strtok(line2,"tT");



			if(strDate == NULL){
				free(temp);
				free(line2);
				return 44;
			}

			char* strTime = strtok(NULL,"Z");

			if(strTime == NULL){
				free(temp);
				free(line2);
				return 45;
			}

			if(strlen(strDate) != 8){
				free(temp);
				free(line2);
				return 46;
			}
			if(strlen(strTime) != 6){
				free(temp);
				free(line2);
				return 35;
			}

			strcpy(temp->time,strTime);
			strcpy(temp->date,strDate);

			if(!numberCheck(strDate) || !numberCheck(strTime)){
				free(temp);
				free(line2);
				return 12;
			}

			temp->UTC = true;

			*dt = temp;
			free(line2);
			return OK;

		}else{
			free(temp);
			free(line2);
			return INV_DT;
		}
	}else if (len == 15){
		char* strDate = strtok(line2,"tT");
		if(strDate == NULL){
			free(temp);
			free(line2);
			return INV_DT;
		}

		char* strTime = strtok(NULL,"\0");
		if(strTime == NULL){
			free(temp);
			free(line2);
			return INV_DT;
		}

		if(strlen(strDate) != 8){
			free(temp);
			free(line2);
			return INV_DT;
		}
		if(strlen(strTime) != 6){
			free(temp);
			free(line2);
			return INV_DT;
		}

		if(!numberCheck(strDate) || !numberCheck(strTime)){
			free(temp);
			free(line2);
			return INV_DT;
		}

		strcpy(temp->time,strTime);
		strcpy(temp->date,strDate);

		temp->UTC = false;

		*dt = temp;
		free(line2);
		return OK;

	}else{
		free(temp);
		return INV_DT;
	}
	return OTHER_ERROR;
}
bool numberCheck(char* strOne){
	for(int i = 0; i < strlen(strOne); i++){
		if(strOne[i] < 48 || strOne[i] > 57){
			return false;
		}
	}
	return true;
}