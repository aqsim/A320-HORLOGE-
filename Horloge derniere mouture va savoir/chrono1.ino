//////////////////////////////////////////////////////////////////////////////
//                      Montre Aquitaine Simulation @2019                    //
//////////////////////////////////////////////////////////////////////////////

#include "Wire.h"
#include "Time.h"
#include "Stepper.h"
#define BAUD 57600
#define LedToggle digitalWrite (Led, !digitalRead(Led))

unsigned long Lec_PCF_=0;
unsigned long Lec_PCF=0;
unsigned long Intervalle_Lec=0;
const byte Led = 13; // LED du module
const byte LedLum = 6;  // led output pour controle luminosité
int led_value=0; // luminosité led6
int Led_Temps=0;  // luminosité leds GMT et ELAPS
#define Led1quart 8 // led 1-15  old 17  new 11 renew 8
#define Led2quart 44 // led 16-30  old 16  new 10 renew 44
#define Led3quart 11  // led 31-45  old 14  new 8 renew 11
#define Led4quart 46  // led 46-59  old 15  new 9 renew 46
const byte LedScde_GMT = 4;  // seconde  GMT    old 18  new 4
const byte LedScde_Elapse = 5; // seconde Elapse old 19  new 5

byte saaGMT = 0x74 >> 1; // BLOC   4 afficheurs GMT
byte saaCHRONO=0x70 >> 1; // BLOC 2 afficheurs CHRONO
byte saaELAPS=0x72 >> 1;  // BLOC 4 afficheurs ELAPS

uint8_t pcf=0x7e>>1; // adresse lecture pcf
byte data;
byte Cde;

//int digitsp[11]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0xC0};  // digits avec point
int  digits[11]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x40};  // digits sans point
int CPTR_GMT=0;
int heures=0;  // valeur actuelle reçue
int minutes=0;   // valeur actuelle reçue
int heures_=-1;  // dernière valeur traitée mémorisée
int minutes_=-1; // dernière valeur traitée mémorisée
int secondes=0;
int Sys_Heure=0;
int Sys_Minute=0;
int Sys_Seconde=0;
bool  dog=false;
bool  Aff_TimeSyst=false; 
bool bHeure_Init=false;
String input;
byte Btn_Pos; // valeurs lues état pcf
byte Btn_Pos_; // valeurs précédentes lues

bool bBtn_HLT_Fast=false; // P0
bool bBtn_HLT_Slow=false; // P1
bool bBtn_HLT_Run=false;  // P2
bool bBtn_DIM_Set=false;  // P3
bool bBtn_ET_=false;      // P4 RAZ/elapse
bool bBtn_ET_Run=false;   // P5 démarre le compteur elapse
bool bBtn_ET_StandBy=false; 
byte Btn_CHR=0;           // P6  1->Départ chrono,2->Arrêt 3->RAZ 
bool Moteur_Reverse=false;// P7 Stop mécanique marche arrière aiguille

bool bBtn_Initial=false;
byte Btn_Zero=0xFF;   
byte Compteur=0;
int CPTR_Seconde=0;   // valeur second timer
int CPTR_Seconde_=0;
int CPTR_Elapse_=0;  // memorisation valeur affichée
int CPTR_Elapse_Ini=0;
bool bBtn_ET_Ini=false; 
int CPTR_Diff=0;
int Elapse_Encours=0;
int Elapse_Heures=0;
int Elapse_Minutes=0;
byte Chrono_Etat=0; // 1->départ;2->Stop;3->RAZ
int CPTR_Chrono=0;
int CPTR_Chrono_=0;
int CPTR_Chrono_Ini=0;
int Chrono_Encours=0;
int Chrono_Encours_=0;
byte Chrono_Etat_=0;
bool bChrono_Stop=true;   // pour recul aiguille
bool bRAZ_Aiguille=true;  // pour recul aiguille tant que btn appuyé

byte seconde=0;
bool bHeure_Fs_Recue=false;   
bool bHLT_Fast=false;
bool bHLT_Slow=false;

/// sans filet !!!!!!!
int Valeur_Potar=0; // valeur pour Led
int Valeur_Lum=0;  // Pour intensité SAA
byte SAA_Lum=0; // luminosité afficheurs sera égale à SAA_CTRL + SAA_Intensité ( valeur potar)
byte SAA_Intensite=0x10; // minimum 3ma
byte SAA_CTRL=0x07; //  dynamic mode on, digits 1+3 on, digits 2+4 on (0x07) == 0x07
byte SAA_Intensite_=-1;
/////////////////////////////////  stepper
int St_NbPasTour = 240;
//// Stepper Aiguille(St_NbPasTour,8,10,9,11); // objet aiguille et sorties commande
Stepper Aiguille(St_NbPasTour,14,16,15,17); // objet aiguille et sorties commande
int St_NbPas = 0; // nombre de pas dans le tour ( pour la marche arrière)

