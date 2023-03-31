#include "fonctions.h"

extern TSYS01 sensor_fastTemp; // External declaration of the BlueRobotics temperature sensor

int i = 0;
enum :byte {INIT, DEPLOYED, RECOVERY, COM, EMERGENCY} state_mode; // State machine to switch mode

//const int led = D9;
//unsigned compteur = 0;
//const int greenled = 25;  // on utilise la Led interne à la board esp32


void setup() {
  Serial.begin(115200);              // Start the console serial port

  //set_date_rtc(0, 0, 0, 2005);     // Time disruption test to check GPS update 
 
  state_mode = INIT;             // Init state machine to deployed mode

  //WiFi.disconnect();
}

void loop() {     
  /*enable_ec_parameters(EC_ENABLED, TDS_ENABLED, SAL_ENABLED, SG_ENABLED); 
  mesureEC();
  mesure_temp();*/

  //scanning_gps_time();
  //scanner_i2c_adress();
  
  switch(state_mode){
    case INIT :               // Waiting state : pass in DEPLOYED mode as soon as the GPS has found its position
      Serial.println("--- Case INIT ---");
      init_cycle();

      //test_cycle_init();  
      
      state_mode = DEPLOYED;  // Change to DEPLOYED state
        
      break;

    case DEPLOYED :           // Standard acquisition cycles
      Serial.println("--- Case DEPLOYED ---");
      deployed_cycle();

      //test_cycle();

      // TODO Condition pour passer en mode RECOVERY
      // TODO Condition pour passer en mode SENDING

      break;

    case RECOVERY :           // More frequent cycles
      Serial.println("--- Case RECOVERY ---");
      recovery_cycle();
      
      // TODO Condition pour passer en mode DEPLOYED
      break;

    case COM :
      Serial.println("--- Case COM ---");
      // TODO Envoi Iridium de la trame datachain 
      // Envoi toutes 30min environ ?
      // Ici qu'on convertit trame en binaire ou avant ?
      // TODO Conditions pour repasser en mode DEPLOYED ou RECOVERY (gestion du lien descendant)
      state_mode = DEPLOYED; 
      break;

    case EMERGENCY :
      Serial.println("--- Case EMERGENCY ---");

      break;
  
    Serial.println("\n----------------------------------------------------------------------------\n");
  }

}

void test_cycle_init(){
  Wire.begin();                      // For I2C communication 
  sensor_fastTemp.init();            // Init temperature sensor
  //init_gps();                        // Init communication UART of GPS
  //initEC();
}

void test_cycle(){
  
}

void init_cycle(){

  // TODO differencier 2 parties de l'init : 
  // l'une qui ne se réalise qu'une seule fois au démarrage 
  // et l'autre dans laquelle on peut revenir apres coup pour mise en veille si pas de gps sur certaine durée ou MàJ RTC...
  pinMode(LED_BUILTIN, OUTPUT); 
  initEC();                // For I2C communication 
  delay(50);
  Wire.begin();   
  sensor_fastTemp.init();  // Init temperature sensor
  init_gps();              // Init communication UART of GPS
  init_sd();               // Init and test SD card
  lecture_config();        // Read config file
  refresh_config_values(); // Refreshes the program values according to those read in the config file

  //pinMode(2, OUTPUT);    // Sleep Iridium modem
  //digitalWrite(2, LOW);

  //init_ina219();         // Init INA219 (current sensor)

  if(!check_rtc_set()){    // if RTC not correctly initialized
    set_rtc_by_gps();      // RTC update via gps data
  }

  init_iridium();

  // TODO veille si pas de signal GPS
}

void deployed_cycle(){
  //Serial.println("--- EC sensor and gps ON ---");
  //scanner_i2c_adress();
  //digitalWrite(9, HIGH);       // GPS ON
  //digitalWrite(4, HIGH);       //EC sensor ON
  mesure_cycle_to_datachain();   // Starts a measurement cycle and stores the measured parameters in datachain
  save_datachain_to_sd();        // Writes the content of datachain to the dataFilename file on the SD card
  readSDbinary_to_struct();

  //send_binary_iridium();
  //send_text_iridium();
  

  delay(10000);                   // 3 seconds delay

  

  /*Serial.println("--- EC sensor and gps OFF ---");
  digitalWrite(9, LOW);         // GPS OFF
  digitalWrite(4, LOW);         //EC sensor OFF
  mesure_cycle_to_datachain();  // Starts a measurement cycle and stores the measured parameters in datachain
  save_datachain_to_sd();       // Writes the content of datachain to the dataFilename file on the SD card

  delay(3000);                  // 3 seconds delay*/

  /*if(i==0){
    Serial.println("EC activated");
    digitalWrite(commut_EC, HIGH);
    delay(2000);
    i=1;
  }else{
    Serial.println("EC desactivated");
    digitalWrite(commut_EC, LOW);
    delay(2000);
    i=0;
  }*/
  

  if(!check_rtc_set()){  // if RTC not correctly initialized
    set_rtc_by_gps();    // RTC update via gps data
  }

  delay(1000);                 // Wait for 1 second before next acquisition
}

void recovery_cycle(){
  mesure_cycle_to_datachain(); // Starts a measurement cycle and stores the measured parameters in datachain
  save_datachain_to_sd();      // Writes the content of datachain to the dataFilename file on the SD card

  delay(500);                  // Wait for 500 ms before next acquisition 
}



