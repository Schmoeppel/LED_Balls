////#include <FastLED.h>
#include <Wire.h>

////#define NUM_LEDS 24

#define Red_PIN 9
#define Green_PIN 11
#define Blue_PIN 10

short Timing_Korrektur = 150;
short Offset = 300;
uint8_t Testzahl = 1; //vllt nicht mehr gebraucht
uint8_t LED_red;
uint8_t LED_green;
uint8_t LED_blue;
uint8_t red_safe;
uint8_t green_safe;
uint8_t blue_safe;
long millis_song_beginning;

//Variablen zum einlesen von BT-Daten
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];   // temporary array for use by strtok() function variables to hold the parsed data
boolean newData = false;
int BT_wert;
char BT_signal[numChars] = {0};

//Variablen für SmartSens (Beschleunigungssensor)
bool SD_MenuPressed = false;
bool SD_ColorChange = false;
bool SD_Sideswaps = false;
bool SD_ShowAccel = false;
bool SD_SS_Kalibrierung = false;
long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;
uint8_t gForceTotal;
uint8_t cnt;
long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;
long millis_ShowAccel;
#define TIMESTEP_SHOWACCEL 150; 

//Beschleunigungssensor Werte Glätten
int GlattFaktor_cnt = 0;
uint8_t gForceGlatt = 0;
uint8_t GlattFaktor = 50; //soviele Werte werden stets zu einer Ausgabe zusammen geführt, und soviele Werte hängt die Ausgabe auch der Realität nach
bool new_gForceGlatt = false;
int gForceGlatt_Summe = 0;

const byte gForceBetragGlatt_samplingrange = 5;
int ThrowThreshold = 1500;
long gForceBetragGlattEinzelWerte[gForceBetragGlatt_samplingrange];
long gForceBetragGlatt_Summe = 0;
int gForceBetrag = 0;

int DataCnt = 0;

uint8_t KorrekturFaktor255 = 255; //muss auch nochmal unten in LED(...) geändert werden, damit Vorzeichen stimmt

//Variablen zur Wurferkennung
/*long Wurfstart_time = 0;
long Wurfende_time = 0;
uint8_t Impulsthreshold = 100; //Wenn dieser Wer überschritten wird, wurde Wurf oder Fangimpuls gemessen. Normaler vom Halten liegt bei 70
bool Impuls_measured = false;
bool WurfCheck = false;
uint8_t Flugphasen_Threshold = 40; //Wenn dieser Wert unterschritten, dann ist Ball in der Luft (sollte dann theoretisch 0 rausgeben)
bool Wurf = true;
long CheckDauer = 0;
long Wurf_Dauer = 0;
*/

//Variablen zur Vereinfachten Erkennung von Color Change
uint8_t ColorChange_FlugPhasen_Threshold = 40;
uint8_t ColorChange_Spin_Threshold = 250;


//Variablen zu vereinfachten Wurferkennung für Sideswaps
long Wurf_Start_Flag = 0;
long Wurf_Ende_Flag = 0;
long Wurf_Dauer_ms = 0;
bool Wurf_Status = true;
float SideSwap_Number_float = 0;
uint8_t SideSwap_Number = 0;
uint8_t SideSwap_FlugPhasen_Threshold = 40; //Unter den Wert müssen die Sensordaten fallen, damit Wurf erkannt wird
bool Wurf_beendet = false;
long Kalibrier_3_Dauer = 300; //Solange läuft eine SideSwap 3. Wert wird über Kalibrierung aktualisiert
//Variablen zum Kalibrieren
uint8_t Kalibriercnt = 0;
long Kalibrierfeedback_timer = 0;
bool Kalibrierfeedback_enable = false;

                 
void setup() { 
  //Initialisiere BT
  Serial.begin(115200);
  
  Serial.println("This demo expects 2 pieces of data - text and an integer");
    Serial.println("Enter data in this style <text,12>  ");
    Serial.println();
    //Serial.write("A123");
    Serial.println("start");

    //Beschleunigungssensor MPU-6050 einrichten
    Wire.begin(); //SCL -> A5;         SDA -> A4
    setupMPU();
    //Serial.println("start");
    
    ////FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    ////for (int i=0; i<14; i++){
    ////leds[i] = CRGB(2, 200, 2);
    ////}
    ////FastLED.show();
    
    pinMode(Red_PIN, OUTPUT);
    pinMode(Green_PIN, OUTPUT);
    pinMode(Blue_PIN, OUTPUT);
    LED(0,250,0);
    delay(500);
    LED(0, 0, 0);
}

void loop() { 
  //Serial.println("loop1");
  //LED(100, 100, 100);
  BT_Empfangen(); 
  BT_Ausfuehren(); //Befehle ausführen
    //LED(100, 100, 100);
    //Serial.println("loop2");
    SmartDetect();
    //LED(100, 100, 100);
    //Serial.println("loop3");
    
  //Serial.println(gForceBetragGlatt_Summe);
  //Serial.println(gForceTotal);
    
    //nur zum testen der Glättungsfunktion
/*    if (millis_ShowAccel <= millis()){
      if (new_gForceGlatt == true){
        millis_ShowAccel = millis() + TIMESTEP_SHOWACCEL;
        Serial.write('P');
        Serial.write((uint8_t)gForceGlatt);
        new_gForceGlatt = false;
      }
    }*/
    
    /////////////////
    
}

void BT_Empfangen(){
  recvWithStartEndMarkers(); //Empfange Daten
    if (newData == true) { //wenn neue Daten angekommen sind
        strcpy(tempChars, receivedChars);  // this temporary copy is necessary to protect the original data because strtok() replaces the commas with \0
        parseData();  //zerlege Daten, da immer erst ein Buchstabe und dann eine Zahl für unterschiedliche Funktionen kommt
        //BT_Ausfuehren(); //Befehle ausführen
        newData = false; //angekommene Daten wurden abgearbeitet
    }   
}