////////////////////////////////////////////////
 ///////////////////////////////////////////////  setup()             
void setup() {

  Serial.begin(BAUD);
  Serial.print("S");
	Wire.begin();
	pinMode(Led,OUTPUT);             // clignote rythme 1 seconde pour contrôle
	pinMode(LedLum,OUTPUT);          // contrôle luminosité
  pinMode(Led1quart, OUTPUT);      // simu bargraphe
  pinMode(Led2quart, OUTPUT);      // simu bargraphe
  pinMode(Led3quart, OUTPUT);      // simu bargraphr
  pinMode(Led4quart, OUTPUT);      // simu bargraphe
  pinMode(LedScde_GMT,OUTPUT);     // seconde GMT
  pinMode(LedScde_Elapse,OUTPUT);  // seconde Elapse
  Aiguille.setSpeed(10);            //  ??? comprends pas cette commande
	  // programmation TIMERS2	pompée sur internet !!!
	  TCCR2A = 0; //default 
	  TCCR2B = 0b00000110; // clk/256 est incrémenté toutes les 16uS  
	  TIMSK2 = 0b00000001; // TOIE2 
	  sei();               // autorise les interruptions
  initDisplay_Tirets(); // au démarrage tirets sur les afficheurs ( ?)afficheurs passent à zéro quand la fonction démarre
  dog=false;
}
/////////////////////////////////////////////////////////  Displays avec tirets

void initDisplay_Tirets()   // tiret sur tous les afficheurs au lancement
{
    displayDigitsT(saaGMT);
    displayDigitsT(saaCHRONO);
    displayDigitsT(saaELAPS);
    delay(250);               // pour avoir le temps de voir les tirets ??????????????????? a supprimer en ops
}
///////////////////////////// init transmission sur circuit
void initDisplay(int adresse)  // turns on dynamic mode and adjusts segment current 
{
    SAA_Lum=SAA_Intensite | SAA_CTRL; // 0xy0 | 0x07  y valeur du potar  ( fonction ou logique )
    //SAA_CTRL=0x47; // ??? 12ma  **********
    Wire.beginTransmission(adresse);
    Wire.write(B00000000); // this is the instruction byte. Zero means the next byte is the control byte
    Wire.write(SAA_Lum); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
    //Wire.write(SAA_CTRL); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
    Wire.endTransmission();
}
/////////////////////////////////////////  affichage tirets
void displayDigitsT(int ad)  // display tirets
{
    initDisplay(ad); // init bus
    Wire.beginTransmission(ad);
    Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
    Wire.write(digits[10]); // digit 1 
    Wire.write(digits[10]); // digit 2
    Wire.write(digits[10]); // digit 3
    Wire.write(digits[10]); // digit 4 
    Wire.endTransmission(true);
    }


//////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////        affichage heure


