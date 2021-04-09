//Finnian Copp
//0928913

#include "CalendarParser.h"
#include <stdio.h>
#include "functions.h"

ICalErrorCode createCalendar(char* fileName, Calendar** obj){


	FILE* file;

	if(fileName == NULL){
		return INV_FILE;
	}

	if(strlen(fileName) < 5){
		return INV_FILE;
	}

	if(!strrchr(fileName,'.')){ //checks for last ".", returns null if cant find
		return INV_FILE;
	}

	char* tmpFileName = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(tmpFileName,fileName);
	char* checker = strrchr(tmpFileName,'.') + 1;
	//printf("|%s|\n",checker);
	if( strcmpCaseInsen(checker,"ics") != 0){
		free(tmpFileName);
		//printf("asdfkl\n");
		return INV_FILE;
	}
	free(tmpFileName);

	file = fopen(fileName, "r");

	if(file == NULL){
		return INV_FILE;
	}

	Calendar* temp = malloc(sizeof(Calendar));

	if(temp == NULL){
		return OTHER_ERROR;
	}

	temp->events = initializeList(printEvent,deleteEvent,compareEvents);
	temp->properties = initializeList(printProperty,deleteProperty,compareProperties);

	Event* event = NULL; //malloc inside begin:valarm
	Alarm* alarm = NULL;

	int insideVCal = 0;
	int insideVEVENT = 0;
	int insideVALARM = 0;

	int verSet = 0;
	int prodIDSet = 0;
	int calscaleSet = 0;
	int methodSet = 0;

	//class / created / description / geo /
    //last-mod / location / organizer / priority /
    //seq / status / summary / transp /
    //url / recurid /

	//typedef enum aTypes {AUDIO,DISPLAY,EMAIL} AlarmTypes;
	int actionSet = 0;
	int triggerSet = 0;
	int repSet = 0;
	int durSet = 0;
	int attachSet = 0;



    char* evtPropStrings[] = {"CLASS","CREATED","DESCRIPTION","GEO","LAST-MODIFIED","LOCATION","ORGANIZER","PRIORITY","SEQ","STATUS","SUMMARY","TRANSP","URL","RECURRENCE-ID"};
	int eventFlags[14] = {0};
	int dtendDurationSet = 0;
	int dtstampSet = 0;
	int UIDSet = 0;
	int dtstartSet = 0;

	char text[10000] = "";
	char text2[10000] = "";
	
	char* lineCheck;

	while( (lineCheck = fgets(text,10000,file)) != NULL ){
	

		int length = strlen(text);

		if(length < 2){
			cleanUp(temp,event,alarm,file);
			return INV_FILE;
		}

		if(text[length-2] != '\r' || text[length-1] != '\n'){
			cleanUp(temp,event,alarm,file);
			return INV_FILE;
		}

		if(text[0] == ';'){
			continue;
		}

		


		int position = ftell(file);

		while(fgets(text2,10000,file) != NULL){

			if(text2[0] == ';'){
				continue;
			}

			if(text2[0] == ' ' || text2[0] == '\t'){ //no colon			
				char* newText = strtok(text,"\r\n");
				int len = strlen(newText); //minus one because ignoring first char

				for (int i = 1; i < strlen(text2); i++){
					text[len + i -1] = text2[i];
				}

				position = ftell(file);

			}else{ //colon, go back
				break;
			}

		}

		fseek(file,position,SEEK_SET);
		//printf("Text:%s",text);

		if(strstr(text,":") == NULL && strstr(text,";") == NULL){
			cleanUp(temp,event,alarm,file);

			return INV_CAL;
		}

		//check lines for empty or semi colon


		char* lineStart = strtok(text,":;");

		//deal with lineStart being fucked up

		if(strcmpCaseInsen(lineStart,"BEGIN") == 0){
			
			char* beginLineType = strtok(NULL,"\r\n");

			if(strcmpCaseInsen(beginLineType,"VCALENDAR") == 0){
				
				if(insideVCal == 1){ //already in calendar, abort
					cleanUp(temp,event,alarm,file);

					return INV_CAL;

				}

				insideVCal = 1;


			}else if(strcmpCaseInsen(beginLineType,"VEVENT") == 0){

				if (insideVCal == 0){
					cleanUp(temp,event,alarm,file);

					return INV_CAL;

				}
				if (insideVEVENT == 1){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}
				if(insideVALARM == 1){
					cleanUp(temp,event,alarm,file);
					return INV_ALARM;
				}

				insideVEVENT = 1;

				event = malloc(sizeof(Event));

				if(event == NULL){
					cleanUp(temp,event,alarm,file);
					return OTHER_ERROR;
				}

				event->properties = initializeList(printProperty,deleteProperty,compareProperties);
				if(event->properties == NULL){
					cleanUp(temp,event,alarm,file);
					return OTHER_ERROR;
				}
				event->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);
				if(event->alarms == NULL){
					cleanUp(temp,event,alarm,file);
					return OTHER_ERROR;
				}

			}else if(strcmpCaseInsen(beginLineType,"VALARM") == 0){

				if (insideVCal == 0){
					cleanUp(temp,event,alarm,file);
					return INV_CAL;

				}
				if (insideVEVENT == 0){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}
				if(insideVALARM == 1){
					cleanUp(temp,event,alarm,file);
					return INV_ALARM;
				}

				insideVALARM = 1;

				alarm = malloc(sizeof(Alarm));

				if(alarm == NULL){
					cleanUp(temp,event,alarm,file);
					return OTHER_ERROR;
				}

				alarm->properties = initializeList(printProperty,deleteProperty,compareProperties);
				if(alarm->properties == NULL){
					cleanUp(temp,event,alarm,file);
					return OTHER_ERROR;
				}
				alarm->trigger = NULL;

			}else{
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}

		}else if (strcmpCaseInsen(lineStart,"VERSION") == 0){

			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;

			}
			if (insideVEVENT == 1){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if(insideVALARM == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			if(verSet == 1){ //if it has been set abort
				cleanUp(temp,event,alarm,file);
				return DUP_VER;
			}else{
				verSet = 1;
			}

			char* strVersion = strtok(NULL,"\r\n");

			if(strVersion == NULL){
				cleanUp(temp,event,alarm,file);
				return INV_VER;
			}

			char* strTest;
			float ftVersion = strtof(strVersion, &strTest);
			
			if(!compareFloats(ftVersion,2)){
				cleanUp(temp,event,alarm,file);
				return INV_VER;
			}

			if(ftVersion < 0.001 && ftVersion > -0.001){
				cleanUp(temp,event,alarm,file);
				return INV_VER;
			}
			if(strcmpCaseInsen(strTest,"") != 0){
				cleanUp(temp,event,alarm,file);
				return INV_VER;
			}

			temp->version = ftVersion;

		}else if (strcmpCaseInsen(lineStart,"PRODID") == 0){


			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;

			}
			if (insideVEVENT == 1){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if(insideVALARM == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}




			if(prodIDSet == 1){
				cleanUp(temp,event,alarm,file);
				return DUP_PRODID;
			}else{
				prodIDSet = 1;
			}
			
			char* strProdID = strtok(NULL,"\r\n");

			if(strProdID == NULL){
				cleanUp(temp,event,alarm,file);
				return INV_PRODID;
			}

			if(strcmpCaseInsen(strProdID,"") == 0){
				cleanUp(temp,event,alarm,file);
				return INV_PRODID;
			}

			strncpy(temp->prodID,strProdID,999);
			
		}else if (strcmpCaseInsen(lineStart,"END") == 0){
			char* endLineType = strtok(NULL,"\r\n");


			if(strcmpCaseInsen(endLineType,"VCALENDAR") == 0){

				
				if(insideVCal == 1){ //in calendar, finish up
					insideVCal = 0;

					break;

				}else{
					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}


			}else if(strcmpCaseInsen(endLineType,"VEVENT") == 0){



				if(insideVCal == 1 && insideVEVENT == 1 && insideVALARM == 0){
					insideVEVENT = 0;

					if(dtstampSet == 0 || UIDSet == 0 || dtstartSet == 0){
						cleanUp(temp,event,alarm,file);
						return INV_EVENT;
					}else{
						dtstampSet = 0;
						UIDSet = 0;
						dtstartSet = 0;
					}
					if(dtendDurationSet > 1){
						cleanUp(temp,event,alarm,file);
						return INV_EVENT;
					}else{
						dtendDurationSet = 0;
					}
					for(int i = 0; i < 14; i++){
						if(eventFlags[i] > 1){
							cleanUp(temp,event,alarm,file);
							return INV_EVENT;
						}else{
							eventFlags[i] = 0;
						}
					}

					char* strEvent = eventToJSON(event);
					if (strEvent != NULL){
						//printf("|%s|\n",strEvent);
					}
					free(strEvent);
					insertBack(temp->events,event);
					event = NULL;


				}else if (insideVCal == 0){

					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}else if (insideVEVENT == 0){
					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}else if (insideVALARM != 0){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}

			}else if(strcmpCaseInsen(endLineType,"VALARM") == 0){

				if(insideVCal == 1 && insideVEVENT == 1 && insideVALARM == 1){
					
					if(actionSet == 0 || triggerSet == 0){
						cleanUp(temp,event,alarm,file);
						return INV_ALARM;
					}
					if(repSet != durSet){
						cleanUp(temp,event,alarm,file);
						return INV_ALARM;
					}

					insideVALARM = 0;

					actionSet = 0;
					triggerSet = 0;
					repSet = 0;
					durSet = 0;
					actionSet = 0;
					attachSet = 0;

					insertBack(event->alarms,alarm);
					alarm = NULL;
					

				}else if (insideVCal == 0){
					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}else if (insideVEVENT == 0){
					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}else if (insideVALARM == 0){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}

			}
			else{
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}

		}else if(strcmpCaseInsen(lineStart,"CALSCALE") == 0 || strcmpCaseInsen(lineStart,"METHOD") == 0){

			if(insideVCal == 0 || insideVEVENT == 1 || insideVALARM == 1){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}

			if(strcmpCaseInsen(lineStart,"CALSCALE") == 0){
				if(calscaleSet == 1){
					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}

				calscaleSet = 1;


			}else if(strcmpCaseInsen(lineStart,"METHOD") == 0){

				if(methodSet == 1){
					cleanUp(temp,event,alarm,file);
					return INV_CAL;
				}

				methodSet = 1;

			}

			Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(propDescrip) + 1));

			if(prop == NULL){
				cleanUp(temp,event,alarm,file);
				return OTHER_ERROR;
			}

			strcpy(prop->propName,lineStart);
			strcpy(prop->propDescr,propDescrip);

			insertBack(temp->properties,prop);

		}else if(strcmpCaseInsen(lineStart,"ACTION") == 0 && insideVALARM == 1){
			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVEVENT == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVALARM == 0){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if (actionSet == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			actionSet = 1;

			strcpy(alarm->action,propDescrip);


//		}else if(strcmpCaseInsen(lineStart,"DESCRIPTION") == 0 && insideVALARM == 1){
		}else if(strcmpCaseInsen(lineStart,"TRIGGER") == 0 && insideVALARM == 1){
			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVEVENT == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVALARM == 0){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if (triggerSet == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			char* strLine = malloc(sizeof(char) * (strlen(propDescrip)+1));
			if(strLine == NULL){
				cleanUp(temp,event,alarm,file);
				return OTHER_ERROR;
			}

			strcpy(strLine,propDescrip);

			triggerSet = 1;

			alarm->trigger = strLine;


		}else if(strcmpCaseInsen(lineStart,"DURATION") == 0 && insideVALARM == 1){

			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVEVENT == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVALARM == 0){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if (durSet == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			durSet = 1;



			Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(propDescrip) + 1));

			if(prop == NULL){
				cleanUp(temp,event,alarm,file);
				return OTHER_ERROR;
			}
			strcpy(prop->propName,lineStart);
			strcpy(prop->propDescr,propDescrip);

			insertBack(alarm->properties,prop);

		}else if(strcmpCaseInsen(lineStart,"REPEAT") == 0 && insideVALARM == 1){

			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVEVENT == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVALARM == 0){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if (repSet == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			repSet = 1;



			Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(propDescrip) + 1));

			if(prop == NULL){
				cleanUp(temp,event,alarm,file);
				return OTHER_ERROR;
			}
			strcpy(prop->propName,lineStart);
			strcpy(prop->propDescr,propDescrip);

			insertBack(alarm->properties,prop);

		}else if(strcmpCaseInsen(lineStart,"ATTACH") == 0 && insideVALARM == 1){

			if (insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVEVENT == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			}
			if (insideVALARM == 0){
				cleanUp(temp,event,alarm,file);
				return INV_EVENT;
			}
			if (attachSet == 1){
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				return INV_ALARM;
			}

			attachSet = 1;



			Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(propDescrip) + 1));

			if(prop == NULL){
				cleanUp(temp,event,alarm,file);
				return OTHER_ERROR;
			}
			strcpy(prop->propName,lineStart);
			strcpy(prop->propDescr,propDescrip);

			insertBack(alarm->properties,prop);


