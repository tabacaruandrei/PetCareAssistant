#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 2908
#define MAXLEN 500


// Structura pentru animale
typedef struct {
    char userid[7];
    int id;
    char nume[25];
    char specie[25];
    char varsta[10];
    char greutate[10];
    char sanatate[25];
    char alimentatie[25];
    char activitate[25];
} Animal;


// Fisierul unde sunt stocate animalele
const char *fisier_animale = "animale.csv";

// Fisierul unde sunt stocati utilizatorii
const char *fisier_utilizatori = "users.csv";

// Fisierul unde sunt stocati utilizatorii conectati; pt. ca alti clienti sa nu se conecteze pe conturi deja conectate la server
const char *fisier_utilizatori_conectati = "users_on.csv";


// Mutex-uri pentru fisiere
pthread_mutex_t mutex_utilizatori = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_utilizatori_conectati = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_animale = PTHREAD_MUTEX_INITIALIZER;


/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[MAXLEN];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);
	        printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
	        fflush (stdout);		 
	        pthread_detach(pthread_self());	
	        //printf("Oare ajungem aici?\n");
	        raspunde((struct thData*)arg);
	        printf("Am inchis unul din thread-uri\n");
	        /* am terminat cu acest client, inchidem conexiunea */
	        close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
        printf("Ajungem in raspunde\n");
        char app_option[] = "00"; // de aici incepe treaba cu identificatorul
        int step = 0;
        char username[25];
        char password[33];
        char cuvant_secret[25];
        char userid[7];
        Animal animal_introdus;
        Animal animal_modificat;
        while(1)
        {
            printf("Ajungem in while\n");
            int i=0;
            int isnum=0;
            char msg[MAXLEN];      //mesajul primit de la client
            char msgrasp[MAXLEN]=" ";        //mesaj de raspuns pentru client
	    struct thData tdL; 
	    tdL= *((struct thData*)arg);
	    	        
            // citirea mesajului
            if (read (tdL.cl, msg, MAXLEN) <= 0)
            {
                printf("[Thread %d]\n",tdL.idThread);
                perror ("Eroare la read() de la client.\n");
            }

            printf ("[server]Mesajul a fost receptionat...%s\n", msg);

	    strncpy(app_option, &msg[strlen(msg)-2], 2);
	    app_option[2] = '\0';
	    msg[strlen(msg)-2] = '\0';
            
			
            
           
	   if (!strcmp(app_option, "00")){
		  // fereastra de inceput
		  printf("[Thread %d] se afla in fereastra de inceput.\n", tdL.idThread);
		  
		  if( (msg >= '1' && msg <= '4') || !strcmp(msg, "logare") || !strcmp(msg, "inregistrare") || !strcmp(msg, "schimbare parola") || !strcmp(msg, "exit") )
		  {
			strcpy(msgrasp, "\nBun venit in aplicatia PetCareAssistant!\nCe comanda doriti sa efectuati?\n1. Logare\n2. Inregistrare\n3. Schimbare parola\n4. Exit\n00");
		  }
		  
		  else
		  {
			strcpy(msgrasp, "\nMesajul introdus este invalid. Te rog alege una dintre optiuni.");
			strcat(msgrasp, "\nBun venit in aplicatia PetCareAssistant!\nCe comanda doriti sa efectuati?\n1. Logare\n2. Inregistrare\n3. Schimbare parola\n4. Exit\n00");
		  }
	    }
			
			
			else if (!strcmp(app_option, "01")) {
				// fereastra de logare
				printf("[Thread %d] se afla in fereastra de logare.\n", tdL.idThread);
				
				printf("\n\nstep = %d\n\n", step);
				
				
				char parola_criptata[33];

				switch (step) {
				case 0:
					strcpy(msgrasp, "\nIntroduceti username-ul: 01");
					step++;
					break;
				case 1:
					if(!valideaza_user(msg, msgrasp))
					{
						strcat(msgrasp, "00");
						step = 0;
					}
					else if(!verifica_user(msg))
					{
						strcpy(msgrasp, "\nUser-ul introdus nu exista.\n00");
						step = 0;
					}
					else
					{
						strcpy(username, msg);
						strcpy(msgrasp, "\nIntroduceti parola: 01");
						step++;
					}
					break;
				case 2:
					if(!valideaza_pass(msg, msgrasp))
					{
						strcat(msgrasp, "00");
						step = 0;
					}
					else if(verifica_pass(msg))
					{
						strcpy(msgrasp, "\nParola introdusa nu exista.\n00");
						step = 0;
					}
					else
					{
					        printf("\n\nLogare 1\n\n");
						// Criptam parola introdusa
						cripteaza_parola(msg, parola_criptata);
					        printf("\n\nLogare 1_1\n\n");

						pthread_mutex_lock(&mutex_utilizatori);
					        printf("\n\nLogare 1_2\n\n");
						
						pthread_mutex_lock(&mutex_utilizatori_conectati); 
					        //printf("\n\nLogare 1_3\n\n");
						
						
					        printf("\n\nLogare 2\n\n");

						FILE *f = fopen(fisier_utilizatori, "r");
						if (!f) {
							//printf("Nu exista utilizatori in baza de date.\n");
							strcpy(msgrasp, "Nu exista utilizatori in baza de date. \n00");
							pthread_mutex_unlock(&mutex_utilizatori);
							break;
						}
						
						
					        printf("\n\nLogare 3\n\n");

						char linie[256];
						int gasit = 0;
						while (fgets(linie, sizeof(linie), f)) {
							char id[7], user[21], pass[33];
							sscanf(linie, "%6[^,],%20[^,],%32[^\n]", id, user, pass);

							if (strcmp(user, username) == 0) {
								strcpy(userid, id);
								gasit = 1;
								break;
							}
						}
						
						
					        printf("\n\nLogare 4\n\n");

						fclose(f);

						FILE *f2 = fopen(fisier_utilizatori_conectati, "a+");
						if (!f2) {
							perror("Eroare la deschiderea fisierului utilizatori_conectati.csv");
							pthread_mutex_unlock(&mutex_utilizatori);
							return 0;
						}
						
						
					        printf("\n\nLogare 5\n\n");

					        int conectat = 0;
						// Verificam daca utilizatorul este deja conectat
						char linie2[256], id[7], user[21];
						while (fgets(linie2, sizeof(linie2), f2)) {
							sscanf(linie2, "%6[^,],%20[^\n]", id, user);
							if (strcmp(id, userid) == 0) {
								printf("Eroare: Utilizatorul \"%s\" este deja conectat.\n", username);
								strcpy(msgrasp, "Eroare: Utilizatorul ");
								strcat(msgrasp, username);
								strcat(msgrasp, " este deja conectat.\n00");
								fclose(f2);
								pthread_mutex_unlock(&mutex_utilizatori_conectati);
						                pthread_mutex_unlock(&mutex_utilizatori);
								conectat = 1;
								step = 0;
								break;
							}
						}
						if(!conectat)
						{
						    fprintf(f2, "%s,%s\n", userid, username);
						    fclose(f2);
						    
						    pthread_mutex_unlock(&mutex_utilizatori_conectati);
						    pthread_mutex_unlock(&mutex_utilizatori);
						    printf("Logare reusita! User ID: %s\n", userid);
						    strcpy(msgrasp, "Logare efectuata cu succes!\n");
						    strcat(msgrasp, "Bine ai venit in aplicatie, ");
						    strcat(msgrasp, username);
						    strcat(msgrasp, "!\n10");
						}
						step = 0;
					}
					break;
				  }
					
			} else if (!strcmp(app_option, "02")) {
				// fereastra de inregistrare
				printf("[Thread %d] se afla in fereastra de inregistrare.\n", tdL.idThread);
				
				int register_check = 1;
				
				char parola_criptata[33];
				
				switch (step) {
				case 0:
					strcpy(msgrasp, "\nIntroduceti username-ul (min 4 caractere, max 20 caractere): 02");
					step++;
					break;
				case 1:
					if(!valideaza_user(msg, msgrasp))
					{
						//strcpy(msgrasp, "\nUser-ul introdus nu respecta conditiile.\n00");
						strcat(msgrasp, "00");
						step = 0;
					}
					else if(verifica_user(msg))
					{
						strcpy(msgrasp, "\nUser-ul introdus deja exista.\n00");
						step = 0;
					}
					else
					{
						strcpy(username, msg);
						strcpy(msgrasp, "\nIntroduceti parola(min 4, max 20, cel putin o litera mare/mica si un caracter special din ?!.#%): 02");
						step++;
					}
					break;
				case 2:
					if(!valideaza_pass(msg, msgrasp))
					{
						//strcpy(msgrasp, "\nParola introdusa nu respecta conditiile.\n00");
						strcat(msgrasp, "00");
						step = 0;
					}
					else if(verifica_pass(msg))
					{
						strcpy(msgrasp, "\nParola introdusa deja exista.\n00");
						step = 0;
					}
					else
					{
						cripteaza_parola(msg, parola_criptata);
						strcpy(password, parola_criptata);
						strcpy(msgrasp, "\nIntroduceti un cuvant secret. Veti avea nevoie de acesta pentru schimbarea parolei. Stocati-l undeva pentru a nu il uita.\n02");
						step++;
					}
					break;
				case 3:
					if(strlen(msg) < 4 && strlen(msg) > 50)
					{
						strcpy(msgrasp, "Cuvantul secret este prea scurt/lung. Introdu ceva concis si unic.\n00");
					}
					else{
						//strcpy(password, msg);
						pthread_mutex_lock(&mutex_utilizatori);

						// Generam un User ID unic
						int ultimul_id = 0;
						FILE *f = fopen(fisier_utilizatori, "a+");
						if (!f) {
							perror("Eroare la deschiderea fisierului utilizatori.csv");
							strcpy(msgrasp, "\nEroare la inregistrare.\n00");
							pthread_mutex_unlock(&mutex_utilizatori);
							break;
						}

						char linie[256];
						while (fgets(linie, sizeof(linie), f)) {
							sscanf(linie, "%d,", &ultimul_id);
						}

						char userid[7];
						snprintf(userid, sizeof(userid), "%06d", ultimul_id + 1);
						

						// Salvam utilizatorul in fisier
						fprintf(f, "%s,%s,%s,%s\n", userid, username, password, msg);
						fclose(f);

						pthread_mutex_unlock(&mutex_utilizatori);

						strcpy(msgrasp, "\nTe-ai inregistrat cu succes!\n00");
					}
					step = 0;
					break;
				}
			  } 
			  
			  
			if (!strcmp(app_option, "03")) {
				printf("[Thread %d] se afla in fereastra de schimbare a parolei.\n", tdL.idThread);

				switch (step) {
				case 0:
					strcpy(msgrasp, "\nIntroduceti username-ul: 03");
					step++;
					break;

				case 1:
					if (!valideaza_user(msg, msgrasp)) {
					        strcat(msgrasp, "00");
						step = 0;
					} else if (!verifica_user(msg)) {
						strcpy(msgrasp, "\nUser-ul introdus nu exista.\n00");
						step = 0;
					} else {
					        strcpy(username, msg);
						// Verificam daca utilizatorul este conectat
						pthread_mutex_lock(&mutex_utilizatori_conectati);

						FILE *f_conectati = fopen(fisier_utilizatori_conectati, "r");
						if (!f_conectati) {
							perror("Eroare la deschiderea fisierului utilizatori_conectati.csv");
							pthread_mutex_unlock(&mutex_utilizatori_conectati);
							strcpy(msgrasp, "Eroare: nu am putut verifica daca utilizatorul este conectat.\n00");
							step = 0;
							break;
						}

						int conectat = 0;
						char linie[256], conectat_user[21], conectat_id[7];
						while (fgets(linie, sizeof(linie), f_conectati)) {
							sscanf(linie, "%6[^,],%20[^\n]", conectat_id, conectat_user);
							if (strcmp(conectat_user, msg) == 0) {
								conectat = 1;
								break;
							}
						}
						fclose(f_conectati);
						pthread_mutex_unlock(&mutex_utilizatori_conectati);

						if (conectat) {
							strcpy(msgrasp, "\nEroare: utilizatorul este conectat.\n00");
							step = 0;
						} else {
							strcpy(username, msg);
							strcpy(msgrasp, "\nIntroduceti cuvantul secret: 03");
							step++;
						}
					}
					break;

				case 2:
					if (strlen(msg) < 4 || strlen(msg) > 26) {
						strcpy(msgrasp, "Cuvantul secret este prea scurt/lung. Introdu ceva concis si unic.\n00");
						step = 0;
					} else {
						pthread_mutex_lock(&mutex_utilizatori);

						FILE *f = fopen(fisier_utilizatori, "r");
						FILE *temp = fopen("temp_utilizatori.csv", "w");
						if (!f || !temp) {
							perror("Eroare la deschiderea fisierului utilizatori.csv");
							strcpy(msgrasp, "Eroare la deschiderea fisierului de utilizatori.\n00");
							if (f) fclose(f);
							if (temp) fclose(temp);
							pthread_mutex_unlock(&mutex_utilizatori);
							break;
						}

						char linie[256];
						char userid_fisier[7];
						char user[21];
						char pass[33];
						char cuvant_secret_fisier[26];
						int gasit = 0;

						while (fgets(linie, sizeof(linie), f)) {
							sscanf(linie, "%6[^,],%20[^,],%32[^,],%25[^\n]", userid_fisier, user, pass, cuvant_secret_fisier);

							if (strcmp(user, username) == 0) {
								if (strcmp(msg, cuvant_secret_fisier) == 0) {
									gasit = 1;
									strcpy(msgrasp, "Cuvant secret valid.\nIntroduceti parola(min 4, max 20, cel putin o litera mare/mica si un caracter special din ?!.#%): 03");
									step++;
									break;
								} else {
									strcpy(msgrasp, "Eroare: cuvantul secret introdus este incorect.\n00");
									step = 0;
								}
							}
						}

						fclose(f);
						fclose(temp);
						pthread_mutex_unlock(&mutex_utilizatori);

						if (!gasit) {
							strcpy(msgrasp, "Eroare: utilizatorul nu a fost gasit.\n00");
							step = 0;
						}
					}
					break;

				case 3:
					if (!valideaza_pass(msg, msgrasp)) {
					        strcat(msgrasp, "00");
						step = 0;
					} else if (verifica_pass(msg)) {
						strcpy(msgrasp, "\nParola introdusa deja exista.\n00");
						step = 0;
					} else {
					        printf("\n\nparola noua = %s\n\n", msg);
					        char parola_criptata[33];
						cripteaza_parola(msg, parola_criptata);
						pthread_mutex_lock(&mutex_utilizatori);

						FILE *f = fopen(fisier_utilizatori, "r");
						FILE *temp = fopen("temp_utilizatori.csv", "w");
						if (!f || !temp) {
							perror("Eroare la deschiderea fisierului utilizatori.csv");
							strcpy(msgrasp, "Eroare la deschiderea fisierului de utilizatori.\n00");
							if (f) fclose(f);
							if (temp) fclose(temp);
							pthread_mutex_unlock(&mutex_utilizatori);
							break;
						}

						char linie[256];
						char userid_fisier[7];
						char user[21];
						char pass[33];
						char cuvant_secret_fisier[26];
						
						    
						        

						while (fgets(linie, sizeof(linie), f)) {
							sscanf(linie, "%6[^,],%20[^,],%32[^,],%26[^\n]", userid, user, pass, cuvant_secret);

                                                        printf("\n\nuser = %s\n\n", user);
						        printf("\n\nusername = %s\n\n", username);
						        
							if (strcmp(user, username) == 0) {
								fprintf(temp, "%s,%s,%s,%s\n", userid, user, parola_criptata, cuvant_secret);
								continue;
							}

							fprintf(temp, "%s", linie);
						}

						fclose(f);
						fclose(temp);
						remove(fisier_utilizatori);
						rename("temp_utilizatori.csv", fisier_utilizatori);
						pthread_mutex_unlock(&mutex_utilizatori);

						strcpy(msgrasp, "Parola a fost schimbata cu succes!\n00");
						step = 0;
					}
					break;
				}
			}
			  
			else if (!strcmp(app_option, "09")){
				// fereastra de exit din intro
				printf("[Thread %d] este in fereastra de exit din intro.\n", tdL.idThread);
				
				switch(step){
				case 0:
					strcpy(msgrasp, "Esti sigur ca vrei sa iesi din aplicatie(Y/n)?: 09");
					step++;
					break;
				case 1:
					if(!strcmp(msg, "Y") || !strcmp(msg, "y"))
					{
						strcpy(msgrasp, "Ai iesit din aplicatie!\n29");
					}
					else
					{
						strcpy(msgrasp, "Iesire intrerupta.\n00");
					}
					step = 0;
					break;
				}
			}
		
			else if (!strcmp(app_option, "10")){
				// fereastra utilizatorului
				printf("[Thread %d] este in fereastra aplicatiei.\n", tdL.idThread);

				if(((msg >= '1' && msg <= '5') || (msg >= '8' && msg <= '9')) || !strcmp(msg, "listare") || !strcmp(msg, "detalii") || !strcmp(msg, "adauga") || !strcmp(msg, "modifica") || !strcmp(msg, "sterge") || !strcmp(msg, "logout") || !strcmp(msg, "log-out") || !strcmp(msg, "exit") || !strcmp(msg, "quit"))
				{
				
				}
				else
				{
					strcpy(msgrasp, "\nMesajul introdus este invalid. Te rog alege una dintre optiuni.");
					strcat(msgrasp, "\nComenzi disponibile:\n1: Listare (a animalelor)\n2: Detalii (despre animale)\n3: Adauga (un animal)\n4: Modifica (un animal)\n5: (Sterge) un animal\n8: Log-out\n9: Exit\n10");
				}
			} 
			  
			else if (!strcmp(app_option, "11")){
				// fereastra de listare a animalelor
				printf("[Thread %d] este in fereastra de afisare a animalelor.\n", tdL.idThread);
				
				pthread_mutex_lock(&mutex_animale); // Blocam accesul la fisierul animalelor
				
				printf("[Thread %d] afisare animale 1.\n", tdL.idThread);

				FILE *f = fopen(fisier_animale, "r");
				printf("[Thread %d] afisare animale 2.\n", tdL.idThread);
				if (!f) {
					strcpy(msgrasp, "Momentan nu ai introdus niciun animal in baza de date.\n10");
					pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
					break;
				}
				
				printf("[Thread %d] afisare animale 3.\n", tdL.idThread);

				char linie[256];
				int gasit = 0;
				int i = 1;

				//printf("Lista animalelor pentru User ID \"%s\":\n", userid);
					
				while (fgets(linie, sizeof(linie), f)) {
						printf("[Thread %d] afisare animale 3_1.\n", tdL.idThread);
						Animal animal;
					
						printf("[Thread %d] afisare animale 3_2.\n", tdL.idThread);
						sscanf(linie, "%d,%6[^,],%25[^,],%25[^,],%10[^,],%10[^,],%25[^,],%25[^,],%25[^\n]",
							   &animal.id, animal.userid, animal.nume,
							   animal.sanatate, animal.alimentatie, animal.activitate);
						   
						printf("[Thread %d] afisare animale 3_3.\n", tdL.idThread);
						printf("animal.userid = %s\n", animal.userid);
						printf("userid = %s\n", userid);

					if (strcmp(animal.userid, userid) == 0) {
						//printf("ID: %d, Nume: %s\n", animal.id, animal.nume);
						char id[4];
						sprintf(id, "%d", i);
						strcat(msgrasp, "ID: ");
						strcat(msgrasp, id);
						strcat(msgrasp, ", Nume: ");
						strcat(msgrasp, animal.nume);
						strcat(msgrasp, "\n");
						gasit = 1;
					}
					printf("[Thread %d] afisare animale 3_4.\n", tdL.idThread);
					i++;
				}
				
				printf("[Thread %d] afisare animale 4.\n", tdL.idThread);
				strcat(msgrasp, "10");

				fclose(f);
				pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul

				if (!gasit) {
					strcpy(msgrasp, "Momentan nu ai introdus niciun animal in baza de date.\n10");
				}
					
			} 
			else if (!strcmp(app_option, "12")){
				// fereastra de detalii a animalelor
				printf("[Thread %d] este in fereastra de afisare a detaliilor animalelor.\n", tdL.idThread);
		
				switch (step){
				case 0:
					strcpy(msgrasp, "Introduceti numele animalului: 12");
					step++;
					break;
				case 1:
					if(!valideaza_nume_animal(msg, msgrasp))
					{
						//strcpy(msgrasp, "Nu detii un animal cu acest nume sau ai introdus gresit numele animalului.\n10");
						strcat(msgrasp, "10");
				                printf("[Thread %d] detalii animale 1.\n", tdL.idThread);
						
						//step = 0;
					}
					else if(!verifica_nume_animal(userid, msg))
					{
						strcpy(msgrasp, "Nu detii un animal cu acest nume sau ai introdus gresit numele animalului.\n10");
				                printf("[Thread %d] detalii animale 2.\n", tdL.idThread);
						//step = 0;
					}
					else
					{
				                printf("[Thread %d] detalii animale 3.\n", tdL.idThread);
					        pthread_mutex_lock(&mutex_animale); // Blocam accesul la fisierul animalelor

				                FILE *f = fopen(fisier_animale, "r");
				                if (!f) {
					                strcpy(msgrasp, "Animalul nu se afla in baza de date.\n10");
					                pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
				                }
				                printf("[Thread %d] detalii animale 4.\n", tdL.idThread);

				                char linie[256];
				                int gasit = 0;

				                while (fgets(linie, sizeof(linie), f)) {
					                Animal animal;
					                sscanf(linie, "%d,%6[^,],%25[^,],%25[^,],%10[^,],%10[^,],%25[^,],%25[^,],%25[^\n]",
						                   &animal.id, animal.userid, animal.nume,
								   animal.specie, animal.varsta, animal.greutate,
						                   animal.sanatate, animal.alimentatie, animal.activitate);

					                if (strcmp(animal.userid, userid) == 0 && strcmp(animal.nume, msg) == 0) {
					                        strcpy(msgrasp, "Detalii animal:");
					                        strcat(msgrasp, "\nNume: ");
					                        strcat(msgrasp, animal.nume);
					                        strcat(msgrasp, "\nSpecie: ");
					                        strcat(msgrasp, animal.specie);
					                        strcat(msgrasp, "\nVarsta: ");
					                        strcat(msgrasp, animal.varsta);
					                        strcat(msgrasp, "\nGreutate: ");
					                        strcat(msgrasp, animal.greutate);
					                        strcat(msgrasp, "\nStare de sanatate: ");
					                        strcat(msgrasp, animal.sanatate);
					                        strcat(msgrasp, "\nAlimentatie:");
					                        strcat(msgrasp, animal.alimentatie);
					                        strcat(msgrasp, "\nActivitate: ");
					                        strcat(msgrasp, animal.activitate);
					                        strcat(msgrasp, "\n10");
					                        
					                        
					                        
					                        /*
						                printf("Detalii animal:\n");
						                printf("Nume: %s\n", animal.nume);
						                printf("Stare de sanatate: %s\n", animal.sanatate);
						                printf("Alimentatie: %s\n", animal.alimentatie);
						                printf("Activitate: %s\n", animal.activitate);
						                */
						                gasit = 1;
						                break;
					                }
				                }

				                fclose(f);
				                pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul

				                if (!gasit) {
						        strcpy(msgrasp, "Animalul nu se afla in baza de date.\n10");
				                }
				        }
				        step = 0;
				        break;
				}
			}
			
			else if (!strcmp(app_option, "13")){
				// fereastra de adaugare a animalelor
				printf("[Thread %d] este in fereastra de adaugare a animalelor.\n", tdL.idThread);
				int valid = 0;
				printf("[Thread %d] step = %d.\n", tdL.idThread, step);
				
				switch (step){
				case 0:
					strcpy(msgrasp, "Introduceti numele animalului: 13");
					step++;
					break;
				case 1:
					if(!valideaza_nume_animal(msg, msgrasp))
					{
						//strcpy(msgrasp, "Nu detii un animal cu acest nume sau ai introdus gresit numele animalului.\n10");
						strcat(msgrasp, "10");
						step = 0;
					}
					else if(verifica_nume_animal(userid, msg))
					{
						strcpy(msgrasp, "Detii deja un animal cu acest nume.\n10");
						step = 0;
					}
					else
					{
						strcpy(animal_introdus.nume, msg);
						strcpy(msgrasp, "Introduceti specia animalului (Enter pentru a lasa gol): 13");
						step++;
					}
					break;
				case 2:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Mesajul introdus e invalid sau prea lung.\n10");
						step = 0;
					}
					else
					{
						strcpy(animal_introdus.specie, msg);
						strcpy(msgrasp, "Introduceti varsta animalului (Enter pentru a lasa gol): 13");
						step++;
					}
					break;
				case 3:
					if(strlen(msg) >= 10)
					{
						strcpy(msgrasp, "Mesajul introdus e invalid sau prea lung.\n10");
						step = 0;
					}
					else
					{
						strcpy(animal_introdus.varsta, msg);
						strcpy(msgrasp, "Introduceti greutatea animalului (Enter pentru a lasa gol): 13");
						step++;
					}
					break;
				case 4:
					if(strlen(msg) >= 10)
					{
						strcpy(msgrasp, "Mesajul introdus e invalid sau prea lung.\n10");
						step = 0;
					}
					else
					{
						strcpy(animal_introdus.greutate, msg);
						strcpy(msgrasp, "Introduceti starea de sanatate animalului (Enter pentru a lasa gol): 13");
						step++;
					}
					break;
				case 5:
					if(strlen(msg) >= 50)
					{
						strcpy(msgrasp, "Mesajul introdus e invalid sau prea lung.\n10");
						step = 0;
					}
					else
					{
						strcpy(animal_introdus.sanatate, msg);
						strcpy(msgrasp, "Introduceti alimentatia animalului (Enter pentru a lasa gol): 13");
						step++;
					}
					break;
				case 6:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Mesajul introdus e invalid sau prea lung.\n10");
						step = 0;
					}
					else
					{
						strcpy(animal_introdus.alimentatie, msg);
						strcpy(msgrasp, "Introduceti activitatea animalului (Enter pentru a lasa gol): 13");
						step++;
					}
					break;
				case 7:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Mesajul introdus e invalid sau prea lung.\n10");
						//step = 0;
					}
					else
					{
						/*
						strcpy(msgrasp, "13");
						step++;
						*/
						strcpy(animal_introdus.activitate, msg);
						valid = 1;
						
						pthread_mutex_lock(&mutex_animale); // Blocam accesul la fisierul animalelor

						// Deschidem fisierul pentru a adauga animalul
						FILE *f = fopen(fisier_animale, "a+");
						if (!f) {
							perror("Eroare la deschiderea fisierului animale.csv");
							strcpy(msgrasp, "Eroare la deschiderea fisierului cu animale.\n10");
							pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
							break;
						}

						// Generam un ID unic pentru animal
						animal_introdus.id = 1;
						char linie[256];
						while (fgets(linie, sizeof(linie), f)) {
							sscanf(linie, "%d", &animal_introdus.id);
						}
						animal_introdus.id++; // Incrementam ID-ul

						// Salvam animalul in fisier
						fprintf(f, "%d,%s,%s,%s,%s,%s,%s,%s,%s\n", animal_introdus.id, userid, animal_introdus.nume,
								animal_introdus.specie[0] ? animal_introdus.specie : "N/A",
								animal_introdus.varsta[0] ? animal_introdus.varsta : "N/A",
								animal_introdus.greutate[0] ? animal_introdus.greutate : "N/A",
								animal_introdus.sanatate[0] ? animal_introdus.sanatate : "N/A",
								animal_introdus.alimentatie[0] ? animal_introdus.alimentatie : "N/A",
								animal_introdus.activitate[0] ? animal_introdus.activitate : "N/A");

						fclose(f);
						pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
						
						strcpy(msgrasp, "Animalul ");
						strcat(msgrasp, animal_introdus.nume);
						strcat(msgrasp, " a fost adaugat cu succes!\n10");
					}
					step = 0;
					break;
				}
				
			}
			
			else if (!strcmp(app_option, "14"))
			{
				// fereastra de modificare a animalelor
				printf("[Thread %d] este in fereastra de modificare a animalelor.\n", tdL.idThread);
				//char nume[50];
				char buffer[256];
				
				switch (step){
				case 0:
				        animal_modificat.id = 0;
                                        strcpy(animal_modificat.userid, "");
                                        strcpy(animal_modificat.nume, "");
                                        strcpy(animal_modificat.specie, "");
                                        strcpy(animal_modificat.varsta, "");
                                        strcpy(animal_modificat.greutate, "");
                                        strcpy(animal_modificat.sanatate, "");
                                        strcpy(animal_modificat.alimentatie, "");
                                        strcpy(animal_modificat.activitate, "");
					strcpy(msgrasp, "Introduceti numele animalului: 14");
					step++;
					break;
				case 1:
					if(!valideaza_nume_animal(msg, msgrasp))
					{
						//strcpy(msgrasp, "Nu detii un animal cu acest nume sau ai introdus gresit numele animalului.\n10");
						strcat(msgrasp, "10");
						step = 0;
					}
					else if(!verifica_nume_animal(userid, msg))
					{
						strcpy(msgrasp, "Nu detii un animal cu acest nume.\n10");
						step = 0;
					}
					else
					{
						pthread_mutex_lock(&mutex_animale); // Blocam accesul la fisierul original

						FILE *f = fopen(fisier_animale, "r");
						if (!f) {
						    perror("Eroare la deschiderea fisierului animale.csv");
						    strcpy(msgrasp, "Eroare la deschiderea fisierului de animale.\n10");
						    pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
						    break;
						}

						// Variabila pentru a stoca animalul gasit
						// Animal animal_modificat;

						// Cautam animalul in fisier
						char linie[256];
						while (fgets(linie, sizeof(linie), f)) {
						    Animal animal;
						    sscanf(linie, "%d,%6[^,],%25[^,],%25[^,],%10[^,],%10[^,],%25[^,],%25[^,],%25[^\n]",
						           &animal.id, animal.userid, animal.nume,
						           animal.specie, animal.varsta, animal.greutate,
						           animal.sanatate, animal.alimentatie, animal.activitate);

						    if (strcmp(animal.userid, userid) == 0 && strcmp(animal.nume, msg) == 0) {
							    //animal_modificat = animal; // Salvam animalul gasit
							    printf("\n\n\n\nAm gasit animalul.\n\n\n\n");
							    animal_modificat.id = animal.id;
							    strcpy(animal_modificat.userid, animal.userid);
							    strcpy(animal_modificat.nume, animal.nume);
							    strcpy(animal_modificat.specie, animal.specie);
							    strcpy(animal_modificat.varsta, animal.varsta);
							    strcpy(animal_modificat.greutate, animal.greutate);
							    strcpy(animal_modificat.sanatate, animal.sanatate);
							    strcpy(animal_modificat.alimentatie, animal.alimentatie);
							    strcpy(animal_modificat.activitate, animal.activitate);
							    break; // Oprim cautarea
						    }
						}

						fclose(f); // Inchidem fisierul original
						pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul pentru fisierul original
						
						strcpy(animal_modificat.nume, msg);
						

						// Solicitam utilizatorului datele noi pentru animal
						//printf("Introduceti noua stare de sanatate (Enter pentru a lasa neschimbat): ");
						strcpy(msgrasp, "Introduceti noua specie (Enter pentru a lasa neschimbat): 14");
						step++;
					}
					break;
				case 2:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Specia introdusa este prea lunga.\n10");
						step = 0;
					}
					else
					{
						    printf("animal_modificat.specie: %s\n", animal_modificat.specie);
					        if(strlen(msg) != NULL)
					        {
						    strcpy(animal_modificat.specie, msg);
						    printf("animal_modificat.specie: %s\n", animal_modificat.specie);
						}
						    printf("animal_modificat.specie: %s\n", animal_modificat.specie);
						strcpy(msgrasp, "Introduceti noua varsta (Enter pentru a lasa neschimbat): 14");
						step++;
					}
					break;
				case 3:
					if(strlen(msg) >= 10)
					{
						strcpy(msgrasp, "Varsta introdusa este prea lunga.\n10");
						step = 0;
					}
					else
					{
						    printf("animal_modificat.varsta: %s\n", animal_modificat.varsta);
					        if(strlen(msg) != NULL)
					        {
						    strcpy(animal_modificat.varsta, msg);
						    printf("animal_modificat.varsta: %s\n", animal_modificat.varsta);
						}
						    printf("animal_modificat.varsta: %s\n", animal_modificat.varsta);
						strcpy(msgrasp, "Introduceti noua greutate (Enter pentru a lasa neschimbat): 14");
						step++;
					}
					break;
				case 4:
					if(strlen(msg) >= 10)
					{
						strcpy(msgrasp, "Greutatea introdusa este prea lunga.\n10");
						step = 0;
					}
					else
					{
						    printf("animal_modificat.greutate: %s\n", animal_modificat.greutate);
					        if(strlen(msg) != NULL)
					        {
						    strcpy(animal_modificat.greutate, msg);
						    printf("animal_modificat.greutate: %s\n", animal_modificat.greutate);
						}
						    printf("animal_modificat.greutate: %s\n", animal_modificat.greutate);
						strcpy(msgrasp, "Introduceti noua stare de sanatate (Enter pentru a lasa neschimbat): 14");
						step++;
					}
					break;
				case 5:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Starea de sanatate introdusa este prea lunga.\n10");
						step = 0;
					}
					else
					{
					        printf("animal_modificat.sanatate: %s\n", animal_modificat.sanatate);
					        if(strlen(msg) != NULL)
					        {
						    strcpy(animal_modificat.sanatate, msg);
						    printf("animal_modificat.sanatate: %s\n", animal_modificat.sanatate);
						}
						    printf("animal_modificat.sanatate: %s\n", animal_modificat.sanatate);
						strcpy(msgrasp, "Introduceti noua alimentatie (Enter pentru a lasa neschimbat): 14");
						step++;
					}
					break;
				case 6:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Alimentatia introdusa este prea lunga.\n10");
						step = 0;
					}
					else
					{
						printf("animal_modificat.alimentatie: %s\n", animal_modificat.alimentatie);
					        if(strlen(msg) != NULL)
					        {
						    strcpy(animal_modificat.alimentatie, msg);
						    printf("animal_modificat.alimentatie: %s\n", animal_modificat.alimentatie);
						}
						printf("animal_modificat.alimentatie: %s\n", animal_modificat.alimentatie);
						strcpy(msgrasp, "Introduceti stilul de activitate modificat (Enter pentru a lasa neschimbat): 14");
						step++;
					}
					break;
				case 7:
					if(strlen(msg) >= 25)
					{
						strcpy(msgrasp, "Activitatea introdusa este prea lunga.\n10");
						//step = 0;
					}
					else
					{
						printf("animal_modificat.activitate: %s\n", animal_modificat.activitate);
					        if(strlen(msg) != NULL)
					        {
						    strcpy(animal_modificat.activitate, msg);
						    printf("animal_modificat.activitate: %s\n", animal_modificat.activitate);
						}
						printf("animal_modificat.activitate: %s\n", animal_modificat.activitate);
						
					        char linie[256];
						// Pregatim rescrierea fisierului
						pthread_mutex_lock(&mutex_animale); // Blocam accesul la fisierul original

						FILE *f = fopen(fisier_animale, "r");
						FILE *temp = fopen("temp_animale.csv", "w");
						if (!f || !temp) 
						{
							perror("Eroare la deschiderea fisierelor.");
							if (f) fclose(f);
							if (temp) fclose(temp);
							pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
							return;
						}

						// Parcurgem fisierul original si scriem in fisierul temporar
						while (fgets(linie, sizeof(linie), f)) 
						{
							Animal animal;
							sscanf(linie, "%d,%6[^,],%25[^,],%25[^,],%10[^,],%10[^,],%25[^,],%25[^,],%25[^\n]",
								   &animal.id, animal.userid, animal.nume,
								   animal.specie, animal.varsta, animal.greutate,
								   animal.sanatate, animal.alimentatie, animal.activitate);

							if (animal.id == animal_modificat.id && strcmp(animal.userid, userid) == 0) 
							{
								// Scriem linia modificata
								fprintf(temp, "%d,%s,%s,%s,%s,%s,%s,%s,%s\n", animal_modificat.id, animal_modificat.userid,
									animal_modificat.nume,
									animal_modificat.specie[0] ? animal_modificat.specie : "N/A",
									animal_modificat.varsta[0] ? animal_modificat.varsta : "N/A",
									animal_modificat.greutate[0] ? animal_modificat.greutate : "N/A",
									animal_modificat.sanatate[0] ? animal_modificat.sanatate : "N/A",
									animal_modificat.alimentatie[0] ? animal_modificat.alimentatie : "N/A",
									animal_modificat.activitate[0] ? animal_modificat.activitate : "N/A");
							} 
							else 
							{
								// Scriem linia originala
								fprintf(temp, "%s", linie);
							}
						}

						fclose(f);
						fclose(temp);

						// Inlocuim fisierul original cu cel temporar
						remove(fisier_animale);
						rename("temp_animale.csv", fisier_animale);

						pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul pentru fisierul original

						//printf("Animalul \"%s\" a fost modificat cu succes.\n", nume);
						strcpy(msgrasp, "Animalul ");
						strcat(msgrasp, animal_modificat.nume);
						strcat(msgrasp, " a fost modificat cu succes.\n10");
				
					}
					step = 0;
					break;
				}
			}
			
			else if (!strcmp(app_option, "15"))
			{
				// fereastra de stergere a animalelor
				printf("[Thread %d] este in fereastra de stergere a animalelor.\n", tdL.idThread);
				switch (step){
				case 0:
					strcpy(msgrasp, "Introduceti numele animalului: 15");
					step++;
					break;
				case 1:
					if(!valideaza_nume_animal(msg, msgrasp))
					{
						//strcpy(msgrasp, "Nu detii un animal cu acest nume sau ai introdus gresit numele animalului.\n10");
						strcat(msgrasp, "10");
						step = 0;
					}
					else if(!verifica_nume_animal(userid, msg))
					{
						strcpy(msgrasp, "Nu detii un animal cu acest nume.\n10");
						step = 0;
					}
					else
					{
						pthread_mutex_lock(&mutex_animale); // Blocam accesul la fisierul animalelor

						FILE *f = fopen("animale.csv", "r");
						FILE *temp = fopen("temp_animale.csv", "w");
						if (!f || !temp) {
							perror("Eroare la deschiderea fisierelor.");
							strcpy(msgrasp, "Eroare la deschiderea fisierelor.\n10");
							if (f) fclose(f);
							if (temp) fclose(temp);
							pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul
							return;
						}

						char linie[256];
						while (fgets(linie, sizeof(linie), f)) {
							Animal animal;
							sscanf(linie, "%d,%6[^,],%25[^,],%25[^,],%10[^,],%10[^,],%25[^,],%25[^,],%25[^\n]",
								   &animal.id, animal.userid, animal.nume,
								   animal.specie, animal.varsta, animal.greutate,
								   animal.sanatate, animal.alimentatie, animal.activitate);

							if (strcmp(animal.userid, userid) == 0 && strcmp(animal.nume, msg) == 0) {
								//gasit = 1;
								continue; // Sarim peste scrierea acestui animal in fisierul temporar
							}

							fprintf(temp, "%d,%s,%s,%s,%s,%s,%s,%s,%s\n", animal_modificat.id, animal_modificat.userid,
								animal_modificat.nume,
								animal_modificat.specie[0] ? animal_modificat.specie : "N/A",
								animal_modificat.varsta[0] ? animal_modificat.varsta : "N/A",
								animal_modificat.greutate[0] ? animal_modificat.greutate : "N/A",
								animal_modificat.sanatate[0] ? animal_modificat.sanatate : "N/A",
								animal_modificat.alimentatie[0] ? animal_modificat.alimentatie : "N/A",
								animal_modificat.activitate[0] ? animal_modificat.activitate : "N/A");
						}

						fclose(f);
						fclose(temp);

						// Inlocuim fisierul original cu cel temporar
						remove("animale.csv");
						rename("temp_animale.csv", "animale.csv");

						pthread_mutex_unlock(&mutex_animale); // Deblocam mutex-ul

						//printf("Animalul \"%s\" a fost sters cu succes.\n", nume);
						strcpy(msgrasp, "Animalul ");
						strcat(msgrasp, msg);
						strcat(msgrasp, " a fost sters cu succes.\n10");
					}
					step = 0;
					break;
				}
			}
			
			
			
			
		
			else if (!strcmp(app_option, "18")){
				// fereastra de log-out
				printf("[Thread %d] este in fereastra de logout.\n", tdL.idThread);
				
				switch(step){
				case 0:
				        strcpy(msgrasp, "Esti sigur ca vrei sa te deloghezi din cont(Y/n)?: 18");
					step++;
					break;
				case 1:
					if(!strcmp(msg, "Y") || !strcmp(msg, "y"))
					{
						pthread_mutex_lock(&mutex_utilizatori_conectati);

						FILE *f = fopen(fisier_utilizatori_conectati, "r");
						FILE *temp = fopen("temp_utilizatori_conectati.csv", "w");
						if (!f || !temp) {
							perror("Eroare la deschiderea fisierelor.");
							if (f) fclose(f);
							if (temp) fclose(temp);
							pthread_mutex_unlock(&mutex_utilizatori);
							return 0;
						}

						char linie[256], id[7], user[21];
						int gasit = 0;

						// Copiem utilizatorii care nu sunt cel care doreste sa se deconecteze
						while (fgets(linie, sizeof(linie), f)) {
							sscanf(linie, "%6[^,],%20[^\n]", id, user);
							if (strcmp(id, userid) != 0) {
								fprintf(temp, "%s", linie); // Scriem liniile nemodificate
							} else {
								gasit = 1; // Gasim utilizatorul deconectat
							}
						}

						fclose(f);
						fclose(temp);

						// Inlocuim fisierul original cu cel temporar
						remove(fisier_utilizatori_conectati);
						rename("temp_utilizatori_conectati.csv", fisier_utilizatori_conectati);

						pthread_mutex_unlock(&mutex_utilizatori_conectati);

						strcpy(msgrasp, "Delogare reusita!\n00");
						
					}
					else
					{
						strcpy(msgrasp, "Delogare intrerupta.\n10");
					}
					step = 0;
					break;
			    }
				
				
			}
			
			
			else if (!strcmp(app_option, "19")){
				// fereastra de exit
				printf("[Thread %d] este in fereastra de exit.\n", tdL.idThread);
				
				switch(step){
				case 0:
					strcpy(msgrasp, "Esti sigur ca vrei sa iesi din aplicatie(Y/n)?: 19");
					step++;
					break;
				case 1:
					if(!strcmp(msg, "Y") || !strcmp(msg, "y"))
					{
						pthread_mutex_lock(&mutex_utilizatori_conectati);

						FILE *f = fopen(fisier_utilizatori_conectati, "r");
						FILE *temp = fopen("temp_utilizatori_conectati.csv", "w");
						if (!f || !temp) {
							perror("Eroare la deschiderea fisierelor.");
							if (f) fclose(f);
							if (temp) fclose(temp);
							pthread_mutex_unlock(&mutex_utilizatori);
							return 0;
						}

						char linie[256], id[7], user[21];
						int gasit = 0;

						// Copiem utilizatorii care nu sunt cel care doreste sa se deconecteze
						while (fgets(linie, sizeof(linie), f)) {
							sscanf(linie, "%6[^,],%20[^\n]", id, user);
							if (strcmp(id, userid) != 0) {
								fprintf(temp, "%s", linie); // Scriem liniile nemodificate
							} else {
								gasit = 1; // Gasim utilizatorul deconectat
							}
						}

						fclose(f);
						fclose(temp);

						// Inlocuim fisierul original cu cel temporar
						remove(fisier_utilizatori_conectati);
						rename("temp_utilizatori_conectati.csv", fisier_utilizatori_conectati);

						pthread_mutex_unlock(&mutex_utilizatori);
						strcpy(msgrasp, "Ai iesit din aplicatie!\n29");
					}
					else
					{
						strcpy(msgrasp, "Iesire intrerupta.\n10");
					}
					step = 0;
					break;
				}
			}
			
		
		
		
		
		
		
		
	    
	    if(strcmp(app_option, "00") && msgrasp[strlen(msgrasp)-2] == '0' && msgrasp[strlen(msgrasp)-1] == '0')
	    {
	        msgrasp[strlen(msgrasp)-2] = '\0';
	        strcat(msgrasp, "\nBun venit in aplicatia PetCareAssistant!\nCe comanda doriti sa efectuati?\n1. Logare\n2. Inregistrare\n3. Schimbare parola\n9. Exit\n00");
	    }
            
            
	    if(strcmp(app_option, "10") && msgrasp[strlen(msgrasp)-2] == '1' && msgrasp[strlen(msgrasp)-1] == '0')
	    {
	        msgrasp[strlen(msgrasp)-2] = '\0';
		strcat(msgrasp, "\nComenzi disponibile:\n1: Listarea animalelor\n2: Detalii despre animale\n3: Adauga un animal\n4: Modifica un animal\n5: Sterge un animal\n8: Log-out\n9: Exit\n10");
	    }
            
            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);


            //returnam mesajul clientului
            if (write (tdL.cl, msgrasp, 500) <= 0)
            {
                printf("[Thread %d] ",tdL.idThread);
                perror ("[Thread]Eroare la write() catre client.\n");
            }
            else
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
                
                
            
	    if(msgrasp[strlen(msgrasp)-2] == '2' && msgrasp[strlen(msgrasp)-1] == '9')
	    {
	        break;
	    }
            
                
        }
		


}












