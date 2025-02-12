#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

void to_lowercase(char *input) {
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] >= 'A' && input[i] <= 'Z') {
            input[i] = input[i] + 32;
        }
    }
    input[strlen(input)-1] = '\0';
}

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  char msg[500];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
    
  //int game_status = 0; // de aici vom verifica in ce zona a clientului ne aflam
  
  char app_option[] = "00";
  
  printf("\nBun venit in aplicatia PetCareAssistant!\nCe comanda doriti sa efectuati?\n1. Logare\n2. Inregistrare\n3. Schimbare parola\n4. Exit\n");
  
  
  
  
  
  
  
  while (1) {
      /* citirea de la tastatura */
      bzero (msg, 500);
      //printf("app_option = %s\n", app_option);
      /*
      if (!game_status)
        {
          printf ("[client]Introduceti un nume: ");
          //fflush (stdout);
          //read (0, msg, 500);
          game_status = 1;
        }
      */
      //printf ("[client]Introduceti un nume: ");
      fflush (stdout);
      read (0, msg, 500);
      
      //printf("[client] Am citit %s\n", msg);
      
      char lower_msg[500];
      strcpy(lower_msg, msg);
      to_lowercase(lower_msg);
      
      
      
      
      
      if(!strcmp(app_option, "00")) // schitam pseudocod momentan
        {
            if(atoi(msg) == 1 || !strcmp(lower_msg, "logare")){
                msg[strlen(msg)-1] = '0';
                msg[strlen(msg)] = '1';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 2 || !strcmp(lower_msg, "inregistrare")){
                msg[strlen(msg)-1] = '0';
                msg[strlen(msg)] = '2';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 3 || !strcmp(lower_msg, "schimbare parola")){
                msg[strlen(msg)-1] = '0';
                msg[strlen(msg)] = '3';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 9 || !strcmp(lower_msg, "exit")){
                msg[strlen(msg)-1] = '0';
                msg[strlen(msg)] = '9';
                msg[strlen(msg)+1] = '\0';
            }
            else{
                msg[strlen(msg)-1] = '0';
                msg[strlen(msg)] = '0';
                msg[strlen(msg)+1] = '\0';
            }
        }
        
      else if(!strcmp(app_option, "10"))
        {
            if(atoi(msg) == 1 || !strcmp(lower_msg, "listare")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '1';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 2 || !strcmp(lower_msg, "detalii")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '2';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 3 || !strcmp(lower_msg, "adauga")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '3';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 4 || !strcmp(lower_msg, "modificare")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '4';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 5 || !strcmp(lower_msg, "stergere")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '5';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 8 || !strcmp(lower_msg, "log-out") || !strcmp(lower_msg, "logout")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '8';
                msg[strlen(msg)+1] = '\0';
            }
            else if(atoi(msg) == 9 || !strcmp(lower_msg, "exit")){
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '9';
                msg[strlen(msg)+1] = '\0';
            }
            else{
                msg[strlen(msg)-1] = '1';
                msg[strlen(msg)] = '0';
                msg[strlen(msg)+1] = '\0';
            }
        }
        
      else
      {
          msg[strlen(msg)-1] = app_option[0];
          msg[strlen(msg)] = app_option[1];
          msg[strlen(msg)+1] = '\0';
      }
      
      
      //printf("[client] Mesajul trimis este: %s\n", msg);
        
      /* trimiterea mesajului la server */
      if (write (sd, msg, 500) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        
      bzero (msg, 500);

      /* citirea raspunsului dat de server 
         (apel blocant pina cind serverul raspunde) */
      if (read (sd, msg, 500) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
      /* afisam mesajul primit */
      //printf("Lungimea mesajului primit este: %d\n", strlen(msg));
      //printf ("[client]Mesajul primit este: \n%s\n", msg);
      //printf("\n%s\n", msg);
      //printf("Ultimul caracter: %c\n\n", msg[strlen(msg)-1]);
      
      //strncpy(app_option, &msg[lungime_msg-2]);
      strncpy(app_option, &msg[strlen(msg)-2], 2);
      app_option[2] = '\0';
      msg[strlen(msg)-2] = '\0';
      
      
      //printf ("[client]Mesajul primit fara token este: \n%s\n", msg);
      printf("\n%s\n", msg);
      
      if(!strcmp(app_option, "29"))                          
        {
          printf("Am dat exit din client\n");
          break;
        }
      
  }
    
  /* inchidem conexiunea, am terminat */
  close (sd);
}
