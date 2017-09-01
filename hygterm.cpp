#include <wiringPi.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <ctime>

using namespace std;

#define MAX_TIME 85
#define MAX_PIN 16
#define DHT11PIN 7
#define MAX_TRY 40
#define NEEDED_READ 10

int data[5] = {0,0,0,0,0};
bool debug = false;
string format;
double temperature;
double humidity;
int successfulRead;


// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%FT%T%z", &tstruct);

    return buf;
}
const string logfilename() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "hygterm-%F.log", &tstruct);

    return buf;
}

void dht11_read_val(int pin)
{
  uint8_t lststate=HIGH;
  uint8_t counter=0;
  uint8_t j=0,i;

  for(i=0;i<5;i++) {
     data[i]=0;
  }
  pinMode(pin,OUTPUT);
  digitalWrite(pin,LOW);
  delay(18);
  digitalWrite(pin,HIGH);
  delayMicroseconds(40);
  pinMode(pin,INPUT);
  
  for(i=0;i<MAX_TIME;i++) {
    counter=0;
    while (digitalRead(pin) == lststate) {
      counter++;
      delayMicroseconds(1);
      if(counter==255)
        break;
    }
    lststate = digitalRead(pin);

    if(counter==255)
       break;
    // top 3 transistions are ignored
    if ((i>=4)&&(i%2==0)){
      data[j/8] <<= 1;
      if(counter>16)
        data[j/8] |= 1;
      j++;
    }
  }
  if (debug) {
    cerr << "Data (" << j << "): " << hex << data[0] << hex << data[1] << hex << data[2] << hex << data[3] << hex << data[4] << endl;
  }

  if (debug) cerr << "Format: " << format << endl;
  // We should get 40 bits and checksum should be ok
  if((j>=40)&&(data[4]==((data[0]+data[1]+data[2]+data[3])& 0xFF))) {
    temperature += data[2];
    humidity += data[0];
    successfulRead++;
  }
}
/*
    if (format == "json") {
      cout << "{\n\t\"humidity\": " << data[0] 
          << ",\n\t\"temperature\": " << data[2] 
          << ",\n\t\"time\": \"" << currentDateTime() << "\"\n}\n";
    }
    else if (format == "log") {
      string logfile = "/var/lib/hygterm/";
      logfile += logfilename();
      ofstream fout;
      fout.open(logfile.c_str(), ios::out | ios::app | ios::ate);
      fout << currentDateTime() << "," << data[0] << "," << data[2] << "\n";
      fout.close();
    }
    else {
      cout << currentDateTime() << "," << data[0] << "," << data[2] << "\n";
    }
*/

void usage(char**argv)
{
  cerr 
  << "Usage: " << argv[0] << " <pin number> [<format>]\n"
  << endl
  << "  <pin number>  wiringPi pin number as follows: \n"
  << "                  number  GPIO|GPIO  number\n"
  << "                             1|2           \n"
  << "                  8          3|4           \n"
  << "                  9          5|6           \n"
  << "                  7          7|8         15\n"
  << "                             9|10        16\n"
  << "                  0         11|12         1\n"
  << "                  2         13|14          \n"
  << "                  3         15|16         4\n"
  << "                            17|18         5\n"
  << "                  12        19|20          \n"
  << "                  9         21|22         6\n"
  << "                  14        23|24        10\n"
  << "                            25|26        11\n"
  << "  <format>      json (default), csv or log\n"
  << "                log stores csv result in \n"
  << "                /var/lib/hygterm/hygterm-YYYY-MM-DD.log\n"
  << endl;
}

void out_json(ostream& out) 
{
        out << "{\n\t\"humidity\": " << humidity 
                << ",\n\t\"temperature\": " << temperature 
                << ",\n\t\"count\": " << successfulRead
                << ",\n\t\"time\": \"" << currentDateTime() << "\"\n}\n";
}
void out_log(string& logfile) 
{
      ofstream fout;
      fout.open(logfile.c_str(), ios::out | ios::app | ios::ate);
      fout << currentDateTime() << "," << humidity << "," << temperature << "\n";
      fout.close();
}
void out_json(string& logfile) 
{
      ofstream fout;
      fout.open(logfile.c_str(), ios::out);
      out_json(fout);
      fout.close();
}

int main(int argc, char** argv)
{
  if (argc < 2 || argc > 3) {
    usage(argv);
    return 1;
  }
  
  // If there is no wiringPi library print instructions for installation.
  if (wiringPiSetup() == -1) {
    cerr << "Wiring Pi library not found, please install it:\n"
        << "  cd ~\n"
        << "  sudo apt-get install git-core\n"
        << "  sudo git clone git://git.drogon.net/wiringPi\n"
        << "  cd wiringPi\n"
        << "  sudo ./build\n";
    return 1;
  }
  
  int pin = atoi(argv[1]);
  format = argc > 2 ? argv[2] : "json";
  
  if (pin > MAX_PIN || pin < 0) {
    cerr << "Invalid argument: pin number should be between 0 and " << MAX_PIN << endl;
    return 1;
  }
  
  temperature = 0.0;
  humidity = 0.0;
  successfulRead = 0;
  for(int i=0;i<MAX_TRY && successfulRead < NEEDED_READ;i++) {
    dht11_read_val(pin);
    delay(100);
  }
  if (successfulRead > 0) {
    humidity = humidity/successfulRead;
    temperature = temperature/successfulRead;
    if (format == "json") {
      out_json(cout);
    }
    else if (format == "log") {
      string logfile = "/var/lib/hygterm/";
      logfile += logfilename();
      out_log(logfile);
      logfile = "/var/lib/hygterm/hygterm-last.json";
      out_json(logfile);
    }
    else {
      cout << currentDateTime() << "," << humidity << "," << temperature << "\n";
    }
  }
  else {
    cerr << "No successful read.\n"; 
  }
  
  return 0;
}