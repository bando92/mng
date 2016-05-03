#include "stdafx.h"   
#include <windows.h>   
#include <stdlib.h>   
#include <stdio.h>   
#include <math.h>   
#include <gl/glut.h>   
#include <gl/gl.h>   
#include <gl/glu.h>   
#include <gl/glui.h> 
  
//DEFINE
#define WINW 650   
#define WINH 650 
#define NUM 50 
  
//Definizione Struttura Punti
	typedef struct{GLfloat x,y;} GLFloatPoint; 
	static GLFloatPoint Punti[NUM];//memorizziamo via via i vertici cliccati sulla finestra 

//-------------------------------------------VARIABILI GLOBALI-------------------------------------------

	//Variabili per Pannelli e Bottoni 
	GLUI_Spinner *spinner_SF_r,*spinner_SF_b,*spinner_SF_g,*spinner_DIS_r,*spinner_DIS_b,*spinner_DIS_g; 
	GLUI_Panel *pannello_sfondo,*pannello_primitive,*pannello_scelte,*pannello_Bezier,*pannello_Hermite,*pannello_Spline; 
	GLUI_Button *bottone_Hermite, *bottone_Bezier, *bottone_Spline; 
	GLUI_Checkbox *check_box_mod_der; 
	GLUI_RadioGroup *radio_scelte,*radio_scelta_metodi,*radio_scelta_algo; 
	GLUI_EditText *dim_vertice; 
  
    //interfaccia Hermite e variabili 
    GLUI_Spinner *indice_pto_mod, *derivata_x, *derivata_y; 
    float val_der_x, val_der_y; 
    int ip; 
    //------------------------------------------- 
  
    //interfaccia Bezier e variabili 
    GLUI_Checkbox *check_box_subd,*check_box_degree; 
    int subd,d_el,n_reit=1; 
    GLUI_Spinner *suddivisione,*degreeelev; 
    float t_sub=0.5; 
    float xploc[70],yploc[70],xpde[70],ypde[70],B[1000][1000]; 
    //------------------------------------------- 
    
	//interfaccia Spline e variabili
	GLUI_Checkbox *check_box_moltep; 
    int mod_moltep,i_nodo,val_moltep; 
	float val_nodo;
    GLUI_EditText *valore_moltep, *indice_nodo_mod; 
	//-------------------------------------------

	//interfaccia Bezier Razionali e variabili
	GLUI_Checkbox *check_box_pesi_bezier;
	int mod_pesi_bez,i_p_b;
	float val_peso;
	GLUI_EditText *valore_peso, *i_peso_bezier;
	//-------------------------------------------

	//interfaccia Spline Razionali e variabili
	GLUI_Checkbox *check_box_pesi_spline;
	int mod_pesi_spline,i_p_s;
	float val_pesos;
	GLUI_EditText *valore_pesos, *i_peso_spline;
	//-------------------------------------------
  
	//variabili varie
	float t[50],dx[50],dy[50]; 
	int mod_der; 
	static int last=0; 
	float rosso_sf=0.0,verde_sf=0.0,blu_sf=0.0,rosso_dis=1.0,verde_dis=1.0,blu_dis=1.0; 
	int scelta_param=-1,scelta_metodi=-1,scelta_algo=-1,dimpunto=2,n=0; 
	static float traslax=10,traslay=10,ruotapos=0,ruotaneg=0,scalax=1,x,f; 
	char metodo,molt[50]; 
  
    //variabili per le spline 
    int m=4;//ordine della spline 
    int k;//numero dei nodi veri m+k=N ->k=N-m 
    float Nodi[50]; 
    //------------------------------------------- 
	
	//pesi curve razionali
	float w[50];

	//variabili per la mouse motion
	static bool first = true;
	static int scelta;
//-----------------------------------------------FINE VARIABILI GLOBALI-------------------------------------------
  
//-----------------------------------------------DICHIARAZIONE FUNZIONI------------------------------------------- 