void displayDigits(int adresse,int heures,int minutes)   // show all digits 0~9, 
{
    
    if( Aff_TimeSyst==false)
    {
    initDisplay(adresse); // init bus
    Wire.beginTransmission(adresse);
    Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
    Wire.write(digits[(minutes/10)]); // digit 1 
    Wire.write(digits[(minutes%10)]); // digit 2
    Wire.write(digits[(heures/10)]); // digit 3
    Wire.write(digits[(heures%10)]); // digit 4 
    Wire.endTransmission(true);
    }
    else
    {

    initDisplay(adresse); // init bus
    Wire.beginTransmission(adresse);
    Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
    Wire.write(digits[(Sys_Minute/10)]); // digit 1 
    Wire.write(digits[(Sys_Minute%10)]); // digit 2
    Wire.write(digits[(Sys_Heure/10)]); // digit 3
    Wire.write(digits[(Sys_Heure%10)]); // digit 4 
    Wire.endTransmission(true); 
    }
}
///////////////////////////////////////////////////////////////////////////////////////
void Gestion_Leds(int CPTR_GMT)
{
    
     
     
     
     
     if(dog == false)
     {
          analogWrite(Led1quart,0);
          analogWrite(Led2quart,0);
          analogWrite(Led3quart,0);
          analogWrite(Led4quart,0);
          analogWrite(LedScde_GMT,0); // clignotent plus
          //displayDigitsT(saaGMT); // tirets
          initDisplay(saaGMT); // init bus
    Wire.beginTransmission(saaGMT);
    Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
    Wire.write(digits[10]); // digit 1 
    Wire.write(digits[10]); // digit 2
    Wire.write(digits[10]); // digit 3
    Wire.write(digits[10]); // digit 4 
    Wire.endTransmission(true);
     }
     else
     {
       if(CPTR_GMT%2) // pour clignotement led
       {
        //digitalWrite(LedScde_GMT,HIGH);
        analogWrite(LedScde_GMT,Led_Temps);
        if(Aff_TimeSyst==true)
        {
    SAA_Lum=SAA_Intensite | SAA_CTRL; // 0xy0 | 0x07  y valeur du potar  ( fonction ou logique )
    //SAA_CTRL=0x47; // ??? 12ma  **********
    Wire.beginTransmission(saaGMT);
    Wire.write(B00000000); // this is the instruction byte. Zero means the next byte is the control byte
    Wire.write(SAA_Lum); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
    //Wire.write(SAA_CTRL); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
    Wire.endTransmission();
        }
        
        }
        else
        {
        // digitalWrite(LedScde_GMT,LOW); 
        analogWrite(LedScde_GMT,0); 
        if(Aff_TimeSyst==true)
        {
            SAA_Lum= SAA_CTRL; // 0xy0 | 0x07  y valeur du potar  ( fonction ou logique )
    //SAA_CTRL=0x47; // ??? 12ma  **********
    Wire.beginTransmission(saaGMT);
    Wire.write(B00000000); // this is the instruction byte. Zero means the next byte is the control byte
    Wire.write(SAA_Lum); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
    //Wire.write(SAA_CTRL); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
    Wire.endTransmission();  
        }      
        }
		byte quart = (CPTR_GMT/15);
        switch(quart)
        {
          case 0:
          {
		        analogWrite(Led1quart, Led_Temps);
            analogWrite(Led2quart, 0);
            analogWrite(Led3quart, 0);
            analogWrite(Led4quart, 0);
            break;
          }
          case 1 :
          {
	      	  analogWrite(Led1quart, Led_Temps);
            analogWrite(Led2quart, Led_Temps);
            analogWrite(Led3quart, 0);
            analogWrite(Led4quart, 0);
            break;
          }
          case 2 :
          {
            analogWrite(Led1quart, Led_Temps);
            analogWrite(Led2quart, Led_Temps);
            analogWrite(Led3quart, Led_Temps);
            analogWrite(Led4quart, 0);
            break;
          }
          case 3:
          {
            analogWrite(Led1quart, Led_Temps);
            analogWrite(Led2quart, Led_Temps);
            analogWrite(Led3quart, Led_Temps);
            analogWrite(Led4quart, Led_Temps);
            break;          
          }
        }
     }          
}

////////////////////////////////////////////////
////////////////////////////////////////////////  lecture boutons sur pcf

uint8_t Lec_Btn(uint8_t adresse)  // appel dans loop()
{
    
    uint8_t un = 1;
    Wire.beginTransmission(adresse);
    Wire.requestFrom(adresse, un);
     while(Wire.available()) 
       {
        data = Wire.read();
       }
     Wire.endTransmission(true);
     return data;
}

///////////////////////////////////////////////////////////////   traitement boutons

void Btn_Init()  // recherche position Btn au démarrage pour init position initiale
{
    if(bBtn_Initial==false) 
    { 
      Btn_Zero=0xFF^Btn_Pos; // recherche bit à zéro : actif
      bBtn_Initial=true;       // init faite
      Btn_Traitement(Btn_Zero);  // traitement des boutons actifs (valeur bit = 1 dans Btn_zero ( ou exclusif))
    } 
}
 //////////////////////////////////////////////////////////////  boutons 
 