int valideaza_user(const char *username, char *error_message) {
    int lungime = strlen(username);

    // Verificam lungimea
    if (lungime < 4 || lungime > 20) {
        strcpy(error_message, "Eroare: Username-ul trebuie sa aiba intre 4 si 20 de caractere.");
        return 0;
    }

    // Verificam caracterele permise
    for (int i = 0; i < lungime; i++) {
        if (!((username[i] >= 'a' && username[i] <= 'z') || 
              (username[i] >= 'A' && username[i] <= 'Z') || 
              (username[i] >= '0' && username[i] <= '9') || 
              username[i] == '_' || username[i] == '-')) {
            strcpy(error_message, "Eroare: Username-ul poate contine doar litere, cifre, '_' sau '-'.");
            return 0;
        }
    }

    // Username valid
    strcpy(error_message, "Validare reusita: Username-ul este valid.");
    return 1;
}


int verifica_user(const char *username) {
    pthread_mutex_lock(&mutex_utilizatori);

    FILE *f = fopen(fisier_utilizatori, "r");
    if (!f) {
        pthread_mutex_unlock(&mutex_utilizatori);
        return 0; // Fisierul nu exista sau nu este accesibil
    }

    char linie[256];
    char userid[7], user[21], pass[33];
    while (fgets(linie, sizeof(linie), f)) {
        sscanf(linie, "%6[^,],%20[^,],%32[^\n]", userid, user, pass);
        if (strcasecmp(user, username) == 0) { // Comparare case-insensitive
            fclose(f);
            pthread_mutex_unlock(&mutex_utilizatori);
            return 1; // Username deja existent
        }
    }

    fclose(f);
    pthread_mutex_unlock(&mutex_utilizatori);
    return 0; // Username-ul nu exista
}