void myMouse(int,int,int,int); //gestisce gli eventi mouse
void Disegna_Funzioni_Base(); //disegna le funzioni base del polinomio di Hermite
void Disegna_fBaseBernstein(); //disegna le funzioni base del polinomio di Bezier
float phi0(float); //funzione base del polinomio di Hermite
float psi0(float); //funzione base del polinomio di Hermite
float phi1(float); //funzione base del polinomio di Hermite
float psi1(float); //funzione base del polinomio di Hermite
void myinit(void); //inizializza l'ambiente
void Derivata_x_rapp_incr(); //calcola le derivate in x con il rapporto incrementale
void Derivata_y_rapp_incr(); //calcola le derivate in y con il rapporto incrementale
void Hermite(); //permette di disegnare il polinomio interpolatore di Hermite
void Berzier(); //permette di disegnare il polinomio interpolatore di Bezier
void Subdivision(); //divide il poligono di controllo in 2 parti a seconda del parametro indicato
void Parametrizzazione_Uniforme(); //suddivide i valori di t in maniera equidistante
void Parametrizzazione_Corde(); //suddivide i valori di t tenendo conto delle distanze tra i punti
void metodi(int); //permette di scegliere il polinomio da utilizzare
void Degree_Elevation(); //algoritmo di degree elevation per i polinomi di Bezier
void Costruisci_Nodi(); //costruisce i nodi per le B-Spline
int Bisezione(float); 
void De_Boor(); //permette di disegnare le B-Spline
void Funzioni_Bspline(float *); //disegna le funzioni base di una B-Spline
void myMotion(int , int ); //gestisce l'evento di movimento del mouse
//----------------------------------------------- 
  
//////////////////////Funzioni 

//Parametrizzazione
void Parametrizzazione_Uniforme() 
{ 
	//divido in maniera equidistante i valori di t
	int i; 
    float passo; 
    passo=1.0/(n-1); 
    t[1]=0; 
    for(i=2;i<=n;i++) 
        t[i]=t[i-1]+passo; 
} 
void Parametrizzazione_Corde() 
{ 
    //i valori di t seguono l'andamento dei punti
	int i; 
    t[1]=0; 
    for(i=2;i<=n;i++) 
        t[i]=t[i-1]+sqrt((Punti[i].x-Punti[i-1].x)*(Punti[i].x-Punti[i-1].x)+(Punti[i].y-Punti[i-1].y)*(Punti[i].y-Punti[i-1].y)); 
  
    for (i=1;i<=n;i++) 
    { 
        t[i]=t[i]/t[n]; 
    } 
} 

//Funzioni phi-psi
float phi0(float t) 
{ 
    float value; 
    value=2*(t*t*t)-3*(t*t)+1; 
    return value; 
}
float phi1(float t) 
{ 
    float value; 
    value=(t*t*t)-2*(t*t)+t; 
    return value; 
}
float psi0(float t) 
{ 
    float value; 
    value=-2*(t*t*t)+3*(t*t); 
    return value; 
} 
float psi1(float t) 
{ 
    float value; 
    value=(t*t*t)-(t*t); 
    return value; 
}
  
//Derivate
void Derivata_x_rapp_incr() 
{ 
    int i; 
    dx[1]=0; 
    for(i=2;i<=n;i++) 
        dx[i]=(Punti[i].x-Punti[i-1].x)/(t[i]-t[i-1]); 
} 
void Derivata_y_rapp_incr() 
{ 
    int i; 
    dy[1]=0; 
    for(i=2;i<=n;i++) 
        dy[i]=(Punti[i].y-Punti[i-1].y)/(t[i]-t[i-1]); 
} 
  
//Scelta metodi
void metodi(int value) 
{ 
    if(value==1) 
        metodo='H'; 
    if(value==2) 
        metodo='B'; 
    if(value==3) 
        metodo='S'; 
} 
  
