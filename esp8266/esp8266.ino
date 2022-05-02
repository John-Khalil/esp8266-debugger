#include <stdint.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
// #include "spiConsole.cpp"
// #include "LittleFS.h"


void timeTicks(unsigned long timeInUs){
	while(timeInUs){ 
		_NOP();
		timeInUs--;
	}
	return;
}

#define calibratedFreq 100

#define devTick 15.2129f
#define devTickError 0.001f//0.000008f


#define argDataTypeMax 0xFFFFFFFF

#define delayUS(timeInUs) timeTicks((unsigned long)(((timeInUs*devTick)-((timeInUs*devTickError)*(timeInUs>calibratedFreq))+((timeInUs*devTickError)*(timeInUs<calibratedFreq)))))

#define _delay_us delayUS
#define _delay_ms delay
 
// void microSec(unsigned long microSeconds){
// 	_delay_us(microSeconds);
// }

#define WD_FEED _delay_ms(0)

unsigned char WATCH_DOG_GLOBAL_TIMER=0;
#define _WDF if(!(--WATCH_DOG_GLOBAL_TIMER))WD_FEED;

unsigned short WATCH_DOG_GLOBAL_TIMER_16BIT=0;
#define _WDSF if(!(--WATCH_DOG_GLOBAL_TIMER_16BIT))WD_FEED;

#define _DW digitalWrite
#define _DR digitalRead
#define _PM pinMode

// #define fileSystem LittleFS

unsigned char UNDEFINED[10]="undefined";

#define lengthInBytes(object) (unsigned char *)(&object+1)-(unsigned char *)(&object)
unsigned long strToUint32(unsigned char *str) {
	unsigned char coun = 0;
	unsigned long num;
	while (*str) {
		if (((*str) > 0x2F) && ((*str) < 0x3A)) {
			if (!coun)
				num =(unsigned) str;
			coun++;
		}
		else if (coun)
			break;
		str++;
	}
	if (!coun)
		return 0;
	str = (unsigned char*)num;
	unsigned long ret = 0;
	unsigned x = 1;
	while (coun) { //while (coun > 0)
		ret += (str[coun - 1] - 0x30)*x;
		x *= 10;
		coun--;
	}
	return ret;
}

#define HEX_CHARS(FOUR_BIT) (((FOUR_BIT+0x30)*(FOUR_BIT<10))|((FOUR_BIT+0x37)*(FOUR_BIT>9)))
void intToHex(unsigned long decimalFrom,unsigned char *hexaDecimalForm){
	CLR(hexaDecimalForm);
	unsigned char numLength=8-((((decimalFrom&0xFF000000UL)!=0)+((decimalFrom&0x00FF0000UL)!=0)+((decimalFrom&0x0000FF00UL)!=0)+1)*2);
	for(unsigned char i=numLength;i<8;i++){
		unsigned char fourBits=(unsigned char)((decimalFrom&((unsigned long)(0xf0000000>>(4*i))))>>(4*(7-i)));
		*hexaDecimalForm=HEX_CHARS(fourBits);
		hexaDecimalForm++;
	}
	return;
}






unsigned char inttostr(unsigned long num, unsigned char *str) {
	static unsigned char ucoun;
	for (unsigned char clrCoun = 0; clrCoun < ucoun; clrCoun++) {
		str[clrCoun] = 0;
	}
	ucoun = 0;
	if (num) {
		unsigned long sum = 0;
		unsigned long r = 10000000;
		for (unsigned char i = 0; i < 8; i++) {
			unsigned char flg = 0;
			for (unsigned char j = 9; j > 0; j--) {
				if ((num - sum) >= r * j) {
					sum += r * j;
					*str = (j)+0x30;
					ucoun++;
					str++;
					flg = 1;
					break;
				}
			}
			if (flg == 0 && sum != 0) {
				*str = 0x30;
				ucoun++;
				str++;
			}
			r /= 10;
		}
	}
	else {
		*str = 0x30;
		ucoun = 1;
	}
	return ucoun;
}



unsigned char globalStringNameThatYouWillNeverUse[11]="";
unsigned char* inttostring(unsigned long num) {
	CLR(globalStringNameThatYouWillNeverUse);
	inttostr(num, globalStringNameThatYouWillNeverUse);
	return globalStringNameThatYouWillNeverUse;
}

unsigned char *intToHexaDecimal(unsigned long num){
	unsigned char *makeStr=globalStringNameThatYouWillNeverUse;
	*makeStr=0x30;
	makeStr++;
	*makeStr=0x78;
	makeStr++;
	intToHex(num,makeStr);
	return globalStringNameThatYouWillNeverUse;
}


#define C_HEX intToHexaDecimal
#define C_INT inttostring


unsigned char* longToString(long num){
	unsigned char *signedStr=globalStringNameThatYouWillNeverUse;
	if(num<0){
		*signedStr=0x2D;
		signedStr++;
		num*=(-1);
	}
	inttostr((unsigned long)num, signedStr);
	return globalStringNameThatYouWillNeverUse;
}


unsigned strint(unsigned char *str){
	unsigned ret=0;
	unsigned char coun=0;
	while((str[coun])&&(str[coun]>0x2F)&&(str[coun]<0x3A)){
		coun++;
	}
	unsigned x=1;
	while(coun>0){
		ret+=(str[coun-1]-0x30)*x;
		x*=10;
		coun--;
	}
	return ret;
}

uint64_t getInt(unsigned char *numStr){
	unsigned char numLength=stringCounter(numStr);
	uint64_t result=0;
	uint64_t factor=1;
	while(numLength--){_WDF;
		result+=(numStr[numLength]-0x30)*factor;
		factor*=10;
	}
	return result;
}

unsigned char UNDEFINED_VALUE=0;
unsigned char *NO_DATA=&UNDEFINED_VALUE;


unsigned short stringCounter(unsigned char *counted){
	unsigned short counter=0;
	while(*counted){_WDF;
		counter++;
		counted++;
	}
	return counter;
}

unsigned short CLR_LENGTH=0;									//this value will be reseted to zero after clearing the string/uint_8 pointer
unsigned char * CLR(unsigned char *deletedString){
	unsigned char *returnedString=deletedString;
	unsigned short watchDogTimer=200;
	while(*deletedString||(CLR_LENGTH-=(CLR_LENGTH!=0))){
		if(!watchDogTimer){
			WD_FEED;
			watchDogTimer=200;
		}
		watchDogTimer--;
		*deletedString=0;
		deletedString++;	
	}
	return returnedString;
}

unsigned char equalStrings(unsigned char *stringPointer1,unsigned char *stringPointer2){_WDF;
	unsigned short diffCounter;
	if((diffCounter=stringCounter(stringPointer1))!=stringCounter(stringPointer2))
		return 0;
	while((stringPointer1[--diffCounter]==stringPointer2[diffCounter])&&diffCounter)_WDF;
	return !diffCounter;
}

unsigned char *_CS(unsigned char *bigString,unsigned char *smallString){
	unsigned char *smallStringLocation=bigString+stringCounter(bigString);		// lucky for us c/c++ support pointer arthematic
	while(*smallString){_WDF;
		*smallStringLocation=*smallString;
		smallString++;
		smallStringLocation++;
	}
	return bigString;
}

// unsigned char ARRAY_OF_INDEX_GLOBAL[13]="";
// unsigned char *listIndex(unsigned long indexOFArray){
// 	CLR(ARRAY_OF_INDEX_GLOBAL);
// 	unsigned char mkStringCounter=0;
// 	ARRAY_OF_INDEX_GLOBAL[mkStringCounter]=0X5B; // square bracket openning
// 	unsigned char *indexOFArrayString=inttostring(indexOFArray);
// 	while(*indexOFArrayString){_WDF;
// 		ARRAY_OF_INDEX_GLOBAL[++mkStringCounter]=*indexOFArrayString;
// 		indexOFArrayString++;
// 	}
// 	ARRAY_OF_INDEX_GLOBAL[++mkStringCounter]=0x5D;	//square bracket closing
// 	return ARRAY_OF_INDEX_GLOBAL;
// }

unsigned char *skipStringJSON(unsigned char *jsonString){
	if(*jsonString==0x22){//stop searching inside a string
		unsigned char *backSlash=jsonString-1;
		if(*backSlash==0x3A){
			while((*jsonString!=0x22)&&(*backSlash!=0x5C)){WD_FEED;
			backSlash=jsonString-1;
			jsonString++;
			}
		}
	}
	return jsonString;
}

unsigned char*skipArrayJSON(unsigned char *jsonString){
	if(*jsonString==0x5B){//skip an array
		jsonString++;
		unsigned short arrayStart=1;
		unsigned short arrayEnd=0;
		while(arrayStart!=arrayEnd){WD_FEED;
			arrayStart+=(*jsonString==0x5B);
			arrayEnd+=(*jsonString==0x5D);
			jsonString=skipStringJSON(jsonString);
			jsonString++;
		}
	}
	return jsonString;
}

unsigned char *skipObjectJSON(unsigned char *jsonString){
	if(*jsonString==0x7B){//skip an object
		jsonString++;
		unsigned short objectStart=1;
		unsigned short objectEnd=0;
		while(objectStart!=objectEnd){WD_FEED;
			objectStart+=(*jsonString==0x7B);
			objectEnd+=(*jsonString==0x7D);
			jsonString=skipStringJSON(jsonString);
			jsonString++;
		}
	}
	return jsonString;
}

unsigned char *JSON_OBJECT_FOUND_LOCATION=UNDEFINED;
unsigned char JSON(unsigned char *requestedJSON,unsigned char *jsonString,unsigned char *objectString){
	#define OBJECT_STRING_MAX_LENGTH 35
	unsigned char objectBuffer[OBJECT_STRING_MAX_LENGTH]="";
	unsigned short jsonArrayIndex=-1;
	unsigned char subObject=1;
	unsigned char *deadEndOfString=jsonString+stringCounter(jsonString);
	//jsonString++;
	unsigned char *objectScanner=requestedJSON;
	while(*objectScanner){
		subObject+=(*objectScanner==0x2E);
		objectScanner++;
	}
	for(unsigned char subObjectCounter=0;subObjectCounter<subObject;subObjectCounter++){	//start
		jsonString++;
		jsonArrayIndex=-1;
		unsigned char *jsonObject=objectBuffer;
		*jsonObject=0x22;
		jsonObject++;
		while(*requestedJSON&&(*requestedJSON!=0x2E)&&(*requestedJSON!=0x5B)){WD_FEED;
			*jsonObject=*requestedJSON;
			requestedJSON++;
			jsonObject++;
		}
		*jsonObject=0x22;
		jsonObject++;
		while(jsonObject<(objectBuffer+OBJECT_STRING_MAX_LENGTH)){WD_FEED;
			*jsonObject=0;
			jsonObject++;
		}
		jsonObject = objectBuffer;
		if(*requestedJSON==0x2E){
			requestedJSON++;
		}
		else if(*requestedJSON==0x5B){
			requestedJSON++;
			jsonArrayIndex=strint(requestedJSON);
			requestedJSON+=stringCounter(inttostring((unsigned long)jsonArrayIndex))+1;//very inefficiant but i dont care!! its a cleaner code
		}

		while(*jsonObject){WD_FEED; //start searching
			if(*jsonObject==*jsonString){//object to be found
				unsigned char *notString=jsonString-1;
				if(*notString!=0x3A){
					jsonObject++;
				}
			}
			else{
				jsonObject=objectBuffer;
			}
			jsonString=skipStringJSON(jsonString);
			jsonString=skipObjectJSON(jsonString);
			jsonString=skipArrayJSON(jsonString);
			if(jsonString==deadEndOfString){//object not found
				return 0;
			}
			jsonString++;
		}//object found

		jsonString++;//skip the ":"
		unsigned char objectTypeArray=0;
		searchInsideArray:WD_FEED;
		unsigned short objectStartBracket=0;
		unsigned short objectEndBracket=0;
		unsigned short arrayStartBracket=0;
		unsigned short arrayEndBracket=0;
		unsigned short doubleQuoates=0;
		unsigned char *objectLocation=jsonString;

		while(1){WD_FEED;
			objectStartBracket+=(*objectLocation==0x7B);
			objectEndBracket+=(*objectLocation==0x7D);
			arrayStartBracket+=(*objectLocation==0x5B);
			arrayEndBracket+=(*objectLocation==0x5D);
			unsigned char *validStringCheck=objectLocation-1;
			doubleQuoates+=((*validStringCheck!=0x5C)&&(*objectLocation==0x22));
			if(objectLocation==deadEndOfString){//object not found
				return 0;
			}
			while(doubleQuoates&0x01){WD_FEED;
				objectLocation++;
				validStringCheck=objectLocation-1;
				doubleQuoates+=((*validStringCheck!=0x5C)&&(*objectLocation==0x22));
			}
			if((*objectLocation==0x2C) && (objectStartBracket == objectEndBracket) && (arrayStartBracket == arrayEndBracket))
				break;
			if(objectEndBracket>objectStartBracket)
				break;
			if(arrayEndBracket>arrayStartBracket)
				break;
			objectLocation++;
		}
		
		if(objectTypeArray){
			goto backToArrayCounter;
		}
		deadEndOfString=objectLocation;
		if((jsonArrayIndex!=0xFFFF)&&(*jsonString==0x5B)){
			//jsonArrayIndex++;
			jsonString++;
			objectTypeArray=1;
			while(jsonArrayIndex){WD_FEED;
				goto searchInsideArray;
				backToArrayCounter:
				jsonString=objectLocation+1;
				if((*objectLocation!=0x2C)&&jsonArrayIndex!=1){
					return 0;
				}
				jsonArrayIndex--;
			}
			objectTypeArray=0;
			jsonArrayIndex=-1;
			goto searchInsideArray;
		}
	}

	// JSON_OBJECT_FOUND_LOCATION=jsonString;
	while(jsonString<deadEndOfString){WD_FEED;
		*objectString=*jsonString;
		objectString++;
		jsonString++;
	}
	while(*objectString){WD_FEED;
		*objectString=0;
		objectString++;
	}
	return 1;
}

/*
unsigned char JSON2(unsigned char *requestedJSON,unsigned char *jsonString,unsigned char *objectString){
	#define OBJECT_STRING_MAX_LENGTH 35
	unsigned char objectBuffer[OBJECT_STRING_MAX_LENGTH]="";
	unsigned short jsonArrayIndex=-1;
	unsigned char subObject=1;
	unsigned char *deadEndOfString=jsonString+stringCounter(jsonString);
	//jsonString++;
	unsigned char *objectScanner=requestedJSON;
	while(*objectScanner){
		subObject+=(*objectScanner==0x2E);
		objectScanner++;
	}
	for(unsigned char subObjectCounter=0;subObjectCounter<subObject;subObjectCounter++){	//start
		jsonString++;
		jsonArrayIndex=-1;
		unsigned char *jsonObject=objectBuffer;
		*jsonObject=0x22;
		jsonObject++;
		while(*requestedJSON&&(*requestedJSON!=0x2E)&&(*requestedJSON!=0x5B)){WD_FEED;
			*jsonObject=*requestedJSON;
			requestedJSON++;
			jsonObject++;
		}
		*jsonObject=0x22;
		jsonObject++;
		while(jsonObject<(objectBuffer+OBJECT_STRING_MAX_LENGTH)){WD_FEED;
			*jsonObject=0;
			jsonObject++;
		}
		jsonObject = objectBuffer;
		if(*requestedJSON==0x2E){
			requestedJSON++;
		}
		else if(*requestedJSON==0x5B){
			requestedJSON++;
			jsonArrayIndex=strint(requestedJSON);
			requestedJSON+=stringCounter(inttostring((unsigned long)jsonArrayIndex))+1;//very inefficiant but i dont care!! its a cleaner code
		}
		while(*jsonObject){WD_FEED; //start searching
			if(*jsonObject==*jsonString){//object to be found
				unsigned char *notString=jsonString-1;
				if(*notString!=0x3A){
					jsonObject++;
				}
			}
			else{
				jsonObject=objectBuffer;
			}
			if(*jsonString==0x5B){//skip an array
				jsonString++;
				unsigned short arrayStart=1;
				unsigned short arrayEnd=0;
				while(arrayStart!=arrayEnd){WD_FEED;
					arrayStart+=(*jsonString==0x5B);
					arrayEnd+=(*jsonString==0x5D);
					if(*jsonString==0X22){
						unsigned char *backSlash=jsonString-1;
						if(*backSlash==0x3A){
							while((*jsonString!=0x22)&&(*backSlash!=0x5C)){
							backSlash=jsonString-1;
							jsonString++;
							}
						}
					}
					jsonString++;
				}
			}
			if(*jsonString==0x7B){//skip an object
				jsonString++;
				unsigned short objectStart=1;
				unsigned short objectEnd=0;
				while(objectStart!=objectEnd){WD_FEED;
					objectStart+=(*jsonString==0x7B);
					objectEnd+=(*jsonString==0x7D);
					if(*jsonString==0X22){
						unsigned char *backSlash=jsonString-1;
						if(*backSlash==0x3A){
							while((*jsonString!=0x22)&&(*backSlash!=0x5C)){
							backSlash=jsonString-1;
							jsonString++;
							}
						}
					}
					jsonString++;
				}
			}
			if(*jsonString==0x22){//stop searching inside a string
				unsigned char *backSlash=jsonString-1;
				if(*backSlash==0x3A){
					while((*jsonString!=0x22)&&(*backSlash!=0x5C)){WD_FEED;
					backSlash=jsonString-1;
					jsonString++;
					}
				}
			}
			if(jsonString==deadEndOfString){//object not found
				return 0;
			}
			jsonString++;
		}//object found
		jsonString++;//skip the ":"
		unsigned char objectTypeArray=0;
		searchInsideArray:WD_FEED;
		unsigned short objectStartBracket=0;
		unsigned short objectEndBracket=0;
		unsigned short arrayStartBracket=0;
		unsigned short arrayEndBracket=0;
		unsigned short doubleQuoates=0;
		unsigned char *objectLocation=jsonString;
		//while(((((*objectLocation!=0x2C) && (objectStartBracket == objectEndBracket) && (arrayStartBracket == arrayEndBracket))&&((*objectLocation!=0x7D) && (objectStartBracket == objectEndBracket))&&((*objectLocation!=0x5D) && (arrayStartBracket == arrayEndBracket))))||(doubleQuoates&0x01))
		//while((((*objectLocation!=0x2C)&&(*objectLocation!=0x7D)&&(*objectLocation!=0x5D))&&(objectStartBracket==objectEndBracket)&&(arrayStartBracket==arrayEndBracket))||(doubleQuoates&0x01)){
		while(1){WD_FEED;
			objectStartBracket+=(*objectLocation==0x7B);
			objectEndBracket+=(*objectLocation==0x7D);
			arrayStartBracket+=(*objectLocation==0x5B);
			arrayEndBracket+=(*objectLocation==0x5D);
			unsigned char *validStringCheck=objectLocation-1;
			doubleQuoates+=((*validStringCheck!=0x5C)&&(*objectLocation==0x22));
			if(objectLocation==deadEndOfString){//object not found
				return 0;
			}
			while(doubleQuoates&0x01){WD_FEED;
				objectLocation++;
				validStringCheck=objectLocation-1;
				doubleQuoates+=((*validStringCheck!=0x5C)&&(*objectLocation==0x22));
			}
			if((*objectLocation==0x2C) && (objectStartBracket == objectEndBracket) && (arrayStartBracket == arrayEndBracket))
				break;
			if(objectEndBracket>objectStartBracket)
				break;
			if(arrayEndBracket>arrayStartBracket)
				break;
			objectLocation++;
		}
		if(objectTypeArray){
			goto backToArrayCounter;
		}
		deadEndOfString=objectLocation;
		if((jsonArrayIndex!=0xFFFF)&&(*jsonString==0x5B)){
			//jsonArrayIndex++;
			jsonString++;
			objectTypeArray=1;
			while(jsonArrayIndex){WD_FEED;
				goto searchInsideArray;
				backToArrayCounter:
				jsonString=objectLocation+1;
				if((*objectLocation!=0x2C)&&jsonArrayIndex!=1){
					return 0;
				}
				jsonArrayIndex--;
			}
			objectTypeArray=0;
			jsonArrayIndex=-1;
			goto searchInsideArray;
		}
	}
	while(jsonString<deadEndOfString){WD_FEED;
		*objectString=*jsonString;
		objectString++;
		jsonString++;
	}
	while(*objectString){WD_FEED;
		*objectString=0;
		objectString++;
	}
	return 1;
}
//*/

