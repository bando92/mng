#include "stdafx.h"            
#include <windows.h>      
#include <list>     
#include <time.h>    
#include <iostream>     
#include <stdlib.h>            
#include <stdio.h>  
#include <sstream>
#include <string.h>
#include <math.h>            
#include <gl/glut.h>            
#include <gl/gl.h>            
#include <gl/glu.h>            
#include <gl/glui.h>       
#include <Windows.h>      
using namespace std;     
//DEFINE     
//relative alla finestra e al mondo del gioco    
#define WINW 650        //larghezza della finestra    
#define WINH 650       //altezza della finestra    
#define HSUOLO WINH/80 //altezza dalla quale parto a disegnare la mia navicella    
#define HEADERHEIGHT WINH/8 //100 //altezza dell'header dove stanno le vite e il tempo di gioco    
        
//relative alla navetta    
#define BATTLESHIPW WINW/16 //larghezza della navetta    
#define SSHEIGHT WINH/20//altezza della navetta a partire dal suolo    
#define DEADLINE SSHEIGHT+5//soglia sotto la quale se scendono le navette hai perso     
#define SHOTHEIGHT WINH/80 //lunghezza del colpo sparato    
        
//relative agli alieni    
#define NAVWIDTH WINW/20    //40--larghezza navi aliene    
#define NAVDIS WINH/10 //70    /distanza tra una nave e l'altra    
//#define VNAVETTA 0.02     //velocità delle navette aliene    
#define ROWS 5     //righe di navi aliene    
#define NUMNAV 45    //numero di navi aliene    
#define DISCESA NAVDIS/2 //di quanto scendono le navi ogni volta che sbattono    
#define ALIENHEIGHT WINH/27    
float VNAVETTA=0.02;    
/////////////////////////////////////////////////     
        
//DICHIARAZIONE VARIABILI GLOBALI    
        
typedef struct
{         
    GLfloat x,y;   //posizione del punto in basso a sinistra della nave aliena      
    bool alive;    //variabile che mi dice se l'alieno è vivo o morto    
} Navetta; //struttura che mi consente di immagazzinare variabili utili per gli alieni    
        
typedef struct
{     
    GLfloat yshot,xshot;  //posizione del vertice basso del colpo    
} Shot; //struttura che mi consente di immagazzinare informazioni riguardo ai colpi    
        
static Navetta NaviAliene[NUMNAV];//vettore di alieni      
static GLfloat traslaship=(WINW/2)-(BATTLESHIPW/2);//posizione da cui parte la nave    
static GLfloat traslaalien=WINW/16;//quanto veloce si muovono gli alieni    
static GLfloat shotspeed=0.2;//quanto veloci vanno i colpi sparati      
bool flag=true;//usato per capire in che direzione devo andare-> se è = true vadoa destra, se false vado a sinistra    
bool linealive[ROWS]; //cambia quando tutte le navicella di una riga sono morte    
bool youwin=false;//diventa true quando hai vinto...mi serve per stampare a video la scritta    
int lastrow[ROWS];//indice dell'ultima navicella viva in riga i         
int firstrow[ROWS];//indice della navicella viva in riga i         
float rimbalzo=0;//ogni volta che rimbalza viene incrementato...questo valore viene poi sottratto dall'altezza del disegno della navetta per farle scendere        
int play=0;//per la gestione della pausa     
int numshot=0;//tiene il conto di quanti colpi sparati ci sono nella lista     
int alienshot=0;//tiene il conto di quanti proiettili hanno sparato gli alieni    
int tempo=0;//tiene il conto per vedere ogni quanto gli alieni devono sparare    
int timeshot=1250;//1500//più è basso più gli alieni sparano ad intervalli più piccoli(compreso tra 2000-3000)    
int life=3;//vite della navicella spaziale    
int numdead=0;//tiene il conto di quante navi aliene sono morte    
int increase=0;//mi serve per quando devo velocizzare le navette    
int pausa=0;//variabile che mi permette di capire se sono mai entrato in pausa o no    
static GLfloat scrittaposy=WINH/2;//posizione della scritta hai vinto/perso    
static GLfloat scrittaposx=WINW/2-54;//posizione della scritta hai vinto/perso    
static GLfloat lifeposy=WINH-WINH/20;//coordinata y dove devo stampare le vite    
static GLfloat lifeposx=WINW/80;//+50;//coordinata x dove devo stampare la prima vita    
list<Shot> Colpi;//lista dei colpi sparati     
list<Shot>::iterator p,p2,alienp,alienp2;//iteratore per percorrere la lista     
list<Shot> ColpiAlieni;//lista dei colpi sparati dagli alieni    
int punteggio=0; //tiene conto del punteggio del giocatore
//////////////////////////////////////////////////     
          