//Hermite
void Hermite() 
{ 
    float passot=0.0001,tg,tc,cx,cy; 
    int i=1; 
    Derivata_x_rapp_incr();//calcola la derivata in x dei punti
    Derivata_y_rapp_incr();//calcola la derivata in y dei punti
  
    if(mod_der==1)//checkbox spuntato -> modifico il valore della derivata in un punto
    { 
        dx[ip]=val_der_x;
        dy[ip]=val_der_y; 
    } 
  
    for (tg=0;tg<=1;tg=tg+passot) //nell'intervallo [0,1] disegno punti variando 0.0001 ogni volta
    { 
          
        //localizziamo l'intervallo iesimo [t(i),t(i+1)] a cui il valore del parametro tg appartiene 
        //all'inizio i=1,parto quindi dal primo intervallo...ogni volta che supero l'intervallo incremento la i,spostandomi quindi nell'intervallo successivo
		if (tg>t[i+1]) 
            i++; 

        // mappare il punto tg appartenente all'intervallo [t(i),t(i+1)] in un punto tcappello in [0,1] 
        tc=(tg-t[i])/(t[i+1]-t[i]); 

        // valutiamo le coordinate del punto sulla curva con le formule di interpolazione di hermite 
        cx=Punti[i].x*phi0(tc)+
			dx[i]*phi1(tc)*(t[i+1]-t[i])+
			Punti[i+1].x*psi0(tc)+
			dx[i+1]*psi1(tc)*(t[i+1]-t[i]);

        cy=Punti[i].y*phi0(tc)+
			dy[i]*phi1(tc)*(t[i+1]-t[i])+
			Punti[i+1].y*psi0(tc)+
			dy[i+1]*psi1(tc)*(t[i+1]-t[i]);
  
		//disegno il punto ottenuto sullo schermo
        glBegin(GL_POINTS);
            glVertex2f(cx,cy); 
        glEnd(); 
    } 
    glFlush(); 
} 
void Disegna_Funzioni_Base() 
{ 
    float fbasephi0[1000],fbasephi1[1000],fbasepsi0[1000],fbasepsi1[1000],tf,vt[1000]; 
    int n_pti_val=900,i=1; 
    float passo=1.0/(n_pti_val-1); 
  
    for(tf=0;tf<=1;tf+=passo) 
    { 
        vt[i]=tf; 
        fbasephi0[i]=phi0(tf); 
        fbasephi1[i]=phi1(tf); 
        fbasepsi0[i]=psi0(tf); 
        fbasepsi1[i]=psi1(tf); 
        i++; 
    } 
    //definiamo le coordinate della finestra reale(sul mondo) 
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, 1.0, -0.8, 1.0); 
  
    //il contenuto della finestra reale prima definita verrà mappato nella viewport sullo schermo che ha origine in (0,500),h 150 e largh 150 
    glViewport(0,500,150,150); 
      
    glColor3f(0.0,0.0,1.0); 
    glBegin(GL_POINTS); 
        for(i=1;i<=n_pti_val;i++) 
            glVertex2f(vt[i],fbasephi0[i]);  
        glEnd(); 
  
    glColor3f(0.0,1.0,1.0); 
    glBegin(GL_POINTS); 
        for(i=1;i<=n_pti_val;i++) 
            glVertex2f(vt[i],fbasephi1[i]);  
        glEnd(); 
  
    glColor3f(0.0,1.0,0.0); 
    glBegin(GL_POINTS); 
    for(i=1;i<=n_pti_val;i++) 
        glVertex2f(vt[i],fbasepsi0[i]);  
    glEnd(); 
  
    glColor3f(1.0,0.0,0.0); 
    glBegin(GL_POINTS); 
    for(i=1;i<=n_pti_val;i++) 
        glVertex2f(vt[i],fbasepsi1[i]);  
    glEnd(); 
    glFlush(); 
  
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, WINH, 0.0, WINH); 
    glViewport(0,0,WINH,WINH); 
} 