#define JSON_OBJECT_SIZE 101
// unsigned char UNDEFINED[10]="undefined";
unsigned char JSON_OBJECT_FOUND[JSON_OBJECT_SIZE]="";

unsigned char *jsonParse(unsigned char *userObject,unsigned char *jsonObject){
	if(JSON(userObject,jsonObject,JSON_OBJECT_FOUND)){
		if(*JSON_OBJECT_FOUND==0x20){
			unsigned short charSpaceCounter=1;
			while(JSON_OBJECT_FOUND[charSpaceCounter++]==0x22)_WDF;
			charSpaceCounter--;
			unsigned char *charSpacePointer=JSON_OBJECT_FOUND;
			unsigned char *realDataPointer=charSpacePointer+charSpaceCounter;
			while(*realDataPointer){_WDF;
				*charSpacePointer=*realDataPointer;
				charSpacePointer++;
				realDataPointer++;
			}
			CLR(charSpacePointer);
		}
		if((*JSON_OBJECT_FOUND)==0x22){
			unsigned char* makeStrJSON=JSON_OBJECT_FOUND;
			unsigned short jsonObjectLength=stringCounter(makeStrJSON)-2;
			unsigned char *clearString=makeStrJSON+1;
			while(jsonObjectLength){
				(*makeStrJSON)=(*clearString);
				*clearString=0;
				clearString++;
				makeStrJSON++;
				jsonObjectLength--;
			}
			*clearString=0;
		}
		return JSON_OBJECT_FOUND;
	}
	return UNDEFINED;
}

#define json(argJSON1,argJSON2) jsonParse((unsigned char *)argJSON1,(unsigned char *)argJSON2)

// unsigned char *jsonReplace(unsigned char *orgObject,unsigned char *jsonKey,unsigned char *newValue){
// 	while(*JSON_OBJECT_FOUND_LOCATION==0x20){_WDF;
// 		JSON_OBJECT_FOUND_LOCATION++;
// 	}
// 	unsigned char *validJsonKey;
// 	if((validJsonKey=json(jsonKey,orgObject))!=UNDEFINED){_WDF;
// 		unsigned short removedValueLength=stringCounter(validJsonKey)+2*(*JSON_OBJECT_FOUND_LOCATION==0x22);
// 		unsigned short addedValueLength=stringCounter(newValue);
// 		unsigned char *allocationPointer=JSON_OBJECT_FOUND_LOCATION+removedValueLength;
// 		unsigned char *destenationPointer=JSON_OBJECT_FOUND_LOCATION+addedValueLength;
// 		if(removedValueLength>addedValueLength){_WDF;
// 			while(*allocationPointer){_WDF;
// 				*destenationPointer=*allocationPointer;
// 				destenationPointer++;
// 				allocationPointer++;
// 			}
// 			CLR(destenationPointer);
// 		}
// 		else if(removedValueLength<addedValueLength){_WDF;
// 			unsigned short nonValueLength=stringCounter(JSON_OBJECT_FOUND_LOCATION)-removedValueLength;
// 			while(nonValueLength--){_WDF;
// 				destenationPointer[nonValueLength]=allocationPointer[nonValueLength];
// 				allocationPointer[nonValueLength]=0;
// 			}
// 		}
// 		unsigned char *mkFinalObject=JSON_OBJECT_FOUND_LOCATION;
// 		while(*newValue){_WDF;
// 			*mkFinalObject=*newValue;
// 			mkFinalObject++;
// 			newValue++;
// 		}
// 	}
// 	return orgObject;
// }


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char inverseBase64Table(unsigned char transBuf){
	return (transBuf-((65*((transBuf<0x5B)&&(transBuf>0x40)))|(71*((transBuf>0x60)&&(transBuf<0x7B)))|(-4*((transBuf>0x2F)&&(transBuf<0x3A)))|(-19*(transBuf==0x2B))|(-16*(transBuf==0x2F))))*(transBuf!=0x3D);//(char64-((0x41*(char64<26))|(71*((char64>25)&&(char64<52)))|(-4*((char64>51)&&(char64<62)))))
}

unsigned char *base64Decode(unsigned char *base64Text){
	unsigned char *startAddress=base64Text;
	unsigned char *lastAddress=startAddress+stringCounter(startAddress);
	unsigned char *base256Text=base64Text;
	unsigned short base64Counter=0;
	while(base64Text[base64Counter]){
		_WDF;
		unsigned char base256Buffer[4]={inverseBase64Table(base64Text[base64Counter++]),inverseBase64Table(base64Text[base64Counter++]),inverseBase64Table(base64Text[base64Counter++]),inverseBase64Table(base64Text[base64Counter++])};
		*base256Text=(base256Buffer[0]<<2)|((base256Buffer[1]&0x30)>>4);
		base256Text++;
		*base256Text=(base256Buffer[1]<<4)|((base256Buffer[2]&0x3C)>>2);
		base256Text++;
		*base256Text=(base256Buffer[2]<<6)|base256Buffer[3];
		base256Text++;
	}
	while(base256Text<lastAddress){
		*base256Text=0;
		base256Text++;
	}
	return startAddress;
}



//#define base64TableUnused(transBuf) transBuf+((65*(transBuf<26))|(71*((transBuf>25)&&(transBuf<52)))|(-4*((transBuf>51)&&(transBuf<62)))|(-19*(transBuf==62))|(-16*(transBuf==63))) //this was never used.....why?
unsigned char base64Table(unsigned char transBuf){
		return transBuf+((65*(transBuf<26))|(71*((transBuf>25)&&(transBuf<52)))|(-4*((transBuf>51)&&(transBuf<62)))|(-19*(transBuf==62))|(-16*(transBuf==63)));
}
unsigned short base64(unsigned char *rawData, unsigned char *base64Text) {
	static unsigned short lastBase64Length;
	unsigned char *clearText=base64Text;
	while(lastBase64Length){
		*clearText=0;
		clearText++;
		lastBase64Length--;
	}
	unsigned short rawDataLength = stringCounter(rawData);
	unsigned char paddingCount = rawDataLength % 3;
	rawDataLength -= paddingCount;
	// rawDataLength *= 1.25;
	rawDataLength *= 1.3334f;
	unsigned short base64Counter = 0;
	unsigned char *rawData1 = rawData + 1;
	unsigned char *rawData2 = rawData + 2;
	while (base64Counter < rawDataLength) {
		base64Text[base64Counter++] = base64Table((*rawData) >> 2);
		base64Text[base64Counter++] = base64Table((((*rawData) & 0x03) << 4) | ((*rawData1) >> 4));
		base64Text[base64Counter++] = base64Table((((*rawData1) & 0x0F) << 2) | (((*rawData2) & 192) >> 6));
		base64Text[base64Counter++] = base64Table((*rawData2) & 0x3F);
		rawData += 3;
		rawData1 += 3; 
		rawData2 += 3;
	}
	if (paddingCount == 2) {
		base64Text[base64Counter++] = base64Table((*rawData) >> 2);
		base64Text[base64Counter++] = base64Table((((*rawData) & 0x03) << 4) | ((*rawData1) >> 4));
		base64Text[base64Counter++] = base64Table(((*rawData1) & 0x0F) << 2);
		base64Text[base64Counter++] = 0x3D;
	}
	else if (paddingCount == 1) {
		base64Text[base64Counter++] = base64Table((*rawData) >> 2);
		base64Text[base64Counter++] = base64Table(((*rawData) & 0x03) << 4);
		base64Text[base64Counter++] = 0x3D;
		base64Text[base64Counter++] = 0x3D;
	}
	lastBase64Length=base64Counter;
	return base64Counter;
}



unsigned short base64WebSocket(unsigned char *rawData, unsigned char *base64Text,unsigned short rawDataLength ) {
	unsigned char *clearText=base64Text;
	while(*clearText){
		*clearText=0;
		clearText++;
	}
	unsigned char paddingCount = rawDataLength % 3;
	rawDataLength -= paddingCount;
	rawDataLength *= 1.25;
	unsigned short base64Counter = 0;
	unsigned char *rawData1 = rawData + 1;
	unsigned char *rawData2 = rawData + 2;
	while (base64Counter < rawDataLength) {
		base64Text[base64Counter++] = base64Table((*rawData) >> 2);
		base64Text[base64Counter++] = base64Table((((*rawData) & 0x03) << 4) | ((*rawData1) >> 4));
		base64Text[base64Counter++] = base64Table((((*rawData1) & 0x0F) << 2) | (((*rawData2) & 192) >> 6));
		base64Text[base64Counter++] = base64Table((*rawData2) & 0x3F);
		rawData += 3;
		rawData1 += 3; 
		rawData2 += 3;
	}
	if (paddingCount == 2) {
		base64Text[base64Counter++] = base64Table((*rawData) >> 2);
		base64Text[base64Counter++] = base64Table((((*rawData) & 0x03) << 4) | ((*rawData1) >> 4));
		base64Text[base64Counter++] = base64Table(((*rawData1) & 0x0F) << 2);
		base64Text[base64Counter++] = 0x3D;
	}
	else if (paddingCount == 1) {
		base64Text[base64Counter++] = base64Table((*rawData) >> 2);
		base64Text[base64Counter++] = base64Table(((*rawData) & 0x03) << 4);
		base64Text[base64Counter++] = 0x3D;
		base64Text[base64Counter++] = 0x3D;
	}
	return base64Counter;
}

unsigned char* base64Encoder(unsigned char* rawText,unsigned char* base64Text,unsigned short rawDataLength){
	base64WebSocket(rawText,base64Text,rawDataLength);
	return base64Text;
}


unsigned char *secWebSocketAccept(unsigned char *clientBase64){
	unsigned char *originalAddress=clientBase64;
	unsigned char RFC6455[37]="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	unsigned char *readStr=RFC6455;
	clientBase64+=stringCounter(clientBase64);
	while(*readStr){
		*clientBase64=*readStr;
		clientBase64++;
		readStr++;
	}
	clientBase64=originalAddress;
	unsigned char buffer1[21]="";
    sha1((char*)clientBase64,&buffer1[0]);
	unsigned char hashedText[21]="";
	for(unsigned char hashCounter=0;hashCounter<20;hashCounter++){
		hashedText[hashCounter]=buffer1[hashCounter];
	}
	base64WebSocket(hashedText,clientBase64,20);
	return originalAddress;
}




#define FRAME_DECODE 0
#define FRAME_ENCODE 1
#define WEBSOCKET_FRAME_SIZE 1024
unsigned char WEBSOCKET_BUFFER_DATA[WEBSOCKET_FRAME_SIZE]="";
unsigned char FINAL_WEBSOCKET_PAYLOAD=1;
unsigned char WEBSOCKET_LAST_OPCODE=0;

unsigned char *webSocketDataFrame(unsigned char *frameData,unsigned char operation){
	unsigned char *originalLoaction=frameData;
	if(operation==FRAME_DECODE){
		unsigned char *payload=frameData+2;
		unsigned char maskingKey[4]="";
		FINAL_WEBSOCKET_PAYLOAD=((frameData[0]&128)==128);
		unsigned char opcode=(frameData[0]&0x0F);
		WEBSOCKET_LAST_OPCODE=opcode;
		unsigned char dataMasked=((frameData[1]&128)==128);
		unsigned long long payloadLength=(frameData[1]&0x7F);
		payload+=(2*(payloadLength==126));
		payload+=(8*(payloadLength==127));
		unsigned char *maskingKeyLocation=payload;
		payload+=(4*dataMasked);
		if(dataMasked){
			for(unsigned char maskCounter=0;maskCounter<4;maskCounter++){
				maskingKey[maskCounter]=maskingKeyLocation[maskCounter];
			}
		}
		unsigned char extraLength=(2*(payloadLength==126))|(8*(payloadLength==127));
		if(extraLength){
			unsigned char *payloadLengthLocation=maskingKeyLocation-1;
			unsigned char loopCounter=0;
			payloadLength=0;
			while(extraLength){
				payloadLength|=((*payloadLengthLocation)<<(8*loopCounter));
				loopCounter++;
				payloadLengthLocation--;
				extraLength--;
			}
		}
		unsigned long long frameLength=payloadLength+(payload-originalLoaction);
		for(unsigned long long transfarCounter=0;transfarCounter<payloadLength;transfarCounter++){
			*frameData=(*payload);
			frameData++;
			payload++;
		}
		unsigned char *endLocation=originalLoaction+frameLength; //avoid wint conv. bullshit
		while(frameData<endLocation){
			*frameData=0;
			frameData++;
		}
		frameData=originalLoaction;
		if(dataMasked){
			for(unsigned long long transfarCounter=0;transfarCounter<payloadLength;transfarCounter++){
				*frameData^=maskingKey[transfarCounter%4];
				frameData++;
			}
		}
	}
	else if(operation==FRAME_ENCODE){
		originalLoaction=WEBSOCKET_BUFFER_DATA;
		while(*originalLoaction){
			*originalLoaction=0;
			originalLoaction++;
		}
		originalLoaction=WEBSOCKET_BUFFER_DATA;
		*originalLoaction=0x81;
		originalLoaction++;
		unsigned long long payloadSize=stringCounter(frameData);
		payloadSize=(payloadSize*(payloadSize<WEBSOCKET_FRAME_SIZE))|((WEBSOCKET_FRAME_SIZE-1)*(payloadSize>(WEBSOCKET_FRAME_SIZE-1)));
		*originalLoaction=(((unsigned char)payloadSize)*(payloadSize<126))|(126*((payloadSize>125)&&(payloadSize<0x10000ULL)))|(127*(payloadSize>0xFFFF));
		unsigned char extraSize=(2*((*originalLoaction)==126))|(8*((*originalLoaction)==127));
		originalLoaction++;
		while(extraSize){
			extraSize--;
			*originalLoaction=(unsigned char)((payloadSize&(0xFF<<(8*extraSize)))>>(8*extraSize));
			originalLoaction++;
		}
		while(payloadSize){
			*originalLoaction=(*frameData);
			frameData++;
			originalLoaction++;
			payloadSize--;
		}
		originalLoaction=WEBSOCKET_BUFFER_DATA;
	}
	return originalLoaction;
}

unsigned char *webSocketDataFrameEncode(unsigned char *frameData){
	uint64_t payloadSize=stringCounter(frameData);
	unsigned char frameLength=(((unsigned char)payloadSize)*(payloadSize<126))|(126*((payloadSize>125)&&(payloadSize<0x10000ULL)))|(127*(payloadSize>0xFFFF));
	unsigned char extraSize=(2*((frameLength)==126))|(8*((frameLength)==127));
	unsigned char *originalLoaction=frameData-(2+extraSize);
	unsigned char *webSocketData=originalLoaction;
	*originalLoaction=0x81;
	originalLoaction++;
	*originalLoaction=frameLength;
	originalLoaction++;
	while(extraSize){
		extraSize--;
		*originalLoaction=(unsigned char)((payloadSize&(0xFF<<(8*extraSize)))>>(8*extraSize));
		originalLoaction++;
	}
	return webSocketData;
}

unsigned char *webSocketDataFrameEncodeBinary(unsigned char *frameData){
	uint64_t payloadSize=stringCounter(frameData);
	unsigned char frameLength=(((unsigned char)payloadSize)*(payloadSize<126))|(126*((payloadSize>125)&&(payloadSize<0x10000ULL)))|(127*(payloadSize>0xFFFF));
	unsigned char extraSize=(2*((frameLength)==126))|(8*((frameLength)==127));
	unsigned char *originalLoaction=frameData-(2+extraSize);
	unsigned char *webSocketData=originalLoaction;
	*originalLoaction=0x82;
	originalLoaction++;
	*originalLoaction=frameLength;
	originalLoaction++;
	while(extraSize){
		extraSize--;
		*originalLoaction=(unsigned char)((payloadSize&(0xFF<<(8*extraSize)))>>(8*extraSize));
		originalLoaction++;
	}
	return webSocketData;
}

