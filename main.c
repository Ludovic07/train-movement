
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define FILEPATH "/tmp/afunix_text" // path per comunicazione tra server e client
FILE *srvlogfp;	// server log fp
#define REQ_ROUTE_INFO 0x01
#define RES_ROUTE_INFO 0x02
pthread_mutex_t mutex;

typedef struct
{
	int snum;
	char traininfo[12][8];
} StRouteInfo; //salva le informazioni relative ad ogni treno

typedef struct
{
	int cmd;
	int seq;
} StReqRouteInfo; // ottiene il percorso attuale del treno

typedef struct
{
	int cmd;
	int seq;
	char MA[8];
	char preMA[8];
} StReqIsRouteUse; // ottiene il segmento MA

typedef struct
{
	int cmd;
	int res;
} StResIsRouteUse; // struct di ritorno dal server che tiene traccia dello stato di ogni segmento MA

typedef struct
{
	int cmd;
	StRouteInfo routeinfo;
} StResRoute;

char *param; //parametro passato in input al main
int fd1[2], fd2[2], fd3[2], fd4[2], fd5[2];
int fd[2];
int srvfd[2];
StRouteInfo tmproute[5];
char isuse[17] = {0};

int wake_sem1; // Synchronization semaphore
int wake_sem2; // Synchronization semaphore
int wake_sem3; // Synchronization semaphore


#define WAKE_KEY 100
#define WAKE_KEY1 101
#define WAKE_KEY2 102
#define WAKE_KEY3 103

typedef struct FileInfo
{
	char name[8];
} StFileInfo;

struct shareMemory
{
	int have_write;
	StFileInfo fileinfo[16];
};

#define shm_KEY1 1001

int shm_id; //identificatore memoria condivisa
void *shm = NULL; //primo indirizzo della memoria condivisa
struct shareMemory *shared;

StRouteInfo route[5];

int P(int sid) // Operazione P: decremento
{
	struct sembuf sb = {0, -1, 0};
	return semop(sid, &sb, 1);
} // Return 0 se ha successo, return -1 se fallisce

int V(int sid) // Operazione V: incremento
{
	struct sembuf sb = {0, 1, 0};
	return semop(sid, &sb, 1);
} // Return 0 se ha successo, return -1 se fallisce

void Shmat()
{
	shm = shmat(shm_id, 0, 0); // connette l'identificatore della memoria condivisa al primo indirizzo di memoria
	if (shm == (void *)-1)
	{
		perror("Fail to shmat");
		exit(EXIT_SUCCESS);
	}

	shared = (struct shareMemory *)shm; //puntatore alla memoria condivisa
}

// Inizializzazione mappa 1
void InitMap1()
{

	route[0].snum = 6;
	snprintf(route[0].traininfo[0],
			 sizeof(route[0].traininfo[0]), "%s", "S1");
	snprintf(route[0].traininfo[1],
			 sizeof(route[0].traininfo[1]), "%s", "MA1");
	snprintf(route[0].traininfo[2],
			 sizeof(route[0].traininfo[2]), "%s", "MA2");
	snprintf(route[0].traininfo[3],
			 sizeof(route[0].traininfo[3]), "%s", "MA3");
	snprintf(route[0].traininfo[4],
			 sizeof(route[0].traininfo[4]), "%s", "MA8");
	snprintf(route[0].traininfo[5],
			 sizeof(route[0].traininfo[5]), "%s", "S6");

	route[1].snum = 7;
	snprintf(route[1].traininfo[0],
			 sizeof(route[1].traininfo[0]), "%s", "S2");
	snprintf(route[1].traininfo[1],
			 sizeof(route[1].traininfo[1]), "%s", "MA5");
	snprintf(route[1].traininfo[2],
			 sizeof(route[1].traininfo[2]), "%s", "MA6");
	snprintf(route[1].traininfo[3],
			 sizeof(route[1].traininfo[3]), "%s", "MA7");
	snprintf(route[1].traininfo[4],
			 sizeof(route[1].traininfo[4]), "%s", "MA3");
	snprintf(route[1].traininfo[5],
			 sizeof(route[1].traininfo[5]), "%s", "MA8");
	snprintf(route[1].traininfo[6],
			 sizeof(route[1].traininfo[6]), "%s", "S6");

	route[2].snum = 7;
	snprintf(route[2].traininfo[0],
			 sizeof(route[2].traininfo[0]), "%s", "S7");
	snprintf(route[2].traininfo[1],
			 sizeof(route[2].traininfo[1]), "%s", "MA13");
	snprintf(route[2].traininfo[2],
			 sizeof(route[2].traininfo[2]), "%s", "MA12");
	snprintf(route[2].traininfo[3],
			 sizeof(route[2].traininfo[3]), "%s", "MA11");
	snprintf(route[2].traininfo[4],
			 sizeof(route[2].traininfo[4]), "%s", "MA10");
	snprintf(route[2].traininfo[5],
			 sizeof(route[2].traininfo[5]), "%s", "MA9");
	snprintf(route[2].traininfo[6],
			 sizeof(route[2].traininfo[6]), "%s", "S3");

	route[3].snum = 6;
	snprintf(route[3].traininfo[0],
			 sizeof(route[3].traininfo[0]), "%s", "S4");
	snprintf(route[3].traininfo[1],
			 sizeof(route[3].traininfo[1]), "%s", "MA14");
	snprintf(route[3].traininfo[2],
			 sizeof(route[3].traininfo[2]), "%s", "MA15");
	snprintf(route[3].traininfo[3],
			 sizeof(route[3].traininfo[3]), "%s", "MA16");
	snprintf(route[3].traininfo[4],
			 sizeof(route[3].traininfo[4]), "%s", "MA12");
	snprintf(route[3].traininfo[5],
			 sizeof(route[3].traininfo[5]), "%s", "S8");
}
//  Inizializzazione mappa 2

