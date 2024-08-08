#pragma once
#include <Adafruit_INA219.h>
#define MIN_TIME_OPEN_CLOSE_IN_MS 5000
#define MAX_ANGLE_BOTH_BATTANT_WORK 10
#define N_MOYENNE_AMP 10
class Battant;

struct configBattant {
	int pont_H_pinOuverture;
	int pont_H_pinFermeture;
	bool battantType;//0 battant qui doit se fermer en premier, 1, battant qui doit se fermer en deuxi�me.
};

class Battant
{
public:

	//Calibration
	bool init_calibration();//demarre la s�quence de calibration du battant(doit �tre appel� en quasi simultann� sur les deux battants), retourne faux si le battant ne peux pas effectuer la calibration
	bool calibration_Ready(); // True si le battant et pret a demarrer son calibrage individuelle (une fois appel� attendre la fin avant d'appeler sur le deux�me battant)
	void calibrateNextStep(); //Une fois le battant en postion legerment ouverte, il faudrat appel� la fonction next step pour pouvoir demarrer la calibration
	bool calibration_inProgress();  // Renvoie vraie tant que l'etat de la calibration n'est pas � 0 soit pas en cours
	void cancel_calibration(); //Arr�te la proc�dure de calibration et r�staure les derni�res valeur de calibration connues, le battant vas passer en erreur d'origine (il faudra faire une prise d'origine pour pouvoir s'en servir a nouveau)
	
	//Prise d'origine	
	bool init_priseOrigine();//demarre la s�quence de calibration du battant(doit �tre appel� en quasi simultann� sur les deux battants), retourne faux si le battant ne peux pas effectuer la calibration
	bool priseOrigine_Ready(); // True si le battant et pret a demarrer son calibrage individuelle (une fois appel� attendre la fin avant d'appeler sur le deux�me battant)
	void priseOrigineNextStep(); //Une fois le battant en postion legerment ouverte, il faudrat appel� la fonction next step pour pouvoir demarrer la calibration
	bool priseOrigine_inProgress();  // Renvoie vraie tant que l'etat de la calibration n'est pas � 0 soit pas en cours

	//Gestion des obstacles
	void initStopSecurity();
	void handleStopSecurity();
	//loop
	void loop();// a appel� le plus r�guli�rement possible pour assurer le bon focntionnement du voolet(minimum 10 fois par seconde)

	//configuration
	Battant(uint8_t addr_i2c_capteur_courant);
	void config(configBattant batConf);
	void setAutreBattant(Battant* autreBattant);
	void set_time_close(long time_);
	void set_time_open(long time_);
	unsigned long get_time_close();
	unsigned long get_time_open();
	void setMaxAmp(float max);

	int config_done;//0 si pas faite, 1 si ready, -1 si l'adresse de l'autre battant communiqu�, 2 si la config est charg�


	//Contr�le Public
	void setTargetPosition(float pos);  //definie la consigne de position du volet sur la valeur pos (en %)
	float getTargetPosition();  // renvoie la position de destination du battant
	float getRealTargetPosition();  // renvoie la position de destination consider� par le battant (peut differer de getTargetPosition si les deux battants sont sur une zone de chevauchement) 

	float getCurrentPosition(); // renvoie la postition actuelle du battant (-1 si inconnue)

	int getState();

	

	
	
	
	
	
	



	

private:
	////Configuration hardware
	Adafruit_INA219 ampVolet;
	int pinOuverture; //pontH (voir doc)
	int pinFermeture; //pontH (voir doc)



	////Configuration fixe (param�tres du battant constants)
	bool battantType; //0 battant qui doit se fermer en premier, 1, battant qui doit se fermer en deuxi�me.
	float maxAmp;  //Couple maximum autoris� en foncitonnement normal
	int maxTimeOverTorque; // temps maximum autoris� au dela du couple maximum autoris�

	////Donn�es de calibration
	int tempsOuverture;  // temps enregistr� pour l'ouverture du battant
	int tempsFermerture; // temps enregistr� pour la fermeture du battant

	////Variables de detection des fin de courses
	unsigned long firstTimeOverTorqueOpen;  //Garde en m�moire le temps o� la couple a �t� trop elev� pour la premi�re fois afin de pouvoir detecter une but�e si la situation dure
	unsigned long firstTimeOverTorqueClose; // idem que firstTimeOverTorqueOpen pour la fermeture
	unsigned long lastCurrentMesure; // idem que firstTimeOverTorqueOpen pour la fermeture

	float moyenneAmp[N_MOYENNE_AMP];
	float amp;
	///Variables d'�tats des fin de course
	bool inStopperOpen;
	bool inStopperClose;