int valideaza_pass(const char *password, char *error_message) {
    int lungime = strlen(password);

    // Verificam lungimea
    if (lungime < 6 || lungime > 15) {
        strcpy(error_message, "Eroare: Parola trebuie sa aiba intre 6 si 15 caractere.");
        return 0;
    }

    // Verificam caracterele cerute
    int are_litera_mica = 0, are_litera_mare = 0, are_cifra = 0, are_special = 0;

    for (int i = 0; i < lungime; i++) {
        if (password[i] >= 'a' && password[i] <= 'z') {
            are_litera_mica = 1;
        } else if (password[i] >= 'A' && password[i] <= 'Z') {
            are_litera_mare = 1;
        } else if (password[i] >= '0' && password[i] <= '9') {
            are_cifra = 1;
        } else if (password[i] == '!' || password[i] == '?' || password[i] == '.' || password[i] == '_') {
            are_special = 1;
        } else {
            strcpy(error_message, "Eroare: Parola contine caractere invalide. Permise: litere, cifre, '!', '?', '.', '_'.");
            return 0;
        }
    }

    if (!are_litera_mica) {
        strcpy(error_message, "Eroare: Parola trebuie sa contina cel putin o litera mica.");
        return 0;
    }

    if (!are_litera_mare) {
        strcpy(error_message, "Eroare: Parola trebuie sa contina cel putin o litera mare.");
        return 0;
    }

    if (!are_cifra) {
        strcpy(error_message, "Eroare: Parola trebuie sa contina cel putin o cifra.");
        return 0;
    }

    if (!are_special) {
        strcpy(error_message, "Eroare: Parola trebuie sa contina cel putin un caracter special ('!', '?', '.', '_').");
        return 0;
    }

    // Parola valida
    strcpy(error_message, "Validare reusita: Parola este valida.");
    return 1;
}