void BT_Ausfuehren(){
  /////einprogrammierte Lichtfolge starten
    if (BT_signal[0] == 'S'){
      if (BT_wert == 0){
        Song1();
      }
      if (BT_wert == 1){
        Song2();  
      }
      if (BT_wert == 2){
        Synchro();  
      }
      if (BT_wert == 3){
        Song3();  
      }
    }
  /////

  /////beliebiges Signal zurückschicken um bluetoothchannel aufzuwecken
  if (BT_signal[0] == 'Y'){
      if (BT_wert == 0){
        Serial.write('Y');
        Serial.write((uint8_t)0);
      }
    }
  //////

  /////BT Verbindung aufgebaut
  if (BT_signal[0] == 'A'){ 
      LED(0, 255, 0);
      delay(150);
      LED(0, 0, 0);
      delay(150);
      LED(0, 255, 0);
      delay(150);
      LED(0, 0, 0);
      delay(150);
      LED(0, 255, 0);
      delay(150);
      LED(0, 0, 0);
  }
    
  //Follow Challenge////////////////////  
  if (BT_signal[0] == 'F'){
    long FollowChallenge_glowDuration = BT_wert*1000;
    long millis_FollowChallengeEnd = millis()+FollowChallenge_glowDuration; //Ball soll x Sekunden lange rot leuchten
    fading(0, 0, 0, 255, 0, 0, millis(), millis()+4000);
    //LED(255, 0, 0);
    do{ //mache nichts
    }while(millis_FollowChallengeEnd > millis()); // solange nichts machen bis der einprogrammierte Zeitpunkt gekommen ist  
    LED(0, 0, 0); 
  }

  // Ball wieder sichtbar machen
  if (BT_signal[0] == 'E'){
    fading(0, 0, 0, 255, 0, 0, millis(), millis()+3500);
  }
  
  /////Farben manuell über Slider einstellen
  if ((BT_signal[0] =='R') || (BT_signal[0] == 'G') || (BT_signal[0] == 'B')){ 
    if (BT_signal[0] == 'R'){
      LED_red = BT_wert;
      Scrollbars();
    }
    if (BT_signal[0] == 'G'){
      LED_green = BT_wert;
      Scrollbars();
    }
    if (BT_signal[0] == 'B'){
      LED_blue = BT_wert;
      Scrollbars();
    }
  } 
  /////
  
  /////SmartDetect Funktionen Ein-/Aus-stellen
  if (BT_signal[0] == 'M'){
    SD_MenuPressed = true;
  }
  if (BT_signal[0] == 'N'){
    if (BT_wert == 1){
      SD_ColorChange = true;
    }
    else if (BT_wert == 0){
      SD_ColorChange = false;
    }
  }
  if (BT_signal[0] == 'O'){
    if (BT_wert == 1){
      SD_Sideswaps = true;
    }
    else if (BT_wert == 0){
      SD_Sideswaps = false;
    }
  }
  if (BT_signal[0] == 'P'){
    if (BT_wert == 1){
      SD_ShowAccel = true;
    }
    else if (BT_wert == 0){
      SD_ShowAccel = false;
    }
  }
  
  if (BT_signal[0] == 'Q'){ 
    if (BT_wert == 1){
      Kalibrier_3_Dauer = 0; //damit Wert neu berechnet wird
      Kalibriercnt = 0;
      Wurf_beendet = false;
      Wurf_Status = false;
      SD_SS_Kalibrierung = true;
      LED(0, 255, 0);
      delay(150);
      LED(0, 0, 0);
      delay(150);
      LED(0, 255, 0);
      delay(150);
      LED(0, 0, 0);
      delay(150);
      LED(0, 255, 0);
      delay(150);
      LED(0, 0, 0);
    }
  }
  //////////
  BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    
}
  
  ////////////Wurfstart und Ende detektieren //Wird nicht gebraucht
/*void Wurfstatus(){
  Impuls_measured = false;
  if (gForceGlatt > Flugphasen_Threshold){
       WurfCheck = false;
     }
  if (((Wurfende_time + 100) < millis()) && (Wurf == false)){ //Wenn das letzte Wurfende erst vor 100ms gemessen werden, wird erstmal alles ignoriert, um überschwingen nicht ausversehen als neuen Wurf zu deuten
     if (gForceGlatt > Impulsthreshold){
       Impuls_measured = true;
       CheckDauer = millis();
       WurfCheck = true;
     }
     if (((CheckDauer + 400) < millis()) &&(WurfCheck == true)){ //Wenn mindestens 400ms lang gecheckt wurde ob beschleunigung unter Flugphasenthreshold lag, dann wird es offiziell zu einem Wurf erklärt
       Wurf = true;
       Wurfstart_time = millis();
     }
  }
  if ((Wurf == true)){
     if (gForceGlatt > Impulsthreshold){ 
       Wurf = false;
       Wurfende_time = millis();
       Wurf_Dauer = Wurfende_time - Wurfstart_time;
     }
  }
}*/
  ///////////////
  
//////Sideswaps erkennen

void SideSwap_Kalibrierung(){
  Kalibrierfeedback_enable = true;  
  
  if ((gForceTotal < SideSwap_FlugPhasen_Threshold) && (Wurf_Status == false)){
    Wurf_Start_Flag = millis();
    Wurf_Status = true;
  }
  else if ((gForceTotal >= SideSwap_FlugPhasen_Threshold) && (Wurf_Status == true)){
    if (Wurf_Status == true){
      Wurf_Status = false;
      Wurf_Ende_Flag = millis();
      Wurf_Dauer_ms = Wurf_Ende_Flag-Wurf_Start_Flag;
      Wurf_beendet = true;
    } 
  }
  if ((Wurf_beendet == true) && (Wurf_Dauer_ms > 300)){ //Testet, dass es wirklich ein Wurf war, und nicht nur ein kurzes Fallen in der Hand etc
    Kalibrier_3_Dauer += Wurf_Dauer_ms;
    Wurf_beendet = false;
    Kalibriercnt++;
    LED(255,0,255); //Kurz den Ball in grün aufleuchten lassen, wenn ein Kalibrierwurf geglückt ist
    Kalibrierfeedback_timer = millis() + 100;
  }
  if (Kalibriercnt >= 3){ //3 Testwürfe um Standardzeit für Wurf zu ermitteln
    Kalibrier_3_Dauer = Kalibrier_3_Dauer/Kalibriercnt; 
    Kalibriercnt = 0;
    Serial.write('Q');
    Serial.write((uint8_t)(Kalibrier_3_Dauer/20)); //durch 20, damit wert als uint8 an Handy geschickt werden kann. Aber eigentlich braucht das Handy den Wert auch nicht. Ist nur für mich als Feedback ob er grob passt 
    SD_SS_Kalibrierung = false;
  }
}

void SideSwap_Detection(){
  if ((gForceTotal < SideSwap_FlugPhasen_Threshold) && (Wurf_Status == false)){
    Wurf_Start_Flag = millis();
    Wurf_Status = true;
  }
  else if ((gForceTotal >= SideSwap_FlugPhasen_Threshold) && (Wurf_Status == true)){
    if (Wurf_Status == true){
      Wurf_Status = false;
      Wurf_Ende_Flag = millis();
      Wurf_Dauer_ms = Wurf_Ende_Flag-Wurf_Start_Flag;
      Wurf_beendet = true;
    } 
  }
  if ((Wurf_beendet == true) && (Wurf_Dauer_ms > 100)){ //Damit keine zu kurzen Würfe gezählt werden
    Wurf_beendet = false;
    SideSwap_Number_float = 3*Wurf_Dauer_ms/Kalibrier_3_Dauer; //Berechnung der Sideswapnummer als float. Die 3, da die Kalibriersauer auf 3en bezogen ist
    SideSwap_Number_float += 0.5; //Damit beim Abschneiden auf uint8_t richtig gerundet wird    
    SideSwap_Number = (uint8_t)(SideSwap_Number_float);
    Serial.write('O');
    Serial.write((uint8_t)SideSwap_Number);
  }  
}

///////////Color Change bei Wurf
void ColorChange_onThrow(int green_Wurf, int red_Wurf, int blue_Wurf, int green_Hand, int red_Hand, int blue_Hand){
  //if ((gForceBetragGlatt_Summe > ThrowThreshold)){
  //if ((gForceTotal < ColorChange_FlugPhasen_Threshold) || (gForceTotal > ColorChange_Spin_Threshold)){
  
  if ((gForceTotal < ColorChange_FlugPhasen_Threshold)){
  //if ((gForceGlatt < ColorChange_FlugPhasen_Threshold) || (gForceGlatt > ColorChange_Spin_Threshold)){  
    LED(green_Wurf, red_Wurf, blue_Wurf); 
  }
  else {
    LED(green_Hand, red_Hand, blue_Hand);
  }
}
//////////////////