// unsigned char *dataMask(unsigned char *dataStr,unsigned char *keyStr){
// 	unsigned short dataStrCounter=0;
// 	unsigned char keyStrLength=stringCounter(keyStr);
// 	while(dataStr[dataStrCounter])
// 		dataStr[dataStrCounter]^=keyStr[(dataStrCounter++)%keyStrLength];	
// 	return dataStr;
// }

unsigned char *dataMask(unsigned char *dataStr,unsigned char *keyStr){
	unsigned short dataStrCounter=0;
	unsigned char keyStrLength=stringCounter(keyStr);
	unsigned short dataStrLength=stringCounter(dataStr);
	while(dataStrLength--){			
		_WDF;
		dataStr[dataStrCounter]=dataStr[dataStrCounter]^keyStr[(dataStrCounter++)%keyStrLength];
	}	
	return dataStr;
}

unsigned char *tcps(unsigned char *rawText,unsigned char *keyStr,unsigned char *cipherText){
	base64(dataMask(rawText,keyStr),cipherText);
	return cipherText;
}

unsigned long long encKey64(unsigned char *base256Str){
	unsigned char encKeyCounter=0;
	unsigned long long key64Bit=0;
	while(encKeyCounter<8)
		key64Bit|=base256Str[encKeyCounter++]<<(64-(encKeyCounter*8));
	return key64Bit;
}



unsigned char asciiHex(unsigned char *base16){
	unsigned char res=0;
	for(unsigned char i=0;i<2;i++){
		//res=res<<4;
		if(((*base16)>0x2F)&&((*base16)<0x3A)){
			res=res<<4;
			res+=(*base16)-0x30;
		}
		else if(((*base16)>0x60)&&((*base16)<0x67)){
			res=res<<4;
			res+=(*base16)-0x57;
		}
		else if(((*base16)>0x40)&&((*base16)<0x47)){
			res=res<<4;
			res+=(*base16)-0x37;
		}
		base16++;
	}
	return res;
}




unsigned char validHex(unsigned char *base16){
	return((((*base16)>0x2F)&&((*base16)<0x3A))||(((*base16)>0x60)&&((*base16)<0x67))||(((*base16)>0x40)&&((*base16)<0x47)));
}




unsigned char *URIdecode(unsigned char *url){ //this fuction will work without checking if the the chars followed by the percentage char is valid
	unsigned char *originalLocation=url;
	unsigned char *shift;
	unsigned char *allocate;
	while(*url){_WDF;
		if((*url)==0x25){
			shift=url+1;
			//if(validHex(shift)){
				(*url)=asciiHex(shift);
				allocate=shift+2;
				while(*allocate){_WDF;
					(*shift)=(*allocate);
					(*allocate)=0;
					allocate++;
					shift++;
				}
			//}
		}
		url++;
	}
	return originalLocation;
}








#define gpioConsoleLog 1
#define txp2(consoleLogPinState) digitalWrite(gpioConsoleLog,consoleLogPinState)

#define gpioSerialTX 9
#define txp(SerialTXPinState) digitalWrite(gpioSerialTX,SerialTXPinState)

#define gpioSerialRX 3
#define rxp() digitalRead(gpioSerialRX)




void sofser_ini(void){
	pinMode(gpioSerialTX,OUTPUT);
	txp(1);
	pinMode(gpioConsoleLog,OUTPUT);
	txp2(1);
	pinMode(gpioSerialRX,INPUT_PULLUP);
	return;
}



/*
2400>>400.0
9600>>97
19200>>48
38400>>24
74880>>11.1 // write only
the end
*/

#define bdrt 25.8  // found to be working better that way
#define bdrtx bdrt
#define bdrtf bdrtx/3

void sofserS(unsigned char pl){
	txp(0);
	_delay_us(bdrt); ///
	txp((pl&0x01)==0x01);
	_delay_us(bdrt);
	txp((pl&0x02)==0x02);
	_delay_us(bdrt);
	txp((pl&0x04)==0x04);
	_delay_us(bdrt);
	txp((pl&0x08)==0x08);
	_delay_us(bdrt);
	txp((pl&0x10)==0x10);
	_delay_us(bdrt);
	txp((pl&0x20)==0x20);
	_delay_us(bdrt);
	txp((pl&0x40)==0x40);
	_delay_us(bdrt);
	txp((pl&0x80)==0x80);
	_delay_us(bdrt); ///
	txp(1);
	_delay_us(bdrt);
	_delay_us(bdrt);
	return;
}





void sofserS2(unsigned char pl){
	txp2(0);
	_delay_us(bdrt); ///
	txp2((pl&0x01)==0x01);
	_delay_us(bdrt);
	txp2((pl&0x02)==0x02);
	_delay_us(bdrt);
	txp2((pl&0x04)==0x04);
	_delay_us(bdrt);
	txp2((pl&0x08)==0x08);
	_delay_us(bdrt);
	txp2((pl&0x10)==0x10);
	_delay_us(bdrt);
	txp2((pl&0x20)==0x20);
	_delay_us(bdrt);
	txp2((pl&0x40)==0x40);
	_delay_us(bdrt);
	txp2((pl&0x80)==0x80);
	_delay_us(bdrt); ///
	txp2(1);
	_delay_us(bdrt);
	_delay_us(bdrt);
	return;
}


void consoleLogger(unsigned char *t){
	pinMode(gpioConsoleLog,OUTPUT);
	txp2(1);
	_delay_us(bdrt*11);
	while((*t!=0)&&(*t<0x7f)){
		sofserS2(*t);
		t++;
	}
	pinMode(gpioConsoleLog,INPUT_PULLUP);
	return;
}

// #define clkPinNum 16
// #define dataPinNum 10



// void clk(unsigned char state){
// 	_DW(clkPinNum,state);
// }

// void data(unsigned char state){
// 	_DW(dataPinNum,state);
// }

#define spiConsoleClk 16
#define spiConsoleData 10

#define shiftRegisterData 16
#define shiftRegisterClk 5
#define shiftRegisterLatch 4

void spiConsoleSetup(void){
	_PM(spiConsoleClk,OUTPUT);
	_PM(spiConsoleData,OUTPUT);
	_DW(spiConsoleData,1);
	_DW(spiConsoleClk,0);

	_PM(shiftRegisterClk,OUTPUT);
	_PM(shiftRegisterLatch,OUTPUT);
	_DW(shiftRegisterClk,0);
	_DW(shiftRegisterLatch,0);
	
	return;
}





const float SPI_CONSOLE_SPEED=(1e6f/360000)/2;			//max is 1.2Mbps

unsigned char *spiConsoleLog(unsigned char *consoleData){
	unsigned char *loggedData=consoleData;
	_DW(spiConsoleData,0);
	_delay_us(60);										//to sync with the receiver
	while(*consoleData){
		unsigned char spiBitCounter=8;
		while(spiBitCounter--){
			_DW(spiConsoleData,((*consoleData&(1<<spiBitCounter))!=0));
			_delay_us(SPI_CONSOLE_SPEED*0.4);
			_DW(spiConsoleClk,1);
			_delay_us(SPI_CONSOLE_SPEED);
			_DW(spiConsoleClk,0);
			_delay_us(SPI_CONSOLE_SPEED*0.6);
		}
		consoleData++;
	}
	_DW(spiConsoleData,1);
	return loggedData;
}






#define consoleLog(inputStringToConsole) spiConsoleLog((unsigned char *)inputStringToConsole)
#define cout_hex(coutStr,coutNum) consoleLog(coutStr); consoleLog(C_HEX(coutNum)); consoleLog("\r\n");
#define COUT_HEX cout_hex
#define cout_int(coutStr,coutNum) consoleLog(coutStr); consoleLog(C_INT(coutNum)); consoleLog("\r\n");
#define COUT_INT cout_int
#define consoleDebug(VARIABLE_X) consoleLog("\n"); consoleLog(#VARIABLE_X); consoleLog(" >> "); consoleLog(C_INT(VARIABLE_X)); consoleLog("\n"); //WD_FEED;
#define _endl "\r\n"

#define systemLog(consoleLogText0) consoleLog("\n<div style=\"color: yellow;\">");consoleLog(consoleLogText0);consoleLog("</div>");
#define systemError(consoleLogText1) consoleLog("\n<div style=\"color: yellow;\">");consoleLog(consoleLogText1);consoleLog("</div>");

// #define consoleLog(inputStringToConsole) consoleLogger((unsigned char *)inputStringToConsole)
// #define systemLog consoleLog
// #define systemError consoleLog


// const float EXTERNAL_OUTPUT_SPEED=(1e6f/50000)/2;

// unsigned long extrenalPort(unsigned long portBits){
// 	unsigned char bitCounter=32;
// 	while(bitCounter--){_WDF;
// 		_DW(shiftRegisterClk,1);
// 		_DW(shiftRegisterData,(portBits&(1<<bitCounter)));
// 		_delay_us(EXTERNAL_OUTPUT_SPEED);
// 		_DW(shiftRegisterClk,0);
// 		_delay_us(EXTERNAL_OUTPUT_SPEED);
// 	}
// 	_DW(shiftRegisterData,0);

// 	_DW(shiftRegisterLatch,1);
// 	_delay_us(EXTERNAL_OUTPUT_SPEED*10);
// 	_DW(shiftRegisterLatch,0);
// 	_delay_us(EXTERNAL_OUTPUT_SPEED);

// 	return portBits;
// }



void sofser_byte(unsigned char b){
	pinMode(gpioSerialTX,OUTPUT);
	txp(1);
	_delay_us(bdrt*11);
	sofserS(b);
	pinMode(gpioSerialRX,INPUT_PULLUP);

	
	
}






void sofser_txt(unsigned char *t,unsigned char sz){
	pinMode(gpioSerialTX,OUTPUT);
	txp(1);
	_delay_us(bdrt*11);
	unsigned char coun=0;
	while(coun<sz){
		if((*t!=0)&&(*t<0x7f)){
			sofserS(*t);
		}
		else{
			break;
		}
		t++;
		coun++;
	}
	pinMode(gpioSerialRX,INPUT_PULLUP);
	return;
}

void serialString(unsigned char *t){
	pinMode(gpioSerialTX,OUTPUT);
	txp(1);
	_delay_us(bdrt*11);
	while((*t!=0)&&(*t<0x7f)){
		sofserS(*t);
		t++;
	}
	pinMode(gpioSerialRX,INPUT_PULLUP);
	return;
}









void svst(unsigned char *byt,unsigned char bit){
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	*byt|=(bit*rxp());
	return;
}


unsigned char sofserR(){
	unsigned char res=0;
	unsigned short t=1;
	while(rxp()){
		if(t==0){
			return 0;
		}
		
		t++;
	}
	_delay_us(bdrtx+bdrtf);
	res|=(0x01*rxp());
	res|=(0x01*rxp());
	res|=(0x01*rxp());
	res|=(0x01*rxp());
	//res|=(0x01*rxp());
	//res|=(0x01*rxp());
	_delay_us(bdrtx);
	res|=(0x02*rxp());
	res|=(0x02*rxp());
	res|=(0x02*rxp());
	res|=(0x02*rxp());
	//res|=(0x02*rxp());
	//res|=(0x02*rxp());
	_delay_us(bdrtx);
	res|=(0x04*rxp());
	res|=(0x04*rxp());
	res|=(0x04*rxp());
	res|=(0x04*rxp());
	//res|=(0x04*rxp());
	//res|=(0x04*rxp());
	_delay_us(bdrtx);
	res|=(0x08*rxp());
	res|=(0x08*rxp());
	res|=(0x08*rxp());
	res|=(0x08*rxp());
	//res|=(0x08*rxp());
	//res|=(0x08*rxp());
	_delay_us(bdrtx);
	res|=(0x10*rxp());
	res|=(0x10*rxp());
	res|=(0x10*rxp());
	res|=(0x10*rxp());
	//res|=(0x10*rxp());
	//res|=(0x10*rxp());
	_delay_us(bdrtx);
	res|=(0x20*rxp());
	res|=(0x20*rxp());
	res|=(0x20*rxp());
	res|=(0x20*rxp());
	//res|=(0x20*rxp());
	//res|=(0x20*rxp());
	_delay_us(bdrtx);
	res|=(0x40*rxp());
	res|=(0x40*rxp());
	res|=(0x40*rxp());
	res|=(0x40*rxp());
	//res|=(0x40*rxp());
	//res|=(0x40*rxp());
	_delay_us(bdrtx);
	res|=(0x80*rxp());
	res|=(0x80*rxp());
	res|=(0x80*rxp());
	res|=(0x80*rxp());
	//res|=(0x80*rxp());
	//res|=(0x80*rxp());
	while(!rxp());
	
	
	
	
	return res;
}


unsigned char sofserR_txt7(unsigned char *txt){
	static unsigned  lad;
	while(lad>0){
		*txt=0;
		txt++;
		lad--;
	}
	unsigned char fub=0;
	void *ind;
	ind=txt;
	while((fub=sofserR())&&fub!=0&&fub<128){
		*txt=fub;
		txt++;
	}
	lad=(unsigned)txt-(unsigned)ind;
	return lad;
}

unsigned char serialGetString(unsigned char *txt){
	static unsigned  lad;
	unsigned char *ind;
	ind=txt;
	while(lad>0){
		*txt=0;
		txt++;
		lad--;
	}
	unsigned char fub=0;
	txt=ind;
	while(rxp());
	while((fub=sofserR())&&fub!=0&&fub<128){
		*txt=fub;
		txt++;
	}
	lad=(unsigned)txt-(unsigned)ind;
	return lad;
}




unsigned char serialScan(unsigned char *txt){
	unsigned char *frstAdr=txt;
	while(rxp());
	unsigned char buf=0;
	while((buf=sofserR())&&(*txt)){
		if(*txt==buf){
			txt++;
		}
		else{
			txt=frstAdr;
			if(*txt==buf){
				txt++;
			}
		}
	}
	if(!(*txt)){
		return 1;
	}
	return 0;
}

#define GPIO_INPUT (*((volatile uint16_t*)0x60000318))
#define LOGIC_ANALIZER_GPIO() (*((volatile uint8_t*)0x60000319))
#define GPIO_INPUT_LOW_BYTE (*((volatile uint8_t*)0x60000318))


#define LOGIC_ANALIZER_BIT_0 12
#define LOGIC_ANALIZER_BIT_1 13
#define LOGIC_ANALIZER_BIT_2 14
#define LOGIC_ANALIZER_BIT_3 15

#define LOGIC_ANALIZER_INPUT_0 (!digitalRead(LOGIC_ANALIZER_BIT_0))
#define LOGIC_ANALIZER_INPUT_1 (!digitalRead(LOGIC_ANALIZER_BIT_1))
#define LOGIC_ANALIZER_INPUT_2 (!digitalRead(LOGIC_ANALIZER_BIT_2))
#define LOGIC_ANALIZER_INPUT_3 (!digitalRead(LOGIC_ANALIZER_BIT_3))

void logicAnalizerSetup(void){
	pinMode(LOGIC_ANALIZER_BIT_0,INPUT_PULLUP);
	pinMode(LOGIC_ANALIZER_BIT_1,INPUT_PULLUP);
	pinMode(LOGIC_ANALIZER_BIT_2,INPUT_PULLUP);
	pinMode(LOGIC_ANALIZER_BIT_3,INPUT_PULLUP);
	return;
}

#define logicAnalizerPort() (unsigned char)((GPIO_INPUT>>12))//(unsigned char)((0x01*LOGIC_ANALIZER_INPUT_0)|(0x02*LOGIC_ANALIZER_INPUT_1)|(0x04*LOGIC_ANALIZER_INPUT_2)|(0x08*LOGIC_ANALIZER_INPUT_3))
//typedef unsigned long sample_t;
#define sample_t unsigned long
#define THREAD_RATIO 100
#define OVER_FLOW_LIMITER 0xFFF00000UL //this parameter controls how slow the signal can get

sample_t logicAnalizerSampleUnused(void){
	unsigned char unchangedState=logicAnalizerPort();
	uint16_t subMicro=1;
/*
	uint8_t fasterThread=1;
	while(subMicro&&(logicAnalizerPort()==unchangedState)){
		if(!fasterThread){
			WD_FEED;
		}
		fasterThread++;
		subMicro++;		
	}
	//*/
	while(subMicro&&(logicAnalizerPort()==unchangedState)){
		subMicro++;		
	}
	if(!subMicro){
		sample_t counter=(OVER_FLOW_LIMITER+((uint16_t)(-1)));
		while(counter&&(logicAnalizerPort()==unchangedState)){
			counter++;
			if(!(counter%THREAD_RATIO)){
				WD_FEED;
			}
		}
		counter+=((sample_t)(-1))*(!counter);
		return (counter-OVER_FLOW_LIMITER);
	}
	return subMicro;
}


sample_t logicAnalizerSample(void){
	unsigned char unchangedState=logicAnalizerPort();
	sample_t counter=OVER_FLOW_LIMITER;
	#define bit17 0x00010000UL;
	timerStart:
	uint16_t subMicro=1;
	while((logicAnalizerPort()==unchangedState)){
		if(!subMicro)
			break;
		subMicro++;		
	}
	if(!subMicro){
		WD_FEED;
		counter+=bit17;
		if(counter==0xFFFF0000UL){
			return (((sample_t)(-1))-OVER_FLOW_LIMITER)+subMicro;
		}
		goto timerStart;
	}
	return (counter-OVER_FLOW_LIMITER)+subMicro;
}


#define LOGIC_ANALIZER_MAX_BUFFER_SIZE 0x0FFFU

