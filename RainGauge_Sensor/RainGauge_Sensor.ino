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

//Curah hujan adalah jumlah air yang jatuh di permukaan tanah selama periode tertentu yang diukur dengan satuan tinggi milimeter (mm) di atas permukaan horizontal.
//Curah hujan 1 mm adalah jumlah air hujan yang jatuh di permukaan per satuan luas (m2) dengan volume sebanyak 1 liter tanpa ada yang menguap, meresap atau mengalir.
// Lebih lengkap silahkan dipelajari lebih lanjut disini https://www.climate4life.info/2015/12/hujan-1-milimeter-yang-jatuh-di-jakarta.html

//Perhitungan rumus
//Tinggi curah hujan (cm) = volume yang dikumpulkan (mL) / area pengumpulan (cm2)
//Luas kolektor (Corong) 5,5cm x 3,5cm = 19,25 cm2
//Koleksi per ujung tip kami dapat dengan cara menuangkan 100ml air ke kolektor kemudian menghitung berapa kali air terbuang dari tip,
//Dalam perhitungan yang kami lakukan air terbuang sebanyak 70 kali. 100ml / 70= 1.42mL per tip.
//Jadi 1 tip bernilai 1.42 / 19.25 = 0,07cm atau 0.70 mm curah hujan.

// PENTING
// Nilai kalibrasi yang kami lakukan berlaku untuk semua sensor curah hujan yang kami jual tentu Anda dapat melakukan kalibrasi ulang sendiri jika dibutuhkan.

// Gunakan pin D5 pada NodeMCU, GPIO14 pada ESP32, Tegangan 3,3V Kemudian upload code ini
const int pin_interrupt = D1; // Menggunakan pin interrupt https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
long int jumlah_tip = 0;
long int temp_jumlah_tip = 0;
float curah_hujan = 0.00;
float milimeter_per_tip = 0.70;

volatile boolean flag = false;

void ICACHE_RAM_ATTR hitung_curah_hujan()
{
  flag = true;
}
String status_rain;
void setup()
{
  Serial.begin(115200);

  WiFiManager wifiManager;
    wifiManager.autoConnect("RainGauge Bendungan");
    
    
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

  pinMode(pin_interrupt, INPUT);
  attachInterrupt(digitalPinToInterrupt(pin_interrupt), hitung_curah_hujan, FALLING); // Akan menghitung tip jika pin berlogika dari HIGH ke LOW
  //printSerial();
}

void loop()
{
  if (flag == true) // don't really need the == true but makes intent clear for new users
  {
    curah_hujan += milimeter_per_tip; // Akan bertambah nilainya saat tip penuh
    jumlah_tip++;
    delay(500);
    flag = false; // reset flag
  }
  curah_hujan = jumlah_tip * milimeter_per_tip;
  if ((jumlah_tip != temp_jumlah_tip)) // Print serial setiap 1 menit atau ketika jumlah_tip berubah
  {
    //printSerial();
  }
  temp_jumlah_tip = jumlah_tip;

  if(curah_hujan>=0 && curah_hujan<0.5){
     status_rain = "Berawan";
   }
   if(curah_hujan>=0.5 && curah_hujan<20){
     status_rain = "Hujan Sedang";
   }
   if(curah_hujan>=20 && curah_hujan<100){
     status_rain = "Hujan Lebat";
   }
   if(curah_hujan>=100 && curah_hujan<150){
     status_rain = "Hujan Sangat Lebat";
   }
   if(curah_hujan>=150){
     status_rain = "Hujan Extream";
   }

  if (WiFi.status() == WL_CONNECTED && Firebase.ready()){
        // aa is the collection id, bb is the document id.
    String documentPath = "Sensor/WLS";

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson content;

    content.set("fields/raingauge/doubleValue", String(curah_hujan).c_str());
    
    content.set("fields/status_rain/stringValue",status_rain);
    

    if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "raingauge,status_rain")){
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

