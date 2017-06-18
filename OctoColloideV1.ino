//////////////////////////////////////////////////////////////////////
// **************************************************************** //
// ***************************** Projet *************************** //
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
/*   Permet de réalisé des electrolises avec controle de la 		*/
/*   Concentration.													*/
/*         						description							*/
/*         						description							*/
/* **************************************************************** */
/*  Version: Alpha				---				Date: 24/12/2015	*/	
/*  OctoGeeK					---				Octo Don Julo		*/
/* **************************************************************** */
/*	REQUIS:															*/
/*  - Arduino Uno													*/
/*  - Shield L239D (dx)												*/
/*  - 																*/
/* **************************************************************** */
//////////////////////////////////////////////////////////////////////
// **************************************************************** //
// ************************* Bibliothèque ************************* //
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
#include <Timer.h>													// Inclusion de la Bibliothèque d'horloge. 

//////////////////////////////////////////////////////////////////////
// **************************************************************** //
// ********** Constantes, Variables, Fonctions Internes *********** //
// **************************************************************** //
//////////////////////////////////////////////////////////////////////

//  ---		---					Entree					---		--- //
int Pin_EntreeAna = A0;
int Pin_pot = A1;
//  ---		---					Sortie					---		--- //
int Enable1_Pin = 11;												//	broche d'activation du pont en H.
// 	---		---			Initialisation 74hc595			---		--- //
int SerialData_Pin = 8;   											// broche 14 du 75HC595	(SerialData -DS)
int Latch_RCLK_Pin = 12;  											// broche 12 du 75HC595	(LATCH	-RCK)
int Clock_SRCLK_Pin = 4; 											// broche 11 du 75HC595	(CLOCK	-SCK)	
int Enable_595_Pin = 7;												// broche 13 du 75HC595
// 	---		---					Registre				---		--- //
int tailleregistre = 8;
boolean registre[8];												// definie un registre 8 bit
// 	---		---				Variables Timer				---		--- //
	Timer horloge;													// Initialisation d'un timer. 
	#define Tmax				900000								// Valeur max du compteur.en millisecondes.
// 	---		---				Variables seuil				---		--- //	
int ValMesure;													// Valeur mesuré en entrée, correspond a la tension aux bornes des electrodes.
int	ValPot;
int SeuilPOT;
int Seuilmin 				= 751;								// Valeur seuil de tension 1, correspond a une tension de 18,2 V aux bornes des electrodes.
int MaxMesure; 													// Valeur seuil de tension, mesuré quelques secondes après allumage.
int etat = 0;