unsigned char *logicAnalizer(unsigned char *logicAnalizerBuffer,unsigned short limiter){
	unsigned char *originalLoaction=logicAnalizerBuffer;
	unsigned char lastState=0;
	unsigned long smallIntervalCounter=0;
	CLR(logicAnalizerBuffer);
	limiter+=LOGIC_ANALIZER_MAX_BUFFER_SIZE*(!limiter);
	
	unsigned short higherThreadRatio;
	if(logicAnalizerPort()){
		*logicAnalizerBuffer=logicAnalizerPort()|0x30;
		logicAnalizerBuffer++;
		*logicAnalizerBuffer=0x7f;
		logicAnalizerBuffer++;
		*logicAnalizerBuffer=0xff;
		logicAnalizerBuffer++;
		*logicAnalizerBuffer=0xff;
		logicAnalizerBuffer++;
		unsigned char logicAnalyzerIdleHigh=logicAnalizerPort();
		higherThreadRatio=(unsigned short)-1;
		superCheckLogicAnalizerSampleIdleHigh:
		while(logicAnalizerPort()==logicAnalyzerIdleHigh){
			if(higherThreadRatio--)
				goto superCheckLogicAnalizerSampleIdleHigh;
			WD_FEED;
		}
	}
	else{
		higherThreadRatio=(unsigned short)-1;
		superCheckLogicAnalizerSampleIdleLow:
		while(!logicAnalizerPort()){
			if(higherThreadRatio--)
				goto superCheckLogicAnalizerSampleIdleLow;
			WD_FEED;
		}
	}
	
	
	while(limiter){
		lastState=logicAnalizerPort();
		sample_t sampleTime=logicAnalizerSample();
		smallIntervalCounter+=sampleTime;
		smallIntervalCounter*=(sampleTime<((unsigned short)-1));
		if(smallIntervalCounter>((uint16_t)-1)){
			WD_FEED;
			smallIntervalCounter=0;
		}
		unsigned char sampleLength=(((sampleTime&0x00FF0000UL)!=0)+((sampleTime&0x0000FF00UL)!=0))+1;//((sampleTime&0xFF000000UL)!=0)+((sampleTime&0x00FF0000UL)!=0)+((sampleTime&0x0000FF00UL)!=0)+1;
		*logicAnalizerBuffer=lastState|((sampleLength)<<4);
		logicAnalizerBuffer++;
		while(sampleLength&&limiter){
			*logicAnalizerBuffer=sampleTime>>(8*(sampleLength-1));
			logicAnalizerBuffer++;
			sampleLength--;
			limiter--;
		}
		limiter--;
		limiter*=(sampleTime!=(((sample_t)(-1))-OVER_FLOW_LIMITER));
	}
	return originalLoaction;
}


unsigned char *playLogicAnalizerSample(unsigned char *compressedSample){
	_PM(LOGIC_ANALIZER_BIT_0,OUTPUT);
	_PM(LOGIC_ANALIZER_BIT_1,OUTPUT);
	_PM(LOGIC_ANALIZER_BIT_2,OUTPUT);
	_PM(LOGIC_ANALIZER_BIT_3,OUTPUT);
	unsigned char *orgLoc=compressedSample;
	volatile uint16_t *outputRegister=(uint16_t*)0x60000300;
	unsigned short pointerAllocator=0;
	while(compressedSample[pointerAllocator]){_WDSF;
		unsigned char baseByte=compressedSample[pointerAllocator++];
		*outputRegister=((uint16_t)(baseByte<<12))|(*outputRegister&0x0FFF);
		baseByte=baseByte>>4;
		unsigned long bitTime=0;
		while(baseByte--){
			bitTime=bitTime<<8;
			bitTime|=compressedSample[pointerAllocator++];
		}
		_delay_us(bitTime*0.2501425f);
	}
	logicAnalizerSetup();
	return orgLoc;
}



unsigned char *superSpeedSample(unsigned char *highSpeedBuffer){
	unsigned char *orgLocation=highSpeedBuffer;
	unsigned short higherThreadRatio;
	if(logicAnalizerPort()){
		unsigned char logicAnalyzerIdleHigh=LOGIC_ANALIZER_GPIO();
		//*highSpeedBuffer=LOGIC_ANALIZER_GPIO();
		//highSpeedBuffer++;
		higherThreadRatio=(unsigned short)-1;
		superCheckLogicAnalizerSuperSpeedSampleIdleHigh:
		while(LOGIC_ANALIZER_GPIO()==logicAnalyzerIdleHigh){
			if(higherThreadRatio--)
				goto superCheckLogicAnalizerSuperSpeedSampleIdleHigh;
			WD_FEED;
		}
	}
	else{
		higherThreadRatio=(unsigned short)-1;
		superCheckLogicAnalizerSuperSpeedSampleIdleLow:
		while(!logicAnalizerPort()){
			if(higherThreadRatio--)
				goto superCheckLogicAnalizerSuperSpeedSampleIdleLow;
			WD_FEED;
		}
	}	
	while(!(*highSpeedBuffer)){
		*highSpeedBuffer=LOGIC_ANALIZER_GPIO();
		highSpeedBuffer++;
	}
	*highSpeedBuffer=0;
	highSpeedBuffer=orgLocation;
	while(*highSpeedBuffer){
		*highSpeedBuffer&=0xF0;
		*highSpeedBuffer|=logicAnalizerPort();
		highSpeedBuffer++;
	}
	return orgLocation;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OUTPUT_PORT_0 0
#define OUTPUT_PORT_1 1
#define OUTPUT_PORT_2 2
#define OUTPUT_PORT_3 3

#define OUTPUT_PORT_4_BIT(REGISTER_DATA) _DW(OUTPUT_PORT_0,(REGISTER_DATA&0x01)) _DW(OUTPUT_PORT_1,(REGISTER_DATA&0x02)) _DW(OUTPUT_PORT_2,(REGISTER_DATA&0x04)) _DW(OUTPUT_PORT_3,(REGISTER_DATA&0x08))



#define SERIAL_PORT() (unsigned char)(GPIO_INPUT_LOW_BYTE&(unsigned char)0x0FU)

#define SERIAL_PORT_BIT(SINGLE_CHANNEL) (unsigned char)(GPIO_INPUT_LOW_BYTE&(unsigned char)SINGLE_CHANNEL)
#define GPIO_INPUT_BIT(SINGLE_PIN) (unsigned char)((GPIO_INPUT&(unsigned short)SINGLE_PIN)!=0)

#define SERIAL_PORT_IDLE() !(SERIAL_PORT()^0x0f)

#define SERIAL_PORT_INPUT_0 0
#define SERIAL_PORT_INPUT_1 1
#define SERIAL_PORT_INPUT_2 2
#define SERIAL_PORT_INPUT_3 3

void serialPortSetup(void){
	pinMode(SERIAL_PORT_INPUT_0,INPUT_PULLUP);
	pinMode(SERIAL_PORT_INPUT_1,INPUT_PULLUP);
	pinMode(SERIAL_PORT_INPUT_2,INPUT_PULLUP);
	pinMode(SERIAL_PORT_INPUT_3,INPUT_PULLUP);
	return;
}

unsigned char SERIAL_PORT_SPI_MODE=0;
unsigned char SPI_MODE_ACTIVE_CHANNEL=1;

/*
unsigned char *quadChannelSpiRead(unsigned char *spiBuffer){
	unsigned char *returnBuffer=spiBuffer;

	unsigned long maxIdleCounter=0x000FFFFFUL;
	while(SERIAL_PORT_IDLE()){
		if(!maxIdleCounter){
			WD_FEED;
			return NO_DATA;
		}
		maxIdleCounter--;
	}
	unsigned short bitValue=~SERIAL_PORT();	

	unsigned char singleBitChecker=0;
	while((!(bitValue&(1<<singleBitChecker++)))&&bitValue);							//handling multipleChannelTrigger
	bitValue=1<<(--singleBitChecker);

	unsigned short clkBitValue=bitValue<<12;
	
	register unsigned long maxIdleTimer;
	register unsigned long superFastCheck;

	unsigned char spiBitsCounter=0;
	unsigned char spiBitCounter;

	while(1){
		spiBitCounter=8;
		while(spiBitCounter--){/*
		//	maxIdleTimer=0x0000FFFFUL;
			
			superFastCheck=0x00001FFFFUL;
			while(!GPIO_INPUT_BIT(clkBitValue)&&(--superFastCheck));
			if(!superFastCheck){
				WD_FEED;
				spiBitCounter++;
				goto timeOut;
			}
			
			
			/*while(!GPIO_INPUT_BIT(clkBitValue)){
				if(!maxIdleTimer){
					WD_FEED;
					spiBitCounter++;
					goto timeOut;
				}
				maxIdleTimer--;
			}//*/
/*
			*spiBuffer=(*spiBuffer)<<1;
			*spiBuffer|=GPIO_INPUT_BIT(bitValue);
			//spiBitsCounter++;
			
			//maxIdleTimer=0x000FFFFFUL;

			spiConsoleReadClkIdleHigh:
			superFastCheck=0x00001FFFFUL;
			while(GPIO_INPUT_BIT(clkBitValue)&&(--superFastCheck));
			if(!superFastCheck){
				WD_FEED;
				//superFastCheck=0;
				goto spiConsoleReadClkIdleHigh;
			}
			*/

			/*while(GPIO_INPUT_BIT(clkBitValue)){
				if(!maxIdleTimer){
					WD_FEED;
					maxIdleTimer=0x000FFFFFUL;
				}
				maxIdleTimer--;
			}//
			//_NOP();//WD_FEED;
		}*/
		//_NOP();//WD_FEED;
		/*
		spiBuffer++;
		if(*spiBuffer){
			*spiBuffer=0;
			break;
		}
	}
	timeOut:
	maxIdleTimer=0x000FFFFFUL;
	while(!GPIO_INPUT_BIT(bitValue)){
		if(!maxIdleTimer){
			WD_FEED;
			maxIdleTimer=0x000FFFFFUL;
		}
		maxIdleTimer--;
	}
	SERIAL_PORT_SPI_MODE=1;
	SPI_MODE_ACTIVE_CHANNEL=singleBitChecker;
	return spiCorrectionAlgorithm((8-spiBitCounter),returnBuffer);
}



*/
/*
unsigned long maxIdleCounter=0x000FFFFFUL;
	while(SERIAL_PORT_IDLE()&&(--maxIdleCounter));
	if(!maxIdleCounter){
		WD_FEED;
		return NO_DATA;
	}
*/



unsigned char *spiCorrectionAlgorithm(unsigned long numberOfBits,unsigned char *spiBuffer){
	unsigned char nonMissingBits=numberOfBits%8;
	if(!nonMissingBits)
		return spiBuffer;
	unsigned char missingBits=8-nonMissingBits;
	unsigned char *currentByte=spiBuffer;
	unsigned char *nextByte=currentByte+1;
	while(*currentByte){
		*currentByte=(*currentByte)<<nonMissingBits;
		*currentByte|=(*nextByte)>>missingBits;
		currentByte++;
		nextByte++;
		// _NOP();//WD_FEED;
		_WDF;
	}
	currentByte--;
	*currentByte=0;
	currentByte--;
	*currentByte=*currentByte<<missingBits;
	return spiBuffer;
}

unsigned char *quadChannelSpiRead(unsigned char *spiBuffer){
	unsigned char *returnBuffer=spiBuffer;

	unsigned long maxIdleCounter=0x0007FFFFUL;
	while(SERIAL_PORT_IDLE()){
		if(!maxIdleCounter){
			WD_FEED;
			return NO_DATA;
		}
		maxIdleCounter--;
	}
	unsigned short bitValue=~SERIAL_PORT();	

	unsigned char singleBitChecker=0;
	while((!(bitValue&(1<<singleBitChecker++)))&&bitValue);							//handling multipleChannelTrigger
	bitValue=1<<(--singleBitChecker);

	unsigned short clkBitValue=bitValue<<12;
	
	register unsigned long maxIdleTimer;
	register unsigned long superFastCheck;

	unsigned char spiBitsCounter=0;
	unsigned char spiBitCounter;

	while(1){
		spiBitCounter=8;
		while(spiBitCounter--){
		//	maxIdleTimer=0x0000FFFFUL;
			
			superFastCheck=0x000000FFFUL;
			while(!GPIO_INPUT_BIT(clkBitValue)&&(--superFastCheck));
			if(!superFastCheck){
				WD_FEED;
				spiBitCounter++;
				goto timeOut;
			}
			
			
			/*while(!GPIO_INPUT_BIT(clkBitValue)){
				if(!maxIdleTimer){
					WD_FEED;
					spiBitCounter++;
					goto timeOut;
				}
				maxIdleTimer--;
			}//*/

			*spiBuffer=(*spiBuffer)<<1;
			*spiBuffer|=GPIO_INPUT_BIT(bitValue);
			//spiBitsCounter++;
			
			//maxIdleTimer=0x000FFFFFUL;

			spiConsoleReadClkIdleHigh:
			superFastCheck=0x000001FFFUL;
			while(GPIO_INPUT_BIT(clkBitValue)&&(--superFastCheck));
			if(!superFastCheck){
				WD_FEED;
				//superFastCheck=0;
				goto spiConsoleReadClkIdleHigh;
			}
			

			/*while(GPIO_INPUT_BIT(clkBitValue)){
				if(!maxIdleTimer){
					WD_FEED;
					maxIdleTimer=0x000FFFFFUL;
				}
				maxIdleTimer--;
			}//*/
			//_NOP();//WD_FEED;
		}
		//_NOP();//WD_FEED;
		spiBuffer++;
		if(*spiBuffer){
			*spiBuffer=0;
			break;
		}
	}
	timeOut:
	maxIdleTimer=0x000FFFFFUL;
	while(!GPIO_INPUT_BIT(bitValue)){
		if(!maxIdleTimer){
			WD_FEED;
			maxIdleTimer=0x000FFFFFUL;
		}
		maxIdleTimer--;
	}
	SERIAL_PORT_SPI_MODE=1;
	SPI_MODE_ACTIVE_CHANNEL=singleBitChecker;
	return spiCorrectionAlgorithm((8-spiBitCounter),returnBuffer);
}



unsigned short SERIAL_DATA_SIZE=0;		  
unsigned char *serialSend(unsigned char channel,unsigned short baudRate,unsigned char *serialData){
	unsigned char serialPortPins[4]={SERIAL_PORT_INPUT_0,SERIAL_PORT_INPUT_1,SERIAL_PORT_INPUT_2,SERIAL_PORT_INPUT_3};
	unsigned char *sentData=serialData;
	baudRate=(1e6*1.019)/baudRate;
	unsigned short serialDataLengthCounter=stringCounter(serialData);
	if(SERIAL_DATA_SIZE)
		serialDataLengthCounter=SERIAL_DATA_SIZE;
	SERIAL_DATA_SIZE=0;
	_PM(serialPortPins[channel],OUTPUT);
	_DW(serialPortPins[channel],1);
	_delay_us(baudRate*11);
	while(serialDataLengthCounter){
		_DW(serialPortPins[channel],0);
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x01));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x02));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x04));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x08));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x10));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x20));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x40));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],(*serialData&0x80));
		_delay_us(baudRate);

		_DW(serialPortPins[channel],1);
		_delay_us(baudRate*2);


		serialData++;
		serialDataLengthCounter--;
	}
	_PM(serialPortPins[channel],INPUT_PULLUP);
	return sentData;
}



sample_t serialPortSample(void){
	unsigned char unchangedState=SERIAL_PORT();
	sample_t counter=OVER_FLOW_LIMITER;
	#define bit17 0x00010000UL;
	serialTimerStart:
	uint16_t subMicro=1;
	while((SERIAL_PORT()==unchangedState)){
		if(!subMicro)
			break;
		subMicro++;		
	}
	if(!subMicro){
		WD_FEED;
		counter+=bit17;
		if(counter==0xFFFF0000UL){
			return (((sample_t)(-1))-OVER_FLOW_LIMITER)+subMicro;
		}
		goto serialTimerStart;
	}
	return (counter-OVER_FLOW_LIMITER)+subMicro;
}

/*
unsigned char *bufferedQuadSerialRead(unsigned char *highSpeedBuffer){
	unsigned char *bufferLocation=highSpeedBuffer;
	serialPortSetup();
	unsigned short maxIdleTimer=-1;
	while(SERIAL_PORT_IDLE()){
		if(!maxIdleTimer){
			WD_FEED;
			return bufferLocation;
		}
		maxIdleTimer--;
	}
	maxIdleTimer=-1;
	while(1){
		*highSpeedBuffer=SERIAL_PORT();
		unsigned short serialSampleDuration=(serialPortSample()*0.2501425F)&0xFFFF;
		*highSpeedBuffer|=(serialSampleDuration&0x0F00)>>8;
		highSpeedBuffer++;
		*highSpeedBuffer=serialSampleDuration&0xFF;
		highSpeedBuffer++;
		if(serialSampleDuration>4095)
			break;
		if(*highSpeedBuffer){
			*highSpeedBuffer=0;
			break;
		}
		if(!maxIdleTimer){
			WD_FEED;
			maxIdleTimer=-1;
		}
	}
	return bufferLocation;
}

//*/
#define realTimeScalling(BIT_TIME) 1.0/(3.99772-(8.405390763188e-6*(1.0/BIT_TIME)))

unsigned long mathRound2(float fraction){
	if((fraction-(float)((unsigned long)fraction))>0.6)
		return (unsigned long)fraction+1;
	return (unsigned long)fraction;
}

#define mathRound3(fraction) ((unsigned long)fraction)+((float)((float)fraction-((unsigned long)fraction))>0.6)

#define mathRound(NUMERATOR,DENOMINATOR,TOLERANCE) ((NUMERATOR+TOLERANCE)/DENOMINATOR)