//		}else if(strcmpCaseInsen(lineStart,"ATTENDEE") == 0 && insideVALARM == 1){
//		}else if(strcmpCaseInsen(lineStart,"SUMMARY") == 0 && insideVALARM == 1){

		}else{ //x - prop of some type
			if(insideVCal == 0){
				cleanUp(temp,event,alarm,file);
				return INV_CAL;
			
			}

			char* propDescrip = strtok(NULL,"\r\n");

			if(propDescrip == NULL || strcmpCaseInsen(propDescrip,"") == 0){ //check new line error maybe???
				cleanUp(temp,event,alarm,file);
				if(insideVALARM == 1){
					return INV_ALARM;
				}else if(insideVEVENT == 1){
					return INV_EVENT;
				}else{
					return INV_CAL;
				}
			}


			for(int i = 0; i < 14; i++){
				if(strcmpCaseInsen(evtPropStrings[i],lineStart) == 0){
					eventFlags[i]++;
					break;
				}
			}

			if(strcmpCaseInsen(lineStart,"DTSTAMP") == 0){
				if(dtstampSet == 1){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}else{
					dtstampSet = 1;
					
					DateTime* dt = NULL;
					int errorCheck = parseDT(propDescrip,&dt);
					if(errorCheck != 0){
						cleanUp(temp,event,alarm,file);
						return errorCheck;
					}

					if(dt == NULL){
						cleanUp(temp,event,alarm,file);
						return INV_DT;
					}

					event->creationDateTime = *dt;

					free(dt);

					continue;

				}
			}

			if(strcmpCaseInsen(lineStart,"UID") == 0){
				if(UIDSet == 1){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}else{
					UIDSet = 1;

					strcpy(event->UID,propDescrip);
					continue;

				}
			}
			if(strcmpCaseInsen(lineStart,"DTSTART") == 0){
				if(dtstartSet == 1){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}else{
					dtstartSet = 1;
					
					DateTime* dt = NULL;
					int errorCheck = parseDT(propDescrip,&dt);
					if(errorCheck != 0){
						cleanUp(temp,event,alarm,file);
						return errorCheck;
					}


					if(dt == NULL){
						cleanUp(temp,event,alarm,file);
						return INV_DT;
					}


					event->startDateTime = *dt;

					free(dt);

					continue;
				}
			}
			if(strcmpCaseInsen(lineStart,"DTEND") == 0 || strcmpCaseInsen(lineStart,"DURATION") == 0){
				if(dtendDurationSet == 1){
					cleanUp(temp,event,alarm,file);
					return INV_EVENT;
				}else{
					dtendDurationSet = 1;
				}
			}

			Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(propDescrip) + 1));

			if(prop == NULL){
				cleanUp(temp,event,alarm,file);
				return OTHER_ERROR;
			}

			strcpy(prop->propName,lineStart);
			strcpy(prop->propDescr,propDescrip);

			if(insideVALARM == 1){
				//free(prop);
				insertBack(alarm->properties,prop);
			}else if(insideVEVENT == 1){
				//free(prop);
				insertBack(event->properties,prop);
			}else {
				insertBack(temp->properties,prop);
			}


		}
	}

	if(getLength(temp->events) == 0){
		cleanUp(temp,event,alarm,file);
		return INV_CAL;
	}

	if(insideVCal == 0 && insideVEVENT == 0 && insideVALARM == 0 && prodIDSet == 1 && verSet == 1){
		*obj = temp;
		//eventListToJSON

		char* strEvent = eventListToJSON((*obj)->events);
		if (strEvent != NULL){
			//printf("|%s|\n",strEvent);
		}
		free(strEvent);

		char* strCal = calendarToJSON((*obj));
		if (strCal != NULL){
			//printf("|%s|\n",strCal);
		}
		free(strCal);

		fclose(file);
		return OK;
	}

	if(prodIDSet != 1 || verSet != 1){
		cleanUp(temp,event,alarm,file);
		return INV_CAL;
	}


	cleanUp(temp,event,alarm,file);

	return INV_FILE;
}