int verifica_pass(const char *password) {
    char parola_criptata[33];
    cripteaza_parola(password, parola_criptata); // Criptam parola pentru verificare

    pthread_mutex_lock(&mutex_utilizatori);

    FILE *f = fopen(fisier_utilizatori, "r");
    if (!f) {
        pthread_mutex_unlock(&mutex_utilizatori);
        return 0; // Fisierul nu exista sau nu este accesibil
    }

    char linie[256];
    char userid[7], user[21], pass[33];
    while (fgets(linie, sizeof(linie), f)) {
        sscanf(linie, "%6[^,],%20[^,],%32[^\n]", userid, user, pass);
        if (strcmp(pass, parola_criptata) == 0) { // Comparam parola criptata
            fclose(f);
            pthread_mutex_unlock(&mutex_utilizatori);
            return 1; // Parola deja existenta
        }
    }

    fclose(f);
    pthread_mutex_unlock(&mutex_utilizatori);
    return 0; // Parola nu este utilizata
}



int valideaza_nume_animal(const char *nume_animal, char *error_message) {
    int lungime = strlen(nume_animal);

    // Verificam lungimea
    if (lungime < 1 || lungime > 50) {
        strcpy(error_message, "Eroare: Numele animalului trebuie sa aiba intre 1 si 50 de caractere.\n");
        return 0;
    }

    // Verificam caracterele permise
    for (int i = 0; i < lungime; i++) {
        if (!((nume_animal[i] >= 'a' && nume_animal[i] <= 'z') ||
              (nume_animal[i] >= 'A' && nume_animal[i] <= 'Z') ||
              (nume_animal[i] >= '0' && nume_animal[i] <= '9') ||
              nume_animal[i] == ' ' || nume_animal[i] == '_' || nume_animal[i] == '-')) {
            strcpy(error_message, "Eroare: Numele animalului poate contine doar litere, cifre, spatii, '_' sau '-'.\n");
            return 0;
        }
    }

    strcpy(error_message, "Validare reusita: Numele animalului este valid.\n");
    return 1;
}