void Scrollbars(){
  //LEDs neu ansteuern 
  ////for (int i=0; i<24; i++){ 
  ////  leds[i] = CRGB(LED_green, LED_red, LED_blue); 
  ////}
  ////FastLED.show(); //Signal an LEDs schicken
  LED(LED_red, LED_green, LED_blue); 
}
  
void Synchro(){
    Serial.write('S');  // Zahl an Handy zurückschicken als Zeichen, dass BT angekommen ist, damit Handy berechnen kann wie lange Übertragung gedauert hat
    millis_song_beginning = millis(); //checken wann Lichtfolge angefangen hat um diesen Wert mit den globalen millis() abzugleichen

    while (Serial.available() < 2){//Serial.write("gefangen");
  }; //Warten auf den Korrekturfaktor den das Handy berechnet und zurück schickt
    Timing_Korrektur = Serial.read() << 8;
    Timing_Korrektur += Serial.read();
    
    millis_song_beginning = millis_song_beginning - int(Timing_Korrektur/2) +Offset; //Startzeitpunkt um den Korrekturfaktor verschieben, damit Handy und Bälle synchron sind; Korrektur durch 2, da Signal einmal hin und wieder zurück zum Handy geht

    aufleuchten(0, 100, 0, 0);
    aufleuchten(0, 0, 0, 500);
    aufleuchten(0, 100, 0, 1000);
    aufleuchten(0, 0, 0, 1500);
    
  //wird glaub auch nicht gebraucht
    BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    millis_song_beginning = 0;
}

void Song1(){
    Serial.write('S');  // Zahl an Handy zurückschicken als Zeichen, dass BT angekommen ist, damit Handy berechnen kann wie lange Übertragung gedauert hat
    millis_song_beginning = millis(); //checken wann Lichtfolge angefangen hat um diesen Wert mit den globalen millis() abzugleichen

    while (Serial.available() < 2){//Serial.write("gefangen");
  }; //Warten auf den Korrekturfaktor den das Handy berechnet und zurück schickt
    Timing_Korrektur = Serial.read() << 8;
    Timing_Korrektur += Serial.read();
    
    millis_song_beginning = millis_song_beginning - int(Timing_Korrektur/2) +Offset; //Startzeitpunkt um den Korrekturfaktor verschieben, damit Handy und Bälle synchron sind; Korrektur durch 2, da Signal einmal hin und wieder zurück zum Handy geht
    //BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    

    //Ball1
    fading(0, 0, 255, 0, 0, 0, 0, 2528);
    fading(0, 0, 0, 0, 0, 255, 2528, 5056);
    fading(0, 0, 255, 0, 0, 0, 5056, 7584);
    fading(0, 0, 0, 0, 0, 255, 7584, 10112);
    fading(0, 0, 255, 0, 0, 0, 10112, 12640);
    fading(0, 0, 0, 0, 0, 255, 12640, 15168);
    fading(0, 0, 255, 0, 0, 0, 15168, 17696);
    fading(0, 0, 0, 0, 0, 255, 17696, 20224);
    fading(0, 0, 255, 0, 0, 0, 20224, 22752);
    fading(0, 0, 0, 0, 0, 255, 22752, 25280);
    fading(0, 0, 255, 0, 0, 0, 25280, 27808);
    fading(0, 0, 0, 0, 0, 255, 27808, 30336);
    fading(0, 0, 255, 0, 0, 0, 30336, 32864);
    fading(0, 0, 0, 0, 0, 255, 32864, 35392);
    fading(0, 0, 255, 0, 0, 0, 35392, 37920);
    fading(0, 0, 0, 0, 0, 255, 37920, 40448);
    fading(0, 0, 255, 0, 0, 0, 40448, 42976);
    fading(0, 0, 0, 0, 0, 255, 42976, 45504);
    fading(0, 0, 255, 0, 0, 0, 45504, 48032);
    fading(0, 0, 0, 0, 0, 255, 48032, 50560);
    fading(0, 0, 255, 0, 0, 0, 50560, 53088);
    fading(0, 0, 0, 0, 0, 255, 53088, 55616);
    fading(0, 0, 255, 0, 0, 0, 55616, 58144);
    fading(0, 0, 0, 0, 0, 255, 58144, 60672);
    fading(0, 0, 255, 0, 0, 0, 60672, 63200);
    fading(0, 0, 0, 0, 0, 255, 63200, 65728);
    fading(0, 0, 255, 0, 0, 0, 65728, 68256);
    fading(0, 0, 0, 0, 0, 255, 68256, 70784);
    fading(0, 0, 255, 0, 0, 0, 70784, 73312);
    fading(0, 0, 0, 0, 0, 255, 73312, 75840);
    fading(0, 0, 255, 0, 0, 0, 75840, 78368);
    fading(0, 0, 0, 0, 0, 255, 78368, 80896);
    fading(0, 0, 255, 0, 0, 0, 80896, 83424);
    fading(0, 0, 0, 0, 0, 255, 83424, 85952);
    fading(0, 0, 255, 0, 0, 0, 85952, 88480);
    fading(0, 0, 0, 0, 0, 255, 88480, 91008);
    fading(0, 0, 255, 0, 0, 0, 91008, 93536);
    fading(0, 0, 0, 0, 0, 255, 93536, 96064);
    fading(0, 0, 255, 0, 0, 0, 96064, 98592);
    fading(0, 0, 0, 0, 0, 255, 98592, 101120);
    fading(0, 0, 255, 0, 0, 0, 101120, 103648);
    fading(0, 0, 0, 0, 0, 255, 103648, 106176);
    fading(0, 0, 255, 0, 0, 0, 106176, 108704);
    fading(0, 0, 0, 0, 0, 255, 108704, 111232);
    fading(0, 0, 255, 0, 0, 0, 111232, 113760);
    fading(0, 0, 0, 0, 0, 255, 113760, 116288);
    fading(0, 0, 255, 0, 0, 0, 116288, 118816);
    fading(0, 0, 0, 0, 0, 255, 118816, 121344);
    fading(0, 0, 255, 0, 0, 0, 121344, 123872);
    fading(0, 0, 0, 0, 0, 255, 123872, 126400);
    fading(0, 0, 255, 255, 255, 255, 126400, 128928);
    fading(255, 255, 255, 0, 0, 0, 131456, 133984);
    fading(0, 0, 0, 255, 255, 255, 133984, 136512);
    fading(255, 255, 255, 0, 0, 0, 136512, 139040);
    fading(0, 0, 0, 255, 255, 255, 139040, 141568);
    fading(255, 255, 255, 0, 0, 0, 141568, 144096);
    fading(0, 0, 0, 255, 255, 255, 144096, 146624);
    fading(255, 255, 255, 0, 0, 0, 146624, 149152);
    fading(0, 0, 0, 255, 255, 255, 149152, 151680);
    fading(255, 255, 255, 0, 0, 0, 151680, 154208);
    fading(0, 0, 0, 255, 255, 255, 154208, 156736);
    fading(255, 255, 255, 0, 0, 0, 156736, 159264);
    fading(0, 0, 0, 255, 255, 255, 159264, 161792);
    fading(255, 255, 255, 0, 0, 0, 161792, 164320);
    fading(0, 0, 0, 255, 255, 255, 164320, 166848);
    fading(255, 255, 255, 0, 0, 0, 166848, 169376);
    fading(0, 0, 0, 255, 255, 255, 169376, 171904);
    fading(255, 255, 255, 0, 0, 0, 171904, 174432);
    fading(0, 0, 0, 255, 255, 255, 174432, 176960);
    fading(255, 255, 255, 0, 0, 0, 176960, 179488);
    fading(0, 0, 0, 255, 255, 255, 179488, 182016);
    fading(255, 255, 255, 0, 0, 0, 182016, 184544);
    fading(0, 0, 0, 255, 255, 255, 184544, 187072);
    fading(255, 255, 255, 0, 0, 0, 187072, 189600);
    fading(0, 0, 0, 255, 255, 255, 189600, 192128);
    fading(255, 255, 255, 0, 0, 0, 197184, 210000);


    /*
    //Leuchtsequenz starten
    aufleuchten(0, 0, 0, 0);
    fading(0, 0, 0, 0, 0, 255, 13000, 15000);
    aufleuchten(0, 0, 0, 20650);
    //aufleuchten(0, 0, 0, 22500);
    //aufleuchten(0, 255, 0, 24500);
    aufleuchten(255, 0, 0, 25400);
    //aufleuchten(0, 0, 0, 26300);
    //aufleuchten(0, 0, 0, 26800);
    aufleuchten(255, 255, 0, 27300);
    aufleuchten(255, 0, 0, 27500);
    aufleuchten(0, 0, 255, 27700);
    aufleuchten(0, 0, 0, 28200);
    fading(0, 0, 0, 255, 255, 255, 27900, 30000);
    fading(255, 255, 255, 0, 0, 0, 30000, 34000);
    */
    
    /*aufleuchten(200, 0, 0, 0);
    aufleuchten(0, 200, 0, 1000);
    aufleuchten(0, 0, 200, 2000);
    aufleuchten(200, 0, 0, 3000);
    aufleuchten(0, 200, 0, 4000);
    aufleuchten(0, 0, 200, 5000);
    aufleuchten(0, 0, 0, 6000);
  */
  
  //ich glaub das hier braucht man nicht
  //  if (Serial.available() > 0){
  //    while (Serial.available() > 0){
  //      Serial.read();  
  //  }
  //  }
  //wird glaub auch nicht gebraucht
    BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    millis_song_beginning = 0;
}