unsigned char *quadSerialInput(unsigned char **serialBuffer,unsigned short *baudRate){
	SERIAL_PORT_SPI_MODE=0;																				//this will take an argument of the start adreess followed by the end address of each channel  
	unsigned char *serialInput0=serialBuffer[0];
	unsigned char *serialInput1=serialBuffer[1];
	unsigned char *serialInput2=serialBuffer[2];
	unsigned char *serialInput3=serialBuffer[3];


	for(unsigned char serialChannelCounter=0;serialChannelCounter<4;serialChannelCounter++){
		if(!baudRate[serialChannelCounter]){
			unsigned char *deadEndSpiBuffer=serialBuffer[4]-5;
			//deadEndSpiBuffer-=5;
			*deadEndSpiBuffer=0xFF;
			return quadChannelSpiRead(serialBuffer[0]);
		}
		baudRate[serialChannelCounter]=1e6/baudRate[serialChannelCounter];
	}	
	#define toleranceFactor 0.679
	unsigned short compressionTolerance[4]={baudRate[0]*toleranceFactor,baudRate[1]*toleranceFactor,baudRate[2]*toleranceFactor,baudRate[3]*toleranceFactor};
	
	unsigned long syncCounter=0;
	unsigned long smallIntervalCounter=0;

	unsigned long stopTimerChannel0=0;
	unsigned long stopTimerChannel1=0;
	unsigned long stopTimerChannel2=0;
	unsigned long stopTimerChannel3=0;

	unsigned char bitCounter0=0;
	unsigned char bitCounter1=0;
	unsigned char bitCounter2=0;
	unsigned char bitCounter3=0;

	unsigned char activeChannel0=0;
	unsigned char activeChannel1=0;
	unsigned char activeChannel2=0;
	unsigned char activeChannel3=0;

	unsigned long maxIdleTimer=0x000FFFFFUL;
	while(SERIAL_PORT_IDLE()){
		if(!maxIdleTimer){
			WD_FEED;
			return NO_DATA;
		}
		maxIdleTimer--;
	}

	unsigned char activeSerialPort;
	unsigned char serialPortCurrentState=SERIAL_PORT();																												//the very first entery must be zero for any channel
	unsigned long serialSampleDuration=0;	
	activeChannel0|=!(serialPortCurrentState&0x01);
	activeChannel1|=!(serialPortCurrentState&0x02);
	activeChannel2|=!(serialPortCurrentState&0x04);
	activeChannel3|=!(serialPortCurrentState&0x08);

	

	do{																																								//the loop will exit if it ran out of buffer or timed out
		activeSerialPort=0;
		unsigned char serialPortLastState=serialPortCurrentState;
		serialSampleDuration+=serialPortSample();
		syncCounter+=serialSampleDuration*0.2501425;																												//	1.0/3.99772		//syncCounter+=serialSampleDuration*realTimeScalling((float)serialSampleDuration); //something is wrong here
		serialPortCurrentState=SERIAL_PORT();
		serialSampleDuration=3;

		smallIntervalCounter+=serialSampleDuration;
		smallIntervalCounter*=(serialSampleDuration<((unsigned short)-1));
		if(smallIntervalCounter>((uint16_t)-1)){
			WD_FEED;																																				// aprox 6us with esp8266@80MHz (tenselica-xtensa)
			smallIntervalCounter=0;
			serialSampleDuration+=26;																																// baseline 6*3.99772 taking in consideration the real time with the scalling factor 
		}


		if(((serialPortLastState&0x01)!=(serialPortCurrentState&0x01))&&(serialInput0!=serialBuffer[1])&&(activeChannel0)){
			serialSampleDuration+=33;
			activeSerialPort=1;
			unsigned long numberOfBits=mathRound((syncCounter-stopTimerChannel0),baudRate[0],compressionTolerance[0]);														//this thing is absolutely brilliant 
			stopTimerChannel0=syncCounter;
			if((numberOfBits+bitCounter0)>9)
				numberOfBits=(10-bitCounter0);
			unsigned char singleStartBit=(numberOfBits-(!bitCounter0)-((numberOfBits+bitCounter0)>9))*(numberOfBits!=0);																			//why is it called single start bit??
			*serialInput0|=(unsigned char)(((1<<singleStartBit)-1)<<(((bitCounter0-1)*(bitCounter0!=0))))*((serialPortLastState&0x01)!=0);
			bitCounter0+=numberOfBits;
			if(bitCounter0>9){
				bitCounter0=0;
				serialInput0++;
			}
		}
		else if(!activeChannel0)
			stopTimerChannel0=syncCounter;
		activeChannel0|=!(serialPortCurrentState&0x01);

		if(((serialPortLastState&0x02)!=(serialPortCurrentState&0x02))&&(serialInput1!=serialBuffer[2])&&(activeChannel1)){
			serialSampleDuration+=33;
			activeSerialPort=1;
			unsigned long numberOfBits=mathRound((syncCounter-stopTimerChannel1),baudRate[1],compressionTolerance[1]);														
			stopTimerChannel1=syncCounter;
			if((numberOfBits+bitCounter1)>9)
				numberOfBits=(10-bitCounter1);
			unsigned char singleStartBit=(numberOfBits-(!bitCounter1)-((numberOfBits+bitCounter1)>9))*(numberOfBits!=0);																			
			*serialInput1|=(unsigned char)(((1<<singleStartBit)-1)<<(((bitCounter1-1)*(bitCounter1!=0))))*((serialPortLastState&0x02)!=0);
			bitCounter1+=numberOfBits;
			if(bitCounter1>9){
				bitCounter1=0;
				serialInput1++;
			}
		}
		else if(!activeChannel1)
			stopTimerChannel1=syncCounter;
		activeChannel1|=!(serialPortCurrentState&0x02);

		if(((serialPortLastState&0x04)!=(serialPortCurrentState&0x04))&&(serialInput2!=serialBuffer[3])&&(activeChannel2)){
			serialSampleDuration+=33;
			activeSerialPort=1;
			unsigned long numberOfBits=mathRound((syncCounter-stopTimerChannel2),baudRate[2],compressionTolerance[2]);														
			stopTimerChannel2=syncCounter;
			if((numberOfBits+bitCounter2)>9)
				numberOfBits=(10-bitCounter2);
			unsigned char singleStartBit=(numberOfBits-(!bitCounter2)-((numberOfBits+bitCounter2)>9))*(numberOfBits!=0);																			
			*serialInput2|=(unsigned char)(((1<<singleStartBit)-1)<<(((bitCounter2-1)*(bitCounter2!=0))))*((serialPortLastState&0x04)!=0);
			bitCounter2+=numberOfBits;
			if(bitCounter2>9){
				bitCounter2=0;
				serialInput2++;
			}
		}
		else if(!activeChannel2)
			stopTimerChannel2=syncCounter;
		activeChannel2|=!(serialPortCurrentState&0x04);

		if(((serialPortLastState&0x08)!=(serialPortCurrentState&0x08))&&(serialInput3!=serialBuffer[4])&&(activeChannel3)){
			serialSampleDuration+=33;
			activeSerialPort=1;
			unsigned long numberOfBits=mathRound((syncCounter-stopTimerChannel3),baudRate[3],compressionTolerance[3]);														
			stopTimerChannel3=syncCounter;
			if((numberOfBits+bitCounter3)>9)
				numberOfBits=(10-bitCounter3);
			unsigned char singleStartBit=(numberOfBits-(!bitCounter3)-((numberOfBits+bitCounter3)>9))*(numberOfBits!=0);																			
			*serialInput3|=(unsigned char)(((1<<singleStartBit)-1)<<(((bitCounter3-1)*(bitCounter3!=0))))*((serialPortLastState&0x08)!=0);
			bitCounter3+=numberOfBits;
			if(bitCounter3>9){
				bitCounter3=0;
				serialInput3++;
			}
		}
		else if(!activeChannel3)
			stopTimerChannel3=syncCounter;
		activeChannel3|=!(serialPortCurrentState&0x08);

	}while(activeSerialPort);

	return serialBuffer[0];
}


unsigned char *serialPortRead(unsigned char **serialBuffer,unsigned short *baudRate){																				//this will take an argument of the start adreess followed by the end address of each channel  
	unsigned char *serialInput0=serialBuffer[0];
	unsigned char *serialInput1=serialBuffer[1];
	unsigned char *serialInput2=serialBuffer[2];
	unsigned char *serialInput3=serialBuffer[3];

	for(unsigned char serialChannelCounter=0;serialChannelCounter<4;serialChannelCounter++)
		baudRate[serialChannelCounter]=1e6/baudRate[serialChannelCounter];

	unsigned short channelTimer0=0;
	unsigned short channelTimer1=0;
	unsigned short channelTimer2=0;
	unsigned short channelTimer3=0;

	unsigned short channelBuffer0=0;
	unsigned short channelBuffer1=0;
	unsigned short channelBuffer2=0;
	unsigned short channelBuffer3=0;

	unsigned char bitCounter0=0;
	unsigned char bitCounter1=0;
	unsigned char bitCounter2=0;
	unsigned char bitCounter3=0; 

	unsigned short serialPortTimeout=0x7FFF;
	unsigned short wdFeedTimer=-1;

	while(SERIAL_PORT_IDLE())
		WD_FEED;

	unsigned char activeChannel0=!(SERIAL_PORT()&0x01);
	unsigned char activeChannel1=!(SERIAL_PORT()&0x02);
	unsigned char activeChannel2=!(SERIAL_PORT()&0x04);
	unsigned char activeChannel3=!(SERIAL_PORT()&0x08);
	
	while(1){
		if(!wdFeedTimer){
			wdFeedTimer=-1;
			WD_FEED;
		}
		wdFeedTimer--;
		
		if(!channelTimer0&&activeChannel0&&(serialInput0!=serialBuffer[1])){
			channelTimer0=baudRate[0]+(0.3*baudRate[0]*(!bitCounter0));
			channelBuffer0|=(1<<bitCounter0)*((SERIAL_PORT()&0x01)!=0);
			bitCounter0++;
			if(bitCounter0==9){
				bitCounter0=0;
				*serialInput0=(channelBuffer0&0x01FE)>>1;
				serialInput0++;
				channelBuffer0=0;
				activeChannel0=0;
			}
		}

		if(!channelTimer1&&activeChannel1&&(serialInput1!=serialBuffer[2])){
			channelTimer1=baudRate[1]+(0.3*baudRate[1]*(!bitCounter1));
			channelBuffer1|=(1<<bitCounter1)*((SERIAL_PORT()&0x02)!=0);
			bitCounter1++;
			if(bitCounter1==9){
				bitCounter1=0;
				*serialInput1=(channelBuffer1&0x01FE)>>1;
				serialInput1++;
				channelBuffer1=0;
				activeChannel1=0;
			}
		}

		if(!channelTimer2&&activeChannel2&&(serialInput2!=serialBuffer[3])){
			channelTimer2=baudRate[2]+(0.3*baudRate[2]*(!bitCounter2));
			channelBuffer2|=(1<<bitCounter2)*((SERIAL_PORT()&0x04)!=0);
			bitCounter2++;
			if(bitCounter2==9){
				bitCounter2=0;
				*serialInput2=(channelBuffer2&0x01FE)>>1;
				serialInput2++;
				channelBuffer2=0;
				activeChannel2=0;
			}
		}

		if(!channelTimer3&&activeChannel3&&(serialInput3!=serialBuffer[4])){
			channelTimer3=baudRate[3]+(0.3*baudRate[3]*(!bitCounter3));
			channelBuffer3|=(1<<bitCounter3)*((SERIAL_PORT()&0x08)!=0);
			bitCounter3++;
			if(bitCounter3==9){
				bitCounter3=0;
				*serialInput3=(channelBuffer3&0x01FE)>>1;
				serialInput3++;
				channelBuffer3=0;
				activeChannel3=0;
			}
		}

		channelTimer0-=(channelTimer0!=0);
		channelTimer1-=(channelTimer1!=0);
		channelTimer2-=(channelTimer2!=0);
		channelTimer3-=(channelTimer3!=0);

		
		activeChannel0|=!(SERIAL_PORT()&0x01);
		activeChannel1|=!(SERIAL_PORT()&0x02);
		activeChannel2|=!(SERIAL_PORT()&0x04);
		activeChannel3|=!(SERIAL_PORT()&0x08);

		if((!channelTimer0)&&(!channelTimer1)&&(!channelTimer2)&&(!channelTimer3)){
			serialPortTimeout--;
			if(!serialPortTimeout)
				break;
		}
		else
			serialPortTimeout=0x7FFF;

		_delay_us(1);
	}
	
	
	return serialBuffer[0];
}