void InitMap2()
{

	route[0].snum = 7;
	snprintf(route[0].traininfo[0],
			 sizeof(route[0].traininfo[0]), "%s", "S2");
	snprintf(route[0].traininfo[1],
			 sizeof(route[0].traininfo[1]), "%s", "MA5");
	snprintf(route[0].traininfo[2],
			 sizeof(route[0].traininfo[2]), "%s", "MA6");
	snprintf(route[0].traininfo[3],
			 sizeof(route[0].traininfo[3]), "%s", "MA7");
	snprintf(route[0].traininfo[4],
			 sizeof(route[0].traininfo[4]), "%s", "MA3");
	snprintf(route[0].traininfo[5],
			 sizeof(route[0].traininfo[5]), "%s", "MA8");
	snprintf(route[0].traininfo[6],
			 sizeof(route[0].traininfo[6]), "%s", "S6");

	route[1].snum = 6;
	snprintf(route[1].traininfo[0],
			 sizeof(route[1].traininfo[0]), "%s", "S3");
	snprintf(route[1].traininfo[1],
			 sizeof(route[1].traininfo[1]), "%s", "MA9");
	snprintf(route[1].traininfo[2],
			 sizeof(route[1].traininfo[2]), "%s", "MA10");
	snprintf(route[1].traininfo[3],
			 sizeof(route[1].traininfo[3]), "%s", "MA11");
	snprintf(route[1].traininfo[4],
			 sizeof(route[1].traininfo[4]), "%s", "MA12");
	snprintf(route[1].traininfo[5],
			 sizeof(route[1].traininfo[5]), "%s", "S8");

	route[2].snum = 6;
	snprintf(route[2].traininfo[0],
			 sizeof(route[2].traininfo[0]), "%s", "S4");
	snprintf(route[2].traininfo[1],
			 sizeof(route[2].traininfo[1]), "%s", "MA14");
	snprintf(route[2].traininfo[2],
			 sizeof(route[2].traininfo[2]), "%s", "MA15");
	snprintf(route[2].traininfo[3],
			 sizeof(route[2].traininfo[3]), "%s", "MA16");
	snprintf(route[2].traininfo[4],
			 sizeof(route[2].traininfo[4]), "%s", "MA12");
	snprintf(route[2].traininfo[5],
			 sizeof(route[2].traininfo[5]), "%s", "S8");

	route[3].snum = 6;
	snprintf(route[3].traininfo[0],
			 sizeof(route[3].traininfo[0]), "%s", "S6");
	snprintf(route[3].traininfo[1],
			 sizeof(route[3].traininfo[1]), "%s", "MA8");
	snprintf(route[3].traininfo[2],
			 sizeof(route[3].traininfo[2]), "%s", "MA3");
	snprintf(route[3].traininfo[3],
			 sizeof(route[3].traininfo[3]), "%s", "MA2");
	snprintf(route[3].traininfo[4],
			 sizeof(route[3].traininfo[4]), "%s", "MA1");
	snprintf(route[3].traininfo[5],
			 sizeof(route[3].traininfo[5]), "%s", "S1");

	route[4].snum = 6;
	snprintf(route[4].traininfo[0],
			 sizeof(route[4].traininfo[0]), "%s", "S5");
	snprintf(route[4].traininfo[1],
			 sizeof(route[4].traininfo[1]), "%s", "MA4");
	snprintf(route[4].traininfo[2],
			 sizeof(route[4].traininfo[2]), "%s", "MA3");
	snprintf(route[4].traininfo[3],
			 sizeof(route[4].traininfo[3]), "%s", "MA2");
	snprintf(route[4].traininfo[4],
			 sizeof(route[4].traininfo[4]), "%s", "MA1");
	snprintf(route[4].traininfo[5],
			 sizeof(route[4].traininfo[5]), "%s", "S1");
}