void Song2(){
    Serial.write('S');  // Zahl an Handy zurückschicken als Zeichen, dass BT angekommen ist, damit Handy berechnen kann wie lange Übertragung gedauert hat
    millis_song_beginning = millis(); //checken wann Lichtfolge angefangen hat um diesen Wert mit den globalen millis() abzugleichen

    
    while (Serial.available() < 2){//Serial.write("gefangen");
  }; //Warten auf den Korrekturfaktor den das Handy berechnet und zurück schickt
    Timing_Korrektur = Serial.read() << 8;
    Timing_Korrektur += Serial.read();
    
    millis_song_beginning = millis_song_beginning - int(Timing_Korrektur/2) +Offset; //Startzeitpunkt um den Korrekturfaktor verschieben, damit Handy und Bälle synchron sind; Korrektur durch 2, da Signal einmal hin und wieder zurück zum Handy geht
    //BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    
    //Leuchtsequenz starten

    //Ball1
    //Ball1
    aufleuchten(0, 0, 0, 0);
    aufleuchten(0, 0, 255, 7450);
    aufleuchten(0, 0, 0, 13000);
    aufleuchten(255, 0, 0, 13100);
    aufleuchten(0, 0, 0, 13480);
    aufleuchten(255, 0, 0, 13580);
    aufleuchten(0, 0, 0, 13960);
    aufleuchten(255, 0, 0, 14060);
    aufleuchten(0, 0, 0, 14440);
    aufleuchten(255, 0, 0, 14540);
    aufleuchten(0, 0, 0, 14920);
    aufleuchten(255, 0, 0, 15020);
    aufleuchten(0, 0, 0, 15400);
    aufleuchten(255, 0, 0, 15500);
    aufleuchten(0, 0, 0, 15880);
    aufleuchten(255, 0, 0, 15980);
    aufleuchten(0, 0, 0, 16360);
    aufleuchten(255, 0, 0, 16460);
    aufleuchten(0, 0, 0, 16840);
    aufleuchten(255, 0, 0, 16940);
    aufleuchten(0, 0, 0, 17320);
    aufleuchten(255, 0, 0, 17420);
    aufleuchten(0, 0, 0, 17800);
    aufleuchten(255, 0, 0, 17900);
    aufleuchten(0, 0, 0, 18280);
    aufleuchten(255, 0, 0, 18380);
    aufleuchten(0, 0, 0, 18760);
    aufleuchten(255, 0, 0, 18860);
    aufleuchten(0, 0, 0, 19240);
    aufleuchten(255, 0, 0, 19340);
    aufleuchten(0, 0, 0, 19720);
    aufleuchten(255, 0, 0, 19820);
    aufleuchten(0, 0, 0, 20200);
    aufleuchten(255, 0, 0, 20300);
    aufleuchten(0, 0, 0, 20680);
    aufleuchten(255, 0, 0, 20780);
    aufleuchten(0, 0, 0, 21160);
    aufleuchten(255, 0, 0, 21260);
    aufleuchten(0, 0, 0, 21640);
    aufleuchten(255, 0, 0, 21740);
    aufleuchten(0, 0, 0, 22120);
    aufleuchten(255, 0, 0, 22220);
    aufleuchten(0, 0, 0, 22600);
    aufleuchten(255, 0, 0, 22700);
    aufleuchten(0, 0, 0, 23080);
    aufleuchten(255, 0, 0, 23180);
    aufleuchten(0, 0, 0, 23560);
    aufleuchten(255, 0, 0, 23660);
    aufleuchten(0, 0, 0, 24040);
    aufleuchten(255, 0, 0, 24140);
    aufleuchten(0, 0, 0, 24520);
    aufleuchten(255, 0, 0, 24620);
    aufleuchten(0, 0, 0, 25000);
    aufleuchten(255, 0, 0, 25100);
    aufleuchten(0, 0, 0, 25480);
    aufleuchten(255, 0, 0, 25580);
    aufleuchten(0, 0, 0, 25960);
    aufleuchten(255, 0, 0, 26060);
    aufleuchten(0, 0, 0, 26440);
    aufleuchten(255, 0, 0, 26540);
    aufleuchten(0, 0, 0, 26920);
    aufleuchten(255, 0, 0, 27020);
    aufleuchten(0, 0, 0, 27400);
    aufleuchten(255, 0, 0, 27500);
    aufleuchten(0, 0, 0, 27880);
    aufleuchten(255, 0, 0, 27980);
    aufleuchten(0, 0, 0, 28360);
    aufleuchten(255, 0, 0, 28460);
    aufleuchten(0, 0, 0, 28840);
    aufleuchten(255, 0, 0, 28940);
    aufleuchten(0, 0, 0, 29320);
    aufleuchten(255, 0, 0, 29420);
    aufleuchten(0, 0, 0, 29800);
    aufleuchten(255, 0, 0, 29900);
    aufleuchten(0, 0, 0, 30280);
    aufleuchten(255, 0, 0, 30380);
    aufleuchten(0, 0, 0, 30760);
    aufleuchten(255, 0, 0, 30860);
    aufleuchten(0, 0, 0, 31240);
    aufleuchten(255, 0, 0, 31340);
    aufleuchten(0, 0, 0, 31720);
    aufleuchten(255, 0, 0, 31820);
    aufleuchten(0, 0, 0, 32200);
    aufleuchten(255, 0, 0, 32300);
    aufleuchten(0, 0, 0, 32680);
    aufleuchten(255, 0, 0, 32780);
    aufleuchten(0, 0, 0, 33160);
    aufleuchten(255, 0, 0, 33260);
    aufleuchten(0, 0, 0, 33640);
    aufleuchten(255, 0, 0, 33740);
    aufleuchten(0, 0, 0, 34120);
    aufleuchten(255, 0, 0, 34220);
    aufleuchten(0, 0, 0, 34600);
    aufleuchten(255, 0, 0, 34700);
    fading(255, 0, 0, 100, 100, 255, 35080, 37000);
    fading(100, 100, 255, 255, 50, 50, 51400, 52360);
    aufleuchten(255, 255, 255, 59000);
    aufleuchten(0, 0, 0, 60000);
    aufleuchten(255, 255, 255, 62000);
    aufleuchten(0, 0, 0, 63000);
    aufleuchten(255, 255, 255, 64100);
    aufleuchten(0, 0, 0, 64200);
    aufleuchten(255, 255, 255, 64400);
    aufleuchten(0, 0, 0, 64500);
    aufleuchten(255, 255, 255, 64700);
    aufleuchten(0, 0, 0, 65000);
    aufleuchten(255, 255, 255, 65100);
    aufleuchten(0, 0, 0, 65500);
    aufleuchten(255, 255, 255, 65600);
    aufleuchten(0, 0, 0, 66000);
    aufleuchten(255, 255, 255, 66100);
    fading(255, 255, 255, 0, 0, 0, 70000, 71920);
    fading(0, 0, 0, 255, 255, 255, 71920, 73840);
    fading(255, 255, 255, 0, 0, 0, 73840, 75760);
    fading(0, 0, 0, 255, 255, 255, 75760, 77680);
    fading(255, 255, 255, 0, 0, 0, 77680, 79600);
    fading(0, 0, 0, 255, 255, 255, 79600, 81520);
    fading(255, 255, 255, 0, 0, 0, 81520, 83440);
    fading(0, 0, 0, 255, 255, 255, 83440, 85360);
    fading(255, 255, 255, 0, 0, 0, 85360, 87280);
    fading(0, 0, 0, 255, 255, 255, 87280, 89200);
    fading(255, 255, 255, 0, 0, 0, 89200, 91120);
    fading(0, 0, 0, 255, 255, 255, 91120, 93040);
    fading(255, 255, 255, 0, 0, 0, 93040, 94960);
    fading(0, 0, 0, 255, 255, 255, 94960, 96880);
    fading(255, 255, 255, 0, 0, 0, 96880, 98800);
    fading(0, 0, 0, 255, 255, 255, 98800, 100720);
    fading(255, 255, 255, 0, 0, 0, 112800, 114000);


/*
    
    aufleuchten(0, 0, 0, 0);
    aufleuchten(0, 0, 0, 0);
    aufleuchten(255, 255, 255, 570);
    aufleuchten(0, 0, 0, 600);
    aufleuchten(255, 255, 255, 1770);
    aufleuchten(0, 0, 0, 1800);
    aufleuchten(255, 255, 255, 2970);
    aufleuchten(0, 0, 0, 3000);
    aufleuchten(255, 255, 255, 4170);
    aufleuchten(0, 0, 0, 4200);
    aufleuchten(255, 255, 255, 5370);
    aufleuchten(0, 0, 0, 5400);
    aufleuchten(255, 255, 255, 6570);
    aufleuchten(0, 0, 0, 6600);
    aufleuchten(255, 255, 255, 7770);
    aufleuchten(0, 0, 0, 7800);
    aufleuchten(255, 255, 255, 9020);
    aufleuchten(0, 0, 0, 9050);
    aufleuchten(255, 255, 255, 10170);
    aufleuchten(0, 0, 0, 10200);
    aufleuchten(255, 255, 255, 11370);
    aufleuchten(0, 0, 0, 11400);
    fading(0, 0, 0, 0, 0, 255, 12260, 13130);
    fading(0, 0, 255, 0, 0, 0, 13130, 14000);
    fading(0, 0, 0, 0, 0, 255, 14900, 15500);
    fading(0, 0, 255, 0, 0, 0, 15500, 16100);
    aufleuchten(255, 255, 255, 16170);
    aufleuchten(0, 0, 0, 16200);
    fading(0, 0, 0, 0, 0, 255, 17070, 17735);
    fading(0, 0, 255, 0, 0, 0, 17735, 18400);
    
    fading(0, 0, 0, 0, 0, 255, 19400, 19950);
    fading(0, 0, 255, 0, 0, 0, 19950, 20500);
    
    fading(0, 0, 0, 0, 0, 255, 20500, 21250);
    fading(0, 0, 255, 0, 0, 0, 21250, 22000);
    
    fading(0, 0, 0, 0, 0, 255, 22000, 22650);
    fading(0, 0, 255, 0, 0, 0, 22650, 23300);
    
    aufleuchten(255, 0, 0, 23800);  //red
    aufleuchten(255, 255, 0, 25000);  //yellow
    aufleuchten(0, 0, 255, 26150);  //blue
    aufleuchten(160, 0, 255, 27350); //purple
    aufleuchten(0, 255, 0, 28550);  //green
    aufleuchten(255, 0, 255, 29000); //pink
    aufleuchten(0, 0, 0, 29500);  //black
    aufleuchten(255, 255, 255, 29950);  //white
    aufleuchten(255, 0, 0, 31000); //every
    aufleuchten(255, 255, 0, 31550); //color
    aufleuchten(0, 0, 255, 32400); //like

    aufleuchten(255, 0, 0, 33400);  //red
    aufleuchten(255, 255, 0, 34550);  //yellow
    aufleuchten(0, 0, 255, 35750);  //blue
    aufleuchten(160, 0, 255, 36950); //purple
    aufleuchten(0, 255, 0, 38150);  //green
    aufleuchten(255, 0, 255, 38600); //pink
    aufleuchten(0, 0, 0, 39100);  //black
    aufleuchten(255, 255, 255, 39550);  //white
    aufleuchten(255, 0, 0, 40550); //every
    aufleuchten(255, 255, 0, 41250); //color
    aufleuchten(0, 0, 255, 41950); //like

    aufleuchten(255, 0, 0, 43000);  //red
    aufleuchten(255, 255, 0, 44150);  //yellow
    aufleuchten(0, 0, 255, 45350);  //blue
    aufleuchten(160, 0, 255, 46550); //purple
    aufleuchten(0, 255, 0, 47750);  //green
    aufleuchten(255, 0, 255, 48200); //pink
    aufleuchten(0, 0, 0, 48650);  //black
    aufleuchten(255, 255, 255, 49100);  //white
    aufleuchten(255, 0, 0, 50150); //every
    aufleuchten(255, 255, 0, 50850); //color
    aufleuchten(0, 0, 255, 51550); //like

    aufleuchten(0, 0, 0, 52170);  //black
    
    */
    
  //wird glaub auch nicht gebraucht
    BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    millis_song_beginning = 0;
}