char* printError(ICalErrorCode err){
	char* enumStrings[] = {"OK", "INV_FILE", "INV_CAL", "INV_VER", "DUP_VER", "INV_PRODID", "DUP_PRODID", "INV_EVENT", "INV_DT", "INV_ALARM", "WRITE_ERROR", "OTHER_ERROR" };

	if(err >=0 && err < 12){

		char* str = malloc(sizeof(char) * (strlen(enumStrings[err]) + 1));
		if(str == NULL){
			return NULL;
		}

		strcpy(str,enumStrings[err]);

		return str;
	}else{
		return NULL;
	}

}


ICalErrorCode writeCalendar(char* fileName, const Calendar* obj){
	

	if(fileName == NULL || strcmp(fileName, "") == 0){
		return WRITE_ERROR;
	}

	if(obj == NULL){
		return WRITE_ERROR;
	}

	char buf[2000];
	char* strProdID = calloc(strlen(obj->prodID) + 1, sizeof(char));

	if(strProdID == NULL){
		return WRITE_ERROR;
	}


	strcpy(strProdID,obj->prodID);

	char* propString = toString(obj->properties);

	char* eventString = toString(obj->events);


	if(strcmpCaseInsen(eventString,"") == 0 && strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VCALENDAR\r\nVERSION:%.1f\r\nPRODID:%sEND:VCALENDAR\r\n",obj->version,strProdID);
	}else if(strcmpCaseInsen(eventString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VCALENDAR\r\nVERSION:%.1f\r\nPRODID:%s%sEND:VCALENDAR\r\n",obj->version,strProdID,propString);
	}else if(strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VCALENDAR\r\nVERSION:%.1f\r\nPRODID:%s%s\r\nEND:VCALENDAR\r\n",obj->version,strProdID,eventString);
	}else{
		snprintf(buf,sizeof(buf), "BEGIN:VCALENDAR\r\nVERSION:%.1f\r\nPRODID:%s%s%s\r\nEND:VCALENDAR\r\n",obj->version,strProdID,propString,eventString);
	}

	

	free(strProdID);
	free(propString);
	free(eventString);

	//returning the string that is in buf
	char* strLine = calloc(strlen(buf) + 1, sizeof(char));
	
	if(strLine == NULL){
		return WRITE_ERROR;
	}

	strcpy(strLine,buf);
	//printf("|\n");
	//printf("%s|",strLine);
	//printf("\n");

	FILE* file = fopen(fileName,"w");
	if(file == NULL){
		return WRITE_ERROR;
	}

	fprintf(file,"%s",strLine);
	fclose(file);
	free(strLine);

	return OK;
}
ICalErrorCode validateCalendar(const Calendar* obj){
	
	if(obj == NULL){
		return INV_CAL;
	}

	if(strcmp(obj->prodID,"") == 0 || strlen(obj->prodID) > 999){
		return INV_CAL;
	}

	if(strcmp(obj->prodID,"") == 0){
		return INV_CAL;
	}
	if(!compareFloats(obj->version,2)){
		return INV_CAL;
	}
	if(obj->events == NULL || obj->properties == NULL){
		return INV_CAL;
	}
	if(getLength(obj->events) == 0){
		return INV_CAL;
	}

	int methSet = 0;
	int calSet = 0;
	ListIterator iterProp = createIterator(obj->properties);
	for(int c = 0; c<getLength(obj->properties); c++){
		
		Property* prop = (Property*)nextElement(&iterProp);

		if(strcmp(prop->propName,"") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr,"") == 0){
			return INV_CAL;
		}
	
		if(strcmpCaseInsen("METHOD",prop->propName) == 0){
			if(methSet == 1){
				return INV_CAL;
			}
			methSet = 1;
			continue;
		}

		if(strcmpCaseInsen("CALSCALE",prop->propName) == 0){
			if(calSet == 1){
				return INV_CAL;
			}
			calSet = 1;
			continue;
		}

		return INV_CAL;
	}

	if(obj->events == NULL){
		return INV_CAL;
	}

	ICalErrorCode check = validEventsList(obj->events);
	if(check != OK){
		return check;
	}

	return OK;
}