void ProcTrainInfo(int seq)
{
	Shmat();
	// printf("train %d start\n", seq);
	if (0 == strcmp(param, "MAPPA1") && 5 == seq)
	{ //il treno5 esce subito dato che in MAPPA1 non ha nessun itinerario da percorrere
		printf("Train %d  is leaving the process\n", seq);
		return;
	}

	char name[12];
	snprintf(name, sizeof(name), "train%d", seq);
	StRouteInfo info;

	while (1)
	{
		P(wake_sem1);
		if (shared->have_write == 0)
		{
			shared->have_write = 1;

			write(fd[1], name, strlen(name)); //scrittura nome del treno in fd[1]
			V(wake_sem1);
			V(wake_sem2);
			P(wake_sem3);

			memset(&info, 0, sizeof(info));
			// il treno legge la propria mappa
			if (1 == seq)
				read(fd1[0], &info, sizeof(info));
			else if (2 == seq)
				read(fd2[0], &info, sizeof(info));
			else if (3 == seq)
				read(fd3[0], &info, sizeof(info));
			else if (4 == seq)
				read(fd4[0], &info, sizeof(info));
			else if (5 == seq)
				read(fd5[0], &info, sizeof(info));

			break;
		}

		V(wake_sem1);
		sleep(1);
	}

	time_t t;

	snprintf(name, sizeof(name), "T%d.log", seq);
	unlink(name);
	FILE *logfp = fopen(name, "a");
	char line[64];
	int idx = 0; //indice
	int previdx = 0; //indice precedente
	int curidx = 0; //indice corrente
	while (1)
	{
		previdx = 0;
		curidx = 0;

		if (0 == strncmp(info.traininfo[idx], "MA", 2))
		{
			previdx = idx;
		}

		if (0 == strncmp(info.traininfo[idx + 1], "MA", 2))
		{
			curidx = idx + 1;
		}

		if (idx + 1 == (info.snum - 1))
		{
			if (0 != previdx)
			{
				FILE *prevfp = fopen(info.traininfo[previdx], "r+");
				fseek(prevfp, 0, SEEK_SET);
				fwrite("0", 1, 1, prevfp); // scrittura di 0 nel file MA precedente al segmento corrente
				fclose(prevfp);
			}

			time(&t);
			snprintf(line, sizeof(line), "[ACTUAL SEGMENT: %s] , [NEXT: %s], %s\n",
					 info.traininfo[previdx], info.traininfo[idx + 1], ctime(&t));
			fprintf(logfp, "%s", line); // scrittura dei log nei file corrispettivi
			fclose(logfp);
			printf("Train %d  is leaving the process\n", seq);
			return;
		}

		if (curidx == 0)
		{
			sleep(2);
			continue;
		}

		char filecontent[12] = {0};
		FILE *curfp = fopen(info.traininfo[curidx], "r+");
		fread(filecontent, 1, sizeof(filecontent), curfp);
		// ritorno dello stato di MA
		if (0 == strcmp(filecontent, "0"))
		{
			time(&t);
		run:
			snprintf(line, sizeof(line), "[ACTUAL SEGMENT: %s] , [NEXT: %s], %s\n",
					 info.traininfo[previdx], info.traininfo[curidx], ctime(&t));
			fprintf(logfp, "%s", line);

			fseek(curfp, 0, SEEK_SET);
			fwrite("1", 1, 1, curfp); //scrittura di 1 per indicare che il binario è occupato
			fclose(curfp);
			if (0 != previdx)
			{
				FILE *prevfp = fopen(info.traininfo[previdx], "r+");
				fseek(prevfp, 0, SEEK_SET);
				fwrite("0", 1, 1, prevfp); // scrittura di 0 nel file MA relativo al segmento precedente
				fclose(prevfp);
			}

			idx++;
		}
		else
		{
			while (1)
			{
				memset(filecontent, 0, sizeof(filecontent));
				fseek(curfp, 0, SEEK_SET);
				fread(filecontent, 1, sizeof(filecontent), curfp);

				if (0 == strcmp(filecontent, "0")) //controllo se il segmento è libero
				{
					goto run;
				}
				sleep(2);
			}
		}

		sleep(2);
	}

	fclose(logfp);
}