//DICHIARAZIONE FUNZIONI     
void idle();     
void SpaceshipDeath(int);     
void DrawSpaceship(float);     
void TastiSpeciali(int,int,int);     
void DrawAlien(float,float);     
void display ();     
void myMouse(int,int,int,int);     
void myinit();     
void myKeyboard(unsigned char,int,int);     
void AlienShot();    
//////////////////////////////////////////////////     
        
//FUNZIONI     
void idle()//funzione che mi dice cosa fare durante l'inattività         
{         
    int i=0;        
    int max=0,min=0;     
    bool hit=false;    
    bool hitme=false;    
    //devo decidere in che direzione devo andare e dove rimbalzare        
    //cerco il massimo dei lastrow[i]...la navetta più a destra ancora viva tra le ultime di riga        
    for(i=0;i<ROWS;i++)        
    {        
        if (i==0)        
            max=i;        
        else
        {        
            if(lastrow[i]%(NUMNAV/ROWS)>=lastrow[max]%(NUMNAV/ROWS))        
                max=i;        
        }        
    }        
                
    if (NaviAliene[lastrow[max]].x+NAVWIDTH>=(WINW-WINW/80))//se la nave aliene numero lastrow[max] (che è l'ultima viva in fila) supera 790 si torna indietro        
    {        
        flag=false;        
        rimbalzo+=DISCESA;      
    }        
                
    //cerco il minimo dei firstrow[i]...la navetta più a sinistra ancora viv tra i primi di ogni riga        
                    
    for(i=0;i<ROWS;i++)        
    {        
        if (i==0)        
            min=i;        
        else
        {        
            if(firstrow[i]%(NUMNAV/ROWS)<=firstrow[min]%(NUMNAV/ROWS))        
                min=i;        
        }        
    }        
                
    if (NaviAliene[firstrow[min]].x<=(WINW/80))         
    {        
        flag=true;         
        rimbalzo+=DISCESA;        
    }        
                
    //controllo per vedere in quale direzione devo andare        
    if(flag==true)         
        traslaalien=traslaalien+VNAVETTA;//velocità navetta         
    if(flag==false)         
        traslaalien=traslaalien-VNAVETTA;      
    //////////////fine controlli per il movimento degli alieni    
        
    //movimento dei proiettili sparati da me    
    for (p = Colpi.begin(); p != Colpi.end(); p++)  //devo muovere tutti i proiettili che ho in lista    
    {        
        //lo incremento     
        (*p).yshot+=shotspeed;     
        //devo controllare se collide con una nave aliena    
        for(i=0;i<NUMNAV;i++)    
        {    
            if(NaviAliene[i].y>=(*p).yshot && NaviAliene[i].y<=((*p).yshot)+SHOTHEIGHT &&NaviAliene[i].alive==true)//se il colpo in uno qualsiasi dei suoi punti è all'altezza della nave    
            {    
                if(NaviAliene[i].x<=(*p).xshot && NaviAliene[i].x+NAVWIDTH>=(*p).xshot &&NaviAliene[i].alive==true)    
                {    
                    SpaceshipDeath(i);//la nave aliena muore    
                    numdead++;    
                    p2=p;//mi salvo literator in una variabile d'appoggio per sapere quale devo eliminare    
                    hit=true;//variabile che mi serve per capire se una qualche navetta è stata colpita    
                }    
            }    
        }    
    }    
    if(hit==true)//se una qualche navetta è stata colpita    
    {    
        p2=Colpi.erase(p2);//elimino il colpo che mi ero segnato dalla lista    
        hit=false;//resetto la variabile    
    }    
    if(numdead>20 && increase==0)    
    {    
        VNAVETTA+=0.01;    
        increase=1;    
    }    
    if(numdead>34 && increase==1)    
    {    
        VNAVETTA+=0.01;    
        increase=2;    
    }    
    if(numdead>39 && increase==2)    
    {    
        VNAVETTA+=0.02;    
        increase=3;    
    }    
    if(numdead>43 && increase==3)    
    {    
        VNAVETTA+=0.02;    
        increase=4;    
    }    
    //////////////////////////////////////////    
        
    //movimento dei proiellitli sparati dalle navi aliene    
    for (alienp = ColpiAlieni.begin(); alienp != ColpiAlieni.end(); alienp++)  //devo muovere tutti i proiettili che ho in lista    
    {        
        //lo incremento     
        (*alienp).yshot-=shotspeed;     
        if(SSHEIGHT>=((*alienp).yshot)+SHOTHEIGHT)//se hanno colpito la mia navicella    
        {    
            if((*alienp).xshot>=traslaship && (*alienp).xshot<=traslaship+BATTLESHIPW)    
            {    
                life--;    
                alienp2=alienp;    
                hitme=true;    
                punteggio=punteggio-100;//diminuisco il punteggio 
                //devo rimuovere il proiettile dalla coda    
            }    
        }    
    }    
    if(hitme==true)//se sono stato colpito 
    {    
        alienp2=ColpiAlieni.erase(alienp2);//elimino il colpo che mi ero segnato dalla lista    
        hitme=false;//resetto la variabile    
    }    
    glutPostRedisplay();         
}         
        
