/*===================================================
 * Tac gia : Nguyen Van Nhan - vnhan.nguyen@gmail.com
 * Author : Nguyen Van Nhan - vnhan.nguyen@gmail.com
 * Date : 24/06/2017
 * ==================================================
 * 
 * Mo ta : Chuong trinh dieu khien thiet bi cam bien nhiet do phong dung module SIM800A- myTempNotifier v1.0 
 * Description : Program of detect temporature of room using module SIM800A
 * 
 * ==================================================
 * Chuc nang : 
 * Thiet bi -> Nguoi dung
 * 1. Tu dong nhan tin nhiet do khi vuot qua nguong yeu cau
 * 2. Tu dong nhan tin phan hoi khi nguoi dung trong danh sach quy dinh dien thoai vao thiet bi
 * 3. Canh bao bang tin nhan khi mat dien.
 * 4. 
 * Nguoi dung -> Thiet bi
 * 1. Cai dat nhiet do quy dinh
 * 2. Cai dat so dien thoai cho phep tuong tac
 * 3. Dien thoai vao thiet bi de nhan ket qua nhiet do
 * 4. Nhan tin cai dat theo cu phap quy dinh truoc
 * Function :
 * 
 */

#include "SoftwareSerial.h"
#include "Hshopvn_GSM.h"
#include <LiquidCrystal_I2C.h>
#include<EEPROM.h>

// Declare lirbrary :
SoftwareSerial sim800A(2,3);
HshopGSM myGSM = HshopGSM(&sim800A);

// set the LCD address to 0x27 for a 16 chars 2 line display
// A FEW use address 0x3F
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// Declare phone number :
unsigned long masterPhoneNo = 972050677; //master phone number
String masterPhoneNo_str="0972050677";
String listPhone[3]={"0868897879","0939266177",""};
String incomingPhoneNo = "";
String smsPhoneNo ="";

// Declare pin :
unsigned char Relay_pin = 13;
unsigned char Relay_active = 0;
int tempSensorPin = A0; // Chan analog ket noi cam bien LM35
int eepromAddr = 0; //EEPROM address

// Declare global variable
int tempCount = 0;
int countGetTime = 0;
String datetime = "";
float finalTemp = 0;
int checkFunction = 0;
int maxTemp1 = 0;
int maxTemp2 = 0;

void setup() {
  // put your setup code here, to run once:
  
  // Pin init
  pinMode(Relay_pin,OUTPUT);
  digitalWrite(Relay_pin, !Relay_active); //turn off relay

  //Show in Serial Monitor
  Serial.begin(9600);
  Serial.println("Start");

  // GSM library init
  myGSM.init(&GSM_Ready,9600);
  setGSMtime();

  //Setup LCD 16x2
  lcd.begin(16,2);

  startScreen();
}

void loop() {
  // put your main code here, to run repeatedly:

  //start getGSM time
  requestGSMtime();
  myGSM.delay(20000);

  //handle GSM data
  myGSM.handle();

  //Get temp from sensor
  if (tempCount == 1000) {
    int reading = analogRead(tempSensorPin);
    float voltage = reading * 5.0 / 1024.0;
    finalTemp = voltage * 100.0;
    tempCount = 0;
    //displayMainLCDinfo(); //Display temp info on LCD screen
  } else tempCount ++;

  //Check Max temp
  if (checkMaxTemp(finalTemp)) {
    sendSMSNotify_All(finalTemp);
  }

  
}

//===============================================================================

// Group command of GSM function
void GSM_Ready() {
  if (myGSM.getDataGSM() != "") {
    
    String respondData = myGSM.getDataGSM();
    Serial.println(respondData);

    boolean isReceiveSMS = false;
    
    //Check function GSM Respond data
    if (respondData.substring(2,6) == "CCLK") {
      datetime = respondData.substring(9,23);
      displayMainLCDinfo();
    } else if (respondData.substring(2,6) == "CLIP") {  //Check valid phoneNo to reply temp SMS.
      incomingPhoneNo = getIncoming_PhoneNo(respondData);
      sendSMSnotify_one_authorized (finalTemp);
      myGSM.hangcall();
    } else if (respondData.substring(2,5) == "CMT") { //Check incoming SMS
      isReceiveSMS = true;
      smsPhoneNo = getSMSnumber(respondData);
    } else if (isReceiveSMS) {
      String SMScontent = getSMScontent(respondData);
      Serial.print("Nhan duoc tin nhan : ");
      Serial.println(SMScontent); // Print SMS content to serial monitor for check info.
      checkSMS (smsPhoneNo);
      
      isReceiveSMS = false;
    }
    
  }
  
}

//Get SMS number
String getSMSnumber (String dataStream) {
  String smsPhoneNo = "";
  
  return smsPhoneNo;
}

//Analysis SMS content and response to authorise phone no
void checkSMS(String inSMSphoneNo) {
  //Check Authorise phoneNo first
  boolean isAuthoriseNo = isAuthenMem(inSMSphoneNo);
  
  if (isAuthoriseNo == true) {
    
  
    //Send temporature to authorise user
  
    //Change all authorized phone list
  
    //Change one authorized phone number
  
    //Set max temp 1
  
    //Set max temp 2
  
    //Check Acc Balance

    } else Serial.println("Not authorise user");
}