int verifica_nume_animal(const char *userid, const char *nume_animal) {
    pthread_mutex_lock(&mutex_animale);

    FILE *f = fopen(fisier_animale, "r");
    if (!f) {
        pthread_mutex_unlock(&mutex_animale);
        return 0; // Fisierul nu exista sau nu este accesibil
    }

    char linie[256];
    Animal animal;

    // Verificam fiecare linie din fisier
    while (fgets(linie, sizeof(linie), f)) {
        sscanf(linie, "%d,%6[^,],%49[^,],%49[^,],%49[^,],%49[^\n]",
               &animal.id, animal.userid, animal.nume,
               animal.sanatate, animal.alimentatie, animal.activitate);

        if (strcmp(animal.userid, userid) == 0 && strcmp(animal.nume, nume_animal) == 0) {
            fclose(f);
            pthread_mutex_unlock(&mutex_animale);
            return 1; // Numele animalului exista deja
        }
    }

    fclose(f);
    pthread_mutex_unlock(&mutex_animale);
    return 0; // Numele animalului nu exista
}

void cripteaza_parola(const char *parola, char *parola_criptata) {
    int lungime = strlen(parola);

    for (int i = 0; i < lungime; i++) {
        // Deplasam fiecare caracter cu 1 si il convertim in hex
        sprintf(parola_criptata + i * 2, "%02X", parola[i] + 1);
    }

    // Adaugam terminatorul '\0' la finalul stringului criptat
    parola_criptata[lungime * 2] = '\0';
}