//Bezier
void Bezier() 
{ 
    float tg,passo_tg=0.0001,xc[NUM],yc[NUM],zc[NUM]; 
    int i,j; 
    for(tg=0;tg<=1;tg+=passo_tg) 
    { 
        for(i=1;i<=n;i++) 
        { 
            xc[i]=Punti[i].x*w[i]; 
            yc[i]=Punti[i].y*w[i]; 
			zc[i]=w[i]; //inizialmente tutti i pesi sono a 1...se aumento il peso la curva viene attirata verso il punto di controllo
        } 
    
    //Algoritmo di De Casteljau
	//si basa sul fatto che ogni polinomio B i,n(t) è esprimibile come combinazione lineare convessa dei due polinomi di Bernstein di grado n-1
        for(j=1;j<=n;j++)
        { 
            for(i=1;i<=n-j;i++) //ottengo i valori dei coefficienti come combinazione convessa dei coefficienti al passo precedente
            { //mi permette di disegnare i punti seguendo l'interpretazione geometrica dell'algoritmo
                xc[i]=xc[i+1]*tg+xc[i]*(1-tg); 
                yc[i]=yc[i+1]*tg+yc[i]*(1-tg); 
				zc[i]=zc[i+1]*tg+zc[i]*(1-tg); 
            } 
        } 
        glBegin(GL_POINTS); 
            glVertex2f(xc[1]/zc[1],yc[1]/zc[1]); 
        glEnd(); 
    } 
    glFlush(); 
} 
void Disegna_fBaseBernstein() 
{ 
    float tg,passo_tg=0.001,tmp,d1,d2; 
    float B[1010][50]; 
    int k=1,l,i,j; 
    for(tg=0;tg<=1;tg+=passo_tg)//per ogni tg voglio valutare la mia funzione base 
    { 
        l=n; 
        B[k][l]=1.0;//condizione iniziale 
        for(i=1;i<n;i++)//è solo minore perchè il grado è uguale al numero di punti meno 1 
        { 
            tmp=0.0; 
            for(j=l;j<=n;j++) 
            { 
                d1=tg; 
                d2=1-tg; 
                B[k][j-1]=d2*B[k][j]+tmp;
                tmp=d1*B[k][j]; 
            } 
            B[k][n]=tmp; 
            l=l-1; 
        } 
        k=k+1; 
    } 
    //definiamo le coordinate della finestra reale(sul mondo) 
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, 1.0, -0.8, 1.0); 
    //il contenuto della finestra reale prima definita verrà mappato nella viewport sullo schermo che ha origine in (0,500),h 300 e largh 800 
    glViewport(0,500,150,150); 
    glColor3f(1.0,0.0,0.0); 

	float s;
	for(i=1;i<=k;i++)
	{
		s=0;
		for(j=1;j<=n;j++)//faccio la sommatoria
		{
			s=s+w[j]*B[i][j];
		}
		for(j=1;j<=n;j++)//ridefinisco le funzioni base
		{
			B[i][j]=w[j]*B[i][j]/s;
		}
	}

	for(j=1;j<=n;j++) 
    { 
        tg=0.0; 
        glBegin(GL_POINTS); 
            for(i=1;i<k;i++) 
            { 
                glVertex2f(tg,B[i][j]); 
                tg=tg+passo_tg; 
            } 
        glEnd(); 
    } 

    glFlush(); 
  
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, WINW, 0.0, WINW); 
    glViewport(0,0,WINW,WINW); 
      
}
void Subdivision() 
{ 
    float xc[NUM],yc[NUM],vc1_x[NUM],vc1_y[NUM],vc2_x[NUM],vc2_y[NUM]; 
    int i,j; 
    for(i=1;i<=n;i++) 
        { 
            xc[i]=Punti[i].x; 
            yc[i]=Punti[i].y; 
        } 
  
    //Algoritmo di De Casteljau 
  
    vc1_x[1]=xc[1];//vertici di controllo della prima curva componente in x 
    vc1_y[1]=yc[1]; 
    vc2_x[1]=xc[n]; 
    vc2_y[1]=yc[n]; 
  
    for(j=1;j<=n;j++) 
    { 
        for(i=1;i<=n-j;i++) 
        { 
            xc[i]=xc[i+1]*t_sub+xc[i]*(1-t_sub); 
            yc[i]=yc[i+1]*t_sub+yc[i]*(1-t_sub); 
        } 
        vc1_x[j+1]=xc[1]; 
        vc1_y[j+1]=yc[1]; 
        vc2_x[j+1]=xc[n-j]; 
        vc2_y[j+1]=yc[n-j]; 
    } 
    glColor3f(0.0,1.0,0.0); 
    glBegin(GL_LINE_STRIP); 
        for(i=1;i<=n;i++) 
            glVertex2f(vc1_x[i],vc1_y[i]); 
    glEnd(); 
    glColor3f(0.0,0.0,1.0); 
    glBegin(GL_LINE_STRIP); 
        for(i=1;i<=n;i++) 
            glVertex2f(vc2_x[i],vc2_y[i]); 
    glEnd(); 
    glFlush(); 
}   
void Degree_Elevation() 
{ //esprimiamo in polinomio di grado n come un polinomio di grado n+1
    int i, N_loc=n,reit; 
    float par, val_g=0,val_r=1; 
    for(i=1;i<=n;i++) 
    { 
        xploc[i]=Punti[i].x; 
        yploc[i]=Punti[i].y; 
  
    } 
    for (reit =1; reit<=n_reit;reit++) 
    { 
        xpde[1]=xploc[1]; 
        ypde[1]=yploc[1]; 
  
        xpde[N_loc+1]=xploc[N_loc]; 
        ypde[N_loc+1]=yploc[N_loc]; 
  
        for (i=2;i<=N_loc;i++) 
        { //nuovi coefficienti (che si trovano sul segmento che unisce xploc[i] e xploc[i-1]) ottenuti come interpolazione lineare dei vecchi coefficienti per il valore del parametro par
            par=(float)(i)/(N_loc+1); 
            xpde[i]=(1-par)*xploc[i]+par*xploc[i-1]; 
            ypde[i]=(1-par)*yploc[i]+par*yploc[i-1]; 
        } 
        glColor3f(val_r,val_g,1.0); 
  
        glBegin(GL_POINTS); 
            for(i=1;i<=N_loc;i++) 
            { 
                glVertex2f(xpde[i],ypde[i]); 
            } 
        glEnd(); 
        glFlush(); 
  
        glBegin(GL_LINE_STRIP); 
            for(i=1;i<=N_loc;i++) 
            { 
                glVertex2f(xpde[i],ypde[i]); 
            } 
        glEnd(); 
        glFlush(); 
  
      
  
        N_loc=N_loc+1; 
            for (i=1;i<=N_loc;i++)//setto xploc al nuovo valore xpde 
            { 
                xploc[i]=xpde[i]; 
                yploc[i]=ypde[i]; 
            } 
            val_g=val_g+0.1; 
            val_r=val_r+0.05; 
  
          
  
    } 
} 

