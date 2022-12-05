/**************************************** 
 * Include Libraries 
 ****************************************/ 
#include <WiFi.h>
#include <PubSubClient.h> 
#include <SPI.h>
#include <MFRC522.h>



/**************************************** 
 * Define Constants 
 ****************************************/ 
#define WIFISSID "redo" //WIFI SSID aqui 
#define PASSWORD "012345678" // WIFI pwd 

#define TOKEN "BBFF-rhBHNkJi0blvHmtCLAcD65AkKNVIAQ" // Ubidots TOKEN name el mismo que usamos en clase 
#define MQTT_CLIENT_NAME "BBFF-e151f0d2b981b22d3548b43e1693ae9badf" //ID del cliente, 8 a 12 chars alfanumericos (ASCII), debe ser random y unico dif a otros devices 
 
 
/*Ultrasonico*/
#define U_sonic_Trig  21
#define U_sonic_Echo  19
//#define led 13
#define DEVICE_LABEL "proyecto" // Nombre del dispositivo a crear 



#define RST_PIN         9          // Configurable, see typical pin layout above
//MOSI 16
//MISO 15
//SCK 14
#define SS_PIN          17         // nfigurable, see typical pin layout above
#define VARIABLE_LABEL_dist "distancia" // Variable Temperatura 
#define VARIABLE_LABEL_rfid "id" // Variable Humedad 
#define RTS_PIN          22
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define led_azul 18
#define led_yellow 23
#define led_red 4


void readingData(void);


MFRC522 rfid = MFRC522(SS_PIN);  // Create MFRC522 instance
//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status; 


char mqttBroker[]  = "industrial.api.ubidots.com"; 
char payload[200]; // Leer y entender el payload aqui una de tantas referencias "https://techterms.com/definition/payload" 
char topic[150]; //Espacio para el nombre del topico 
 
// Space to store values to send 
char str_dist[10]; 
char str_rfid[10]; 
 
 

 
/**************************************** 
 * Funciones auxiliares 
 ****************************************/ 
WiFiClient ubidots; 
PubSubClient client(ubidots); 
 
void callback(char* topic, byte* payload, unsigned int length) { 
  char p[length + 1]; 
  memcpy(p, payload, length); 
  p[length] = NULL; 
  String message(p); 
  Serial.write(payload, length); 
  Serial.println(topic); 
} 
 
void reconnect() { 
  // Loop until we're reconnected 
  while (!client.connected()) { 
    Serial.println("Attempting MQTT connection..."); 
     
    // Attemp to connect 
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) { 
      Serial.println("Connected"); 
    } else { 
      Serial.print("Failed, rc="); 
      Serial.print(client.state()); 
      Serial.println(" try again in 2 seconds"); 
      // Wait 2 seconds before retrying 
      delay(2000); 
    } 
  } 
} 
 
/**************************************** 
 * Main Functions 
 ****************************************/ 
void setup() { 
   
//*Para Ultrasonico*/
  
  pinMode(U_sonic_Trig, OUTPUT); //pin como salida
//  pinMode(led, OUTPUT); //pin como salida

  pinMode(U_sonic_Echo, INPUT);  //pin como entrada
  digitalWrite(U_sonic_Trig, LOW);//Inicializamos el pin con 0
//  digitalWrite(led, HIGH);//Inicializamos el pin con 0

    Serial.begin(115200);

  WiFi.begin(WIFISSID, PASSWORD); 
 
   
  Serial.println(); 
  Serial.print("Wait for WiFi..."); 
   
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print("."); 
    delay(500); 
  } 
   
  Serial.println(""); 
  Serial.println("WiFi Connected"); 
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP()); 
  client.setServer(mqttBroker, 1883); 
  client.setCallback(callback);   
 
  
	SPI.begin();			// Init SPI bus
	rfid.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	rfid.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  
} 

void loop() { 
  if (!client.connected()) { 
    reconnect(); 
  } 
  
  /*__________________SENSOR ULTRASONICO_____________________________________________*/
  
  long tiempo; //timepo que demora en llegar el eco
  long d; //distancia en centimetros
  digitalWrite(U_sonic_Trig, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(U_sonic_Trig, LOW);
  tiempo = pulseIn(U_sonic_Echo, HIGH); //obtenemos el ancho del pulso
  d = tiempo/59;             //escalamos el tiempo a una distancia en cm  
  Serial.print("Ultrasonico distancia: -----> ");
  Serial.print(d);      //Enviamos serialmente el valor de la distancia
  Serial.print(" cm");
  Serial.println();
  



 // Publica en el topic de distancia
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL); 
  sprintf(payload, "%s", ""); // Cleans the payload 
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_dist); // Adds the variable label  
 
  Serial.println(d); // Imprime temperatura en el serial monitor  
 
  /* numero maximo 4 precision 2 y convierte el valor a string*/ 
  dtostrf(d, 4, 2, str_dist); 
    
  sprintf(payload, "%s {\"value\": %s", payload, str_dist); // formatea el mensaje a publicar 
  sprintf(payload, "%s } }", payload); // cierra el mensaje 
  Serial.println("Publicando distancia en Ubidots cloud");  
  client.publish(topic, payload); 
 
  delay(2000); // 15 segundos en milisegundos entre publicaciones en ubidots  // Aguarda a aproximacao do cartao

   //waiting the card approach
  if ( ! rfid.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select a card
  if ( ! rfid.PICC_ReadCardSerial()) 
  {
    return;
  }

    // Dump debug info about the card; PICC_HaltA() is automatically called
  rfid.PICC_DumpToSerial(&(rfid.uid)); //call menu function and retrieve the desired option
  readingData();

  // Publica en el topic de humedad 
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL); 
  sprintf(payload, "%s", ""); // Cleans the payload 
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_rfid); // Adds the variable label 
  float h1 = 1; 
 
  Serial.println(h1); // Imprime temperatura en el serial monitor  
 
  /* numero maximo 4 precision 2 y convierte el valor a string*/ 
  dtostrf(h1, 4, 2, str_rfid); 
   
  sprintf(payload, "%s {\"value\": %s", payload, str_rfid); // formatea el mensaje a publicar 
  sprintf(payload, "%s } }", payload); // cierra el mensaje 
  Serial.println("Publicando humedad en Ubidots cloud");  
  client.publish(topic, payload); 
   
  client.loop(); 
  delay(2000); // 15 segundos en milisegundos entre publicaciones en ubidots 
}
  
void readingData()
{
  //prints the technical details of the card/tag
  rfid.PICC_DumpDetailsToSerial(&(rfid.uid)); 
  
  //prepare the key - all keys are set to FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  //buffer for read data
  byte buffer[SIZE_BUFFER] = {0};
 
  //the block to operate
  byte block = 1;
  byte size = SIZE_BUFFER; //</p><p>  //authenticates the block to operate
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    delay(1000);
    return;
  }
}