void deleteCalendar(Calendar* obj){

	if(obj == NULL){
		return;
	}
	if(obj->events != NULL){
		freeList(obj->events);
	}
	if(obj->properties != NULL){
		freeList(obj->properties);
	}
	free(obj);

}

char* printCalendar(const Calendar* obj){

	if(obj == NULL){
		return NULL;
	}

	char buf[2000];
	char* strProdID = calloc(strlen(obj->prodID) + 1, sizeof(char));

	if(strProdID == NULL){
		return NULL;
	}


	strcpy(strProdID,obj->prodID);

	char* propString = toString(obj->properties);

	char* eventString = toString(obj->events);


	if(strcmpCaseInsen(eventString,"") == 0 && strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "Calendar:\nVersion:%.1f\nProdID:%s",obj->version,strProdID);
	}else if(strcmpCaseInsen(eventString,"") == 0){
		snprintf(buf,sizeof(buf), "Calendar:\nVersion:%.1f\nProdID:%s%s",obj->version,strProdID,propString);
	}else if(strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "Calendar:\nVersion:%.1f\nProdID:%s%s",obj->version,strProdID,eventString);
	}else{
		snprintf(buf,sizeof(buf), "Calendar:\nVersion:%.1f\nProdID:%s%s%s",obj->version,strProdID,propString,eventString);
	}

	

	free(strProdID);
	free(propString);
	free(eventString);

	//returning the string that is in buf
	char* strLine = calloc(strlen(buf) + 1, sizeof(char));
	
	if(strLine == NULL){
		return NULL;
	}

	strcpy(strLine,buf);

	return strLine;
}