//B-Spline
void Funzioni_Bspline(float *Nodi) 
{ 
    int k=1,kk,i,j,l; 
    float tg,tmp,d1,d2,beta;  
    // inizializziamo a 0 la matrice delle f base 
    for(i=1;i<=1001;i++) 
        for(j=1;j<=n;j++) 
            B[i][j]=0; 
    for (tg=0;tg<=1;tg=tg+0.001) 
    { 
        l=Bisezione (tg); 
        B[k][l]=1; 
        kk=l; 
        // formule di k vere e proprie 
        for(i=1;i<m;i++) 
        { 
            tmp=0.0; 
            for (j=l;j<=kk;j++) 
            { 
                d1=tg-Nodi[j]; 
                d2=Nodi[i+j]-tg; 
                beta = B[k][j]/(d1+d2); 
                B[k][j-1]=d2*beta+tmp; 
                tmp=d1*beta; 
            } 
            B[k][kk]=tmp; 
            l=l-1; 
        } 
        k=k+1; 
    } 
    // Definizione della finestra sul mondo  
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, 1.0, -0.01, 1.2);// le funzioni sono positive quidni -0.01, e diamo 1.2 perche il massimo sara più grande di 1 
    glViewport(0,500,150,150); 
    glColor3f(1.0,0.0,0.0); 
	
	float s;
	for(i=1;i<=k;i++)
	{
		s=0;
		for(j=1;j<=n;j++)//faccio la sommatoria
		{
			s=s+w[j]*B[i][j];
		}
		for(j=1;j<=n;j++)//ridefinisco le funzioni base
		{
			B[i][j]=w[j]*B[i][j]/s;
		}
	}

    for(j=1;j<=n;j++) 
    { 
        tg=0.0; 
        glBegin(GL_POINTS); 
            for(i=1;i<k;i++) 
            { 
                glVertex2f(tg,B[i][j]); 
                tg=tg+0.001; 
            } 
        glEnd(); 
    } 
    glFlush(); 
	glColor3f(0.0,1.0,0.0); 
    for(j=1;j<=m+n;j++) 
    { 
        glRasterPos2f(Nodi[j],0.0); 
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,molt[j]); 
    } 
  
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, WINW, 0.0, WINW); 
    glViewport(0,0,WINW,WINW); 
  
    glColor3f(rosso_dis,verde_dis,blu_dis); 
} 
void Costruisci_Nodi() 
{ 
    int i,cont=3; 
    k=n-m; 
	//[a,b] -> [0,1]
    for(i=1;i<=m;i++)//m nodi fittizzi a sx in corrispondenza di a
    { 
        Nodi[i]=0; 
        molt[i]='4'; 
  
    } 
    for(i=m+1;i<=m+k;i++)//k nodi veri 
    { 
        Nodi[i]=t[cont]; 
        cont++; 
        molt[i]='1'; 
    } 
    for(i=m+k+1;i<=k+2*m;i++)//m nodi fittizzi a dx in corrispondenza di b
    { 
        Nodi[i]=1; 
    } 
  
    if(mod_moltep==1) 
    { 
        //più aumenta la molteplicità, più si riducono le condizioni di raccordo nei nodi
		//se m1=m non c'è raccordo nel nodo i-esimo
		val_nodo=Nodi[i_nodo];
        if(val_moltep == 2) 
            molt[i]='2'; 
        if(val_moltep == 3) 
            molt[i]='3'; 
        for(i=1;i<=val_moltep;i++) 
        { 
            Nodi[i_nodo+i-1]=val_nodo; 
        }
    }
}  
void De_Boor() 
{ 
    float tg,cx[5],cy[5],cz[5],denom,a1,a2,sx,sy; 
    int l,j,i; 
    for(tg=0.0;tg<=1;tg+=0.0001)//per ogni parametro 
    { 
        //localizziamo l'intervallo internodale a cui tg appartiene [Nodi[l],Nodi[l+1]] 
        l=Bisezione(tg); 
  
		//nell'algoritmo di de boor sostituisco alla b spline di ordine m la combinazione di due b-spline di ordine inferiore
        for(j=1;j<=m;j++) 
        { //moltiplico per i pesi...all'inizio sono tutti 1...poi li posso variare con una funzione apposita
            cx[j]=Punti[j+l-m].x*w[j+l-m]; 
            cy[j]=Punti[j+l-m].y*w[j+l-m]; 
			cz[j]=w[j+l-m]; 
        } 
        for(j=2;j<=m;j++) //valuto la curva in un punto mediante la combinazione di coefficienti
        { //il valore della spline calcolata in x è quello di una spline di ordine uno per un opportuno coefficiente
            for(i=m;i>=j;i--) 
            { 
                denom=Nodi[i+l-j+1]-Nodi[i+l-m]; 
                a1=(tg-Nodi[i+l-m])/denom; 
                a2=1-a1; 
                cx[i]=a1*cx[i]+a2*cx[i-1]; 
                cy[i]=a1*cy[i]+a2*cy[i-1]; 
				cz[i]=a1*cz[i]+a2*cz[i-1]; 

            } 
        } 
        sx=cx[m]/cz[m]; 
        sy=cy[m]/cz[m]; 
  
        glBegin(GL_POINTS); 
            glVertex2f(sx,sy); 
        glEnd(); 
    } 
    glFlush(); 
}
int Bisezione(float z) 
{ 
    int  mid; 
    int l=m,u=m+k+1; 
    while((u-l)>1) 
    { 
        mid=(l+u)/2; 
        if(z<Nodi[mid]) 
            u=mid; 
        else
            l=mid; 
    } 
    return l; 
} 
  