void Btn_Traitement(byte Btn_Etat)  // traitement état boutons
{

      byte x=0x01; // valeur initiale de test de rang
      byte y=Btn_Etat; // 
  
  for (int i=0;i<8;i++)  // boucle pour tester les 8 bits ( 0 à 7)
  {
    if((y & x)>0) {  // bit à 1 ? x donne le rang du bit+1 ( 1,2,4,8,16,32,64,128)
                     // i donne le numéro du bit
      switch(i)
      {
        case 0:                         // HLD Fast
        {         
          if(bitRead(Btn_Pos_,i))
          {
            //Serial.println("HLT fast off");  // bit à 1 P0=1
            bHLT_Fast=false;
           Aff_TimeSyst=false; 
           dog=false;
            // ?????
          }
            else
            {
            //Serial.println("HLT fast on");   // bit à 0 P0=0 bouton appuyé ( actif) 
            bHLT_Fast=true;
            Aff_TimeSyst=true;
            dog=false;
             // ????? 
            }     
          break;
        }
        case 1:                             // HLD Slow
        {
           if(bitRead(Btn_Pos_,i))
            {
            //Serial.println("Slow off");
             bHLT_Slow=false; 
             // ????
            }
            else
            {
            //Serial.println("Slow on"); 
              bHLT_Slow=true; 
              // ???   
            }     
          break;
        }
        case 2:                        //            HLD Run
        {
         if(bitRead(Btn_Pos_,i))
            {
            //Serial.println("Run Off");
            // ???
            }
            else
            {
            //Serial.println("Run On");   
            // ????    
            }      
          break;
        }
        case 3:                           // DIM  poussé affichage 8, relaché valeurs
        {
         if(!bitRead(Btn_Pos_,i))
          {
                       // poussoir test des afficheurs,
                       // sauvegardes des informations affichées
            Affichage_Test(); 
                                      
          } 
          else   // relachement poussoir
          {
             Maj_Affichage(); // réaffichage des informations
            }
           break;
        }
        case 4:                                // ET Reset elapse
        {
          if(!bitRead(Btn_Pos_,i)) // reset on
          {
            //Serial.println("  ET Reset On");
            //CPTR_Elapse_Ini=0;
            CPTR_Diff=0;
            CPTR_Elapse_=0;
            bBtn_ET_Ini=false;
            bBtn_ET_Run=false;
            bBtn_ET_StandBy=false;
            Elapse_Heures=0;
            Elapse_Minutes=0;

            initDisplay(saaELAPS);
                // affichage tirets
              Wire.beginTransmission(saaELAPS);             
              Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
              Wire.write(digits[10]); // digit 1 
              Wire.write(digits[10]); // digit 2
              Wire.write(digits[10]); // digit 3
              Wire.write(digits[10]); // digit 4 
              Wire.endTransmission(true);
          }
          else
          {
               //Serial.println("ET Reset Off");
 
          }
          
          break;
        }
        case 5:                          /// ET Run  elapse
        {
    
         if(bitRead(Btn_Pos_,i))
          {
              //Serial.println("ET Run Off"); bascule sur stop
              bBtn_ET_StandBy=true; // arrêt affichage mais pas comptage (stop)
              //CPTR_Elapse=0;
              //CPTR_Elapse_=0;
              //bBtn_ET_Run=false;   // arret comptage 21/07
            }
          else
          {
            //Serial.println("ET Run on");   
            if(bBtn_ET_Run==false)
            {
             bBtn_ET_Run=true;  // début comptage
             bBtn_ET_StandBy=false;  // raz standbye
              initDisplay(saaELAPS);     
              Wire.beginTransmission(saaELAPS);             
              Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
              Wire.write(digits[0]); // digit 1 
              Wire.write(digits[0]); // digit 2
              Wire.write(digits[0]); // digit 3
              Wire.write(digits[0]); // digit 4 
              Wire.endTransmission(true);
             }
             else
             {
                bBtn_ET_StandBy=false;             
             }
		    }
          break;
        }       
        case 6:      ////////////////////////     CHR  chrono
        {
         if(!bitRead(Btn_Pos_,i))
          {
                                        //Serial.println("CHR On");
           Chrono_Etat++;  // mèmo etat chrono 1 start,2 stop, 3 raz , 0 non actif
                                       //Serial.print("Etat = ");
                                      //Serial.println(Chrono_Etat);
           if(Chrono_Etat==1)
           {
             CPTR_Chrono_=CPTR_Seconde;
                                //Serial.println("Chrono init , lancé");
             Chrono_RAZ(); // affichage des zeros le chrono affiche des minutes !!! montre chrono actif
           }
           if(Chrono_Etat!=1)   // stop ou reset !!!!
           {
            Chrono_Tr(CPTR_Seconde);
           }
          }           
          break;
        }
      
        case 7: // Arret moteur en marche arriere
        {
         if(!bitRead(Btn_Pos_,i))
         {
          Moteur_Reverse=true;
         }
 /*        else
         {
          Moteur_Reverse=false; // trop tot
         }
         */
          break;
        }      
      }//switch
	}//if
    x=x*2; // decalage pour tester le bit suivant
   } // for

  }