//////////////////////////////////////////////////////////////////////
// **************************************************************** //
// *********************** Progamme Principal ********************* //
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
void setup()
{
	// --- 					Initialisation  Serie.					---	//	
	Serial.begin(57600);											// Initialise la vitesse de connexion série à 115200 bauds. // 9600, 19200, 115200	
	Serial.println(" Octo-Colloide Va"); 							// Support : affichage serie.	
	Serial.println("debut du programme :");							// Support : affichage serie.
	// --- 					Initialisation  Registre.				---	//	
	pinMode(Latch_RCLK_Pin, OUTPUT);
	pinMode(Clock_SRCLK_Pin, OUTPUT);
	pinMode(SerialData_Pin, OUTPUT);
	pinMode(SerialData_Pin, OUTPUT);
	pinMode(Enable_595_Pin, OUTPUT);
	digitalWrite(Enable_595_Pin, LOW);

	// --- 					Initialisation du Timer. 				--- // 
	horloge.every (Tmax, Inverseursenselectrolise);
	horloge.every (10000, ComparateurSeuil);
	MesureRef();
} 
//////////////////////////////////////////////////////////////////////
// **************************************************************** //
// ************************* Progamme Boucle ********************** //
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
void loop()
{
 	horloge.update();												// incrémente le timer.
	//ComparateurSeuil();
}
//////////////////////////////////////////////////////////////////////
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
void clearRegisters()	// force tout les états du registre a 0.
{
	for(int i = tailleregistre - 1; i >=  0; i--)
	{
		registre[i] = LOW;
	}
	//Serial.println("Clear registre");
}
void writeRegisters()		// afecte le registre du 74hc595 avec le registre PREALABLEMENT chargé.
{
	digitalWrite(Latch_RCLK_Pin, LOW);		// "bloque les sorties dans l'etat ou elles sont"
	for(int i = tailleregistre - 1; i >=  0; i--)
	{
		digitalWrite(Clock_SRCLK_Pin, LOW);	// front de detection a 0.
		int val = registre[i];			// initialise la valeur du registre a recopier
		digitalWrite(SerialData_Pin, val);		// envoi la valeur pour le registre du 74hc595
		digitalWrite(Clock_SRCLK_Pin, HIGH);	// Prise en compte de la valeur envoyé.
	}
	digitalWrite(Latch_RCLK_Pin, HIGH);		// envoi la valeur du registre sur les broches de sortie.
	//Serial.println("Ecriture registre");
}
void setUnitaireRegistre(int index, int value)
{

	registre[index] = value;
}
//////////////////////////////////////////////////////////////////////
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
void Inverseursenselectrolise()
{
	if(etat == 1)	// premier sens d'electrolise
	{
		setUnitaireRegistre(0, LOW);
		setUnitaireRegistre(1, LOW);
		setUnitaireRegistre(2, HIGH);
		setUnitaireRegistre(3, LOW);
		setUnitaireRegistre(4, LOW);
		setUnitaireRegistre(5, LOW);
		setUnitaireRegistre(7, LOW);
		Serial.println("ETAT 1");
		etat =2;
	} 
	else if(etat == 2) 
	{
		setUnitaireRegistre(0, LOW);
		setUnitaireRegistre(1, LOW);
		setUnitaireRegistre(2, LOW);
		setUnitaireRegistre(3, HIGH);
		setUnitaireRegistre(4, LOW);
		setUnitaireRegistre(5, LOW);
		setUnitaireRegistre(7, LOW);
		Serial.println("ETAT 2");
		etat = 1;
	}
	else //if(etat == 0) 
	{
		setUnitaireRegistre(0, LOW);
		setUnitaireRegistre(1, LOW);
		setUnitaireRegistre(2, LOW);
		setUnitaireRegistre(3, LOW);
		setUnitaireRegistre(4, LOW);
		setUnitaireRegistre(5, LOW);
		setUnitaireRegistre(7, LOW);
		Serial.println("ETAT 0");
	}
	writeRegisters();
	Serial.print("MES = "); Serial.println(ValMesure);
	Serial.print("POT = "); Serial.println(SeuilPOT);
	Serial.print("VMES = "); Serial.print(ValMesure*0.02); Serial.println(" V");
	Serial.print("VPOT = "); Serial.print(SeuilPOT*0.02);	Serial.println(" V");
}
//////////////////////////////////////////////////////////////////////
// **************************************************************** //
//////////////////////////////////////////////////////////////////////
void ComparateurSeuil() //*A FAIRE
{
	ValMesure 	= analogRead(Pin_EntreeAna);						 	// Mesure la tension image aux bornes des électrodes.
	ConvertionPOT();									// Mesure la tension image du seuil (potentiometre).

	Serial.print("MES = "); Serial.println(ValMesure);
	// Serial.print("VMES = "); Serial.print(ValMesure*0.02); Serial.println(" V");		//0.0049*4.8 théorique //0.0049*4.1?								
	Serial.print("POT = "); Serial.println(SeuilPOT);
	// Serial.print("VPOT = "); Serial.print(SeuilPOT*0.02);	Serial.println(" V");
	//Serial.println(etat);
	if (ValMesure > 100)
	{
		digitalWrite(Enable1_Pin, HIGH);
		if (etat == 0)
		{
			etat = 1;
			Serial.println("Reinitialise");										// Support : affichage serie.
			Inverseursenselectrolise();
		}
		//Serial.print("MES = "); Serial.println(ValMesure);
		//Serial.print("POT = "); Serial.println(SeuilPOT);
	}
	else 															// STOP
	{
		digitalWrite(Enable1_Pin, LOW);
		etat = 0;
		clearRegisters();
		writeRegisters();
		Serial.println("STOP");
		Serial.print("MES = "); Serial.println(ValMesure);
		Serial.print("POT = "); Serial.println(SeuilPOT);
	}
}

void ConvertionPOT ()
{
	ValPot 	= analogRead(Pin_pot);										// Mesure la tension image du seuil (potentiometre).	
	SeuilPOT = (ValPot/10)+716;											// Equation de des seuils min et max (ax+b) max=(mesureMAX/a)+min. min=b. qui  correspond a une valeur fictive de max a 4V et un min a 3.5V
	if(SeuilPOT >1023)													// Securité de dépassement de valeur max.
	{
		SeuilPOT =1023;													// Valeur max du convertiseur.
	}
}

void MesureRef()
{
	int TensionMax;
	int deltaRef;
	clearRegisters();
	writeRegisters();
	digitalWrite(Enable1_Pin, LOW);
	// ---					Mesure des référenciels.	 			--- //		
	// digitalWrite(PinRelayON, HIGH);									// Active l'electrolise. (electrodes reliées au polariseur)
	MaxMesure = analogRead(Pin_EntreeAna);							// Mesure la tension d'alimentation des electrodes.
	// ---					Calcul des théoriques.	 				--- //		
	deltaRef = MaxMesure - Seuilmin;								//
	TensionMax = MaxMesure * 4.88; 
	Serial.print("delta reference = "); Serial.println(deltaRef);	
	Serial.print("Tension max = "); 	Serial.println(TensionMax);	
}