void deleteEvent(void* toBeDeleted){

	Event* event = (Event*)toBeDeleted;

	if(event == NULL){
		return;
	}
	if(event->properties != NULL){
		freeList(event->properties);
	}
	if(event->alarms != NULL){
		freeList(event->alarms);
	}
	
	free(event);

	return;
}
int compareEvents(const void* first, const void* second){

	if(first == NULL || second == NULL){
		return 1;
	}

	Event* eOne = (Event*)first;
	Event* eTwo = (Event*)second;

	if(strcmpCaseInsen(eOne->UID,eTwo->UID) == 0){
		return 0;
	}

	return 1;
}
char* printEvent(void* toBePrinted){

	char buf[2000];

	Event* event = (Event*)toBePrinted;

	char* propString = toString(event->properties);

	char* alarmString = toString(event->alarms);

	char* dtstamp = printDate(&(event->creationDateTime));

	char* dtstart = printDate(&(event->startDateTime));

	if(strcmpCaseInsen(alarmString,"") == 0 && strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%s\r\nDTSTART:%s\r\nEND:VEVENT",event->UID,dtstamp,dtstart);
	}else if(strcmpCaseInsen(alarmString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%s\r\nDTSTART:%s%s\r\nEND:VEVENT",event->UID,dtstamp,dtstart,propString);
	}else if(strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%s\r\nDTSTART:%s%s\r\nEND:VEVENT",event->UID,dtstamp,dtstart,alarmString);
	}else{
		
		snprintf(buf,sizeof(buf), "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%s\r\nDTSTART:%s%s%s\r\nEND:VEVENT",event->UID,dtstamp,dtstart,propString,alarmString);
	}
	
	free(dtstamp);
	free(dtstart);
	free(propString);
	free(alarmString);

	char* strLine = calloc(strlen(buf) + 1, sizeof(char));

	if(strLine == NULL){
		return NULL;
	}

	strcpy(strLine,buf);

	//printf("[%s]\n",strLine);

	return strLine;
}




void deleteAlarm(void* toBeDeleted){

	Alarm* alarm = (Alarm*)toBeDeleted;

	if(alarm == NULL){
		return;
	}

	if(alarm->properties != NULL){
		freeList(alarm->properties);
	}
	if(alarm->trigger != NULL){
		free(alarm->trigger);
	}
	free(alarm);

	return;
}
int compareAlarms(const void* first, const void* second){
	
	if(first == NULL || second == NULL){
		return 1;
	}

	Alarm* aOne = (Alarm*)first;
	Alarm* aTwo = (Alarm*)second;

	if(strcmpCaseInsen(aOne->action,aTwo->action) == 0 && strcmpCaseInsen(aOne->trigger,aTwo->trigger) == 0){
		return 0;
	}

	return 1;
}
char* printAlarm(void* toBePrinted){

	char buf[2000];

	Alarm* alarm = (Alarm*)toBePrinted;

	char* propString = toString(alarm->properties);

	if(strcmpCaseInsen(propString,"") == 0){
		snprintf(buf,sizeof(buf), "BEGIN:VALARM\r\nACTION:%s\r\nTRIGGER:%sEND:VALARM",alarm->action,alarm->trigger);
	}else{
		snprintf(buf,sizeof(buf), "BEGIN:VALARM\r\nACTION:%s\r\nTRIGGER:%s%s\r\nEND:VALARM",alarm->action,alarm->trigger,propString);
	}
	
	free(propString);

	char* strLine = calloc(strlen(buf) + 1, sizeof(char));

	if(strLine == NULL){
		return NULL;
	}

	strcpy(strLine,buf);


	return strLine;



	return NULL;
}




void deleteProperty(void* toBeDeleted){
	free((Property*)toBeDeleted);
	return;
}
int compareProperties(const void* first, const void* second){
	
	if(first == NULL || second == NULL){
		return 1;
	}

	Property* pOne = (Property*)first;
	Property* pTwo = (Property*)second;

	if(strcmpCaseInsen(pOne->propName,pTwo->propName) == 0 && strcmpCaseInsen(pOne->propDescr,pTwo->propDescr) == 0){
		return 0;
	}

	return 1;

}
char* printProperty(void* toBePrinted){

	Property* prop = (Property*)toBePrinted;
	
	char buf[2000];

	//printf("Name:%s|\n",prop->propName);
	//printf("Descrip:%s|\n",prop->propDescr);

	char* propName = malloc(sizeof(char) * (strlen(prop->propName) + 1));
	if(propName == NULL){
		return NULL;
	}
	char* propDescrip = malloc(sizeof(char) * (strlen(prop->propDescr) + 1));
	if(propDescrip == NULL){
		free(propName);
		return NULL;
	}

	strcpy(propName,prop->propName);
	strcpy(propDescrip,prop->propDescr);

	snprintf(buf,sizeof(buf), "%s:%s",propName,propDescrip);
	free(propName);
	free(propDescrip);

	char* strLine = calloc(strlen(buf) + 1, sizeof(char));
	


	if(strLine == NULL){
		return NULL;
	}

	strcpy(strLine,buf);
	return strLine;
}





void deleteDate(void* toBeDeleted){
	free((DateTime*)toBeDeleted);
	return;
}
int compareDates(const void* first, const void* second){
	return 1;
}
char* printDate(void* toBePrinted){

	DateTime* dt = (DateTime*)toBePrinted;

	if(dt == NULL){
		return NULL;
	}

	if(dt->UTC){
		
		char* strDT = malloc(sizeof(char) * (8+1+6+1+1));
		if(strDT == NULL){
			return NULL;
		}

		for(int i = 0; i < 15; i++){
			if(i < 8){
				strDT[i] = dt->date[i];
			}else if(i == 8){
				strDT[i] = 'T';
			}else if(i > 8){
				strDT[i] = dt->time[i-9];
			}

		}
		strDT[15] = 'Z';
		strDT[16] = '\0';

		return strDT;

	}else{
		
		char* strDT = malloc(sizeof(char) * (8+1+6+0+1));
		if(strDT == NULL){
			return NULL;
		}

		for(int i = 0; i < 15; i++){
			if(i < 8){
				strDT[i] = dt->date[i];
			}else if(i == 8){
				strDT[i] = 'T';
			}else if(i > 8){
				strDT[i] = dt->time[i-9];
			}
			

		}
		strDT[15] = '\0';

		return strDT;

	}

	return NULL;
}

char* dtToJSON(DateTime prop){
	//if(prop == NULL){
	//	return NULL;
	//}

	if(strcmpCaseInsen(prop.date,"") == 0 || strcmpCaseInsen(prop.time,"") == 0){
		char* rtn = malloc(sizeof(char) * (3));
		strcpy(rtn,"{}");
		return rtn;
	}

	char text[10000] = "";
	if(prop.UTC){
		snprintf(text,sizeof(text),"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":true}",prop.date,prop.time);
	}else{
		snprintf(text,sizeof(text),"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":false}",prop.date,prop.time);
	}

	char* strLine = malloc(sizeof(char) * (strlen(text) + 1));
	if(strLine == NULL){
		char* rtn = malloc(sizeof(char) * (3));
		strcpy(rtn,"{}");
		return rtn;
	}
	strcpy(strLine,text);

	return strLine;
}
int addEventtoCal(char* UID, char* dtstamp, char* dtstart, char* summary, char* fileName){
	

	if(fileName == NULL || strcmpCaseInsen(fileName,"") == 0){
		return 1;
	}

	Calendar* temp = NULL;
	ICalErrorCode check = createCalendar(fileName,&temp);

	if(check != 0){
		deleteCalendar(temp);
		return 1;
	}

	Event* event = malloc(sizeof(Event));
	event->properties = initializeList(printProperty,deleteProperty,compareProperties);
	if(event->properties == NULL){
		return 1;
	}
	event->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);
	if(event->alarms == NULL){
		return 1;
	}



	strcpy(event->UID,UID);

	DateTime* dt = NULL;
	int errorCheck = parseDT(dtstamp,&dt);
	if(errorCheck != 0){
		free(event);
		deleteCalendar(temp);
		return 1;
	}
	event->creationDateTime = *dt;
	free(dt);

	DateTime* dt2 = NULL;
	int errorCheck2 = parseDT(dtstart,&dt2);
	if(errorCheck2 != 0){
		free(event);
		deleteCalendar(temp);
		return 1;
	}
	event->startDateTime = *dt2;
	free(dt2);

	if(summary != NULL){
		Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(summary) + 1));
		strcpy(prop->propName,"SUMMARY");
		//char* des = malloc(sizeof(char) * (strlen(summary) + 1));
		strcpy(prop->propDescr,summary);
		insertBack(event->properties,prop);
	}

	addEvent(temp,event);

	int result = writeCalendar(fileName,temp);

	deleteCalendar(temp);

	return result;
}

