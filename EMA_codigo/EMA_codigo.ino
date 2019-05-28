//Libraries en ingles
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Ethernet.h>
#include <SPI.h>
#include "SFE_BMP180.h"
#include "sha1.h"
#include "mysql.h"

//Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF } ;
IPAddress ip(172,23,0,199);
IPAddress gateway(172,23,0,1);
IPAddress subnet(255,255,252,0);
IPAddress server_addr(172,23,0,12);

EthernetClient client;

//mysql

Connector my_conn;

char user[] = "arduino";
char password[] = "ep3t";

//LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//Sensor de temperatura y humedad
#define pin_dht 2
#define dhttype DHT22
DHT dht (pin_dht, dhttype);

//Sensor de presión
SFE_BMP180 bmp180;


//Caracteres LCD

byte grado[8] = {
  0b00001110,
  0b00001010,
  0b00001110,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};

byte acentoo[8] = {
  0b00000010,
  0b00000100,
  0b00001110,
  0b00010001,
  0b00010001,
  0b00010001,
  0b00001110,
  0b00000000,
};
//variable incrementable
int minutos = 1;
int segundos = 0;

void setup() {
  Serial.begin(9600);
  
  //caracteres
  lcd.createChar (0, grado);
  lcd.createChar (1, acentoo);
  
  //Config del LCD
  lcd.begin(16, 2);
  lcd.home ();
  lcd.print("Iniciando");
  delay (500);
  lcd.print(".");
  delay (500);
  lcd.print(".");
  delay (500);
  lcd.print(".");
  delay (500);
  
  dht.begin();
  lcd.clear();
  lcd.print("DHT22 Iniciado");
  delay (500);
  
  if (bmp180.begin()) {
    //Inicio correcto de sensor de presión
    lcd.clear();
    lcd.print("BMP180 Iniciado");
    delay (500);
  } else {
    //Error de inicio del mismo sensor
    lcd.clear();
    lcd.print("ERROR BMP180");
    lcd.setCursor ( 0, 1 );
    lcd.print("Fallo en el inicio");
    delay (3000);
  }
  
  //Config Ethernet
  
  lcd.clear();
  lcd.print("Conenctando a la");
  lcd.setCursor ( 0, 1 );
  lcd.print("red...");
  delay (500);
  
  if (Ethernet.begin(mac) == 0)
  {       
    lcd.clear();
    lcd.print("ERROR ETHERNET");
    lcd.setCursor ( 0, 1 );
    lcd.print("No DHCP o Red");
    delay (3000);
  } else {
    lcd.clear();
    lcd.print("Conectado a Red");
    delay (500);
    
  }
  Ethernet.begin(mac, ip);
  /*
  Serial.print("IP: ");
  for (byte B = 0; B < 4; B++)   
  {
    Serial.print(Ethernet.localIP()[B], DEC);
    Serial.print(".");
  }
  Serial.println();
  
  Serial.print("Router por defecto : ");
  for (byte B = 0; B < 4; B++) {
    Serial.print(Ethernet.gatewayIP()[B], DEC);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Subred: ");
  for (byte B = 0; B < 4; B++) {
    Serial.print(Ethernet.subnetMask()[B], DEC);
    Serial.print(".");
  }
  Serial.println();
  
  Serial.print("DNS: ");
  for (byte B = 0; B < 4; B++) {
    Serial.print(Ethernet.dnsServerIP()[B], DEC);
    Serial.print(".");
  }
  Serial.println();
  
  Serial.print("IP Servidor: ");
  Serial.print("172.23.0.12");
  Serial.println();
  */
  
  //Iniciado
  lcd.clear();
  lcd.setCursor ( 0, 0 );
  lcd.print("Inicio completo");
  delay (250);
  lcd.setCursor ( 0, 1 );
  lcd.print("Bienvenido a EAM");
  delay (2000);


}

void loop() {
  //Temperatura
  float tempf = leer_temp();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura:");
  lcd.setCursor(0, 1);
  lcd.print(tempf);
  lcd.print(" C");
  lcd.write((byte)0);
  delay (3000);
  
  //Humedad
  float humf = leer_hum();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humedad:");
  lcd.setCursor(0, 1);
  lcd.print(humf);
  lcd.print(" RH");
  delay (3000);
  
  //Presion
  
  char estado;
  double P, T;
  double presionf = leer_pres(P, T, estado);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Presi");
  lcd.write((byte)1);
  lcd.print("n:");
  lcd.setCursor(0, 1);
  lcd.print(presionf);
  lcd.print(" hPa");
  delay (3000);

  //Viento
  float v1 = 0;
  float vientof = leer_viento(v1);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Velocidad viento:");
  lcd.setCursor(0, 1);
  lcd.print(vientof);
  lcd.print(" Km/h");
  delay (3000);
  
  if (minutos < 25){
    //Serial.print ("Ha(n) pasado: ");
    //Serial.println (minutos);
    segundos = 12 * minutos;
    minutos = minutos + 1;
    //Serial.print ("En total ");
    //Serial.print (segundos);
    //Serial.println (" segundo(s)");
  } else {
    //Serial.print ("LLEGADA: ");
    //Serial.println (minutos);
    segundos = 12 * minutos;
    //Serial.print ("En total ");
    //Serial.print (segundos);
    //Serial.println (" segundo(s)");
    minutos = 1;

    almacen_temp();
    almacen_hum();
    almacen_pres();
    almacen_viento();
  }
}

float leer_temp(){
  float temperatura = dht.readTemperature();
  return temperatura;
}

float leer_hum(){
  float humedad = dht.readHumidity();
  return humedad;
}

double leer_pres(char estado, double P, double T){
  

  estado = bmp180.startPressure(3);
  if (estado != 0) {
    delay (estado);
    estado = bmp180.getPressure(P, T);
    if (estado != 0) {
      //Mostrar datos de presión
    }
  }
  
  return P;
}

float leer_viento(float v1){

  float veloc1;
  v1 = analogRead(0);
  veloc1 = v1 * 0.190;
  return veloc1;
}

int almacen_temp(){
  
}

int almacen_hum(){
  
}

int almacen_pres(){
  
}

int almacen_viento(){
  
}