////////////////////////////////////////////////////// Affichage suite test ( aff 8)
void Affichage_Test()
{

                      // Afficheurs Chrono             
                      initDisplay(saaCHRONO);      
                      Wire.beginTransmission(saaCHRONO);      
                      Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
                      Wire.write(digits[8]); // digit 1 
                      Wire.write(digits[8]); // digit 2
                      Wire.write(digits[0]); // digit 3
                      Wire.write(digits[0]); // digit 4 
                      Wire.endTransmission(true); 
                  // afficheurs elapse
                        initDisplay(saaELAPS);      
                        Wire.beginTransmission(saaELAPS);     
                        Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
                        Wire.write(digits[8]); // digit 1 
                        Wire.write(digits[8]); // digit 2
                        Wire.write(digits[8]); // digit 3
                        Wire.write(digits[8]); // digit 4 
                        Wire.endTransmission(true);
                                      
                 // heure
                        initDisplay(saaGMT);      
                        Wire.beginTransmission(saaGMT);                       
                        Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
                        Wire.write(digits[8]); // digit 1 
                        Wire.write(digits[8]); // digit 2
                        Wire.write(digits[8]); // digit 3
                        Wire.write(digits[8]); // digit 4 
                        Wire.endTransmission(true); 
                                                    
}

///////////////////////////////////////////////// affichage informations origines
void Maj_Affichage()
{
              // chrono
          initDisplay(saaCHRONO);       
          Wire.beginTransmission(saaCHRONO);        
          Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
          Wire.write(digits[((Chrono_Encours/10))]); // digit 1 
          Wire.write(digits[(Chrono_Encours%10)]); // digit 2
          Wire.write(digits[0]); // digit 3
          Wire.write(digits[0]); // digit 4 
          Wire.endTransmission(true);
        
        // elapse
if((Elapse_Heures==0) && (Elapse_Minutes==0) && (bBtn_ET_StandBy==false))
           {  
              initDisplay(saaELAPS);
              //Serial.println(CPTR_Diff);             
              Wire.beginTransmission(saaELAPS);     
              Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
              Wire.write(digits[(Elapse_Heures/10)%10]); // digit 1 
              Wire.write(digits[Elapse_Heures%10]); // digit 2
              Wire.write(digits[(CPTR_Diff/10)%10]); // digit 3
              Wire.write(digits[CPTR_Diff%10]); // digit 4 
              Wire.endTransmission(true);            
           } 
           else
           {
          initDisplay(saaELAPS);      
          Wire.beginTransmission(saaELAPS);     
          Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
          Wire.write(digits[(Elapse_Heures/10)%10]); // digit 1 (RHS)
          Wire.write(digits[Elapse_Heures%10]); // digit 2
          Wire.write(digits[(Elapse_Minutes/10)%10]); // digit 3
          Wire.write(digits[Elapse_Minutes%10]); // digit 4 (LHS)
          Wire.endTransmission(true);
           }
          
         // heure
         if((heures==0)&&(minutes==0))
         {
          displayDigitsT(saaGMT); // tirets
         }
          else
          {
           if( Aff_TimeSyst==false)
              {
              initDisplay(saaGMT); // init bus
              Wire.beginTransmission(saaGMT);
              Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
              Wire.write(digits[(minutes/10)]); // digit 1 
              Wire.write(digits[(minutes%10)]); // digit 2
              Wire.write(digits[(heures/10)]); // digit 3
              Wire.write(digits[(heures%10)]); // digit 4 
              Wire.endTransmission(true);
              }
              else
              {
          
              initDisplay(saaGMT); // init bus
              Wire.beginTransmission(saaGMT);
              Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
              Wire.write(digits[(Sys_Minute/10)]); // digit 1 
              Wire.write(digits[(Sys_Minute%10)]); // digit 2
              Wire.write(digits[(Sys_Heure/10)]); // digit 3
              Wire.write(digits[(Sys_Heure%10)]); // digit 4 
              Wire.endTransmission(true); 
              }
          }     
}
//////////////////////////////////////////////////////     raz chrono affiche zeros au démarrage chrono

void Chrono_RAZ()
{
      initDisplay(saaCHRONO);      
      Wire.beginTransmission(saaCHRONO);      
      Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
      Wire.write(digits[((CPTR_Chrono/10)%10)]); // digit 1 (RHS)
      Wire.write(digits[(CPTR_Chrono%10)]); // digit 2
      Wire.write(digits[0]); // digit 3
      Wire.write(digits[0]); // digit 4 (LHS)
      Wire.endTransmission(true); 
  }