int RandomShot()  //funzione che mi scelglie un indoce di navetta a caso tra quelle ancora in vita  
{    
    int indice=0;    
    srand((unsigned)time(NULL));    
    do
    {    
        indice = rand() % NUMNAV;    
    }while(NaviAliene[indice].alive==false);    
    return indice;    
}    
        
void bitmap_output(int x, int y, int z, char *string, void *font)  //funzione per la stampa a video di caratteri  
{    
    int i;    
    glRasterPos3f(x,y,0);    
    int len=(int)strlen(string);    
    for(i=0;i<len;i++)    
        glutBitmapCharacter(font,string[i]);    
}    
        
void myKeyboard(unsigned char key, int x, int y) //funzione che mi permette di gestire l'evento tastiera      
{       
    switch (key)       
    {       
    case 13: //se spingi INVIO il gioco parte    
        if(youwin==false)    
        {    
            play=1;      
            glutIdleFunc(idle);           
            glutPostRedisplay();      
        }    
        break;  
    
    case 27://se premi esc esci dal gioco      
        exit(-1);   
    
    case 'd': //se premi d ti muovi a destra    
        if(play==1)//se il gioco non è in pausa mi posso muovere    
        {    
            if(traslaship+BATTLESHIPW<=WINW-5)//se non sono arrivato al marigine destro della finestra    
            {    
                traslaship=traslaship+5;  //mi muovo       
                glutPostRedisplay();      
            }   
        }    
        break;   
    
    case 'a': //se premi a ti muovi a sinistra    
        if(play==1)//se il gioco non è in pausa mi posso muovere    
        {    
            if(traslaship>=5)//se non sono arrivato al margine sinistro della finestra    
            {    
                traslaship=traslaship-5;   //mi muovo      
                glutPostRedisplay();      
            }    
        }    
        break;   
    
    case 'w'://se premi w il gioco va in pausa       
        if(play==0 && youwin==false)//se non sto giocando faccio cominciare(o riprendere) il gioco      
        {    //il controllo su youwin c'è se no quando vinco mi da dei problemi se invece di spingere esc spingo invio...    
            glutIdleFunc(idle);         
            play=1;        
        }        
        else//altrimenti lo fermo bloccando l'idlefunc    
        {        
            glutIdleFunc(NULL);      
            pausa=1;    
            glutPostRedisplay();         
            play=0;        
        }        
        break;  
    
    //per le maiuscole:  
    
    case 'D': //se premi d ti muovi a destra    
        if(play==1)//se il gioco non è in pausa mi posso muovere    
        {    
            if(traslaship+BATTLESHIPW<=WINW-5)//se non sono arrivato al marigine destro della finestra    
            {    
                traslaship=traslaship+5;  //mi muovo       
                glutPostRedisplay();      
            }    
        }    
        break;  
    
    case 'A': //se premi a ti muovi a sinistra    
        if(play==1)//se il gioco non è in pausa mi posso muovere    
        {    
            if(traslaship>=5)//se non sono arrivato al margine sinistro della finestra    
            {    
                traslaship=traslaship-5;   //mi muovo      
                glutPostRedisplay();      
            }    
        }    
        break;  
    
    case 'W'://se premi w il gioco va in pausa       
        if(play==0 && youwin==false)//se non sto giocando faccio cominciare(o riprendere) il gioco      
        {    //il controllo su youwin c'è se no quando vinco mi da dei problemi se invece di spingere esc spingo invio...    
            glutIdleFunc(idle);         
            play=1;        
        }        
        else//altrimenti lo fermo bloccando l'idlefunc    
        {        
            glutIdleFunc(NULL);      
            pausa=1;    
            glutPostRedisplay();         
            play=0;        
        }        
        break;       
    }     
}       
          
