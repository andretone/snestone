/*Andrea Tone's code     andrea.aka,tone@gmail.com for infos and more*/
/*codice arduino per implementazione SNES controller bluetooth HID*/
/* alcune scelte progettuali sono state ispirate dai seguenti progetti/tutorial liberamente 
accessibili dal web ai seguenti linnk:
http://www.instructables.com/id/Bluetooth-SNES-Controller-for-Android/
http://www.kobakant.at/DIY/?p=3310
*/
/*componenti essenziali per il mio progetto
-arduino mini pro 3.3V
-Modem Bluetooth - BlueSMiRf HID
*/

/* todo: bottone che se premuto sposta l'utilizzo della croce da 
analogico a D-pad*/

//pin
int latch = 5;
int clock = 6;
int data = 7;

//pin scelta modo DPAD va attivato il push pull
int mode = 12;

//inizio del rawinput
byte preambolo = 0xFD;
byte length = 0x06;

//global array, SNES POLLING/SHIFTING ORIGINALS RESULTS
boolean controllerData[16];

boolean dpadmode = false;

//4 byte definiscono rispettivamente 2 assi e  tasti
byte b [4];

//sapere se devo inviare qualcosa o no
boolean anyP = false;

//il raw input da inviare ogni volta, preambolo e lunghezza restano 
//sempre uguali
byte rawinput [8];

void setup() {
  Serial.begin(57600);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, INPUT);
  
  //modepin e attivo push pull
  pinMode(mode, INPUT);
  digitalWrite(mode, HIGH);
  
  //inizializzazione raw input, parte che non cambia
  rawinput[0] = preambolo;
  rawinput[1] = length;
  dpadmode = false;
}

//read buttons pressed on snes controller, write results on controllerData global array
void readButtons(){
  digitalWrite(latch, LOW);
  digitalWrite(clock, LOW);

  //segnale di latch
  digitalWrite(latch, HIGH);
  delayMicroseconds(12);
  digitalWrite(latch, LOW);

  delayMicroseconds(6);

  controllerData[0] = (digitalRead(data) == HIGH);

  for (int i = 1; i < 16; i++){
    digitalWrite(clock, HIGH);
    delayMicroseconds(6);
    digitalWrite(clock, LOW);
    delayMicroseconds(6);
    controllerData[i] = (digitalRead(data) == HIGH);
  }
  // Button Reference
  // 01111111 11111111 - B
  // 10111111 11111111 - Y
  // 11011111 11111111 - Select
  // 11101111 11111111 - Start
  // 11110111 11111111 - Up
  // 11111011 11111111 - Down
  // 11111101 11111111 - Left
  // 11111110 11111111 - Right
  // 11111111 01111111 - A
  // 11111111 10111111 - X
  // 11111111 11011111 - L
  // 11111111 11101111 - R
}


/*
i nuovi joypad hanno questo schema:
posizione bottone
0  b
1  a
2  y
3  x
4  L
5  R
6  (L2)
7  (L3)
8  select
9  start
10  (L3)
11  (L4)
12  (D-up)
13  (D-down)
14  (D-left)
15  (D-right)
*/

void inTo2Bytes(byte * bytes){//ritorna 3 byte
  //si guarda controllerData nelle giuste posizioni:
  
  bytes[0] = 0x00;//asseX -127 a +127
  bytes[1] = 0x00;//asseY -127 a +127
  bytes[2] = 0x00;//tutto zero
  bytes[3] = 0x00;//tutto zero
  
  for( int p = 0; p < 12; p++){
    if(!controllerData[p]){
      switch(p){
        case 4://UP
          if(dpadmode){
            bytes[3] = bytes[3] | 0x10;
          }else{
            bytes[1] = 0x81;
          }
          break;
        case 5://DOWN
          if(dpadmode){
            bytes[3] = bytes[3] | 0x20;
          }else{
            bytes[1] = 0x7F; //+127
          }
          break;
        case 6://LEFT
          if(dpadmode){
            bytes[3] = bytes[3] | 0x40;
          }else{
            bytes[0] = 0x81;
          }
          break;
        case 7://RIGHT
          if(dpadmode){
            bytes[3] = bytes[3] | 0x80;
          }else{
            bytes[0] = 0x7F;
          }
          break;
        //cambiamo in un ordine naturale, dal primario al meno primario
        case 0://B
          bytes[2] = bytes[2] | 0x01;
          break;        
        case 8://A
          bytes[2] = bytes[2] | 0x02;
          break;        
        case 1://Y
          bytes[2] = bytes[2] | 0x04;
          break;
        case 9://X
          bytes[2] = bytes[2] | 0x08;
          break;
        case 10://L
          bytes[2] = bytes[2] | 0x10;
          break;
        case 11://R
          bytes[2] = bytes[2] | 0x20;
          break;
        case 2://SEL
          bytes[3] = bytes[3] | 0x01;
          break;
        case 3://START
          bytes[3] = bytes[3] | 0x02;
          break;
      }
    }
  }
}

//compone ed invia i dati secondo la descrizione specificata dal Bluesmirf
void makeRawHIDInput(byte data []){  
  rawinput[2] = data[0];
  rawinput[3] = data[1];
  rawinput[4] = 0x00;
  rawinput[5] = 0x00;
  rawinput[6] = data[2];
  rawinput[7] = data[3];
}

//invio dati al modulo bluetooth
void send2serial(){
  for(int a= 0; a < 8; a++){
    Serial.write(rawinput[a]);
  }
  /*
  //test
  for(int a= 0; a < 8; a++){
    Serial.print(rawinput[a],HEX);
    Serial.print("|");
  }
  Serial.print("\n");*/
  
}

void readMode(){
  if( digitalRead(mode) == HIGH ){
    dpadmode = true;
  }else{
    dpadmode = false;
  }
}

void loop() {
  readButtons();
  readMode();
  inTo2Bytes(b);
  makeRawHIDInput(b);
  send2serial();
  delay(16);
}