//Display
void display (void) 
{ 
    glClearColor(rosso_sf,verde_sf,blu_sf,0.0);//rendo variabili i parametri che mi definiscono il colore 
    glClear(GL_COLOR_BUFFER_BIT); 
    int i; 

    glColor3f(0.0,0.0,1.0); 
    glPointSize(dimpunto); 
    glBegin(GL_POINTS);//per visualizzare anche i punti dove si clicca di colore blu
        for (i=1;i<=last;i++) 
            glVertex2f(Punti[i].x,Punti[i].y); 
    glEnd(); 
    glColor3f(rosso_dis,verde_dis,blu_dis); 
    glBegin(GL_LINE_STRIP); //poligono di controllo
        for (i=1;i<=last;i++) 
            glVertex2f(Punti[i].x,Punti[i].y); 
    glEnd(); 
    n=last;//setto n al numero di punti disegnato
  
	for(i=1;i<=n;i++)
		w[i]=1;//metto tutti i pesi a 1

	//scelta parametrizzazione
    if (scelta_param==0) 
    { 
        Parametrizzazione_Uniforme();    
    } 
    else if(scelta_param==1) 
    { 
        Parametrizzazione_Corde(); 
    } 
  
	//scelta della funzione
	glPointSize(2);//la curva la disegno sempre di dimensione 2
    if (metodo=='H' && last>1) 
    { 
        Hermite(); 
        Disegna_Funzioni_Base(); 
    } 
  
    if (metodo=='B' && last>1) 
    { 
        if(mod_pesi_bez==1)
			w[i_p_b]=val_peso;
		Bezier(); 
        if (subd==1) 
            Subdivision(); 
        if (d_el==1) 
            Degree_Elevation(); 
        Disegna_fBaseBernstein(); 
    } 
  
    if (metodo=='S' && last>3) 
    { 
        if(mod_pesi_spline==1)
			w[i_p_s]=val_pesos;
		Costruisci_Nodi(); 
        Funzioni_Bspline(Nodi); 
        De_Boor();   
    } 
    //glutPostRedisplay();//è per questo che le funzioni base saltellano :)...se non lo metti però quando cambi il metodo non viene cambiato immediatamente ma dopo aver spinto un altro punto 
    glFlush(); 
} 