void TastiSpeciali(int key,int x, int y) //funzione che mi permette di gestire l'evento tasti speciali        
{         
    switch(key)         
    {           
        case GLUT_KEY_F4:  //funzione fasulla ke mi permette di stoppare il gioco       
            play=0;    
            glutIdleFunc(NULL);        
            glutPostRedisplay();         
    }         
}         
        
void SpaceshipDeath(int indice) //funzione da richiamare quando una navicella viene colpita...gli devo passare il suo indice    
{      
    int indicesupp=0;//indice di supporto->indica la riga nella quale sono    
    int i=0;      
    bool flagalive=false;//se è uguale a false la riga è morta    
    int winflag=0;//se rimane uguale a 0 dopo i controlli hai vinto la partita    
        
    //lo stato della navetta passa a "morta"      
    NaviAliene[indice].alive=false;   

    //aumento il punteggio 
  
    //devo impostare il valore di indicesupp...non mi veniva in mente un modo più veloce o almeno diverso per farlo :)    
    if(indice<=8)  //se è nella riga 0    
    { 
        indicesupp=0;  
        punteggio+=25; 
    } 
    if(indice<=17 && indice>=9)   //se è nella riga 1    
    { 
        indicesupp=1; 
        punteggio+=20; 
    }   
    if(indice<=26 && indice>=18)  //se è nella riga 2    
    { 
        indicesupp=2;   
        punteggio+=15; 
    }     
    if(indice<=35 && indice>=27)  //se è nella riga 3    
    { 
        indicesupp=3;  
        punteggio+=10; 
    }   
    if(indice<=44 && indice>=36)  //se è nella riga 4    
    { 
        indicesupp=4; 
        punteggio+=5; 
    } 
    if(indice==lastrow[indicesupp]) //se era l'ultimo della sua fila    
    {      
        //devo controllare qual'è l'ultima viva nella riga      
        for(i=(NUMNAV/ROWS)-2;i>=0;i--)  //li scorro dal penultimo di riga(in quanto all'inizio il lastrow è settato all'ultimo di ogni riga)per arrivare fino al primo    
        {      
            if(NaviAliene[i+(indicesupp*(NUMNAV/ROWS))].alive==true) //se la nave precedente è viva    
            {      
                lastrow[indicesupp]=i+(indicesupp*(NUMNAV/ROWS));  //lastrow passa a quella nave    
                flagalive=true;    
                break;      
            }    
        }     
        if(flagalive==false)//non ho trovato nessuno a cui sostituirlo...quindi sono tutti morti    
            {    
                linealive[indicesupp]=false;//la riga non ha più nessun vivo    
                lastrow[indicesupp]=0;//setto il lastrow al primo elemento    
            }    
    }    
        
    if(indice==firstrow[indicesupp])//se era il primo della sua fila    
    {    
        //devo controllare qual'è la prima viva nella sua fila    
        for(i=1;i<(NUMNAV/ROWS);i++)  //li scorro dal penultimo di riga(in quanto all'inizio il lastrow è settato all'ultimo di ogni riga)per arrivare fino al primo    
        {      
            if(NaviAliene[i+(indicesupp*(NUMNAV/ROWS))].alive==true) //se la nave successiva è viva    
            {      
                firstrow[indicesupp]=i+(indicesupp*(NUMNAV/ROWS));  //firstrow passa alla successiva    
                flagalive=true;    
                break;      
            }    
        }    
        if(flagalive==false)//non ho trovato nessuno a cui sostituirlo...quindi sono tutti morti    
            {    
                linealive[indicesupp]=false;//la riga non ha più nessun vivo    
                firstrow[indicesupp]=44;    
            }    
    }    
        
    // se tutte le righe sono morte hai vinto!!    
    for(i=0;i<ROWS;i++)    
    {    
        if(linealive[i]==true)//se quella riga è viva    
        {    
            winflag=1;//se questo valore non viene mai messo a 1 vuol dire che non ci sono rimaste righe vive quindi ho vinto    
        }    
    }    
    if(winflag==0)//se è rimasto uguale a 0 ho vinto    
    {    
                
        TastiSpeciali(GLUT_KEY_F4,NULL,NULL);//si ferma l'idle    
        youwin=true;    
    }    
}     
            