void Song3(){
    Serial.write('S');  // Zahl an Handy zurückschicken als Zeichen, dass BT angekommen ist, damit Handy berechnen kann wie lange Übertragung gedauert hat
    millis_song_beginning = millis(); //checken wann Lichtfolge angefangen hat um diesen Wert mit den globalen millis() abzugleichen

    
    while (Serial.available() < 2){//Serial.write("gefangen");
  }; //Warten auf den Korrekturfaktor den das Handy berechnet und zurück schickt
    Timing_Korrektur = Serial.read() << 8;
    Timing_Korrektur += Serial.read();
    
    millis_song_beginning = millis_song_beginning - int(Timing_Korrektur/2) +Offset; //Startzeitpunkt um den Korrekturfaktor verschieben, damit Handy und Bälle synchron sind; Korrektur durch 2, da Signal einmal hin und wieder zurück zum Handy geht
    //BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    
    //Leuchtsequenz starten

    //Ball1
    aufleuchten(0, 0, 0, 40000);
    aufleuchten(0, 0, 255, 47450);
    aufleuchten(0, 0, 0, 53000);
    aufleuchten(255, 0, 0, 53100);
    aufleuchten(0, 0, 0, 53480);
    aufleuchten(255, 0, 0, 53580);
    aufleuchten(0, 0, 0, 53960);
    aufleuchten(255, 0, 0, 54060);
    aufleuchten(0, 0, 0, 54440);
    aufleuchten(255, 0, 0, 54540);
    aufleuchten(0, 0, 0, 54920);
    aufleuchten(255, 0, 0, 55020);
    aufleuchten(0, 0, 0, 55400);
    aufleuchten(255, 0, 0, 55500);
    aufleuchten(0, 0, 0, 55880);
    aufleuchten(255, 0, 0, 55980);
    aufleuchten(0, 0, 0, 56360);
    aufleuchten(255, 0, 0, 56460);
    aufleuchten(0, 0, 0, 56840);
    aufleuchten(255, 0, 0, 56940);
    aufleuchten(0, 0, 0, 57320);
    aufleuchten(255, 0, 0, 57420);
    aufleuchten(0, 0, 0, 57800);
    aufleuchten(255, 0, 0, 57900);
    aufleuchten(0, 0, 0, 58280);
    aufleuchten(255, 0, 0, 58380);
    aufleuchten(0, 0, 0, 58760);
    aufleuchten(255, 0, 0, 58860);
    aufleuchten(0, 0, 0, 59240);
    aufleuchten(255, 0, 0, 59340);
    aufleuchten(0, 0, 0, 59720);
    aufleuchten(255, 0, 0, 59820);
    aufleuchten(0, 0, 0, 60200);
    aufleuchten(255, 0, 0, 60300);
    aufleuchten(0, 0, 0, 60680);
    aufleuchten(255, 0, 0, 60780);
    aufleuchten(0, 0, 0, 61160);
    aufleuchten(255, 0, 0, 61260);
    aufleuchten(0, 0, 0, 61640);
    aufleuchten(255, 0, 0, 61740);
    aufleuchten(0, 0, 0, 62120);
    aufleuchten(255, 0, 0, 62220);
    aufleuchten(0, 0, 0, 62600);
    aufleuchten(255, 0, 0, 62700);
    aufleuchten(0, 0, 0, 63080);
    aufleuchten(255, 0, 0, 63180);
    aufleuchten(0, 0, 0, 63560);
    aufleuchten(255, 0, 0, 63660);
    aufleuchten(0, 0, 0, 64040);
    aufleuchten(255, 0, 0, 64140);
    aufleuchten(0, 0, 0, 64520);
    aufleuchten(255, 0, 0, 64620);
    aufleuchten(0, 0, 0, 65000);
    aufleuchten(255, 0, 0, 65100);
    aufleuchten(0, 0, 0, 65480);
    aufleuchten(255, 0, 0, 65580);
    aufleuchten(0, 0, 0, 65960);
    aufleuchten(255, 0, 0, 66060);
    aufleuchten(0, 0, 0, 66440);
    aufleuchten(255, 0, 0, 66540);
    aufleuchten(0, 0, 0, 66920);
    aufleuchten(255, 0, 0, 67020);
    aufleuchten(0, 0, 0, 67400);
    aufleuchten(255, 0, 0, 67500);
    aufleuchten(0, 0, 0, 67880);
    aufleuchten(255, 0, 0, 67980);
    aufleuchten(0, 0, 0, 68360);
    aufleuchten(255, 0, 0, 68460);
    aufleuchten(0, 0, 0, 68840);
    aufleuchten(255, 0, 0, 68940);
    aufleuchten(0, 0, 0, 69320);
    aufleuchten(255, 0, 0, 69420);
    aufleuchten(0, 0, 0, 69800);
    aufleuchten(255, 0, 0, 69900);
    aufleuchten(0, 0, 0, 70280);
    aufleuchten(255, 0, 0, 70380);
    aufleuchten(0, 0, 0, 70760);
    aufleuchten(255, 0, 0, 70860);
    aufleuchten(0, 0, 0, 71240);
    aufleuchten(255, 0, 0, 71340);
    aufleuchten(0, 0, 0, 71720);
    aufleuchten(255, 0, 0, 71820);
    aufleuchten(0, 0, 0, 72200);
    aufleuchten(255, 0, 0, 72300);
    aufleuchten(0, 0, 0, 72680);
    aufleuchten(255, 0, 0, 72780);
    aufleuchten(0, 0, 0, 73160);
    aufleuchten(255, 0, 0, 73260);
    aufleuchten(0, 0, 0, 73640);
    aufleuchten(255, 0, 0, 73740);
    aufleuchten(0, 0, 0, 74120);
    aufleuchten(255, 0, 0, 74220);
    aufleuchten(0, 0, 0, 74600);
    aufleuchten(255, 0, 0, 74700);
    fading(255, 0, 0, 100, 100, 255, 75080, 77000);
    fading(100, 100, 255, 255, 50, 50, 91400, 92360);
    aufleuchten(255, 255, 255, 99000);
    aufleuchten(0, 0, 0, 100000);
    aufleuchten(255, 255, 255, 102000);
    aufleuchten(0, 0, 0, 103000);
    aufleuchten(255, 255, 255, 104100);
    aufleuchten(0, 0, 0, 104200);
    aufleuchten(255, 255, 255, 104400);
    aufleuchten(0, 0, 0, 104500);
    aufleuchten(255, 255, 255, 104700);
    aufleuchten(0, 0, 0, 105000);
    aufleuchten(255, 255, 255, 105100);
    aufleuchten(0, 0, 0, 105500);
    aufleuchten(255, 255, 255, 105600);
    aufleuchten(0, 0, 0, 106000);
    aufleuchten(255, 255, 255, 106100);
    fading(255, 255, 255, 0, 0, 0, 110000, 111920);
    fading(0, 0, 0, 255, 255, 255, 111920, 113840);
    fading(255, 255, 255, 0, 0, 0, 113840, 115760);
    fading(0, 0, 0, 255, 255, 255, 115760, 117680);
    fading(255, 255, 255, 0, 0, 0, 117680, 119600);
    fading(0, 0, 0, 255, 255, 255, 119600, 121520);
    fading(255, 255, 255, 0, 0, 0, 121520, 123440);
    fading(0, 0, 0, 255, 255, 255, 123440, 125360);
    fading(255, 255, 255, 0, 0, 0, 125360, 127280);
    fading(0, 0, 0, 255, 255, 255, 127280, 129200);
    fading(255, 255, 255, 0, 0, 0, 129200, 131120);
    fading(0, 0, 0, 255, 255, 255, 131120, 133040);
    fading(255, 255, 255, 0, 0, 0, 133040, 134960);
    fading(0, 0, 0, 255, 255, 255, 134960, 136880);
    fading(255, 255, 255, 0, 0, 0, 136880, 138800);
    fading(0, 0, 0, 255, 255, 255, 138800, 140720);
    fading(255, 255, 255, 0, 0, 0, 152800, 154000);

    
  //wird glaub auch nicht gebraucht
    BT_signal[0] = 'Z'; //BT_signal auf 'Z' setzen, damit nicht ungewollt nochmal ein Befehl ausgeführt wird (Z nie mit einer Funktion zum Ausführen belegen)
    millis_song_beginning = 0;
}