int createCalfromUI(char* fileName, char* prodID, float version, char* UID, char* dtstamp, char* dtstart, char* summary){
	
	Calendar* temp = malloc(sizeof(Calendar));
	if(temp == NULL){
		return -1;
	}
	temp->events = initializeList(printEvent,deleteEvent,compareEvents);
	temp->properties = initializeList(printProperty,deleteProperty,compareProperties);

	if(temp->events == NULL || temp->properties == NULL){
		deleteCalendar(temp);
		return -1;
	}

	temp->version = version;

	strcpy(temp->prodID, prodID);

	Event* event = malloc(sizeof(Event));
	event->properties = initializeList(printProperty,deleteProperty,compareProperties);
	if(event->properties == NULL){
		deleteCalendar(temp);
		return -1;
	}
	event->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);
	if(event->alarms == NULL){
		deleteCalendar(temp);
		return -1;
	}

	strcpy(event->UID,UID);

	DateTime* dt = NULL;
	int errorCheck = parseDT(dtstamp,&dt);
	if(errorCheck != 0){
		free(event);
		deleteCalendar(temp);
		return INV_DT;
	}
	event->creationDateTime = *dt;
	free(dt);

	DateTime* dt2 = NULL;
	int errorCheck2 = parseDT(dtstart,&dt2);
	if(errorCheck2 != 0){
		free(event);
		deleteCalendar(temp);
		return INV_DT;
	}
	event->startDateTime = *dt2;
	free(dt2);

	if(summary != NULL){
		Property* prop = malloc(sizeof(Property) + sizeof(char) * (strlen(summary) + 1));
		strcpy(prop->propName,"SUMMARY");
		strcpy(prop->propDescr,summary);
		insertBack(event->properties,prop);
	}

	addEvent(temp,event);

	int result = writeCalendar(fileName,temp);

	deleteCalendar(temp);

	return result;

}

char* eventToJSON(const Event* event){

	if(event == NULL){
		char* strLine = malloc(sizeof(char) * (2 + 1));
		strcpy(strLine,"{}");
		return strLine;
	}

	char text[10000] = "";

	if(event->alarms != NULL && event->properties != NULL){
		Property sumProp;
		strcpy(sumProp.propName,"summary");
		
		Property* prop = (Property*)findElement(event->properties,summaryFind,&sumProp);
		
		char* summDescrip;
		if(prop == NULL || strcmpCaseInsen(prop->propName,"") == 0){
			summDescrip = malloc(sizeof(char) * (0 + 1));
			strcpy(summDescrip,"");
		}else{
			summDescrip = malloc(sizeof(char) * (strlen(prop->propDescr) + 1));
			strcpy(summDescrip,prop->propDescr);
		}

		char* dtText = dtToJSON(event->startDateTime);
		snprintf(text,sizeof(text),"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\"}",dtText,getLength(event->properties) + 3,getLength(event->alarms),summDescrip);
		free(summDescrip);
		free(dtText);

		char* strLine = malloc(sizeof(char) * (strlen(text) + 1));
		if(strLine == NULL){
			return NULL;
		}
		strcpy(strLine,text);

		return strLine;
	}else{
		return NULL;
	}

	char* strLine = malloc(sizeof(char) * (strlen(text) + 1));
	if(strLine == NULL){
		return NULL;
	}
	strcpy(strLine,text);

	return strLine;
}