void DrawSpaceship(float traslaship)  //disegna la tua navetta       
{         
    glBegin(GL_TRIANGLES);     //è un triangolo    
        glVertex2f(traslaship,HSUOLO);         
        glVertex2f(traslaship+BATTLESHIPW,HSUOLO);         
        glVertex2f(traslaship+BATTLESHIPW/2,SSHEIGHT);         
    glEnd();         
}         
                  
void DrawAlien(float xbasso,float ybasso)    //disegno di una nave aliena     
{         
    glRecti(xbasso,ybasso,xbasso+NAVWIDTH,ybasso+ALIENHEIGHT);//corpo        
        
    glColor3f(0.0,0.0,0.0);  //colore occhi e bocca      
        
    glBegin(GL_QUADS);//occhi dell'alieno         
        //occhio sinistro         
        glVertex2f(xbasso+WINW/400,ybasso+WINH/32);         
        glVertex2f(xbasso+WINW/67,ybasso+WINH/32);         
        glVertex2f(xbasso+WINW/67,ybasso+WINH/40);         
        glVertex2f(xbasso+WINW/400,ybasso+WINH/40);        
        //occhio destro         
        glVertex2f(xbasso+WINW/21,ybasso+WINH/40);         
        glVertex2f(xbasso+WINW/28,ybasso+WINH/40);         
        glVertex2f(xbasso+WINW/28,ybasso+WINH/32);         
        glVertex2f(xbasso+WINW/21,ybasso+WINH/32);         
    glEnd();     
        
    glBegin(GL_LINE_STRIP);//bocca         
        glVertex2f(xbasso+WINW/114,ybasso+WINH/160);        
        glVertex2f(xbasso+WINW/24,ybasso+WINH/160);         
    glEnd();         
                  
}      
        
void AlienShot(int indice)//come parametro gli passo l'indice della nave aliena che ha sparato    
{    
    Shot n;  //creo un elemento shot    
    n.yshot=NaviAliene[indice].y;    
    n.xshot=(NaviAliene[indice].x)+(NAVWIDTH/2);     
    alienshot++; //incremento il numero dei colpi    
    ColpiAlieni.push_back(n);//metto in coda un nuovo colpo     
    glutPostRedisplay(); //faccio il refresh della finestra    
}    
                  