////Funktion um in einer Lichtabfolge alle LEDs ab einem festen Zeitpunkt in derselben Farbe leuchten zu lassen
void aufleuchten(int red,int green,int blue,long time){
    long millis2; 
    do{ //mache nichts
      millis2 = millis(); //gucken wie spät es jetzt ist
    }while(time > millis2-millis_song_beginning); // solange nichts machen bis der einprogrammierte Zeitpunkt gekommen ist  
    //dann die Farben aller LEDs ändern
    ////for (int i=0; i<24; i++){ 
    ////    leds[i] = CRGB(green, red, blue); 
    ////}
    ////FastLED.show();  //Signal mit neuen Farben an LEDs senden
    LED(red, green, blue); 
    BT_Empfangen();
    if (BT_signal[0] == 'x'){
        millis_song_beginning = - 2000000000;
    }
}
////

////Funktion um alle LEDs von einer Fade zu einer anderen Farbe in einem festen Zeitintervall die Farbe wechseln zu lassen
void fading(int red_start, int green_start, int blue_start, int red_end, int green_end, int blue_end, unsigned long time_start, unsigned long time_end){
  float green; //Variablen für zwischen Farben als float, falls die Schritte mal keine ganze Zahl sind
  float red;
  float blue;
  uint8_t green_int; //um den Wert an die LEDs zu übergeben wird die aktuelle Farbe in int gewandelt
  uint8_t red_int;
  uint8_t blue_int;
  float green_step; //Schritt der Farbe pro Dauer[ms]/solution
  float red_step;
  float blue_step;
  unsigned long duration; //Dauer über die der Farbwechsel erfolgt
  float fadewidth_green; //Abstand der Farb Start- und Endwerte 
  float fadewidth_red;
  float fadewidth_blue;
  unsigned long solution = 10; //Auflösung der Schritte. solution gibt die Anzahl der ms zwischen Farbwechseln
  duration = (time_end - time_start)/solution;

  fadewidth_green = green_end-green_start; //bestimmen wie groß der Farbabstand ist, um die Schrittweite zu berechnen
  fadewidth_red = red_end-red_start;
  fadewidth_blue = blue_end-blue_start;

  green_step = (fadewidth_green)/duration; //Schrittweite berechnen
  red_step = (fadewidth_red)/duration;
  blue_step = (fadewidth_blue)/duration;

  green = green_start; //erste Farbe auf den Startwert setzen
  red = red_start;
  blue = blue_start;
  
  while((time_start) > (millis()-millis_song_beginning)){} //warten bis die Startzeit erreicht wird

  for (int duration_cnt=0; duration_cnt<duration; duration_cnt++){  //Alle Schritte durchgehen und jeweils die Farben anpassen
    green = green + green_step;  //Farbe um den nächsten Farbschritt erhöhen 
    red = red + red_step;   
    blue = blue + blue_step;
    
    green_int = (int) green; //aktuelle float Farben in int konvertieren, um sie an die LEDs weiterleiten zu können
    red_int = (int) red;
    blue_int = (int) blue;

    
    LED(red_int, green_int, blue_int); 
    
    while((time_start+(duration_cnt*solution)) > (millis()-millis_song_beginning)){} //warten bis der Zeitschritt vorbei ist bevor zum nächsten weiter gegangen wird
  }
  BT_Empfangen();
    if (BT_signal[0] == 'x'){
        millis_song_beginning = - 2000000000;
    }
}

