import processing.serial.*;
import java.util.Arrays; 

int bufferSize= 1;
int visLen = 1024;
String serialString = "tty.wch";
int curPos = 0;
Table saveTable = new Table();
boolean startRecord = false;

class SerialThread implements Runnable {
  Serial serial;
  public SerialThread(Serial serial) {
    this.serial = serial;
  }

  public void run() {
    
    byte[] buffer = new byte[65536];
    while(true) {
      while(serial.available() <= 0){
        delay(1);
      }
      
      int readlen = serial.readBytesUntil(0x80, buffer);
      if(readlen > 0) {
        onBufferReceived(buffer, readlen);
        //print(","+readlen);
        //flush();
      }
    }
  }
}



float spectrum[] = new float[visLen];


Serial myPort;  // The serial port
Thread serialThread;

String samplingFreq;

void setup() {
  size(1024,400);
  frameRate(120);
  background(0);
  
  for(int i =0; i< bufferSize; i++){
    saveTable.addColumn("bin" + i);
    
  }

  // List all the available serial ports
  printArray(Serial.list());
  // Open the port you are using at the rate you want:
  for(String s : Serial.list()){
    if(s.contains(serialString)){
      if(!tryRepeatConnect(s)){
        return;
      }
    }
  }
  
  serialThread = new Thread(new SerialThread(myPort));
  serialThread.setDaemon(true);
  serialThread.start();
}

void draw() {

  background(0);
    stroke(255);
    noStroke(); 
    if(spectrum != null && spectrum.length >= visLen){    
      for(int i = 0; i < visLen; i++){
        float xPos = map(i,0,visLen,0,width);
        rect(xPos, 400, 1, -spectrum[i]/10);
      }
    }  
}

boolean tryRepeatConnect(String portName){
  for(int i = 0; i < 10; i++){
      myPort = tryConnect(portName, 115200);
      if(myPort == null){
        println("Retrying ...");
        continue;
      }
      println("Connection succeeded!");
      return true;
  }
  return false;
}

Serial tryConnect(String portName, int baudrate){
  Serial port = null;
  try{
    port = new Serial(this, portName, baudrate);
    return port;
  }catch (Exception e){
    return null;
  }
}

void onBufferReceived(byte[] buffer, int readlen) {
  
  
  if(readlen != (bufferSize * 2 + 1)) {
    println("Warning: got bad buffer length: " + readlen);
    return;
  }
  
  short val = 0;
  
  for(int i=0; i<bufferSize; i++) {
    val = (short)((buffer[2*i] & 0xff) | ((buffer[2*i+1] & 0xff) << 8));
    
  }
  
  spectrum[curPos] = val;
  
  if(startRecord){
    TableRow newRow;
    newRow = saveTable.addRow();
    for(int i =0; i < bufferSize; i++){
     newRow.setFloat("bin" + i, val);
    }
  }
  
  curPos = (curPos+1) % visLen;
}

void keyPressed(){
  
  if(key == ' '){
    startRecord = !startRecord;
    if(startRecord == false){
      saveTable();
      
      // result table
      saveTable = new Table();
      for(int i =0; i< bufferSize; i++){
        saveTable.addColumn("bin" + i);
      }
    }
  }
}

void saveTable(){
  String filename = "results/measuredData" + millis() + ".csv";
  saveTable(saveTable, filename);
}