int ProcSockTrainInfo(int seq)
{
	Shmat();
	if (0 == strcmp(param, "MAPPA1") && 5 == seq)
	{
		printf("Train %d  is leaving the process\n", seq);
		return 0;
	}

	char name[12];
	snprintf(name, sizeof(name), "train%d", seq);

	int sock;
	struct sockaddr_un s_un;

	char buf[128];

	sock = socket(AF_UNIX, SOCK_STREAM, 0); //creazione socket
	if (sock < 0)
	{
		perror("socket");
		return 1;
	}

	s_un.sun_family = AF_UNIX;
	strcpy(s_un.sun_path, "/tmp/afunix_text");
	// connessione al server
	if (connect(sock, (struct sockaddr *)&s_un, sizeof(s_un)) != 0)
	{
		perror("connect");
		return 1;
	}

	memset(buf, 0, sizeof(buf));
	StReqRouteInfo reqinfo;
	reqinfo.cmd = htonl(REQ_ROUTE_INFO);
	reqinfo.seq = htonl(seq);
	write(sock, &reqinfo, sizeof(reqinfo));
	StRouteInfo info;
	read(sock, &info, sizeof(info));

	time_t t;

	snprintf(name, sizeof(name), "T%d.log", seq);
	unlink(name);
	FILE *logfp = fopen(name, "a");
	char line[64];
	int idx = 0;
	int previdx = 0;
	int curidx = 0;

	StResIsRouteUse res;
	StReqIsRouteUse isrouteuse;
	isrouteuse.cmd = htonl(2);
	isrouteuse.seq = htonl(seq);
	while (1)
	{
		previdx = 0;
		curidx = 0;

		if (0 == strncmp(info.traininfo[idx], "MA", 2))
		{
			previdx = idx;
		}

		if (0 == strncmp(info.traininfo[idx + 1], "MA", 2))
		{
			curidx = idx + 1;
		}

		if (idx + 1 == (info.snum - 1))
		{
			if (0 != previdx)
			{
				snprintf(isrouteuse.MA, sizeof(isrouteuse.MA),
						 "end");
				snprintf(isrouteuse.preMA, sizeof(isrouteuse.preMA),
						 "%s", info.traininfo[idx]);
				write(sock, &isrouteuse, sizeof(isrouteuse));
				FILE *prevfp = fopen(info.traininfo[previdx], "r+");
				fseek(prevfp, 0, SEEK_SET);
				fwrite("0", 1, 1, prevfp);
				close(sock);
				fclose(prevfp);
			}

			time(&t);

			// scrittura dei log
			snprintf(line, sizeof(line), "[ACTUAL SEGMENT: %s] , [NEXT: %s], %s\n",
					 info.traininfo[previdx], info.traininfo[idx + 1], ctime(&t));
			fprintf(logfp, "%s", line);
			fclose(logfp);
			printf("Train %d  is leaving the process\n", seq);
			return 0;
		}

		if (curidx == 0)
		{
			sleep(2);
			continue;
		}

		char filecontent[12] = {0};
		FILE *curfp = fopen(info.traininfo[curidx], "r+");
		fread(filecontent, 1, sizeof(filecontent), curfp);

		snprintf(isrouteuse.MA, sizeof(isrouteuse.MA),
				 "%s", info.traininfo[idx + 1]);
		snprintf(isrouteuse.preMA, sizeof(isrouteuse.preMA),
				 "%s", info.traininfo[idx]);
		write(sock, &isrouteuse, sizeof(isrouteuse));
		read(sock, &res, sizeof(res));
		int status = ntohl(res.res);
		if (status == 0)
		{
			time(&t);

			printf("Train %d received the authorization from RBC\n", seq);
		run:
			snprintf(line, sizeof(line), "[ACTUAL SEGMENT: %s] , [NEXT: %s], %s\n",
					 info.traininfo[previdx], info.traininfo[curidx], ctime(&t));
			fprintf(logfp, "%s", line);

			fseek(curfp, 0, SEEK_SET);
			fwrite("1", 1, 1, curfp);
			fclose(curfp);
			if (0 != previdx)
			{ //settato a 0 il file MA relativo al segmento di binario appena attraversato
				FILE *prevfp = fopen(info.traininfo[previdx], "r+");
				fseek(prevfp, 0, SEEK_SET);
				fwrite("0", 1, 1, prevfp);
				fclose(prevfp);
			}

			idx++;
		}
		else
		{
			while (1)
			{
				memset(filecontent, 0, sizeof(filecontent));
				fseek(curfp, 0, SEEK_SET);
				fread(filecontent, 1, sizeof(filecontent), curfp);

				write(sock, &isrouteuse, sizeof(isrouteuse));

				read(sock, &res, sizeof(res));
				int status = ntohl(res.res);
				if (status == 0)
				{
					goto run;
				}
				sleep(2);
			}
		}

		sleep(2);
	}

	fclose(logfp);
	return 0;
}