/////////////////////////////////////////////////////////////// timer2 pompé sur internet telquel
ISR (TIMER2_OVF_vect) 
{  
      // 256-6 --> 250X16uS = 4mS  
      // Recharge le timer pour que la prochaine interruption se déclenche dans 4mS
      TCNT2 = 6;
   
      if (Compteur++ == 250) {
      //250*4mS = 1S - la Led est allumée 1 S et éteinte 1 S
      Compteur=0;  
      LedToggle;  
      CPTR_Seconde++; // incrémentation valeur comptage elapse ( ajout perso, faut bien recuperer le temps)
       //Serial.println(CPTR_Seconde);
   
      }   
    
} 

///////////////////////////////////////////////////////          elapse
void Elapse(int CPTR_Seconde)
{
               if(bBtn_ET_Ini==false)   // init une fois pour retour direct à run
             {
              CPTR_Elapse_=CPTR_Seconde;
              bBtn_ET_Ini=true;
             }
  Gestion_LedElaps(CPTR_Seconde);
  CPTR_Diff=CPTR_Seconde - CPTR_Elapse_;
         //Serial.print("CPTR_Diff1 : ");
         //Serial.println(CPTR_Diff);
    
     if(CPTR_Diff >= 60 )  
     {
      Elapse_Minutes++;  // minutes
          if(Elapse_Minutes == 60)
          {
            Elapse_Heures++;
            Elapse_Minutes=0;
          }
        CPTR_Elapse_=CPTR_Seconde;         
     } 
      else
      {
         // affichage seconde durant la première minute
        if((Elapse_Heures==0) && (Elapse_Minutes==0) && (bBtn_ET_StandBy==false))
           {  
              initDisplay(saaELAPS);
              //Serial.println(CPTR_Diff);             
              Wire.beginTransmission(saaELAPS);     
              Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
              Wire.write(digits[(Elapse_Heures/10)%10]); // digit 1 
              Wire.write(digits[Elapse_Heures%10]); // digit 2
              Wire.write(digits[(CPTR_Diff/10)%10]); // digit 3
              Wire.write(digits[CPTR_Diff%10]); // digit 4 
              Wire.endTransmission(true);            
           } 

            else
            {

              if(bBtn_ET_StandBy==false)  // pas d'affichage si stand by ( position stop )
              {
                initDisplay(saaELAPS);      
                Wire.beginTransmission(saaELAPS);     
                Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
                Wire.write(digits[(Elapse_Heures/10)%10]); // digit 1 
                Wire.write(digits[Elapse_Heures%10]); // digit 2
                Wire.write(digits[(Elapse_Minutes/10)%10]); // digit 3
                Wire.write(digits[Elapse_Minutes%10]); // digit 4 
                Wire.endTransmission(true);
                }

            
            }
      }


 
}

  void Gestion_LedElaps(int CPTR_Seconde)
 {    
     if(CPTR_Seconde%2)    // pour faire clignoter led 
     {
      // digitalWrite(LedScde_Elapse,HIGH);
      analogWrite(LedScde_Elapse,Led_Temps);
      }
      else
      {
      //digitalWrite(LedScde_Elapse,LOW);
      analogWrite(LedScde_Elapse,0);     
      }  
 } 
///////////////////////////************* Chrono  3 etats :  ici 1 lançé,  2 ou 3 stop ou reset !!!
  