//Mouse Events
void myMouse(int button,int state,int x,int y) 
{ 
    const int screenHeight=WINH; 
    float y1 = WINH - y;
	int i; 
    if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN && last <(NUM-1)) //se il sinistro è premuto
    { 
        Punti[++last].x=x; //aggiungo un punto nel vettore di coordinate (x,screenHeight-y),perchè il (0,0) parte da basso a sx
        Punti[last].y=screenHeight-y; 
        glClear(GL_COLOR_BUFFER_BIT); 
        glColor3f(0.0,0.0,1.0); 
        glBegin(GL_POINTS); 
            for (i=1;i<=last;i++) 
                glVertex2f(Punti[i].x,Punti[i].y); 
        glEnd(); 
        glColor3f(rosso_dis,verde_dis,blu_dis); 
        glBegin(GL_LINE_STRIP); 
            for (i=1;i<=last;i++) 
                glVertex2f(Punti[i].x,Punti[i].y); 
        glEnd(); 
        glutPostRedisplay(); 
          
    } 
  //  else if(button==GLUT_RIGHT_BUTTON &&state==GLUT_DOWN)//pulisco lo schermo 
  //  {   
		//screen clean
  //     last = 0;
		///* n=last; 
  //      printf("n = %d\n",n); */
	 //  glutPostRedisplay(); 
  //  } 
    else if(button==GLUT_MIDDLE_BUTTON &&state==GLUT_DOWN)//col tasto centrale torno indietro di un punto 
    {    
        if(last>0) 
            last=last-1; 
        glClear(GL_COLOR_BUFFER_BIT); 
        glColor3f(0.0,0.0,1.0); 
        glBegin(GL_POINTS); 
            for (i=1;i<=last;i++) 
                glVertex2f(Punti[i].x,Punti[i].y); 
        glEnd(); 
        glColor3f(rosso_dis,verde_dis,blu_dis); 
        glBegin(GL_LINE_STRIP); 
            for (i=1;i<=last;i++) 
                glVertex2f(Punti[i].x,Punti[i].y); 
        glEnd(); 
        glutPostRedisplay(); 
    } 
	
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		for (i=1;i<=last;i++)//scorro tutti i punti della lista
		{
			// se ho preso in un punto
			if (x >= Punti[i].x - dimpunto / 2 && x <= Punti[i].x + dimpunto / 2 && y1 >= Punti[i].y - dimpunto / 2 && y1 <= Punti[i].y + dimpunto / 2)
			{
				scelta = i;//conosco quale punto ho selezionato
				if (first == true)//alla prima volta che vede che il tasto è premuto lo setto a false, in modo tale da dirgli che da adesso in poi può trascinare
					first = false;
			}
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		first = true;

    glFlush(); 
} 
void myMotion(int x, int y)
{
    y = WINH - y;
    if (first == false)//se ho preso qualcosa e sto tenendo premuto il mouse
    {
		Punti[scelta].x = x;
		Punti[scelta].y = y;

		glBegin(GL_POINTS); 
			glVertex2f(Punti[scelta].x,Punti[scelta].y);
		glEnd(); 

        glutPostRedisplay();
    }
}

//MyInit
void myinit(void)   
{   
    glClearColor(1.0, 1.0, 1.0, 0.0);        
    glColor3f(0.0, 0.0, 0.0);            
    glMatrixMode(GL_PROJECTION);   
    glLoadIdentity();   
    gluOrtho2D(0.0, WINW, 0.0, WINH); 
}   
  