//============
//Check ob ein Startzeichen unter empfangenen Daten ist, wenn ja werden folgende Daten gespeichert, bis das Endzeichen empfangen wird
void recvWithStartEndMarkers() {
    if (Serial.available() > 0 && newData == false) { //Abfrage ob was da ist, damit nicht jedes mal Variablen neu initialisiert werden müssen
  
    static boolean recvInProgress = false;  //
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) { //solange Daten lesbar sind
        rc = Serial.read(); //Daten in rc lesen

        if (recvInProgress == true) { //wenn Startmaker erkannt wurde:
            if (rc != endMarker) { //und kein Endmaker erkannt wurde:
                receivedChars[ndx] = rc; //erweiter das Array um das empfangene Zeichen
                ndx++; //nächste Stelle im Array
                if (ndx >= numChars) {  //wenn Array voll, wird das neue Zeichen über den letzten Eintrag geschrieben
                    ndx = numChars - 1; 
                }
            }
            else {  //wenn das empfangene Zeichen der Endmaker ist ">"
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;  //stoppe das speichern der eingehenden Daten
                ndx = 0; //zeige wieder auf den Eingang des Arrays
                newData = true; //gib Bescheid, dass neue Daten empfangen wurden und abgerufen werden können
            }
        }

        else if (rc == startMarker) { //wenn erste empfangene Daten = "<" dann werden die folgenden Daten gelesen und in Variablen gespeichert (wenn kein Startmaker vorhanden, werden die folgenden Daten ignoriert)
            recvInProgress = true;
        }
    }
    }
}

//============
//zerlegen der Daten in einen Buchstaben und eine Zahl
void parseData() { 

    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    strcpy(BT_signal, strtokIndx); // copy it to messageFromPC
 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    BT_wert = atoi(strtokIndx);     // convert this part to an integer
}

//============