String getAccBalance (String dataStream) {
  String accBalance;

  return accBalance;
}

//Get incoming phone number, return String value.
String getIncoming_PhoneNo (String DataStream) {
  String inPhoneNo;
  if (DataStream.substring(11,12) =="9") {
    inPhoneNo = DataStream.substring(10,20);
  } else inPhoneNo = DataStream.substring(10,21);
    
  return inPhoneNo;
}

//Get SMS content from message
String getSMScontent(String DataStream) {
  String content = "";
  
  return content;
}

//Request incoming phone number
void requestInPhoneNo() {
  myGSM.atcm("AT+CLIP=1");
}


//Send SMS temporature to all list member when receive SMS request from authen phone number
void requestSendSMSNotify_All(float temporature) {
  Serial.println("Function send SMS to all authorise phonelist"); //Display on Terminal for Test
  String smsContent = "";
  if (isAuthenMem(incomingPhoneNo)) {
    myGSM.delay(500);
    smsContent = "Nhiet do phong : ";
    smsContent += temporature; 
    smsContent += "; ";
    smsContent += datetime;
    for (int i=0;i<4;i++) {
      if (listPhone[i] != "") {
        myGSM.delay(500);
        Serial.print("Start send SMS to : ");
        Serial.println(listPhone[i]);
        myGSM.sendsms(listPhone[i],smsContent);
      }    
    }
  }  
}

//Send SMS temporature to all list member when overtemp
void sendSMSNotify_All(float temporature) {
  Serial.println("Function send SMS to all authorise phonelist"); //Display on Terminal for Test
  String smsContent = "";
  
  myGSM.delay(500);
  smsContent = "Nhiet do phong : ";
  smsContent += temporature; 
  smsContent += "; ";
  smsContent += datetime;
  for (int i=0;i<4;i++) {
    if (listPhone[i] != "") {
      myGSM.delay(500);
      Serial.print("Start send SMS to : ");
      Serial.println(listPhone[i]);
      myGSM.sendsms(listPhone[i],smsContent);
    }    
  }
  
}

//Send SMS temporature to authorized member
void sendSMSnotify_one_authorized (float temporature) {
  Serial.println("Function send SMS to one authorized member who call to myTemp"); //Display on Terminal for Test
  if (isAuthenMem(incomingPhoneNo)) {
    myGSM.delay(500);
    Serial.print("Start send SMS to : ");
    Serial.println(incomingPhoneNo);
    String smsContent = "Nhiet do phong : ";
    smsContent += temporature; 
    smsContent += "; ";
    smsContent += datetime;
    myGSM.sendsms(incomingPhoneNo, smsContent);
  } else {
    myGSM.delay(500);
    Serial.print("Permission Deined");
  };
}

//Check authorize member
boolean isAuthenMem(String phoneNo) {
  for (int i=0;i<4;i++) {
    if (phoneNo == listPhone[i]) {
      return true;
    } else return false;
  };
}

//Get authorized member list from EEPROM
void getAuthenList() {
  
}

//send authorized member list to EEPROM
void sendAuthenList() {
  
}

void setGSMtime() {
  Serial.println("Start set GSM location time");
  myGSM.atcm("AT+COPS=2");
  myGSM.atcm("AT+CLTS=1");
  myGSM.atcm("AT+COPS=0");
}

//Request GSM time
void requestGSMtime() {
  myGSM.atcm("AT+CCLK?");
}

//Check respone function
/*
 * 1. Default Function => Display info of temp on screen
 * 2. Incoming call
 * 3. Outgoing Call
 * 4. Incoming SMS
 * 5. Sending SMS
 * 6. Check Info of caller ID
 * 7. Lost Connection
 * 8. Error
 * 9. Request GSM Time
 * 10. Checking temp
 */

int checkReponseFunc(String responeMessage) {
  
}

//===============================================================================
//Group command of LCD
//Display startscreen
void startScreen() {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("myTemp Notifier v1.0");
  lcd.setCursor(0,1);
  lcd.print("hitechmekong.vn");
  delay(2000);
}

//Display LCD screen info
void displayMainLCDinfo() {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(datetime);
  lcd.setCursor(0,1);
  lcd.print("Nhiet do : ");
  lcd.setCursor(12,1);
  lcd.print(finalTemp);
}

//===============================================================================
// Store data to EEPROM memory
void writeEEPROMdata(String str) {
  for (int i=0; i<str.length();i++) {
    EEPROM.write(i,str[i]);
    delay(5);
  }
}

String readEEPROMdata () {
  char data[] ="";
  for (int i=0; i<500; i++) {
    char ch = EEPROM.read(i);
    if (ch != NULL) {
      data[i]=ch;
    } else break;
  }
  String str(data);
  return str;
}

void addPhone(String phoneNo) {
  String readPhoneNo = readEEPROMdata();
  String newPhoneData = readPhoneNo += "";
  newPhoneData = newPhoneData += phoneNo;
  writeEEPROMdata(newPhoneData);
}

void readPhoneList() {
  
}


//===============================================================================
//Function of Temp sensor

boolean checkMaxTemp(float temp) {
  if (temp > maxTemp1 || temp > maxTemp2) {
    return true;
  }
  return false;
}