void display (void)   //funzione per la visualizzazione di immagini a video      
{          
    int i=0,j=0;    
    int k=-1;   //mi serve per vedere a quale navetta sono arrivato...quando viene utilizzata viene incrementata di 1,quindi le navi partono da 0      
    float ynav=(WINH-WINH/8+NAVDIS)-rimbalzo-ALIENHEIGHT;//altezza alla quale disegnare le navicelle    
    float miny=WINH;//mi serve per vedere a quale altezza è la navicella più basso  
	//converto il punteggio in stringa
	stringstream strs;
	strs << punteggio;
	string temp_str = strs.str();
	char* punteggioStringa = (char*) temp_str.c_str();
    //parte di codice che mi permette di far sparare gli alieni in maniera casuale    
    tempo=tempo+1;    
    if(tempo==timeshot && play==1)//tempo giusto ke mi permette di far sparare le navi aliene circa 1 al secondo..sparano 1 volta ogni 3000 refresh    
    {    
        i=RandomShot();    
        AlienShot(i);    
        tempo=0;    
    }    
    /////////////////////////////////////    
        
    glClearColor(0.0,0.0,0.0,0.0);  //colore dello sfondo nero       
    glClear(GL_COLOR_BUFFER_BIT);    
        
    //disegno le vite nell'header in alto a sinistra    
    glColor3f(1.0,1.0,1.0);      

    glColor3f(1.0,0.0,0.0);      
    if(life>0)    
    {    
        for(i=0;i<life;i++)    
        {    
            glBegin(GL_TRIANGLES);    
                glVertex2f(lifeposx+(i*30),lifeposy);    
                glVertex2f(lifeposx+15+(i*30),lifeposy);    
                glVertex2f(lifeposx+7.5+(i*30),lifeposy+15);    
            glEnd();    
        }    
    }    
    ////////////////////////////////////////    
        
    //disegno le navi aliene con due cicli for e faccio tutti i controlli necessari     
    for(j=0;j<ROWS;j++)    //per ogni riga     
    {         
        ynav=ynav-NAVDIS;     //abbasso la riga    
        for(i=0;i<NUMNAV/ROWS;i++)     //per ogni navetta di quella riga    
        {            
            k++;    //incremento il numero di navette    
            NaviAliene[k].x=traslaalien+NAVWIDTH*i+NAVWIDTH*i;     //mi salvo la sua cordinata x(basso a sx)nel vettore di struct NaviAliene    
            NaviAliene[k].y=ynav;    //mi salvo la sua cordinata y(basso a sx)nel vettore di struct NaviAliene    
            if(NaviAliene[k].y<=miny && NaviAliene[k].alive==true)   //se la nave aliena è viva ed è più in basso del minimo     
            {        
                miny=NaviAliene[k].y;//per vedere se sono arrivate in fondo devi controllare se questo valore è minore di una certa soglia data        
                                
                if(miny<DEADLINE)//se la navetta più in basso arriva alla deadline,hai perso      
                {        
                    glColor3f(1.0,1.0,1.0);      
                    bitmap_output(scrittaposx,scrittaposy,0,"HAI PERSO!!!",GLUT_BITMAP_9_BY_15);    
                    bitmap_output(scrittaposx-40,scrittaposy-20,0,"PREMI ESC PER USCIRE",GLUT_BITMAP_9_BY_15);    
                    TastiSpeciali(GLUT_KEY_F4,NULL,NULL);//si ferma l'idle     
                }        
                
            }        
            if(NaviAliene[k].alive==true)//se sono vive le stampo di verde e possono essere colpite siccome alive=true         
			{
				if(k<=8)  //se è nella riga 0    
				{ 
					glColor3f(0.0,0.4,0.6);
				} 
				if(k<=17 && k>=9)   //se è nella riga 1    
				{ 
					glColor3f(0.0,0.5,0.5);
				}   
				if(k<=26 && k>=18)  //se è nella riga 2    
				{ 
					glColor3f(0.0,0.6,0.4);
				}     
				if(k<=35 && k>=27)  //se è nella riga 3    
				{ 
					glColor3f(0.0,0.8,0.2);
				}   
				if(k<=44 && k>=36)  //se è nella riga 4    
				{ 
					glColor3f(0.0,1.0,0.0);
				} 
			}        
            else//altrimenti le stampo di nero ed è come se non ci fossero     
                glColor3f(0.0,0.0,0.0);        
            DrawAlien(traslaalien+NAVWIDTH*i+NAVWIDTH*i,ynav); //disegno la nave aliena       
        }         
    }        
    /////////////////////////////////////////////////     
          
    //disegno della navetta di colore bianco     
    glColor3f(1.0,1.0,1.0);         
    DrawSpaceship(traslaship);      
    ///////////////////////////////////////////     
        
    //disegno la deadline(non necessario)     
    /*glBegin(GL_LINE_STRIP);      
        glVertex2f(0.0,DEADLINE);     
        glVertex2f(WINW,DEADLINE);      
    glEnd(); */ 
    ///////////////////////////////////////////     
        
    //disegno l'headerline (non necessario)   
    /* glBegin(GL_LINE_STRIP);      
        glVertex2f(0.0,WINH-HEADERHEIGHT);     
        glVertex2f(WINW,WINH-HEADERHEIGHT);      
    glEnd();   */
    ////////////////////////////////////////    
        
    //disegno i colpi partiti iterando dentro la lista    
    glColor3f(1.0,1.0,1.0);       
    if(Colpi.empty()==false)//se la lista di colpi non è vuota    
    {    
        if((Colpi.front()).yshot>WINH-HEADERHEIGHT)//se il colpo più in alto che è quasi sicuramente il primo della lista arriva in cima lo tolgo dalla coda     
          {     
              Colpi.pop_front();     
          }     
    }    
    for (p = Colpi.begin(); p != Colpi.end(); p++)  //con un iterator scorro la lista per disegnare tutti i colpi    
    {        
        //disegno i colpi    
        glBegin(GL_LINE_STRIP);  //sono delle linee    
            glVertex2f((*p).xshot,(*p).yshot);     
            glVertex2f((*p).xshot,(*p).yshot+SHOTHEIGHT);     
        glEnd();     
    }     
          
    ///////////////////////////////////////////////     
        
    //disegno i colpi sparati dagli alieni iterando dentro la lista    
    glColor3f(1.0,0.0,0.0);      
    if(ColpiAlieni.empty()==false)//se la lista di colpi non è vuota    
    {    
        if((ColpiAlieni.front()).yshot<=0)//se il colpo più in basso che è quasi sicuramente il primo della lista arriva in fondo lo tolgo dalla coda     
          {     
              ColpiAlieni.pop_front();     
          }     
        for (alienp = ColpiAlieni.begin(); alienp != ColpiAlieni.end(); alienp++)  //con un iterator scorro la lista per disegnare tutti i colpi    
        {        
            //disegno i colpi    
            glBegin(GL_LINE_STRIP);  //sono delle linee    
                glVertex2f((*alienp).xshot,(*alienp).yshot);     
                glVertex2f((*alienp).xshot,(*alienp).yshot-SHOTHEIGHT);     
            glEnd();     
        }     
    }    
    ///////////////////////////////////////////////////    
    glColor3f(1.0,1.0,1.0);      
    if(play==0 && youwin==false && life>0 && miny>DEADLINE && pausa==0)    
    {    
        bitmap_output(scrittaposx-54,scrittaposy-80,0,"PREMI INVIO PER INIZIARE",GLUT_BITMAP_9_BY_15);    
        bitmap_output(scrittaposx-85,scrittaposy-100,0,"DURANTE IL GIOCO W PER LA PAUSA",GLUT_BITMAP_9_BY_15);    
        bitmap_output(scrittaposx-144,scrittaposy-120,0,"PER SPARARE PREMI IL TASTO SINISTRO DEL MOUSE",GLUT_BITMAP_9_BY_15);    
        bitmap_output(scrittaposx-99,scrittaposy-140,0,"PER MUOVERE LA NAVETTA PREMI A E D",GLUT_BITMAP_9_BY_15);   
    }    
    if(play==0 && youwin==false && life>0 && miny>DEADLINE && pausa==1)    
    {    
        bitmap_output(scrittaposx-45,scrittaposy-80,0,"PREMI W PER CONTINUARE",GLUT_BITMAP_9_BY_15);  
		bitmap_output(scrittaposx-9,scrittaposy-100,0,"PUNTEGGIO: ",GLUT_BITMAP_9_BY_15);
		bitmap_output(scrittaposx+90,scrittaposy-100,0,punteggioStringa,GLUT_BITMAP_9_BY_15);
    }    
    if(life<=0)//se non hai più vite hai perso    
    {    
        bitmap_output(scrittaposx,scrittaposy,0,"HAI PERSO!!!",GLUT_BITMAP_9_BY_15);    
        bitmap_output(scrittaposx-36,scrittaposy-20,0,"PREMI ESC PER USCIRE",GLUT_BITMAP_9_BY_15);//40   
		bitmap_output(scrittaposx-9,scrittaposy-40,0,"PUNTEGGIO: ",GLUT_BITMAP_9_BY_15);
		bitmap_output(scrittaposx+90,scrittaposy-40,0,punteggioStringa,GLUT_BITMAP_9_BY_15);
        TastiSpeciali(GLUT_KEY_F4,NULL,NULL);//si ferma l'idle    
    }    
    if(youwin==true)//se youwin è true hai vinto    
    {    
        bitmap_output(scrittaposx,scrittaposy,0,"HAI VINTO!!!",GLUT_BITMAP_9_BY_15);    
        bitmap_output(scrittaposx-40,scrittaposy-20,0,"PREMI ESC PER USCIRE",GLUT_BITMAP_9_BY_15); 
		
		bitmap_output(scrittaposx-9,scrittaposy-40,0,"PUNTEGGIO: ",GLUT_BITMAP_9_BY_15);
		bitmap_output(scrittaposx+90,scrittaposy-40,0,punteggioStringa,GLUT_BITMAP_9_BY_15);
    } 
    glFlush();          
}          
                        
