#if defined(ESP32)
//#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>

#endif

#include <Firebase_ESP_Client.h>
#include <WiFiManager.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>

#define API_KEY "AIzaSyDpVAGTuTKRUtV-WAhIaXaqS2mktl09KoQ"
#define FIREBASE_PROJECT_ID "firedata-d6bd1"
#define USER_EMAIL "nurhadisutra.ns@gmail.com"
#define USER_PASSWORD "123Sandiku"

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

int val1;
int val2;
int val3;
int Map;
String status;

void setup()
{

  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect("WLS Bendungan");
    
    
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
  config.api_key = API_KEY;

    /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
    
  #if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
  #endif

    // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
   
  Firebase.reconnectWiFi(true);
  //Code Setup WLS---------------------------------------------------------------------------
  //pinMode WLS
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  pinMode(A0, INPUT);
}

void fire(){
    
if (WiFi.status() == WL_CONNECTED && Firebase.ready()){
        // aa is the collection id, bb is the document id.
    String documentPath = "Sensor/WLS";

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson content;

    content.set("fields/wls/doubleValue", String(Map).c_str());
    
    content.set("fields/status_wls/stringValue",status);
    

    if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "wls,status_wls")){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }
  }
}
void loop()
{
  digitalWrite(D6, HIGH);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
  val1= analogRead(A0);
  Serial.print("Sensor 1: ");
  Serial.println(val1);
  delay(10);
   
  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);
  digitalWrite(D8, LOW);
  val2= analogRead(A0);
  Serial.print("Sensor 2: ");
  Serial.println(val2);
  delay(10);

  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, HIGH);
  val3= analogRead(A0);
  Serial.print("Sensor 3: ");
  Serial.println(val3);
  delay(10);

  int hasil = val1+val2+val3;
  Map=map(hasil, 300, 1000, 32, 42);
  Serial.print("Hasil : ");
  Serial.println(hasil);
  Serial.print("Tinggi : ");
  Serial.println(Map);
   
  if(Map>=32 && Map<39){
   status = "Normal";
  }
  if(Map>=39 && Map<40){
   status = "Waspada";
  }
  if(Map>=40 && Map<41){
   status = "Siaga";
  }
  if(Map>=41 && Map<43){
   status = "Awas";
  }
  
  fire();
  delay(1000);
}

 
    