char* eventListToJSON(const List* eventList){
	
	List* eventList2 = (List*)eventList;

	if(eventList2 == NULL || getLength(eventList2) == 0){
		char* strLine = malloc(sizeof(char) * (2 + 1));
		strcpy(strLine,"[]");
		return strLine;
	}

	char* strLine = malloc(sizeof(char) * (1 + 1));
	strcpy(strLine,"[");

	int numEvents = 0;
	ListIterator iterProp = createIterator(eventList2);
	for(int i = 0; i < getLength(eventList2); i++){
		Event* event = (Event*)nextElement(&iterProp);

		if(event == NULL){
			continue;
		}


		char* eventStr = eventToJSON(event);

		if(eventStr == NULL){
			continue; 
		}

		numEvents++;

		size_t newLen = 0;//= strlen(strLine)+50+strlen(eventStr);
		//strLine = (char*)realloc(strLine, newLen);
		if(i != 0){
			newLen = sizeof(char) * (strlen(strLine)+1+1+strlen(eventStr));
			strLine = (char*)realloc(strLine, newLen);
			strcat(strLine, ",");
		}else{
			newLen = sizeof(char) * (strlen(strLine)+1+strlen(eventStr));;
			strLine = (char*)realloc(strLine, newLen);
		}
		strcat(strLine, eventStr);
		free(eventStr);
	}

	if(numEvents < 1){
		free(strLine);
		strLine = NULL;
		strLine = malloc(sizeof(char) * (2 + 1));
		strcpy(strLine,"[]");
		return strLine;
	}

	int newLen = strlen(strLine)+50;
	strLine = (char*)realloc(strLine, newLen);
	strcat(strLine, "]");

	return strLine;
}

char* getEventAlarms(char* fileName, int eventNumber){
	if(fileName == NULL || strcmpCaseInsen(fileName,"") == 0){
		return NULL;
	}

	Calendar* temp = NULL;
	ICalErrorCode check = createCalendar(fileName,&temp);

	if(check != 0){
		deleteCalendar(temp);
		return NULL;
	}


	if(getLength(temp->events) < eventNumber){
		deleteCalendar(temp);
		return NULL;
	}

	Event* event;
	ListIterator iterProp = createIterator(temp->events);
	for(int c = 0; c < eventNumber; c++){
		event = (Event*)nextElement(&iterProp);
	}

	char* text = malloc(sizeof(char) * (1 + 1));
	strcpy(text,"[");

	ListIterator iterProp2 = createIterator(event->alarms);
	for(int c = 0; c < getLength(event->alarms); c++){
		
		Alarm* alarm = (Alarm*)nextElement(&iterProp2);
		char hold[10000];

		snprintf(hold,sizeof(hold),"{\"action\":\"%s\",\"trigger\":\"%s\"}",alarm->action,alarm->trigger);

		size_t newLen = 0;
		newLen = sizeof(char) * (strlen(text) + strlen(hold) + 1);
		text = (char*)realloc(text, newLen);
		strcat(text,hold);
		

		if((c + 1) == getLength(event->alarms)){

		}else{
			size_t newLen = 0;
			newLen = sizeof(char) * (strlen(text) + 1 + 1);
			text = (char*)realloc(text, newLen);
			strcat(text,",");
		}

		
	}

	deleteCalendar(temp);

	size_t newLen = 0;
	newLen = sizeof(char) * (strlen(text) + 1 + 1);
	text = (char*)realloc(text, newLen);

	strcat(text,"]");

	return text;
}

char* getEventProps(char* fileName, int eventNumber){

	if(fileName == NULL || strcmpCaseInsen(fileName,"") == 0){
		return NULL;
	}

	Calendar* temp = NULL;
	ICalErrorCode check = createCalendar(fileName,&temp);

	if(check != 0){
		deleteCalendar(temp);
		return NULL;
	}


	if(getLength(temp->events) < eventNumber){
		deleteCalendar(temp);
		return NULL;
	}

	Event* event;
	ListIterator iterProp = createIterator(temp->events);
	for(int c = 0; c < eventNumber; c++){
		event = (Event*)nextElement(&iterProp);
	}

	char* text = malloc(sizeof(char) * (1 + 1));
	strcpy(text,"[");

	ListIterator iterProp2 = createIterator(event->properties);
	for(int c = 0; c < getLength(event->properties); c++){
		
		Property* prop = (Property*)nextElement(&iterProp2);
		char hold[10000];

		snprintf(hold,sizeof(hold),"{\"propName\":\"%s\",\"propDescr\":\"%s\"}",prop->propName,prop->propDescr);

		size_t newLen = 0;
		newLen = sizeof(char) * (strlen(text) + strlen(hold) + 1);
		text = (char*)realloc(text, newLen);
		strcat(text,hold);
		

		if((c + 1) == getLength(event->properties)){

		}else{
			size_t newLen = 0;
			newLen = sizeof(char) * (strlen(text) + 1 + 1);
			text = (char*)realloc(text, newLen);
			strcat(text,",");
		}

		
	}

	deleteCalendar(temp);

	size_t newLen = 0;
	newLen = sizeof(char) * (strlen(text) + 1 + 1);
	text = (char*)realloc(text, newLen);

	strcat(text,"]");

	return text;
}

char* getEventList(char* fileName){


	if(fileName == NULL || strcmpCaseInsen(fileName,"") == 0){
		return NULL;
	}

	Calendar* temp = NULL;
	ICalErrorCode check = createCalendar(fileName,&temp);

	if(check != 0){
		deleteCalendar(temp);
		return NULL;
	}

	char* str = eventListToJSON(temp->events);
	deleteCalendar(temp);
	return str;

}

char* fileCaltoJSON(char* fileNames){

	char* json;

	if(fileNames == NULL || strcmpCaseInsen(fileNames,"") == 0){
		return NULL;
	}

	Calendar* temp = NULL;
	ICalErrorCode check = createCalendar(fileNames,&temp);

	if(check != 0){
		
		char text[10000];

		snprintf(text,sizeof(text),"{\"version\":null,\"prodID\":\"invalid file\",\"numProps\":null,\"numEvents\":null}");

		//return "guh";

		json = malloc(sizeof(char) * (strlen(text) + 1));
		strcpy(json,text);

	}else{

		json = calendarToJSON(temp);

		if(json == NULL){
			deleteCalendar(temp);
			return NULL;
		}
	}


	deleteCalendar(temp);


	return json;
}