int ProcTrainFinishInfo()
{
	int sock;
	struct sockaddr_un s_un;

	char buf[128];

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		return 1;
	}

	s_un.sun_family = AF_UNIX;
	strcpy(s_un.sun_path, "/tmp/afunix_text");
	// connessione al server
	if (connect(sock, (struct sockaddr *)&s_un, sizeof(s_un)) != 0)
	{
		perror("connect");
		return 1;
	}

	memset(buf, 0, sizeof(buf));
	StReqRouteInfo reqinfo;
	reqinfo.cmd = htonl(3);

	write(sock, &reqinfo, sizeof(reqinfo));
	close(sock);
	return 0;
}

void ProcRegisterInfo()
{
	Shmat();
	if (0 == strcmp(param, "MAPPA1")) //in base all'input che viene passato al main viene Inizializzata una delle due mappe
		InitMap1();
	else
		InitMap2();
	while (1)
	{
		P(wake_sem2);
		P(wake_sem1);

		char read_buf1[100] = {0};
		if (shared->have_write == 1)
		{
			read(fd[0], read_buf1, 100);
			shared->have_write = 0;
			if (0 == strcmp(read_buf1, "train1"))
			{//in base al treno che farà richiesta andrà a scrivere nel lato di scrittura del fd il corrispettivo itinerario
				write(fd1[1], &route[0], sizeof(route[0]));
				V(wake_sem3);
			}
			else if (0 == strcmp(read_buf1, "train2"))
			{
				write(fd2[1], &route[1], sizeof(route[1]));
				V(wake_sem3);
			}
			else if (0 == strcmp(read_buf1, "train3"))
			{
				write(fd3[1], &route[2], sizeof(route[2]));
				V(wake_sem3);
			}
			else if (0 == strcmp(read_buf1, "train4"))
			{
				write(fd4[1], &route[3], sizeof(route[3]));
				V(wake_sem3);
			}
			else if (0 == strcmp(read_buf1, "train5"))
			{
				write(fd5[1], &route[4], sizeof(route[4]));
				V(wake_sem3);
			}
			else if (0 == strcmp(read_buf1, "srv"))
			{
				write(srvfd[1], route, sizeof(route));
				V(wake_sem3);
			}
		}
		else if (-1 == shared->have_write) //tutti i treni sono arrivati a destinazione
		{
			V(wake_sem1);
			return;
		}

		V(wake_sem1);
	}
}

void delete () //chiusi tutti i lati di scrittura e di lettura dei fd
{
	close(fd2[0]);
	close(fd2[1]);

	close(fd1[0]);
	close(fd1[1]);

	close(fd[0]);
	close(fd[1]);

	close(fd3[0]);
	close(fd3[1]);

	close(fd4[0]);
	close(fd4[1]);

	close(fd5[0]);
	close(fd5[1]);

	close(srvfd[0]);
	close(srvfd[1]);

	semctl(wake_sem1, 0, IPC_RMID);
	semctl(wake_sem2, 0, IPC_RMID);
	semctl(wake_sem3, 0, IPC_RMID);

	shmdt(shm);	// scollegamento del segmento di memoria condivisa del processo invocante dallo spazio degli indirizzi di memoria condivisa
	shmctl(shm_id, IPC_RMID, 0); // rimozione della memoria condivisa dal relativo identificatore
}