unsigned char serialRead(unsigned char pinNumber,unsigned short bitTime){
	unsigned char buffer1=0;
	unsigned char buffer2=0;
	unsigned char buffer3=0;

	unsigned short bitTime70=bitTime*0.7F;
	unsigned short bitTime15=bitTime*0.15F;

	pinNumber=1<<pinNumber;
	unsigned short watchDogTimer=-1;
	while((GPIO_INPUT&pinNumber)){
		if(!watchDogTimer){
			WD_FEED;
			return 0;
		}
		watchDogTimer--;
	}

	_delay_us((bitTime*0.35));

	_delay_us(bitTime70);
	buffer1|=(0x01*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x01*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x01*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime70);
	buffer1|=(0x02*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x02*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x02*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime70);
	buffer1|=(0x04*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x04*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x04*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime70);
	buffer1|=(0x08*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x08*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x08*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime);
	buffer1|=(0x10*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x10*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x10*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime70);
	buffer1|=(0x20*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x20*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x20*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime70);
	buffer1|=(0x40*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x40*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x40*((GPIO_INPUT&pinNumber)!=0));

	_delay_us(bitTime70);
	buffer1|=(0x80*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer2|=(0x80*((GPIO_INPUT&pinNumber)!=0));
	_delay_us(bitTime15);
	buffer3|=(0x80*((GPIO_INPUT&pinNumber)!=0));

	unsigned char finalResult=0;
	for(unsigned char bitCounter=0;bitCounter<8;bitCounter++){
		unsigned char highCounter=((buffer1&(1<<bitCounter))!=0)+((buffer2&(1<<bitCounter))!=0)+((buffer3&(1<<bitCounter))!=0);
		unsigned char lowCouter=3-highCounter;
		finalResult|=((1<<bitCounter)*(highCounter>lowCouter));
	}

	watchDogTimer=-1;
	while(!(GPIO_INPUT&pinNumber)){
		if(!watchDogTimer){
			watchDogTimer=-1;
			WD_FEED;
		}
		watchDogTimer--;
	}
	return finalResult;
}

sample_t serialPortSingleBitSample(unsigned char serialPortBit){
	unsigned char unchangedState=SERIAL_PORT_BIT(serialPortBit);
	sample_t counter=OVER_FLOW_LIMITER;
	#define bit17 0x00010000UL;
	serialTimerStart:
	uint16_t subMicro=1;
	while((SERIAL_PORT_BIT(serialPortBit)==unchangedState)){
		if(!subMicro)
			break;
		subMicro++;		
	}
	if(!subMicro){
		WD_FEED;
		counter+=bit17;
		if(counter==0xFFFF0000UL){
			return (((sample_t)(-1))-OVER_FLOW_LIMITER)+subMicro;
		}
		goto serialTimerStart;
	}
	return (counter-OVER_FLOW_LIMITER)+subMicro;
}


unsigned char *serialReadString(unsigned char *serialBuffer,unsigned short *baudRate){
	for(unsigned char serialChannelCounter=0;serialChannelCounter<4;serialChannelCounter++)
		baudRate[serialChannelCounter]=1e6/baudRate[serialChannelCounter];
	
	unsigned short watchDogTimer=-1;
	while(SERIAL_PORT_IDLE()){
		if(!watchDogTimer){
			watchDogTimer=-1;
			WD_FEED;
		}
		watchDogTimer--;
	}
	watchDogTimer=-1;

	unsigned char channelSelelctor=~SERIAL_PORT();
	if(channelSelelctor&0x01)
		channelSelelctor=0;
	else if(channelSelelctor&0x02)
		channelSelelctor=1;
	else if(channelSelelctor&0x04)
		channelSelelctor=2;
	else if(channelSelelctor&0x08)
		channelSelelctor=3;

	unsigned char buffer;
	while(buffer=serialRead(channelSelelctor,baudRate[channelSelelctor])){
		if(!watchDogTimer){
			watchDogTimer=-1;
			WD_FEED;
		}
		watchDogTimer--;

		*serialBuffer=buffer;
		serialBuffer++;
	}

	
	return serialBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char GLOBAL_IPADDRESS_VARIABLE_NAME[16]="";
unsigned char * ipAddressString(IPAddress deviceIP){
		unsigned char *ipString=GLOBAL_IPADDRESS_VARIABLE_NAME;
	while(*ipString){
		*ipString=0;
		ipString++;
	}
	ipString=GLOBAL_IPADDRESS_VARIABLE_NAME;
	unsigned char byteCounter=0;
	byteCounterEqual4:
	unsigned char *byteCounterString=inttostring((unsigned long)deviceIP[byteCounter]);
	while(*byteCounterString){
		*ipString=*byteCounterString;
		ipString++;
		byteCounterString++;
	}
	if(byteCounter!=3){
		byteCounter++;
		*ipString=0x2E;
		ipString++;
		goto byteCounterEqual4;
	}
	return GLOBAL_IPADDRESS_VARIABLE_NAME;  
}








#define MAX_EEPROM_SIZE 512U
#define SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 63
#define WIFI_CONFIG 0

#define LINKER_CONFIG WIFI_CONFIG+SSID_MAX_LENGTH+WIFI_PASSWORD_MAX_LENGTH
#define LINKER_CONFIG_LENGTH 100

#define EXTERNAL_PORT_CONFIG LINKER_CONFIG+LINKER_CONFIG_LENGTH
#define EXTERNAL_PORT_LENGTH 8

#define EEPROM_BUFFER_SIZE 200
unsigned char EEPROM_BUFFER[EEPROM_BUFFER_SIZE]="";



unsigned char ssidSave(unsigned char *networkSSID){
	unsigned short startAddress=WIFI_CONFIG;
	while(startAddress<(WIFI_CONFIG+SSID_MAX_LENGTH)){
		EEPROM.write(startAddress&((MAX_EEPROM_SIZE-1)),*networkSSID);
		startAddress++;
		networkSSID+=(*networkSSID!=0);
	}
	return (unsigned char)EEPROM.commit();
}


unsigned char wifiPasswordSave(unsigned char *networkPassword){
	unsigned short startAddress=WIFI_CONFIG+SSID_MAX_LENGTH;
	while(startAddress<(WIFI_CONFIG+SSID_MAX_LENGTH+WIFI_PASSWORD_MAX_LENGTH)){
		EEPROM.write(startAddress&((MAX_EEPROM_SIZE-1)),*networkPassword);
		startAddress++;
		networkPassword+=(*networkPassword!=0);
	}
	return (unsigned char)EEPROM.commit();
}



unsigned char NETWORK_SSID[SSID_MAX_LENGTH+1]="";
unsigned char NETWORK_PASSWORD[WIFI_PASSWORD_MAX_LENGTH+1]="";

unsigned char *userSSID(void){
	unsigned char *getSSID=NETWORK_SSID;
	unsigned short startAddress=WIFI_CONFIG;
	while(startAddress<(WIFI_CONFIG+SSID_MAX_LENGTH)){
		*getSSID=EEPROM.read(startAddress);
		getSSID++;
		startAddress++;
	}
	return NETWORK_SSID;
}


unsigned char *userPassword(void){
	unsigned char *getPassword=NETWORK_PASSWORD;
	unsigned short startAddress=WIFI_CONFIG+SSID_MAX_LENGTH;
	while(startAddress<(WIFI_CONFIG+SSID_MAX_LENGTH+WIFI_PASSWORD_MAX_LENGTH)){
		*getPassword=EEPROM.read(startAddress);
		getPassword++;
		startAddress++;
	}
	return NETWORK_PASSWORD;
}

unsigned char linkerSave(unsigned char *linkerJson){
	unsigned short startAddress=LINKER_CONFIG;
	while(startAddress<(LINKER_CONFIG+LINKER_CONFIG_LENGTH)){
		EEPROM.write(startAddress&((MAX_EEPROM_SIZE-1)),*linkerJson);
		startAddress++;
		linkerJson+=(*linkerJson!=0);
	}
	return (unsigned char)EEPROM.commit();
}

unsigned char *linkerData(void){
	CLR_LENGTH=EEPROM_BUFFER_SIZE;
	CLR(EEPROM_BUFFER);
	unsigned char *getLinkerData=EEPROM_BUFFER;
	unsigned short startAddress=LINKER_CONFIG;
	while(startAddress<(LINKER_CONFIG+LINKER_CONFIG_LENGTH)){
		*getLinkerData=EEPROM.read(startAddress);
		getLinkerData++;
		startAddress++;
	}
	return EEPROM_BUFFER;
}

unsigned char *EXTERNAL_PORT_LAST_VALUE=EEPROM_BUFFER;
const float EXTERNAL_OUTPUT_SPEED=(1e6f/50000)/2;
unsigned char EXTERNAL_PORT_SET_COUNTER=1;

unsigned char *externalPort(void){
	static unsigned char externalPortOnChange;
	CLR_LENGTH=EEPROM_BUFFER_SIZE;
	CLR(EEPROM_BUFFER);
	unsigned char externalPortBuffer[EXTERNAL_PORT_LENGTH];
	unsigned char externalPortLength=EXTERNAL_PORT_LENGTH;
	while(externalPortLength--){_WDF;
		externalPortBuffer[externalPortLength]=EEPROM.read(EXTERNAL_PORT_CONFIG+externalPortLength);
	}
	
	if(externalPortOnChange!=EXTERNAL_PORT_SET_COUNTER){
		externalPortLength=EXTERNAL_PORT_LENGTH;
		while(externalPortLength--){_WDF;
			unsigned char shiftRegisterByte=EEPROM.read(EXTERNAL_PORT_CONFIG+externalPortLength);
			unsigned char bitCounter=8;
			while(bitCounter--){
				_DW(shiftRegisterData,(shiftRegisterByte&(1<<bitCounter)));
				_DW(shiftRegisterClk,1);
				_delay_us(EXTERNAL_OUTPUT_SPEED);
				_DW(shiftRegisterClk,0);
				_delay_us(EXTERNAL_OUTPUT_SPEED);
			}
		}
		_DW(shiftRegisterData,0);

		_DW(shiftRegisterLatch,1);
		_delay_us(EXTERNAL_OUTPUT_SPEED*10);
		_DW(shiftRegisterLatch,0);
		_delay_us(EXTERNAL_OUTPUT_SPEED);
	}
	externalPortOnChange=EXTERNAL_PORT_SET_COUNTER;

	unsigned char *getExternalPortObject=EEPROM_BUFFER;
	const char *constExternalPortObjectStart="{\"externalPort\":[";
	unsigned char *externalPortObject=(unsigned char*)constExternalPortObjectStart;
	while(*externalPortObject){_WDF;
		*getExternalPortObject=*externalPortObject;
		getExternalPortObject++;
		externalPortObject++;
	}
	externalPortLength=0;
	while(externalPortLength<EXTERNAL_PORT_LENGTH){_WDF;
		externalPortObject=inttostring(externalPortBuffer[externalPortLength++]);
		while(*externalPortObject){
			*getExternalPortObject=*externalPortObject;
			getExternalPortObject++;
			externalPortObject++;
		}
		*getExternalPortObject=0x2C*(externalPortLength<EXTERNAL_PORT_LENGTH);
		getExternalPortObject+=(externalPortLength<EXTERNAL_PORT_LENGTH);
	}
	*getExternalPortObject=0x5D;
	getExternalPortObject++;
	*getExternalPortObject=0x7D;
	getExternalPortObject++;

	return EEPROM_BUFFER;
}

unsigned char *externalPortSet(unsigned char *externalPortArrayObject){
	unsigned char *orgLocation=externalPortArrayObject;
	unsigned char *finalInt;
	unsigned char arrayIndexBuffer[19]="externalPort[";
	unsigned char *arrayIndexReset=arrayIndexBuffer+stringCounter(arrayIndexBuffer);
	if(json("externalPort",externalPortArrayObject)!=UNDEFINED){
		EXTERNAL_PORT_SET_COUNTER++;
		unsigned short startAddress=EXTERNAL_PORT_CONFIG;
		unsigned char arrayIndex=0;
		while(startAddress<EXTERNAL_PORT_CONFIG+EXTERNAL_PORT_LENGTH){_WDF;
			unsigned char *mkIndexStr=arrayIndexReset;
			CLR(mkIndexStr);
			unsigned char *arrayIndexStr=inttostring(arrayIndex++);
			while(*arrayIndexStr){
				*mkIndexStr=*arrayIndexStr;
				mkIndexStr++;
				arrayIndexStr++;
			}
			*mkIndexStr=0x5D;
			mkIndexStr++;
			if((finalInt=json(arrayIndexBuffer,externalPortArrayObject))!=UNDEFINED){
				EEPROM.write(startAddress&((MAX_EEPROM_SIZE-1)),(strint(finalInt)&0xff));
			}
			startAddress++;
		}
		EEPROM.commit();
	}
	return orgLocation;
}

unsigned char *externalPortBitToggle(unsigned char bitNumber){
	unsigned char bitValue=1<<(bitNumber%8);
	unsigned char byteIndex=EXTERNAL_PORT_CONFIG+(bitNumber/8)%8;
	EEPROM.write(byteIndex,(EEPROM.read(byteIndex)^bitValue));
	EEPROM.commit();
	EXTERNAL_PORT_SET_COUNTER++;
	return externalPort();
}

unsigned char FIRST_RUN=0;
void eepromInit(void){
	EEPROM.begin(MAX_EEPROM_SIZE);
	if(EEPROM.read((MAX_EEPROM_SIZE-1))==0xFF){
		for(unsigned short clearCounter=0;clearCounter<MAX_EEPROM_SIZE;clearCounter++){
			EEPROM.write(clearCounter,0);
		}
		EEPROM.commit();
		FIRST_RUN=1;
		wifiPasswordSave((unsigned char*)"12345678");
		linkerSave(UNDEFINED);
	}
	return;
}
/*
String scanForWifi_didnt_work(void){
	unsigned char n = WiFi.scanNetworks();
	String availableNetworks="\"network\":{[\"";
	for (unsigned char i = 0; i < n; ++i) {
		WD_FEED;
		availableNetworks+=WiFi.SSID(i);
		availableNetworks+="\",";
		availableNetworks+=WiFi.RSSI(i);
		if(i!=(n-1))
			availableNetworks+=",\"";
	}
	availableNetworks+="]}";
	return availableNetworks;
}
*/
unsigned char *scanForWifi2(unsigned char *networksObject){
	unsigned char *orgLocation=networksObject;
	int n = WiFi.scanNetworks();
	String availableNetworks="{[";
	for (unsigned char i = 0; i < n; ++i) {
		WD_FEED;
		availableNetworks+=WiFi.SSID(i);
		availableNetworks+=",";
		availableNetworks+=WiFi.RSSI(i);
		if(i!=(n-1))
			availableNetworks+=",";
	}
	availableNetworks+="]}";
	unsigned char *mkstr=(unsigned char *)availableNetworks.c_str();
	while(*mkstr){
		WD_FEED;
		(*networksObject)=(*mkstr);
		networksObject++;
		mkstr++;
	}
	return orgLocation;
}

unsigned char *scanForWifi(unsigned char *networksObject){
	unsigned char *orgLocation=networksObject;
	int n = WiFi.scanNetworks();
	const unsigned char baseStr[14]="{\"network\":[\"";
	unsigned char *makeBaseStr=(unsigned char *)baseStr;
	while(*makeBaseStr){
		WD_FEED;
		*networksObject=(*makeBaseStr);
		makeBaseStr++;
		networksObject++;
	}
	for (unsigned char i = 0; i < n; ++i) {
		WD_FEED;
		
		unsigned char scannedSSIDLength=WiFi.SSID(i).length();
		for(unsigned char makeObject=0;makeObject<scannedSSIDLength;makeObject++){
			WD_FEED;
			*networksObject=WiFi.SSID(i)[makeObject];
			networksObject++;
		}

		*networksObject=0x22;
		networksObject++;
		*networksObject=0x2C;
		networksObject++;

		unsigned char *scannedRSSI=longToString(WiFi.RSSI(i));
		unsigned char avoidErrorCounter=0;
		while(*scannedRSSI&&avoidErrorCounter<3){
			WD_FEED;
			*networksObject=(*scannedRSSI);
			scannedRSSI++;
			networksObject++;
			avoidErrorCounter++;
		}

		if(i!=(n-1)){
			*networksObject=0x2C;
			networksObject++;
			*networksObject=0x22;
			networksObject++;
		}
	}
	*networksObject=0x5D;
	networksObject++;
	*networksObject=0x7D;
	networksObject++;

	while(*networksObject){
		*networksObject=0;
		networksObject++;
	}


	
	return orgLocation;
}

unsigned short networkConnect(void){
	WD_FEED;
	unsigned char *ssid=userSSID();
	unsigned char *password=userPassword();
	unsigned short exitStatus;
	WiFi.mode(WIFI_STA);
	WiFi.softAP("",(char*)password);
	WiFi.softAP((char*)ipAddressString(WiFi.softAPIP()),(char*)password);
	if(stringCounter(ssid)){
		WiFi.begin((char*)ssid,(char*)password);
		unsigned short connectTimeout=0x8000;
		while ((exitStatus=WiFi.status()) != WL_CONNECTED){
			//ESP.wdtFeed();
			WD_FEED;
			if(!connectTimeout){
				// WiFi.disconnect();
				return exitStatus;
			}
			_delay_ms(1);
			connectTimeout++;
		}
		WiFi.setAutoReconnect(true);
		WiFi.persistent(true);
		WiFi.softAP((char*)ipAddressString(WiFi.localIP()),(char*)password);
	}
	return exitStatus;
}


unsigned char *tcpGetString(WiFiClient &client,unsigned char *memoryBuffer){
	unsigned char *tcpBuffer=memoryBuffer;
	unsigned short watchDogTimer=0;
	while(client.available()){
		if(!(--watchDogTimer))
			WD_FEED;
		if(!(*memoryBuffer)){
			*memoryBuffer=client.read();
			memoryBuffer++;
		}	
		else{
			client.read();
		}
	}
	*memoryBuffer=0;
	return tcpBuffer;
}

unsigned char USER_KEY[21]="";

unsigned char serverConnect(WiFiClient &socket){
	unsigned char *remoteHostObject=linkerData();
	if((json("D",remoteHostObject)!=UNDEFINED)&&(json("P",remoteHostObject)!=UNDEFINED)&&(json("K",remoteHostObject)!=UNDEFINED)&&(json("U",remoteHostObject)!=UNDEFINED)){
		CLR(USER_KEY);
		unsigned char *mkUserKey=USER_KEY;
		unsigned char *getUserKey=base64Decode(json("K",remoteHostObject));
		while(*getUserKey){_WDF;
			*mkUserKey=*getUserKey;
			mkUserKey++;
			getUserKey++;
		}
		unsigned short socketPort=strint(json("P",remoteHostObject));
		if(!(socket.connect((char*)json("D",remoteHostObject),socketPort))){
			socket.stop();
			consoleLog("failed to connect\n");
			return 0;
		}
		_delay_ms(150);
		socket.write((char*)json("U",remoteHostObject));
		socket.write("new xtensa connection");
		consoleLog("server connected\n");
		return 1;
	}
	consoleLog("invalid network object \n");
	return 0;
}

#define USER_REQUEST 0x01
#define WEB_SOCKET 0x02
#define UPLOADED_DATA 0x04
#define WEB_SERVER 0x08
unsigned char *EVENT_DATA;



unsigned char eventIdentifier(unsigned char *userRequest){
	unsigned char str1[5]="def:";
	unsigned char str2[20]="Sec-WebSocket-Key: ";
	unsigned char str3[6]="data:";
	unsigned char str4[10]="GET /home";
	#define cl1 4
	#define cl2 19
	#define cl3 5
	#define cl4 9
	unsigned char c1=0;
	unsigned char c2=0;
	unsigned char c3=0;
	unsigned char c4=0;
	while((cl1-c1)&&(cl2-c2)&&(cl3-c3)&&(cl4-c4)&&(*userRequest)){
		c1*=((*userRequest)==str1[c1++]);
		c2*=((*userRequest)==str2[c2++]);
		c3*=((*userRequest)==str3[c3++]);
		c4*=((*userRequest)==str4[c4++]);
		userRequest++;
	}
	EVENT_DATA=userRequest;
	return((USER_REQUEST*(c1==cl1))|(WEB_SOCKET*(c2==cl2))|(UPLOADED_DATA*(c3==cl3))|(WEB_SERVER*(c4==cl4)));
}



unsigned char *eventData(unsigned char* startLocation,unsigned char* originalString,unsigned char *endString){
	unsigned char *endLocation=startLocation;
	unsigned short endStringLength=stringCounter(endString);
	unsigned short valueMatchCounter=0;
	while((endStringLength-valueMatchCounter)&&(*endLocation)){
		valueMatchCounter*=((*endLocation)==endString[valueMatchCounter++]);
		endLocation++;
	}
	if(endStringLength==valueMatchCounter){
		endLocation-=endStringLength;
		unsigned char *makeStr=originalString;
		while(startLocation<endLocation){
			*makeStr=(*startLocation);
			makeStr++;
			startLocation++;
		}
		while(*makeStr){
			*makeStr=0;
			makeStr++;
		}
	}
	return originalString;
}

unsigned char responeseHeaders=0;
#define fecthHeadersEnable() responeseHeaders=1
#define fetchHeadersDisable() responeseHeaders=0

unsigned char *urlEncode(unsigned char *originalUrl){
	const unsigned char urlSpecialChars[23]="-._~:/?#[]@!$&'()*+,;=";
	unsigned char *originalUrlStartLocation=originalUrl;
	while(*originalUrlStartLocation){_WDF;
		if(!(((*originalUrlStartLocation>0x40)&&(*originalUrlStartLocation<0x5B))||((*originalUrlStartLocation>0x60)&&(*originalUrlStartLocation<0x7B))||((*originalUrlStartLocation>0x2F)&&(*originalUrlStartLocation<0x3A))||((*originalUrlStartLocation==0x25)&&validHex(originalUrlStartLocation+1)&&validHex(originalUrlStartLocation+2)))){
			unsigned char specialCharCounter=sizeof(urlSpecialChars);
			while((*originalUrlStartLocation!=urlSpecialChars[--specialCharCounter])&&specialCharCounter)_WDF;
			if(*originalUrlStartLocation!=urlSpecialChars[specialCharCounter]){
				unsigned char *urlEncodedValue=intToHexaDecimal(*originalUrlStartLocation)+1;
				urlEncodedValue[0]=0x25;
				unsigned short originalUrlStartLocationLength=stringCounter(originalUrlStartLocation);
				while(originalUrlStartLocationLength--){_WDF;
					originalUrlStartLocation[originalUrlStartLocationLength+2]=originalUrlStartLocation[originalUrlStartLocationLength];
					originalUrlStartLocation[originalUrlStartLocationLength]=0;
				}
				_CS(originalUrlStartLocation,urlEncodedValue);
			}
		}
		originalUrlStartLocation++;
	}
	return originalUrl;
}

struct httpLink{
	unsigned char domain[40]="";
	unsigned short port;
	unsigned char secure;
	unsigned char userName[30]="";
	unsigned char password[30]="";
	unsigned char authDefined=0;
	unsigned char *requestPath=UNDEFINED;
};

struct httpLink urlBreakDown(unsigned char *httpRequest){WD_FEED;
	struct httpLink urlParameters;
	unsigned short httpRequestLength=stringCounter(httpRequest);

	unsigned short requestPathLocatorLimit=httpRequestLength;
	unsigned short requestPathLocator=0;
	unsigned char slashCounter=0;
	while(((slashCounter+=(httpRequest[requestPathLocator++]==0x2F))<3)&&(requestPathLocatorLimit--)) _WDF;
	if(slashCounter!=3)
		urlParameters.requestPath=UNDEFINED;
	else
		urlParameters.requestPath=(httpRequest+requestPathLocator-1);

	urlParameters.secure=(httpRequest[4]==0x73);
	urlParameters.port=80+((443-80)*(httpRequest[4]==0x73));
	unsigned short urlStartLocation=7+urlParameters.secure;

	while((--httpRequestLength)&&(httpRequest[httpRequestLength]!=0x40)) _WDF;
	if(httpRequestLength){_WDF;			//get basic auth
		urlParameters.authDefined=1;
		unsigned char authCharCounter=0;
		while(httpRequest[urlStartLocation]!=0x3A){	_WDF;
			urlParameters.userName[authCharCounter++]=httpRequest[urlStartLocation++];
		}
		urlStartLocation++;
		authCharCounter=0;
		while(httpRequest[urlStartLocation]!=0x40){	_WDF;
			urlParameters.password[authCharCounter++]=httpRequest[urlStartLocation++];
		}
		urlStartLocation=httpRequestLength+1;
	}

	unsigned short domainStartLocation=urlStartLocation;
	while((httpRequest[urlStartLocation]!=0x3A)&&(httpRequest[urlStartLocation++]!=0x2F)) _WDF;		//trying to locate : or /
	if(httpRequest[(urlStartLocation)]==0x3A)
		urlParameters.port=strint(httpRequest+urlStartLocation+1);
	unsigned char domainFillCounter=0;
	while(domainStartLocation<(urlStartLocation-=(httpRequest[(urlStartLocation-1)]==0x2F))){	_WDF;
		urlParameters.domain[domainFillCounter++]=httpRequest[domainStartLocation++];
	}
	if(!urlParameters.port)
		urlParameters.port=80+((443-80)*(httpRequest[4]==0x73));
	return urlParameters;
}

unsigned short fetchMemoryLimiter=4069;

unsigned char *fetch(unsigned char *httpRequest,unsigned char *requestBody,unsigned char *responseBuffer){WD_FEED;
	unsigned char* returnedBuffer=responseBuffer;
	struct httpLink urlParameters=urlBreakDown(httpRequest);
	unsigned char *requestBodyLocation=_CS(responseBuffer,(unsigned char*)((requestBody==UNDEFINED)?("GET "):("POST ")));
	if(urlParameters.requestPath!=UNDEFINED)
		urlEncode(_CS(requestBodyLocation,urlParameters.requestPath)+4+(requestBody!=UNDEFINED));
	_CS(responseBuffer,(unsigned char*)((urlParameters.requestPath==UNDEFINED)?("/"):("")));_WDF;
	_CS(responseBuffer,(unsigned char*)" HTTP/1.1\r\n");_WDF;
	_CS(responseBuffer,(unsigned char*)"Host: ");_WDF;
	_CS(responseBuffer,urlParameters.domain);_WDF;
	if(urlParameters.port!=80+((443-80)*(httpRequest[4]==0x73))){_WDF;
		_CS(responseBuffer,(unsigned char*)":");
		_CS(responseBuffer,inttostring((unsigned long)(urlParameters.port)));
	}
	_CS(responseBuffer,(unsigned char*)"\r\n");
	_CS(responseBuffer,(unsigned char*)"User-Agent: PostmanRuntime/7.29.0\r\n");_WDF;
	_CS(responseBuffer,(unsigned char*)"Accept: */*\r\n");_WDF;
	_CS(responseBuffer,(unsigned char*)"Connection: keep-alive\r\n");
	if(requestBody!=UNDEFINED){_WDF;
		_CS(responseBuffer,(unsigned char*)"Content-Type: application/json\r\n");_WDF;
		_CS(responseBuffer,(unsigned char*)"Content-Length: ");_WDF;
		_CS(responseBuffer,(unsigned char*)inttostring(stringCounter(requestBody)));_WDF;
		_CS(responseBuffer,(unsigned char*)"\r\n\r\n");_WDF;
		_CS(responseBuffer,requestBody);_WDF;
		_CS(responseBuffer,(unsigned char*)"\r\n");_WDF;
	}
	_CS(responseBuffer,(unsigned char*)"\r\n");

	if(urlParameters.secure){WD_FEED;
		WiFiClientSecure client;
		client.setInsecure();
		if(!client.connect((char*)urlParameters.domain,urlParameters.port)){WD_FEED;
			consoleLog("couldn't connect !!\n");
			return UNDEFINED;
		}
		client.write((char*)responseBuffer);
		CLR(responseBuffer);
		while(!client.available())WD_FEED;
		unsigned char *makeStr=responseBuffer;
		unsigned long memoryLimter=0;
		while(client.available()){
			_WDF;
			if(memoryLimter<fetchMemoryLimiter){
				*makeStr=client.read();
				makeStr++;
			}
			else{
				client.read();
			}
			memoryLimter++;
		}
		makeStr=responseBuffer;
	}
	else{
		WiFiClient client;
		if(!client.connect((char*)urlParameters.domain,urlParameters.port)){WD_FEED;
			consoleLog("couldn't connect !!\n");
			return UNDEFINED;
		}
		client.write((char*)responseBuffer);
		CLR(responseBuffer);
		while(!client.available())WD_FEED;
		unsigned char *makeStr=responseBuffer;
		unsigned long memoryLimter=0;
		while(client.available()){
			_WDF;
			if(memoryLimter<fetchMemoryLimiter){
				*makeStr=client.read();
				makeStr++;
			}
			else{
				client.read();
			}
			memoryLimter++;
		}
		makeStr=responseBuffer;
	}

	return returnedBuffer;
}



unsigned char nextClient(unsigned char allClients){
	for(unsigned char i=0;i<8;i++){
		if(!(allClients&(1<<i)))
			return i;
	}
	return (-1U);
}

void hw_wdt_disable(){
	*((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF
//	volatile uint32_t *watchDogRegister=((volatile uint32_t*) 0x60000900);
//	*watchDogRegister &= ~(1);
}

void hw_wdt_enable(){
	*((volatile uint32_t*) 0x60000900) |= 1; // Hardware WDT ON
}

void setup() {
	ESP.wdtDisable();
	hw_wdt_disable();
}

void loop() {
	main();
}


#define REMOTE_HOST "192.168.1.130"
#define REMOTE_PORT 80U
#define GENERAL_RESPONSE "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: *\r\n\r\nSk9ITiBLSEFMSUw=\r\n"
#define CORS_HEADERS (char *)"HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: *\r\n\r\n"
#define CLIENT_ACK "REQUEST_ACK\r\n"

#define EXPORTED_DATA_MAX_SIZE 10240UL
unsigned char EXPORTED_DATA[EXPORTED_DATA_MAX_SIZE]="";


int main(void){
	sofser_ini();
	logicAnalizerSetup();
	eepromInit();
	serialPortSetup();
	spiConsoleSetup();
	static unsigned short lastNetworkStat;

	// unsigned char fileSystemMounted=1;
	// if(!fileSystem.begin()){
	// 	fileSystemMounted=0;
	// 	consoleLog("cannot mount FileSystem\r\n");
	// }

	// spiConsole console;
	// console.setup(clk,data,microSec,360000);
// 
// 
	// console.log("test >> ", 5012);

	

	consoleLog("\nserial port\n");



	const char *ssid="RISC-V";
	const char *password="threadripper";
	ssidSave((unsigned char*)ssid);
	wifiPasswordSave((unsigned char*)password);
	consoleLog(userSSID());
	consoleLog("\nserial port\n");
	consoleLog(userPassword());
	consoleLog("\nserial port\n");
	
	externalPort();
	lastNetworkStat=networkConnect();
	consoleLog(inttostring(lastNetworkStat==WL_CONNECTED));
 	consoleLog(" connected\n");
	unsigned char networkWasConnect=(lastNetworkStat==WL_CONNECTED);

	WiFiServer server(80);
	server.begin();


	WiFiClient socket;
	//socket.connect(REMOTE_HOST,REMOTE_PORT);
	socket.setTimeout(5000);
	// linkerSave((unsigned char*)"{\"D\":\"xtensa32plus.ddns.net\",\"P\":50,\"K\":\"0123456789123456\",\"U\":{\"auth\":\"test0\"}}");
	unsigned char serverConnected=serverConnect(socket);
	uint64_t lastServerTime=0;

	WiFiClient client;
	client.setTimeout(5000);

	#define enc(_RAWTEXT) tcps((unsigned char*)_RAWTEXT,USER_KEY,EXPORTED_DATA)
	unsigned char *_serverData;// not to call a pointer returnning func twice
	#define SERVER_SEND(serverData)_serverData=serverData; if(serverConnected){socket.write((char*)enc(_serverData));dataMask((unsigned char*)_serverData,USER_KEY);CLR(EXPORTED_DATA);}

	#define remoteUserData 0x00010000UL
	#define remoteUserDisconnected 0x00020000UL
	#define portListener 0x00040000UL
	#define clientDisconnected 0x00080000UL

	unsigned long eventListener=0;
	unsigned long staticListener=0;
	unsigned char clientJustConnected=0; //this method might lead to lots of synchronization problems but in a very few conditions
	unsigned char socketConnection=0;
	
	consoleLog("start the loop\n");
	WD_FEED;
	while(1){
		WD_FEED;
		eventListener=0;
		unsigned short reconnectTimer=0;
		#define NETWORK_BUFFER_SIZE 2048
		unsigned char tcpText[NETWORK_BUFFER_SIZE]="";

		while(1){
			//ESP.wdtFeed();
			WD_FEED;
			if(lastNetworkStat!=WiFi.status()){ //network event listener -auto handle
				lastNetworkStat=WiFi.status();
				if(lastNetworkStat==WL_CONNECTED){
					networkWasConnect=1;
					WiFi.softAP((char*)ipAddressString(WiFi.localIP()),(char*)userPassword());
				}
				else if(networkWasConnect){ //open up an access point with the matching ip address 
					networkWasConnect=0;
					WiFi.softAP("",(char*)userPassword());
					WiFi.softAP((char*)ipAddressString(WiFi.softAPIP()),(char*)userPassword());
				}
			}
			if(!clientJustConnected&&(!socketConnection)){//if there was a connection it would tell new connection simply to fuck off
				if(client=server.available()){
					clientJustConnected=1;
				}
			}
			if((clientJustConnected||socketConnection)&&(client.available())){
				clientJustConnected=0;
				eventListener|=portListener;
				staticListener|=portListener;
			}
			if(socketConnection&&(!client.connected())){
				socketConnection=0;
				eventListener|=clientDisconnected;
				staticListener|=clientDisconnected;
			}

			if(socket.available()&&serverConnected){
				tcpText[NETWORK_BUFFER_SIZE-1]=0xFF;
				dataMask(base64Decode(tcpGetString(socket,tcpText)),base64Decode(json("K",linkerData())));
				if(!lastServerTime){
					lastServerTime=getInt(json("time",tcpText));
					CLR(tcpText);
				}
				else if(getInt(json("time",tcpText))>lastServerTime){
					lastServerTime=getInt(json("time",tcpText));

					unsigned char timeDigits=stringCounter(json("time",tcpText))+9;
					CLR(tcpText+(stringCounter(tcpText)-timeDigits));
					unsigned char *orgLoc=tcpText;
					unsigned char *dataLoc=tcpText+8;
					while(*dataLoc){_WDF;
						*orgLoc=*dataLoc;
						dataLoc++;
						orgLoc++;
					}
					CLR(orgLoc);

					eventListener|=remoteUserData;
					staticListener|=remoteUserData;
				}
				else
					CLR(tcpText);
			}
			if(!(--reconnectTimer)){
				unsigned char *remoteHostObject=linkerData();
				static unsigned char remoteConnectionResetTimer;
				if(!(--remoteConnectionResetTimer)&&serverConnected)
					socket.write("Sk9ITiBLSEFMSUw=");
				if((json("D",remoteHostObject)!=UNDEFINED)&&(json("P",remoteHostObject)!=UNDEFINED)&&(json("K",remoteHostObject)!=UNDEFINED)&&(json("U",remoteHostObject)!=UNDEFINED)){
					if(!serverConnected)
						serverConnected=serverConnect(socket);
					else if(!(serverConnected=socket.connected()))
						consoleLog("server disconnected\n");
						//socket.stop();
				}
			}

			if(eventListener)
				break;
		}




		ALL_EVENTS_HANDLER:
		
		
		unsigned char *exportedData=EXPORTED_DATA;
		CLR(exportedData);

		if(eventListener&portListener){			WD_FEED;

			unsigned char *makeStr=tcpText;
			unsigned long memoryLimter=0;
			while(client.available()){
				WD_FEED;
				if(memoryLimter<(NETWORK_BUFFER_SIZE-1)){
					*makeStr=client.read();
					makeStr++;
				}
				else{
					client.read();
				}
				memoryLimter++;
			}
			makeStr=tcpText;

			if(socketConnection){
				webSocketDataFrame(tcpText,FRAME_DECODE);
				if(WEBSOCKET_LAST_OPCODE==9){
					unsigned char *webSocketPongFrame=webSocketDataFrame(tcpText,FRAME_ENCODE);
					webSocketPongFrame[0]=0x8A;
					_delay_ms(1);
					client.write((char*)webSocketPongFrame);
					CLR(tcpText);
					CLR(webSocketPongFrame);
					continue;
				}
				//client.write((char*)webSocketDataFrame(tcpText,FRAME_ENCODE));
			}
			else{
				if(!((eventIdentifier(tcpText)&WEB_SERVER)==WEB_SERVER))
					URIdecode(tcpText);
			}
			
			consoleLog(tcpText);
			consoleLog("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");



			unsigned char eventType=eventIdentifier(tcpText);


			if((eventType&WEB_SOCKET)==WEB_SOCKET){// never wanna mess with this line
				consoleLog("user data >> ");
				consoleLog(eventData(EVENT_DATA,tcpText,(unsigned char*)"\r\n"));
				consoleLog("\n");

				socketConnection=1;
				client.write("HTTP/1.1 101 Switching Protocols\r\n");
				client.write("Upgrade: websocket\r\n");
				client.write("Connection: Upgrade\r\n");
				client.write("Sec-WebSocket-Accept: ");
				client.write((char*)secWebSocketAccept(tcpText));
				client.write("\r\n\r\n");
			}

			
			// else if(((eventType&WEB_SERVER)==WEB_SERVER)&&(!socketConnection)){
			// 	unsigned char *filePath=URIdecode(eventData(EVENT_DATA,tcpText,(unsigned char*)" HTTP/"));
			// 	consoleLog("filePath >> ");
			// 	consoleLog(filePath);
			// 	consoleLog(_endl);
				
			// 	if(fileSystemMounted){_WDF;
			// 		client.write(CORS_HEADERS);
			// 		unsigned char *finalFilePath=filePath;
			// 		unsigned char homePageHTML[19]="/oscilloscope.html";
			// 		if(equalStrings(filePath,(unsigned char*)"/")||(!stringCounter(filePath)))
			// 			finalFilePath=homePageHTML;
			// 		File xtensaFile=fileSystem.open((char*)finalFilePath,"r");
			// 		while(1){_WDF;
			// 			unsigned short webPageBuffer=4069;
			// 			unsigned char *mkWebPageBuffer=exportedData;
			// 			while((--webPageBuffer)&&xtensaFile.available()){_WDF;
			// 				*mkWebPageBuffer=(unsigned char)xtensaFile.read();
			// 				mkWebPageBuffer++;
			// 			}
			// 			client.write((char *)exportedData);
			// 			CLR(exportedData);
			// 			if(webPageBuffer)
			// 				break;
			// 		}
			// 		client.flush();
			// 		client.stop();
			// 		xtensaFile.close();
			// 	}
			// 	else{
			// 		client.write(GENERAL_RESPONSE);
			// 		client.flush();
			// 		client.stop();
			// 	}
			// 	CLR(tcpText);
			// 	CLR(filePath);
			// }



			else if((eventType&UPLOADED_DATA)==UPLOADED_DATA){			WD_FEED;
				#define SCAN_FOR_WIFI 0x00000001UL
				#define NETWORK_DATA 0x00000002UL
				#define LOGIC_ANALIZER 0x00000004UL
				#define SUPER_SPEED_SAMPLE 0x00000008UL
				#define ENABLE_SERIAL_PORT 0x00000010UL
				#define SERVER_DATA 0x00000020UL
				#define EXTERNAL_PORT 0x00000040UL
				#define EXTERNAL_PORT_LIVE 0x00000100UL

				consoleLog("user data >> ");
				consoleLog(eventData(EVENT_DATA,tcpText,(unsigned char*)"<@>"));
				consoleLog("\n");

				unsigned char *instructionDecode=json("instruction",tcpText);
				if(instructionDecode!=UNDEFINED){WD_FEED;
					unsigned long userInstruction=strint(instructionDecode);
					consoleLog("user instruction >> ");
					consoleLog(intToHexaDecimal(userInstruction));
					consoleLog("\n");
					
					if(userInstruction&SCAN_FOR_WIFI){WD_FEED;
						if(socketConnection){
							client.write((char*)webSocketDataFrame(scanForWifi(exportedData),FRAME_ENCODE));			WD_FEED;
						}
						else{
							client.write(CORS_HEADERS);
							client.write((char*)scanForWifi(exportedData));WD_FEED;
							client.flush();
							client.stop();
						}
						CLR(exportedData);
					}

					if(userInstruction&NETWORK_DATA){WD_FEED;
						ssidSave(json("networkSSID",tcpText));
						wifiPasswordSave(json("networkPASS",tcpText));
						if(socketConnection){
							client.write((char*)webSocketDataFrame((unsigned char*)CLIENT_ACK,FRAME_ENCODE));
						}
						else{
							client.write(CORS_HEADERS);
							client.write(CLIENT_ACK);
							client.flush();
							client.stop();
						}
						lastNetworkStat=networkConnect();
						networkWasConnect=(lastNetworkStat==WL_CONNECTED);
					}

					if((userInstruction&SERVER_DATA)&&(!socketConnection)){WD_FEED;
						unsigned char *serverConfig;
						client.write(CORS_HEADERS);
						if((serverConfig=json("serverData",tcpText))!=UNDEFINED)
							linkerSave(serverConfig);
						client.write((char*)linkerData());
						client.flush();
						client.stop();
					}

					if(userInstruction&EXTERNAL_PORT){WD_FEED;
						unsigned char *checkLastServerTime;
						if((checkLastServerTime=json("lastServerTime",tcpText))!=UNDEFINED){
							uint64_t externalPortInstructionOrder=getInt(checkLastServerTime);
							if(externalPortInstructionOrder!=lastServerTime){
								lastServerTime=externalPortInstructionOrder;
								externalPortSet(tcpText);
								unsigned char *extPrtDirectBit;
								if((extPrtDirectBit=json("extPrtDirectBit",tcpText))!=UNDEFINED){
									externalPortBitToggle(getInt(extPrtDirectBit));
								}
							}
						}
						SERVER_SEND(externalPort());
						if(socketConnection){
							client.write((char*)webSocketDataFrame(externalPort(),FRAME_ENCODE));
						}
						else{
							client.write(CORS_HEADERS);
							client.write((char*)externalPort());
							client.flush();
							client.stop();
						}
					}
					
					if((userInstruction&EXTERNAL_PORT_LIVE)&&socketConnection){WD_FEED;

						while(client.connected()){_WDF;
							if(client.available()){
								unsigned char *makeStr=exportedData;
								unsigned long memoryLimter=0;
								while(client.available()){
									_WDF;
									if(memoryLimter<(EXPORTED_DATA_MAX_SIZE-1)){
										*makeStr=client.read();
										makeStr++;
									}
									else{
										client.read();
									}
									memoryLimter++;
								}
								makeStr=exportedData;
								webSocketDataFrame(exportedData,FRAME_DECODE);

								unsigned char *liveExternalPortObject;
								if((liveExternalPortObject=json("liveExternalPort",exportedData))!=UNDEFINED){_WDF;
									uint64_t liveExternalPort=getInt(liveExternalPortObject);
									unsigned long livePortBitCounter=32;											//limited to 4 bytes
									while(livePortBitCounter--){_WDF;
										_DW(shiftRegisterData,((liveExternalPort&(1<<livePortBitCounter))!=0));		// !=0 not necessary but it only works that way 
										_DW(shiftRegisterClk,1);
										_delay_us(EXTERNAL_OUTPUT_SPEED);
										_DW(shiftRegisterClk,0);
										_delay_us(EXTERNAL_OUTPUT_SPEED);
									}
									_DW(shiftRegisterData,0);
									_DW(shiftRegisterLatch,1);
									_delay_us(EXTERNAL_OUTPUT_SPEED*10);
									_DW(shiftRegisterLatch,0);
									_delay_us(EXTERNAL_OUTPUT_SPEED);
								}

								else if(json("JRM",exportedData)!=UNDEFINED){WD_FEED;
									unsigned char *xtensaJrm=exportedData+9;
									unsigned short clearTextSize=stringCounter(xtensaJrm);
									base64Decode(xtensaJrm);					//JRM is not turing complete
									#define JRM_OPERAND(OPERAND_POINTER) (unsigned long)((xtensaJrm[(OPERAND_POINTER*4)+3]|(xtensaJrm[(OPERAND_POINTER*4)+2]<<8)|(xtensaJrm[(OPERAND_POINTER*4)+1]<<16)|(xtensaJrm[OPERAND_POINTER*4]<<24))&0x3FFFFFFF)
									#define JRM_OPCODE(OPCODE_POINTER) (xtensaJrm[OPCODE_POINTER*4]>>6)
									
									#define _output 		0
									#define _delay 			1
									#define _pointerSet 	2
									#define _endOfStack 	3

									unsigned short stackPointer=0;
									unsigned short stackLimiter=-1;
									while(stackLimiter--){
										_WDF;
										unsigned char opCode=JRM_OPCODE(stackPointer);
										unsigned long operand=JRM_OPERAND(stackPointer);
										stackPointer++;
										if(opCode==_output){
											const float JRM_PORT_SPEED=(1e6f/1000000)/2;
											unsigned long livePortBitCounter=30;											//30 bits in total 
											while(livePortBitCounter--){_WDF;
												_DW(shiftRegisterData,((operand&(1<<livePortBitCounter))!=0));		// !=0 not necessary but it only works that way 
												_DW(shiftRegisterClk,1);
												_delay_us(JRM_PORT_SPEED);
												_DW(shiftRegisterClk,0);
												_delay_us(JRM_PORT_SPEED);
											}
											_DW(shiftRegisterData,0);
											_DW(shiftRegisterLatch,1);
											_delay_us(JRM_PORT_SPEED*10);
											_DW(shiftRegisterLatch,0);
											_delay_us(JRM_PORT_SPEED);
										}
										else if(opCode==_delay){
											_delay_us((operand&255));
										}
										else if(opCode==_pointerSet){
											// gpioStack[stackPointer]&=0xC000FFFFUL;
											// gpioStack[stackPointer]|=((operand>>16)-((operand>>16)!=0))<<16;
											unsigned short countPointerDown=((operand>>16)-((operand>>16)!=0));
											xtensaJrm[(stackPointer-1)*4]=0x80|(0x3f&((countPointerDown)>>8));
											xtensaJrm[((stackPointer-1)*4)+1]=countPointerDown&0xFF;
											if((operand>>16))
												stackPointer=(unsigned short)(operand&0xFFFF);
										}
										else if(opCode==_endOfStack){
											if(!operand)
												break;
											else{
												xtensaJrm[(stackPointer-2)*4]=xtensaJrm[(stackPointer-1)*4]&0xBF;
												xtensaJrm[((stackPointer-2)*4)+1]=xtensaJrm[((stackPointer-1)*4)+1];
												xtensaJrm[((stackPointer-2)*4)+2]=xtensaJrm[((stackPointer-1)*4)+2];
												xtensaJrm[((stackPointer-2)*4)+3]=xtensaJrm[((stackPointer-1)*4)+3];
											}
										}

										static unsigned char clientCallTimeOut;
										if(!(--clientCallTimeOut)){
											if(client.available())
												break;
											if(!(client.connected()))
												break;
										}


									}

									CLR_LENGTH=clearTextSize;
									CLR(xtensaJrm);_NOP();

								}

								// #define stackIdentifier "S"
								// else if((liveExternalPortObject=json(stackIdentifier,tcpText))!=UNDEFINED){WD_FEED;
								// 	unsigned short stackPointer=0;
								// 	unsigned char stackIdentifierList[20]=stackIdentifier;
								// 	stackIdentifierList[stringCounter(stackIdentifierList)]=0x5B;	//square bracket


								// }
								// CLR_LENGTH=EXPORTED_DATA_MAX_SIZE;
								CLR(exportedData);
							}
						}
						socketConnection=0;

						
					}

					if(userInstruction&LOGIC_ANALIZER){WD_FEED;
						unsigned char *userDefinedSampleSize=json("contScan",tcpText);
						if(userDefinedSampleSize!=UNDEFINED){
							CLR(exportedData);
							unsigned char logicAnalizerFrameSize=strint(userDefinedSampleSize);
							if((logicAnalizerFrameSize>(EXPORTED_DATA_MAX_SIZE-11))||(!logicAnalizerFrameSize))
								logicAnalizerFrameSize=EXPORTED_DATA_MAX_SIZE-11;
								
							if(socketConnection){
								unsigned char *webSocketClientLogicAnalizerData=exportedData+11;
								logicAnalizer(webSocketClientLogicAnalizerData,logicAnalizerFrameSize);
								unsigned short webSocketClientLogicAnalizerDataLength=stringCounter(webSocketClientLogicAnalizerData);
								webSocketClientLogicAnalizerDataLength+=2+(2*(webSocketClientLogicAnalizerDataLength>125));
								webSocketClientLogicAnalizerData=webSocketDataFrameEncodeBinary(webSocketClientLogicAnalizerData);
								if(client.connected())
									client.write((char*)webSocketClientLogicAnalizerData,webSocketClientLogicAnalizerDataLength);
								else{
									socketConnection=0;
									consoleLog("\t\tclient disconnected\n");
								}
								webSocketClientLogicAnalizerData[2]=0xff;
								CLR(webSocketClientLogicAnalizerData);//there is a very high chance that the returned address is not equal to the exportedData pointer so it wont clear the buffer properly
							}
							else{
								client.write(CORS_HEADERS);
								client.write((char*)logicAnalizer(exportedData,logicAnalizerFrameSize));
								client.flush();
								client.stop();
							}
						}
					}
					
					if(userInstruction&SUPER_SPEED_SAMPLE){WD_FEED;
						CLR(exportedData);
						if(socketConnection){

						}
						else{
							client.write(CORS_HEADERS);
							exportedData[EXPORTED_DATA_MAX_SIZE-1]=1;
							superSpeedSample(exportedData);WD_FEED;
							/*
							#define superSpeedSamplePacketBufferSize 24UL
							#define superSpeedSamplePacketSize superSpeedSamplePacketBufferSize*0.75
							unsigned short superSpeedSamplePartitionCounter=0;
							while(superSpeedSamplePartitionCounter<(uint16_t)((EXPORTED_DATA_MAX_SIZE-1)/superSpeedSamplePacketSize)){WD_FEED;
								unsigned char superSpeedSamplePacketBuffer[superSpeedSamplePacketBufferSize]="";
								client.write((char*)base64Encoder((exportedData+(18*superSpeedSamplePartitionCounter)),superSpeedSamplePacketBuffer,superSpeedSamplePacketSize));
								superSpeedSamplePartitionCounter++;
							}*/
							client.write(exportedData,(EXPORTED_DATA_MAX_SIZE-1));
							client.flush();
							client.stop();
						}
						CLR_LENGTH=(EXPORTED_DATA_MAX_SIZE-1);
						CLR(exportedData);
					}

					if((userInstruction&ENABLE_SERIAL_PORT)&&socketConnection){WD_FEED;
						CLR(exportedData);
						CLR(tcpText);

						#define SERIAL_TEXT 0x00000001UL
						#define SERIAL_BAUDRATE 0x00000002UL
						
						unsigned char *serialBuffer0=exportedData+15;
						unsigned char *serialBuffer1=exportedData+(EXPORTED_DATA_MAX_SIZE/4);
						unsigned char *serialBuffer2=exportedData+((EXPORTED_DATA_MAX_SIZE/4)*2);
						unsigned char *serialBuffer3=exportedData+((EXPORTED_DATA_MAX_SIZE/4)*3);
						unsigned char *serialReadBuffer[5]={serialBuffer0,serialBuffer1,serialBuffer2,serialBuffer3,(exportedData+EXPORTED_DATA_MAX_SIZE-5)};
						unsigned short serialInputBaudRate[4]={9600,9600,9600,9600};
						
						while(client.connected()){WD_FEED;

							if(client.available()){
								unsigned char *makeStr=tcpText;
								unsigned long memoryLimter=0;
								while(client.available()){
									WD_FEED;
									if(memoryLimter<(NETWORK_BUFFER_SIZE-1)){
										*makeStr=client.read();
										makeStr++;
									}
									else{
										client.read();
									}
									memoryLimter++;
								}
								makeStr=tcpText;
								webSocketDataFrame(tcpText,FRAME_DECODE);

								unsigned long serialUserInstruction=strint(json("serialInst",tcpText));

								if(serialUserInstruction&SERIAL_TEXT){
									unsigned char userSelectedChannel=strint(json("serialChannel",tcpText));
									unsigned short userSelectedBaudRate=strint(json("baudRate",tcpText));
									JSON(((unsigned char*)"serialData"),tcpText,exportedData);
									
									unsigned short serialDataLength=stringCounter(exportedData);
									exportedData[--serialDataLength]=0;
									if(strint(json("_CR",tcpText))){
										exportedData[serialDataLength]=13;
										serialDataLength++;
									}
									if(strint(json("_NL",tcpText))){
										exportedData[serialDataLength]=10;
									}

									serialSend(userSelectedChannel,userSelectedBaudRate,(exportedData+1));
								}
								if(serialUserInstruction&SERIAL_BAUDRATE){
									serialInputBaudRate[0]=strint(json("serialInputBaudRate[0]",tcpText));
									serialInputBaudRate[1]=strint(json("serialInputBaudRate[1]",tcpText));
									serialInputBaudRate[2]=strint(json("serialInputBaudRate[2]",tcpText));
									serialInputBaudRate[3]=strint(json("serialInputBaudRate[3]",tcpText));
								}
							}

							// static unsigned char serialPortPingTimer;
							// if(!(--serialPortPingTimer))
								// client.write((char*)webSocketDataFrame((unsigned char*)"Sk9ITiBLSEFMSUw=",FRAME_ENCODE));//this line was added to prevent instablity 
							// client.write((char*)webSocketDataFrame((unsigned char*)"Sk9ITiBLSEFMSUw=",FRAME_ENCODE));
							
							if(quadSerialInput(serialReadBuffer,serialInputBaudRate)!=NO_DATA){WD_FEED;
								
								if(!client.connected())
									break;

								unsigned char jsonSync[8]="{\"ch\":[";

									
								for(unsigned char readChannelCounter=0;readChannelCounter<4;readChannelCounter++){WD_FEED;
									unsigned char *jsonSyncLoc=serialReadBuffer[readChannelCounter];
									if(*jsonSyncLoc){
										while(*jsonSyncLoc){
											*jsonSyncLoc&=~0x80;
											jsonSyncLoc++;
										}
										jsonSyncLoc=serialReadBuffer[readChannelCounter];
										jsonSyncLoc-=stringCounter(jsonSync);
										unsigned char *serialReadChannelNumber=inttostring(readChannelCounter+(SPI_MODE_ACTIVE_CHANNEL*(SERIAL_PORT_SPI_MODE!=0)));
										jsonSyncLoc-=stringCounter(serialReadChannelNumber)+2;
										unsigned char *jsonSyncLocClear=jsonSyncLoc;
										unsigned char *jsonSyncRead=jsonSync;
										while(*jsonSyncRead){
											*jsonSyncLoc=*jsonSyncRead;
											jsonSyncRead++;
											jsonSyncLoc++;
										}
										while(*serialReadChannelNumber){
											*jsonSyncLoc=*serialReadChannelNumber;
											serialReadChannelNumber++;
											jsonSyncLoc++;
										}
										*jsonSyncLoc=0x2C;
										jsonSyncLoc++;
										*jsonSyncLoc=0x22;
										jsonSyncLoc++;
										if(((jsonSyncLocClear+stringCounter(jsonSyncLocClear)+4)>=serialReadBuffer[readChannelCounter+1])&&(!SERIAL_PORT_SPI_MODE))
											jsonSyncLoc=serialReadBuffer[readChannelCounter+1]-4;
										else
											jsonSyncLoc=jsonSyncLocClear+stringCounter(jsonSyncLocClear);
										*jsonSyncLoc=0x22;
										jsonSyncLoc++;
										*jsonSyncLoc=0x5D;
										jsonSyncLoc++;
										*jsonSyncLoc=0x7D;
										jsonSyncLoc++;
										*jsonSyncLoc=0;
										jsonSyncLoc++;											

										//consoleLog(jsonSyncLocClear);
										//consoleLog("\n");

										unsigned char *serialReadFinalData=webSocketDataFrameEncode(jsonSyncLocClear);
										client.write(serialReadFinalData,(stringCounter(jsonSyncLocClear)+(jsonSyncLocClear-serialReadFinalData)));
										CLR(jsonSyncLocClear);
										CLR(serialReadFinalData);
										if(SERIAL_PORT_SPI_MODE)
											break;
									}

								}
								
							}
							
							WD_FEED;
							CLR_LENGTH=EXPORTED_DATA_MAX_SIZE-20;
							CLR(exportedData);
							CLR(tcpText);
						}
						socketConnection=0;
						CLR(exportedData);
						CLR(tcpText);
					}
				}	
			}




			else if(!socketConnection){
				client.write(GENERAL_RESPONSE);
				client.flush();
				client.stop();
			}
			/*
			CLR(exportedData);
			logicAnalizer(exportedData,20);
			consoleLog("logicAnalizer >> \n");
			while(*exportedData){
				consoleLog("\t");
				consoleLog(intToHexaDecimal((*exportedData)));
				consoleLog("\n");
				exportedData++;
			}

			*/

			WD_FEED;
			
		}

		if(eventListener&=remoteUserData){		WD_FEED;
			consoleLog("DATA FROM SERVER >> ");
			consoleLog(tcpText);
			consoleLog("\r\n");

			#define SERVRE_EXTERNAL_PORT 0x00000040UL
			#define SERVER_SYNC_EXTERNAL_PORT 0x00000080UL

			unsigned char *serverInstructionDecode;
			if((serverInstructionDecode=json("instruction",tcpText))!=UNDEFINED){WD_FEED;
				unsigned long serverInstruction=getInt(serverInstructionDecode);

				if(serverInstruction&SERVRE_EXTERNAL_PORT){WD_FEED;
					externalPortSet(tcpText);
					unsigned char *extPrtDirectBit;
					if((extPrtDirectBit=json("extPrtDirectBit",tcpText))!=UNDEFINED){
						SERVER_SEND(externalPortBitToggle(getInt(extPrtDirectBit)));		
					}
					else
						SERVER_SEND(externalPort());
				}

				if(serverInstruction&SERVER_SYNC_EXTERNAL_PORT){WD_FEED;
					unsigned char *localPortValue=json("externalPort",externalPort());
					CLR(exportedData);
					unsigned char *localPortValueCopy=exportedData;
					while(*localPortValue){_WDF;
						*localPortValueCopy=*localPortValue;
						localPortValue++;
						localPortValueCopy++;
					}
					if(!equalStrings(exportedData,json("externalPort",tcpText))){
						externalPortSet(tcpText);
						externalPort();
					}
					CLR(exportedData);
				}
			}
		}



		
	}
}