//============
void LED(uint8_t red, uint8_t green, uint8_t blue){
  red_safe = int(1.8/5*red); //5V 1.6V fallen an roten LEDs ab. So wird Spannung bei auf Bereich zwischen 0 und 1.6V gemapt
  green_safe = int(2.1/5*green);
  blue_safe = int(2.9/5*blue);
  
  analogWrite(Red_PIN, red_safe);
  analogWrite(Green_PIN, green_safe);
  analogWrite(Blue_PIN, blue_safe); 
  /*
  analogWrite(Red_PIN, KorrekturFaktor255 - red);
  analogWrite(Green_PIN, KorrekturFaktor255 - green);
  analogWrite(Blue_PIN, KorrekturFaktor255 - blue); */
}
//============


//=========== Funktion zum ausführen der unterschiedlichen Beschleunigungssensor bezogenen Funktionen
void SmartDetect(){
  //Serial.println("in smartsetect");
  //wenn ins Menu zurück gewechselt wird, sollen alle Beschleunigungssensorfunktionen beendet werden
  if (SD_MenuPressed == true){
    SD_ColorChange = false;
    SD_Sideswaps = false;
    SD_ShowAccel = false;
    SD_MenuPressed = false;
  }
  
  //wenn eine der Funktionen angefordert wird, sollen Beschleunigungsdaten vom Sensor gelesen werden
  if ((SD_ColorChange == true) || (SD_Sideswaps == true) || (SD_ShowAccel == true) || (SD_SS_Kalibrierung)){
     recordAccelRegisters();
  }
  
  if (SD_ColorChange == true){
    ColorChange_onThrow(255,0,0,0,0,0);
  }
  
  if(SD_SS_Kalibrierung == true){
    SideSwap_Kalibrierung();
  }
  
    //Abfrage um letztes Signal der Kalibrierung (grün aufleuchten auch wieder abzuschalten, obwohl Funktion SideSwap_Kalibrierung nicht mehr ausgeführt wird
    if ((Kalibrierfeedback_timer < millis()) && (Kalibrierfeedback_enable == true)){
      LED(255,255,255); //Ball wieder ausmachen wenn timerZeit abgelaufen ist
      if (SD_SS_Kalibrierung == false){
        Kalibrierfeedback_enable = false;  
      }
    }  
  
  if (SD_Sideswaps == true){
    SideSwap_Detection(); //Daten werden noch in der Funktion ans Handy geschickt
  }
  
  if (SD_ShowAccel == true){
    if (millis_ShowAccel <= millis()){
      if (new_gForceGlatt == true){
        millis_ShowAccel = millis() + TIMESTEP_SHOWACCEL;
        Serial.write('P');
        Serial.write((uint8_t)gForceGlatt); 
        new_gForceGlatt = false;
        /*Serial.println();
        Serial.print("DataCnt: ");
        Serial.println(DataCnt);*/
        DataCnt = 0;
      }
    }
  } 
}
//=============


//============Einrichten des Beschleunigungssensors MPU-6050
void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  //Wire.end();
  //Serial.println("jo!");
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  //Wire.end();
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g 
  // 0b00000000 = +-2g    16384LSB/g
  // 0b00001000 = +-4g    8192LSB/g
  // 0b00010000 = +-8g    4069LSB/g
  // 0b00011000 = +-16g   2048LSB/g 
  Wire.endTransmission();  
  //Wire.end(); 
  //Serial.println("SetupMPU beendet");
}

void recordAccelRegisters() {
  //Serial.println("loopq");
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  //Serial.println("loopw");
  Wire.write(0x3B); //Starting register for Accel Readings
  //Serial.println("loope");
  Wire.endTransmission();
  //Wire.end();
  //Serial.println("loopr");
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  //Serial.println("hänge weil mpu keine Daten schickt");
  while(Wire.available() < 6);
  //Serial.println("hänge irgendwo");
  //Serial.println("loopu");
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  //Serial.println("loopg");
  processAccelData();
  //Serial.println("loopt");
}

void processAccelData(){
  gForceX = accelX / 16384.0; //diesen wert anpassen an Genauigkeit
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;

  gForceX = gForceX+0.01;
  gForceY = gForceY-0.02;
  gForceZ = gForceZ-0.15;

  /*Serial.print("gForceX: ");
  Serial.print(gForceX);
  Serial.print("    ");
  Serial.print("gForceY: ");
  Serial.print(gForceY);
  Serial.print("    ");
  Serial.print("gForceZ: ");
  Serial.println(gForceZ);
  */
  gForceX = abs(gForceX);  //damit egal ist wierum man den Sensor hält
  gForceY = abs(gForceY);
  gForceZ = abs(gForceZ);
  
  //Sensorungenauigkeit korrigieren, sodass alle Richtung bei perfekter Ausrichtung 1g zurück geben
  gForceX = gForceX/1.0;
  gForceY = gForceY/1.02;
  gForceZ = gForceZ/0.87;

  
  
  
  //gForceY = gForceY-11;
 /* Serial.print(gForceX);
    Serial.print("|");
    Serial.print(gForceY);
    Serial.print("|");
    Serial.println(gForceZ);
  */
  gForceX = (gForceX)*73.9; //Wenn alle Werte maximalen Ausschlag geben (2g) dann soll übermittelter Wert 256 ergeben für unsigned 8bit-zahl. also 256/(2*sqrt(3)) 
  gForceY = (gForceY)*73.9; //muss natürlich alles angepasst werden, falls größere Beschleunigungen gegeben sind
  gForceZ = (gForceZ)*73.9;
  gForceTotal = sqrt(gForceX*gForceX+gForceY*gForceY+gForceZ*gForceZ);

 //Serial.println(gForceTotal);
 
 ///////////Glätten nach Raui
  gForceBetrag = abs((gForceTotal-74));
  //gForceBetrag = 2;
  for(int i=0; i<(gForceBetragGlatt_samplingrange-1); i++){
    gForceBetragGlattEinzelWerte[i] = gForceBetragGlattEinzelWerte[i+1];
  }
  gForceBetragGlattEinzelWerte[gForceBetragGlatt_samplingrange-1] = gForceBetrag*gForceBetrag; //quadrieren um höhere Werte mehr zu gewichten
  
  gForceBetragGlatt_Summe = 0;
  for(int i=0; i<gForceBetragGlatt_samplingrange; i++){
    gForceBetragGlatt_Summe = gForceBetragGlatt_Summe + gForceBetragGlattEinzelWerte[i];
  }
  gForceBetragGlatt_Summe = gForceBetragGlatt_Summe/gForceBetragGlatt_samplingrange;
  ////////////
  
  
  DataCnt++;
  
  
  //Wert glätten
  GlattFaktor_cnt++;
  gForceGlatt_Summe = gForceGlatt_Summe + gForceTotal;
  if (GlattFaktor_cnt >= GlattFaktor){
    gForceGlatt = gForceGlatt_Summe/GlattFaktor_cnt;
    gForceGlatt_Summe = 0;
    GlattFaktor_cnt = 0;
    new_gForceGlatt = true;
  }
}

void printData() {
  Serial.write(gForceTotal);
}


void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  //Wire.end();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0; 
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0; 
  //Auflösung 0  +-250°/s  131 LSB(°/s)
  //Auflösung 1  +-500°/s  65,5 LSB(°/s)
  //Auflösung 2  +-1000°/s  32,8 LSB(°/s)
  //Auflösung 3  +-2000°/s  16,4 LSB(°/s)
}