void handler(int sig) //metodo di cattura del segnale SIGINT
{
	printf("SIGINT is capture sig=%d\n", sig);

	delete ();
	exit(0);
}

void *procclimsg(void *pParam)
{
	pthread_detach(pthread_self());
	int sock = *(int *)pParam;
	char buf[32];
	time_t t;
	while (1)
	{
		int ret = read(sock, buf, sizeof(buf)); // riceve messaggio del client
		if (ret <= 0)
		{
			close(sock);
			free(pParam);
			return NULL;
		}

		char line[256];

		int cmd = ntohl(*(int *)buf);
		// command id = 1
		if (1 == cmd)
		{ // invio dei dati al treno
			StReqRouteInfo *info = (StReqRouteInfo *)buf;
			int seq = ntohl(info->seq);

			write(sock, &tmproute[seq - 1], sizeof(tmproute[seq - 1]));
		}
		else if (2 == cmd)
		{

			StReqIsRouteUse *info = (StReqIsRouteUse *)buf;
			int use = 1;//segmento occupato
			char buf[8];


			if (0 == strcmp(info->MA, "end"))
			{

				for (int i = 1; i < 17; i++)
				{
					snprintf(buf, sizeof(buf), "MA%d", i);
					if (0 == strcmp(buf, info->preMA))
					{
						pthread_mutex_lock(&mutex);
						isuse[i] = 0;
						pthread_mutex_unlock(&mutex);
						break;
					}
				}

				continue;
			}
			// sblocca l'altro treno in attesa
			for (int i = 1; i < 17; i++)
			{
				snprintf(buf, sizeof(buf), "MA%d", i);
				pthread_mutex_lock(&mutex);
				if (0 == strcmp(buf, info->MA))
				{
					if (0 == isuse[i])
					{
						use = 0; // segmento libero
						isuse[i] = 1;

						for (int i = 1; i < 17; i++)
						{
							snprintf(buf, sizeof(buf), "MA%d", i);
							if (0 == strcmp(buf, info->preMA))
							{
								isuse[i] = 0;
								break;
							}
						}

						pthread_mutex_unlock(&mutex);
						break;
					}
				}
				pthread_mutex_unlock(&mutex);
			}

			StResIsRouteUse res;
			res.res = htonl(use);
			time(&t);

			snprintf(line, sizeof(line), "[TRAIN REQUESTING AUTHORIZATION: T%d] , [ACTUAL SEGMENT: %s],"
										 " [SEGMENT REQUESTED: %s], [AUTHORIZED: NO], [%s]\n",
					 ntohl(info->seq), info->preMA, info->MA, ctime(&t));
			if (0 == use)
			{
				snprintf(line, sizeof(line), "[TRAIN REQUESTING AUTHORIZATION: T%d] , [ACTUAL SEGMENT: %s],"
											 " [SEGMENT REQUESTED: %s], [AUTHORIZED: YES], [%s]\n",
						 ntohl(info->seq), info->preMA, info->MA, ctime(&t));
			}

			fwrite(line, 1, strlen(line), srvlogfp);
			fflush(srvlogfp);
			write(sock, &res, sizeof(res));
		}
		else if (3 == cmd)
		{
			printf("All the trains arrived to their own stations\n");
		}
	}

	close(sock);
	free(pParam);
	return NULL;
}