void myMouse(int button,int state,int x,int y) //funzione che viene richiamata quando premo un tasto del mouse         
{          
    Shot n;  //creo un elemento shot    
    n.yshot=SSHEIGHT; //che parte dalla punta della mia navetta    
    n.xshot=traslaship+25;     
    if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN) //premo il tasto sinistro    
    {     
        if(play==1)//se sto giocando e non sono in pausa    
        {    
            punteggio=punteggio-2;//diminuisco il punteggio ad ogni colpo sparato 
            numshot++; //incremento il numero dei colpi    
            Colpi.push_back(n);//metto in coda un nuovo colpo     
            glutPostRedisplay(); //faccio il refresh della finestra    
        }    
    }     
    glFlush();           
}          
                    
void myinit(void)      //inizializzo l'ambiente di lavoro  
{            
    glClearColor(1.0, 1.0, 1.0, 0.0);                            
    glPointSize(4.0);          
    glMatrixMode(GL_PROJECTION);            
    glLoadIdentity();            
    gluOrtho2D(0.0, WINW, 0.0, WINH);          
}            
                        
void main(int argc, char** argv)       
{            
    //inizializzazioni dei vettori utili      
    int i=0;         
    for(i=0;i<NUMNAV;i++)    //tutte le navi all'inizio sono vive     
        NaviAliene[i].alive=true;         
    for(i=0;i<ROWS;i++)     //setto l'ultimo della riga    
        lastrow[i]=8+i*NUMNAV/ROWS;         
    for(i=0;i<ROWS;i++)     //setto il primo delle righe    
        firstrow[i]=i*NUMNAV/ROWS;        
    for (i=0;i<ROWS;i++)  //ogni riga all'inizio è viva    
        linealive[i]=true;    
    //////////////////////////////////////    
        
    int wmain;        
    glutInit(&argc,argv);          
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);          
    glutInitWindowSize(WINW,WINH);          
    glutInitWindowPosition(WINW/8,WINH/8);          
    wmain=glutCreateWindow("MySpaceInvaders");//creo la finestra principale e la chiamo wmain          
    glutDisplayFunc(display);          
    glutMouseFunc(myMouse);          
    glutKeyboardFunc(myKeyboard);     
    glutSpecialFunc(TastiSpeciali);         
    myinit();          
    glutMainLoop();          
}