char* calendarToJSON(const Calendar* cal){

	if(cal == NULL){
		char* strLine = malloc(sizeof(char) * (2 + 1));
		strcpy(strLine,"{}");
		return strLine;
	}
	
	int numEvents;
	int numProps;
	if(cal->events == NULL){
		numEvents = 0;
	}else{
		numEvents = getLength(cal->events);
	}

	if(cal->properties == NULL){
		numProps = 0;
	}else{
		numProps = getLength(cal->properties);
	}

	if(cal->prodID[0] == '\0' || strcmp(cal->prodID,"") == 0){
		char* strLine = malloc(sizeof(char) * (2 + 1));
		strcpy(strLine,"{}");
		return strLine;
	}

	char text[10000] = "";

	snprintf(text,sizeof(text),"{\"version\":%.1f,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",cal->version,cal->prodID,numProps + 2,numEvents);

	char* strLine = malloc(sizeof(char) * (strlen(text) + 1));
	if(strLine == NULL){
		return NULL;
	}
	strcpy(strLine,text);

	return strLine;
}

Calendar* JSONtoCalendar(const char* str){
//{"version":verVal,"prodID":"prodIDVal"
	
	if(str == NULL){
		return NULL;
	}
	
	if(strlen(str) < 27){
		return NULL;
	}

	char* newStr = (char*)str;

	if(newStr[0] != '{' || newStr[1] != '\"'){

		return NULL;
	}

	char* versionStr = getSubString(newStr,2,9);
	if(versionStr == NULL){
		return NULL;
	}

	if(strcmpCaseInsen(versionStr,"version") != 0){
		free(versionStr);
		return NULL;
	}
	free(versionStr);



	int position = 11;
	int firstPos = position;

	while(newStr[position] != ','){
		if(newStr[position] == '\"'){
			return NULL;
		}
		position++;
	}

	if((position - firstPos) < 1){
		return NULL;
	}
	
	char* version = getSubString(newStr,firstPos,position);
	if(version == NULL){

		return NULL;
	}

	char* strTest;
	float ftVersion = strtof(version, &strTest);

	if(strcmpCaseInsen(strTest,"") != 0){
		free(version);
		return NULL;
	}
	free(version);

	position++; //advance to "
	position++; //advance to first char of prodID

	char* prodIDStr = getSubString(newStr,position,position + 6);
	if(prodIDStr == NULL){
		return NULL;
	}
	if(!strcmpCaseInsen(prodIDStr,"prodID") == 0){
		free(prodIDStr);
		return NULL;
	}
	free(prodIDStr);


	if(newStr[position + 6] != '\"' || newStr[position + 6 + 1] != ':' || newStr[position + 6 + 2] != '\"'){
		return NULL;
	}


	position += (6 + 3);

	firstPos = position;
	while(newStr[position] != '"'){
		position++;
	}

	if((position - firstPos) < 1){
		return NULL;
	}

	char* prodID = getSubString(newStr,firstPos,position);

	if(prodID == NULL){
		return NULL;
	}

	//printf("Version:%.0f\nProdID:%s|\n",ftVersion,prodID);

	Calendar* cal = malloc(sizeof(Calendar));
	if(cal == NULL){
		free(prodID);
		free(version);
		return NULL;
	}
	cal->version = ftVersion;
	strcpy(cal->prodID,prodID);
	free(prodID);

	cal->events = initializeList(printEvent,deleteEvent,compareEvents);
	cal->properties = initializeList(printProperty,deleteProperty,compareProperties);


	return cal;
}

Event* JSONtoEvent(const char* str){


	if(str == NULL){
		return NULL;
	}

	if(strlen(str) < 11){ // 4", 2{, 1:, 3UID
		return NULL;
	}

	char* newStr = (char*)str;

	if(newStr[0] != '{' || newStr[1] != '\"'){

		return NULL;
	}

	char* versionStr = getSubString(newStr,2,5);
	if(versionStr == NULL){
		return NULL;
	}

	if(strcmpCaseInsen(versionStr,"UID") != 0){
		return NULL;
	}
	free(versionStr);

	if(newStr[5] != '\"' || newStr[6] != ':' || newStr[7] != '\"'){
		return NULL;
	}

	int position = 8;
	int firstPos = position;
	while(newStr[position] != '\"'){
		position++;
	}

	if((position - firstPos) < 1){
		return NULL;
	}

	char* value = getSubString(newStr,firstPos,position);
	if(value == NULL){
		return NULL;
	}


	Event* event = malloc(sizeof(Event));
	if(event == NULL){
		return NULL;
	}
	strcpy(event->UID,value);
	free(value);

	event->properties = initializeList(printProperty,deleteProperty,compareProperties);
	if(event->properties == NULL){
		return NULL;
	}
	event->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);
	if(event->alarms == NULL){
		return NULL;
	}

	DateTime* dt = malloc(sizeof(DateTime));
	dt->UTC = false;
	
	DateTime* dt2 = malloc(sizeof(DateTime));
	dt2->UTC = false;
/*`
	strcpy(dt->time,"");
	strcpy(dt2->time,"");
	strcpy(dt->date,"");
	strcpy(dt2->date,"");

	event->creationDateTime = *dt;
	free(dt);
	dt = NULL;
	event->startDateTime = *dt2;
	free(dt2);
*/	
	

	return event;
}

void addEvent(Calendar* cal, Event* toBeAdded){

	if(cal == NULL || toBeAdded == NULL){
		return;

	}

	if(cal->events == NULL){
		return;
	}


	insertBack(cal->events,toBeAdded);


	return ;
}