int srv() // server
{

	pid_t p2;
	p2 = fork();
	if (0 == p2)
	{
		ProcRegisterInfo();
		exit(1);
	}

	shared->have_write = 1;

	write(fd[1], "srv", 3); // scrittura sul fd
	V(wake_sem2);
	P(wake_sem3);

	read(srvfd[0], tmproute, sizeof(tmproute));

	int s0, sock;
	struct sockaddr_un s_un;
	struct sockaddr_un s_un_accept;
	socklen_t addrlen;

	s0 = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s0 < 0)
	{
		perror("socket");
		return 1;
	}

	s_un.sun_family = AF_UNIX;
	strcpy(s_un.sun_path, FILEPATH);

	if (bind(s0, (struct sockaddr *)&s_un, sizeof(s_un)) != 0)
	{
		perror("bind");
		return 1;
	}

	if (listen(s0, 5) != 0)
	{
		perror("listen");
		return 1;
	}

	char name[12];
	snprintf(name, sizeof(name), "RBC.log");
	unlink(name);
	srvlogfp = fopen(name, "a");

	while (1)
	{
		addrlen = sizeof(s_un_accept);
		sock = accept(s0, (struct sockaddr *)&s_un_accept, &addrlen);
		if (sock < 0)
		{
			perror("accept");
			return 1;
		}

		pthread_t thread_id = 0;
		int *psock = malloc(sizeof(int));
		*psock = sock;
		pthread_create(&thread_id, NULL, procclimsg, psock); //creazione thread
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("param error\n");
		return -1;
	}
	wake_sem1 = semget(WAKE_KEY, 1, IPC_CREAT | 0660);	// creazione del semaforo wake_sem
	semctl(wake_sem1, 0, SETVAL, 1);					// il valore del semaforo wake_sem value inizializzato a 1
	wake_sem2 = semget(WAKE_KEY1, 1, IPC_CREAT | 0660); // creazione del semaforo wake_sem
	semctl(wake_sem2, 0, SETVAL, 0);					//  il valore del semaforo wake_sem value inizializzato a 0

	wake_sem3 = semget(WAKE_KEY2, 1, IPC_CREAT | 0660); // creazione del semaforo wake_sem
	semctl(wake_sem3, 0, SETVAL, 0);					//  il valore del semaforo wake_sem value inizializzato a 0

	if ((shm_id = shmget(shm_KEY1, 1024, 0666 | IPC_CREAT)) == -1) // creazione memoria condivisa
	{
		perror("Fail to shmget");
		exit(EXIT_SUCCESS);
	}

	Shmat();
	pipe(fd);
	pipe(fd1);
	pipe(fd2);
	pipe(fd3);
	pipe(fd4);
	pipe(fd5);
	pipe(srvfd);

	shared->have_write = 0;

	char cmd[24];
	for (int i = 0; i < 16; i++)
	{
		snprintf(shared->fileinfo[i].name,
				 sizeof(shared->fileinfo[i].name), "MA%d", i + 1);

		unlink(shared->fileinfo[i].name);
		FILE *fp = fopen(shared->fileinfo[i].name, "w");

		if (NULL == fp)
		{
			printf("fopen error\n");
		}
		else
		{
			fprintf(fp, "0");
		}

		snprintf(cmd, sizeof(cmd), "chmod 666 %s",
				 shared->fileinfo[i].name);
		system(cmd);
		fclose(fp);
	}

	if ((0 == strcmp(argv[1], "ETCS1")) && ((0 == strcmp(argv[2], "MAPPA1")) || (0 == strcmp(argv[2], "MAPPA2"))))
	{
		param = argv[2];
		pid_t p1, p2;
		signal(SIGINT, handler);

		int i;
		for (i = 0; i < 5; i++)
		{
			// creazione dei processi
			p1 = fork();
			if (0 == p1)
			{
				ProcTrainInfo(i + 1);
				exit(1);
			}
		}

		p2 = fork();
		if (0 == p2)
		{
			ProcRegisterInfo();
			exit(1);
		}

		for (i = 0; i < 5; i++)
		{
			wait(0);
		}

		shared->have_write = -1;
		V(wake_sem2);
		write(fd[1], "1", 1);

		wait(0);
		delete ();
	}
	else if (0 == strcmp(argv[1], "ETCS2"))
	{
		if (0 == strcmp(argv[2], "RBC"))
		{
			pthread_mutex_init(&mutex, NULL);
			signal(SIGINT, handler);
			unlink(FILEPATH);
			param = argv[3];
			srv();

			for (int i = 0; i < 1; i++)
			{
				wait(0);
			}

			delete ();
		}
		else
		{
			param = argv[2];
			int i;
			for (i = 0; i < 5; i++)
			{
				pid_t p1 = fork();
				// creazione dei processi
				if (0 == p1)
				{
					ProcSockTrainInfo(i + 1);
					exit(1);
				}
			}

			for (i = 0; i < 5; i++)
			{
				wait(0);
			}

			ProcTrainFinishInfo();
		}
	}
	else
	{
		printf("param error\n");
		return -1;
	}
	return 0;
}