void Chrono_Tr(int CPTR_Seconde)
{
                                    //Serial.print("Etat Chrono")  
                                    //Serial.println(Chrono_Etat);
		//bool bChrono_Stop=false;
		switch(Chrono_Etat)
    {
      case 1:   //etat un lancé par start, comptage
      {
        //Serial.println("Etat Un");
        Aiguille.step(4);        // gestion aiguille  
        
        // empêche marche arrière 
        bChrono_Stop=false; // sur stop passe true
        bRAZ_Aiguille=false; // sur raz passe true        
        Moteur_Reverse=false; // passe true sur  arret mecanique
              
        CPTR_Chrono=CPTR_Seconde - CPTR_Chrono_Ini;
                                                        
           if((CPTR_Chrono >= (CPTR_Chrono_ +60 ))&& bChrono_Stop==false)  // 
           {
          		Chrono_Encours++;
                 if(Chrono_Encours==100)  // visu sur deux chiffres remise à zero
                 {
                  Chrono_Encours=0; // affichage zero suite à débordement 99->100
                  CPTR_Chrono_Ini=CPTR_Seconde; // reinit ini
                  CPTR_Chrono_=0;
                  CPTR_Chrono=0;
                 }
          		initDisplay(saaCHRONO);    		
          		Wire.beginTransmission(saaCHRONO);     		
          		Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
          		Wire.write(digits[((Chrono_Encours/10))]); // digit 1 
          		Wire.write(digits[(Chrono_Encours%10)]); // digit 2
          		Wire.write(digits[0]); // digit 3 ?
          		Wire.write(digits[0]); // digit 4 ?
          		Wire.endTransmission(true);
          		CPTR_Chrono_=CPTR_Chrono;  
      	   }
        break;
      }
      case 2: // chrono à l'arrêt par Stop : comptage arreté mais pas raz
      {
        //Serial.println("Etat deux");        
        bChrono_Stop=true;          
        bRAZ_Aiguille=false;  // sécurité
        break;
      }
      case 3: //RAZ  compteur à l'arret : raz affichage et memoires ET RAZ AIGUILLE 
      {
         //Serial.println("Etat trois");    		
    		CPTR_Chrono_Ini=0;
    		CPTR_Chrono_=CPTR_Chrono=0;
    		Chrono_Encours=0;
    		Chrono_Etat=0;
          initDisplay(saaCHRONO);       // affichage tirets, chrono non actif
          Wire.beginTransmission(saaCHRONO);        
          Wire.write(1); // instruction byte 
          Wire.write(digits[10]); // digit 1        tiret pour effacer le zero
          Wire.write(digits[10]); // digit 2        tiret pour effacer le zero
          Wire.write(digits[0]); // digit 3 ?
          Wire.write(digits[0]); // digit 4 ?
          Wire.endTransmission(true);
          
        bRAZ_Aiguille=true; // autorisation Moteur_Reverse 
        break;
      }   // 3
    }   // switch
}


void RAZ_Aiguille()   // raz aiguille marche arrière !!!
{
      if( Moteur_Reverse==false) // P7 passe à zéro pour avoir true contact fin de course
      {
        Aiguille.step(-4);
      }
      else
      {
        bRAZ_Aiguille=false; // pour ne plus appeler la fonction
      }
      
    

}

//////////////////////////////////////////////  Affichage intensité des SAA et leds
///////////////////////// ********** partie sans filet *************
void Affichage_Intensite()
{

   Valeur_Potar=analogRead(A0); //lit la valeur de la tension, la numérise et la stocke dans valPot
   led_value=map(Valeur_Potar,0,1023,0,255); //fonction de mappage pour les leds
   // led_value --> ok pour la led 6
   
   // pour leds GMT et ELAPS potar=0 eteintes , puis max et décrement
 /*
   if(led_value < 5)
   {
    Led_Temps=0;  // éteintes
   }
  
   else
   {
    Led_Temps=255-(led_value-5); // max puis diminution , jamais éteintes par le max
   }
*/
   //  luminosité afficheurs
   if(Valeur_Potar < 10)
   {
    SAA_Intensite=0x00; // ? mA 16/07
    Led_Temps=0;
   }
   else
   {
   Valeur_Lum=1023-Valeur_Potar;
   if(Valeur_Lum > 875)
   {

     SAA_Intensite=0x60;// 18 mA  // 16/07
    Led_Temps=240;
   }
   else if(Valeur_Lum > 700)
   {

    SAA_Intensite=0x50; //  15 mA// 16/07
    Led_Temps=200;
   }
    else if(Valeur_Lum > 525)
    {

      SAA_Intensite=0x40; // 12 mA 16/07
    Led_Temps=160;
    }
    else if(Valeur_Lum > 350)
    {

     SAA_Intensite=0x30; // 9 mA
    Led_Temps=120;     
    }
    else if(Valeur_Lum > 175)
    {

      SAA_Intensite=0x20; // 6 mA 16/07
      Led_Temps=80;
      
    }
    else 
    {

     SAA_Intensite=0x10; //3 mA 16/07
    Led_Temps=40;
    }
   }
  
    
      
      if(SAA_Intensite_!=SAA_Intensite)
      {
        analogWrite(6,led_value); // valeur max 1023  / 4 pour byte ( 255):  cde ouput 14
        //Gestion_LedElaps(CPTR_Seconde); 
        // Gestion_Leds(CPTR_GMT);
        Maj_Affichage();
        SAA_Intensite_=SAA_Intensite;
      }   
}