//Main
void main(int argc, char** argv)   
{   
    int wmain; 
    glutInit(&argc,argv); 
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
    glutInitWindowSize(WINW,WINH); 
    glutInitWindowPosition(50,50); 
    wmain=glutCreateWindow("Finestra Principale");//creo la finestra principale e la chiamo wmain 
    GLUI *glui=GLUI_Master.create_glui("Opzioni");//crea un oggetto di tipo glui che invoca il metodo createglui 
      
    //Pannello Colore Sfondo 
    pannello_sfondo=glui->add_rollout("Gestione Colore Sfondo",false); 
    spinner_SF_r=glui->add_spinner_to_panel(pannello_sfondo,"Rosso",GLUI_SPINNER_FLOAT,&rosso_sf);//aggiungo spinner al pannello 
    spinner_SF_r->set_float_limits(0,1);//limiti dello spinner 
    spinner_SF_g=glui->add_spinner_to_panel(pannello_sfondo,"Verde",GLUI_SPINNER_FLOAT,&verde_sf); 
    spinner_SF_g->set_float_limits(0,1); 
    spinner_SF_b=glui->add_spinner_to_panel(pannello_sfondo,"Blu",GLUI_SPINNER_FLOAT,&blu_sf); 
    spinner_SF_b->set_float_limits(0,1); 
    //----------------------------------- 
  
    //Pannello Colore Disegno 
    pannello_primitive=glui->add_rollout("Gestione Colore Disegno",false); 
    spinner_DIS_r=glui->add_spinner_to_panel(pannello_primitive,"Rosso",GLUI_SPINNER_FLOAT,&rosso_dis); 
    spinner_DIS_r->set_float_limits(0,1); 
    spinner_DIS_g=glui->add_spinner_to_panel(pannello_primitive,"Verde",GLUI_SPINNER_FLOAT,&verde_dis); 
    spinner_DIS_g->set_float_limits(0,1); 
    spinner_DIS_b=glui->add_spinner_to_panel(pannello_primitive,"Blu",GLUI_SPINNER_FLOAT,&blu_dis); 
    spinner_DIS_b->set_float_limits(0,1); 
    //------------------------------------ 
  
    //Pannello Scelta Parametrizzazione 
    pannello_scelte=glui->add_rollout("Scelta Parametrizzazione",false);//creo un pannello 
    radio_scelte=glui->add_radiogroup_to_panel(pannello_scelte,&scelta_param);//costruisco il radiogruppo 
    glui->add_radiobutton_to_group(radio_scelte,"Uniforme");//gli attacco i bottoni(sono mutuamente esclusivi) 
    glui->add_radiobutton_to_group(radio_scelte,"Secondo le Corde"); 
    //------------------------------------ 
  
    //Pannello Hermite 
    pannello_Hermite=glui->add_rollout("Hermite",false);//creo un pannello 
    bottone_Hermite=glui->add_button_to_panel(pannello_Hermite,"Hermite",1,metodi);//passo il valore 1 alla funzione metodi 
    check_box_mod_der=glui->add_checkbox_to_panel(pannello_Hermite,"Modifica Derivata",&mod_der); 
    indice_pto_mod=glui->add_spinner_to_panel(pannello_Hermite,"Indice Punto da Modificare",GLUI_SPINNER_INT,&ip); 
    derivata_x=glui->add_spinner_to_panel(pannello_Hermite,"Componente X della Derivata",GLUI_SPINNER_FLOAT,&val_der_x); 
    derivata_y=glui->add_spinner_to_panel(pannello_Hermite,"Componente Y della Derivata",GLUI_SPINNER_FLOAT,&val_der_y); 
    //------------------------------------ 
  
    //Pannello Bezier 
    pannello_Bezier=glui->add_rollout("Bezier",false);//creo un pannello 
    bottone_Bezier=glui->add_button_to_panel(pannello_Bezier,"Bezier",2,metodi);//passo il valore 1 alla funzione metodi 
    check_box_subd=glui->add_checkbox_to_panel(pannello_Bezier,"Subdivision",&subd); 
    suddivisione=glui->add_spinner_to_panel(pannello_Bezier,"Parametro per la Suddivisione",GLUI_SPINNER_FLOAT,&t_sub); 
    check_box_degree=glui->add_checkbox_to_panel(pannello_Bezier,"DegreeElevation",&d_el); 
    degreeelev=glui->add_spinner_to_panel(pannello_Bezier,"Numero Di Reiterazioni",GLUI_SPINNER_INT,&n_reit); 

	check_box_pesi_bezier=glui->add_checkbox_to_panel(pannello_Bezier,"PesiBezier",&mod_pesi_bez);
	i_peso_bezier=glui->add_edittext_to_panel(pannello_Bezier,"Indice Punto",GLUI_EDITTEXT_INT,&i_p_b); 
	valore_peso=glui->add_edittext_to_panel(pannello_Bezier,"Valore Peso",GLUI_EDITTEXT_FLOAT,&val_peso); 
    //------------------------------------ 
	
	//Pannello Spline
    pannello_Spline=glui->add_rollout("Spline",false);//creo un pannello 
    bottone_Spline=glui->add_button_to_panel(pannello_Spline,"Spline",3,metodi);//passo il valore 1 alla funzione metodi 
    check_box_moltep=glui->add_checkbox_to_panel(pannello_Spline,"Modifica Molteplicita",&mod_moltep); 
    indice_nodo_mod=glui->add_edittext_to_panel(pannello_Spline,"Indice Nodo",GLUI_EDITTEXT_INT,&i_nodo); 
    valore_moltep=glui->add_edittext_to_panel(pannello_Spline,"Molteplicità",GLUI_EDITTEXT_INT,&val_moltep); 

	check_box_pesi_spline=glui->add_checkbox_to_panel(pannello_Spline,"PesiSpline",&mod_pesi_spline);
	i_peso_spline=glui->add_edittext_to_panel(pannello_Spline,"Indice Punto",GLUI_EDITTEXT_INT,&i_p_s); 
	valore_pesos=glui->add_edittext_to_panel(pannello_Spline,"Valore Peso",GLUI_EDITTEXT_FLOAT,&val_pesos); 
	//------------------------------------

    //creo un edit text per cambiare la dimensione del punto 
    dim_vertice=glui->add_edittext("Dimensione Punto",GLUI_EDITTEXT_INT,&dimpunto); 
    //------------------------------------ 
  
    glui->set_main_gfx_window(wmain);//dico a tutti a quale finestra si devono riferire 
  
    glutDisplayFunc(display); 
    glutMouseFunc(myMouse); 
	glutMotionFunc(myMotion);
    myinit(); 
    glutMainLoop(); 
}