	///Variables d'�tats general
	/*
	indique l'�tat actuel du battant:
	   -4 : Erreur de butt�, le volet a rencontr� un obstacle et attent une nouvelle position pour agir
	   -3 : En erreur d'origine, le battant ne connait pas sa postion et doit faire une prise d'origine
	   -2 : En cours de calibration
	   -1 : Prise d'origine
		0 : En attente de configuration
		1 : Demarrage
		2 : En position souhait�
		3 : En meilleur position possible (Au plus porche de la postion souhait� sans chevauchement)
		4 : En cours d'ouverture
		5 : En cours de fermeture
	*/
	int battant_state;
	float currentPos; // indique la postion estim�e actuel du battant (entre 0% -> ferm� et 100% -> ouvert)
	float targetPos;  // Indique la postion qui � �t� donn� en consigne au battant
	float true_targetPos; //Indique la position vers la quelle se dirige actuellement le battant. Peut differer de targetPos si les battants sont dans leurs zone de chevauchement pour eviter les collisions
	int speed; // vittesse actuelle des battant (entre 0 -> arret et 255 -> vitesse max)
	int dir;   // direction actuelle du battant (0: fermeture, 1: ouverture)

	///Variables permettant le calcul de la position actuelle
	unsigned long lastMesurePosition;  //millis au quel a ete effectu� le dernier calcul

	//Etat autre battant
	Battant* autreBattant;

	
	
	///Variable de calibrate_loop (pour fonctionnement assync)
	struct calibration_var_set {
		int state;
		/* calibration_state
		indique l'avancement actuel de la calibration:
		-1 : Erreur inconnue lors de la calibration
		0  : pas de calibration en cours
		10 : lancement calibration
		11 : En preparation pour calibration
		12 : En attente pour demarrage de la calibration
		20 : Demarrage de la mise en position ferm�
		21 : mise en position ferm�
		22 : Lancement de l'ouverture
		23 : mesure du temps d'ouverture
		24 : Erreur lors de la mesure du temps d'ouverture retour en postion ferm�
		30 : demmarrage de la mise en position ouverte
		31 : mise en position ouverte
		32 :
		33 : mesure du temps de fermeture
		34 : Erreur lors de la mesure du temps de fermeture retour en postion ouvert
		99:  ouverture leg�re pour permettre l'eventuelle calibration du deuxi�me battant
		100: en cours d'ouverture leg�re
		*/
		int mem_tempsOuverture;  // temps enregistr� pour l'ouverture du battant en cas d'annulation
		int mem_tempsFermerture; // temps enregistr� pour la fermeture du battant en cas d'annulation
		unsigned long clock1;    // Permet de faire des mes mesures de temps dans la loop
	};
	calibration_var_set calibration_var;

	///Variable pour la prise d'origine(pour fonctionnement assync)
	struct origine_var_set {
		int state;
		/* origine_var_set.state
		indique l'avancement actuel de la prise d'origine:
			0  : Pas de prise d'origine en cours
			10 : lancement de la prise d'origine
			11 : En preparation pour la prise d'origine
			12 : En attente pour demarrage de la prise d'origine
			20 : Demarrage de la mise en position ferm�
			21 : mise en position ferm�
		*/
		unsigned long clock1;    // Permet de faire des mes mesures de temps dans la loop

	};
	origine_var_set origine_var;

	///Variable pour la gestion des obstacles
	struct emmergency_stopper_var_set {
		int state;
		/* emmergency_stopper_var_set.state
		indique l'avancement actuel de la prise d'origine:
			0  : Inversion du sens de fontionnement et vitesse max
			1 : Attente de 3 secondes
			2 : Arret

		*/
		unsigned long clock1;    // Permet de faire des mes mesures de temps dans la loop

	};
	emmergency_stopper_var_set emmergency_stopper_var;

	//config
	bool setBattantState(int state);    // Permet de mettre � jour l'�tat actuelle du battant (variable char battant_state)

	
	//loops
	void calibrate_loop(); //G�re la chaine d'action li� � la calibration
	void priseOrigine_loop();


	//Contr�le Priv�
	void setSpeed(int speed);  //Modifie la consigne de vitesse du battant, serat appliqu� a la prochaine mise � jour du pont en H (void updatePontH())
	void setDir(bool dir);     //Modifie la direction du battant (0: fermeture, 1: ouverture) , serat appliqu� a la prochaine mise � jour du pont en H (void updatePontH())
	void updatePontH();		   // Mets � jour le partie Hardware (pont en H) en fonction des consignes de vitesse et direction defini
	void stop();		//Arrete imm�diattement le mouvement du battant. !! Rien d'emp�che le programme de relancer le mouvement imm�diatement apr�s

	
	//Detection de but�es
	bool isInStopperClose();   //Retourne True si le volet est en but� ouverte, False Sinon.
	bool isInStopperOpen();	   //Retourne True si le volet est en but� ferm�, False Sinon
	bool overCurrentSecurityDetector();
	float getAndUpdateIntensite(bool instant = false);// Permet de recuperer l'intensit� du courant absorber par le moteur


	//Contr�le autonome de la position
	void updateTrueTargetPos();
	//Contr�le haut niveau
	void updateSpeedAndDirForTarget(); //Mets � jours les consignes de vitesse afin d'atteindre la position voulue pour le battant
	
	//autre
	int getAutreBatantPos();
};