///////////////////////////////////////////            
void loop() {
//if(bHeure_Fs_Recue==false)
//{
if(dog == false)
{
  Serial.write("A");
  delay(500);
  Serial.println("A");
  if( Serial.available()>0)
  {
    dog=true;
  }
}
else
{
	
	if(Serial.available()>0)        // récupération heure/minute de fs
    {
  	Serial.print("B");	  		
  		heures = Serial.readStringUntil(';').toInt();  		
  		minutes = Serial.readStringUntil(';').toInt();
      secondes=Serial.readStringUntil(';').toInt();
    Sys_Heure = Serial.readStringUntil(';').toInt();
    Sys_Minute= Serial.readStringUntil(';').toInt();
    Sys_Seconde = Serial.readStringUntil('\n').toInt();


      
          analogWrite(Led1quart,0);
          analogWrite(Led2quart,0);
          analogWrite(Led3quart,0);
          analogWrite(Led4quart,0);  
      //Affichage_Heure(heures,minutes);  // heure auto par fs . pb faut attendre le deuxième envoi pour afficher l'heure !     
          displayDigits(saaGMT,heures,minutes); // affichage h/m
         /*
          initDisplay(saaGMT);// Correction du 14/07, ajout ligne
          Wire.beginTransmission(saaGMT);
          Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
          Wire.write(digits[(minutes/10)]); // digit 1 
          Wire.write(digits[(minutes%10)]); // digit 2
          Wire.write(digits[(heures/10)]); // digit 3
          Wire.write(digits[(heures%10)]); // digit 4 
          Wire.endTransmission(true);
          */
          CPTR_GMT=-1;  
        Serial.flush();    
    }
}
//////////////////////////////////////////////////////
/////////////////////////////////////////////////// loop /  test position boutons à chaque tour de loop()
   Lec_PCF=millis();
   Intervalle_Lec=Lec_PCF-Lec_PCF_;
   if(Intervalle_Lec>=200)
   {
   Btn_Pos=Lec_Btn(0x3f);  // lecture de l'état des boutons à chaque tour de loop()
   if(Btn_Pos_!=Btn_Pos)   // changement ? boucle état initial démarrage
   {
  	if(bBtn_Initial==false)  // init faite ?
  	{
  		Btn_Init();            // traitement / mémorisation de la position des boutons au démarrage
  		Btn_Pos_=Btn_Pos;      // pour détecter les changements
  		//digitalWrite(led, HIGH);
  		//delay(500);
  		//digitalWrite(led, LOW); 
  		//delay(500);
  		//Serial.println("fin init");  
  	}
  	else                             // boucle de test changement init faite
  	 {
  		Btn_Zero=Btn_Pos^Btn_Pos_;       // recherche du changement, btn_zero donne le btn ayant changé ( état zéro)
  		//Serial.print("CHGT : ");
  		//Serial.println(Btn_Zero);	   
  		Btn_Pos_=Btn_Pos;               // sauvegarde nouvelle position
  		Btn_Traitement(Btn_Zero);  // traitement des entrées  (changement 1/0 ou 0/1)
  	 }
   }
   Lec_PCF_=Lec_PCF;
   }
// Ramène l'aiguille à zéro : fin quand Moteur_Reverse == true ( contact mécanique (P7))
// P7 restera à zéro tant que le Chrono ne sera pas démarré donc Moteur_Reverse=true
// bRAZ_Aiguille = true dans le setup() pour init une fois et sur commande stop 
// Un cran en arrière à chaque tours de loop()  
// bChrono_Stop passe à true sur deuxième appui (Stop)
// bRAZ_aiguille passe à true sur troisème appui (RAZ)
// bRAZ_aiguille passe à false quand P7 passe à zéro
   if (( bRAZ_Aiguille==true) && (bChrono_Stop==true))
    { 
      RAZ_Aiguille();
    }

    
//////////////////////////////////////////////////           loop / timer / compteur sur une seconde
  if(CPTR_Seconde_!=CPTR_Seconde) // 
  {                             //     
      
      
      if(Chrono_Etat==1)           // chrono lancé ?
      {

        Chrono_Tr(CPTR_Seconde); 
        }     
      
      if (bBtn_ET_Run==true)        // elapse lancé
      {   

        Elapse(CPTR_Seconde);
      }

     /* if(bHeure_Fs_Recue==false)  // on gére l'heure après la première reception de l'heure faut mettre true en ops
      {
        // Gestion_Heure_Locale();   // bidon ne fonctione pas encore    
      }
     */
    /*
      if((heures!=heures_)||(minutes!=minutes_))
      {
         Affichage_Heure(heures,minutes);
         heures_=heures;
         minutes_=minutes;
       }
       */
       CPTR_GMT+=1;
       if(CPTR_GMT > 60)
       {
        dog=false;
       }
       Gestion_Leds(CPTR_GMT);
      
       CPTR_Seconde_=CPTR_Seconde; 
  }
      Affichage_Intensite(); // sans filet début : lecture du potar, calcul et !

}

//}

  
