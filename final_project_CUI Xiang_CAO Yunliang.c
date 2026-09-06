
/*=============version finale===================*/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include<MLV/MLV_all.h>
#include<time.h>


/*=========================Définition des variables globales==========================*/


//  Dimensions  de  la  grille  en  nombre  de  cases  (origine  en  haut  a  gauche)  :
#define  COLONNES  12
#define  LIGNES  18

//  Les  deux  camps  :
#define  ABEILLE  'A'
#define  FRELON  'F'

//  Les  types  d'unites  :
#define  REINE  'r'
#define  OUVRIERE  'o'
#define  ESCADRON  'e'
#define  GUERRIERE  'g'
#define  TYFRELON  'f'
#define  RUCHE  'R'
#define  NID  'N'
#define VIDE '-'
//  Pour  la  recolte  de  pollen
#define  RECOLTE  'p'

//  Les  temps  necessaires  a  la  production  :
#define  TREINEA  8
#define  TREINEF  8
#define  TOUVRIERE  2
#define  TGUERRIERE  4
#define  TESCADRON  6
#define  TFRELON  5
#define  TRECOLTE  4

//  Les  couts  necessaires  a  la  production  :
#define  CREINEA  7
#define  CREINEF  8
#define  COUVRIERE  3
#define  CGUERRIERE  5
#define  CESCADRON  6
#define  CFRELON  3
#define  CRUCHE  10
#define  CNID  10

//  La  force  des  unites
#define  FREINE  6
#define  FOUVRIERE  1
#define  FGUERRIERE  5
#define  FESCADRON  12
#define  FFRELON  8



/*========================Définition des structures et des tableaux chaînés===========================*/


//  La  structure Unite  :
typedef  struct  unite  {
char  camp;// ABEILLE ou FRELON 
char  type;//  RUCHE,NID,REINE,OUVRIER,GUERRIERE,ESCADRON ou FRELON
int  force;// la force de l'unite
int  posx,  posy;//  position actuelle sur la grille
int  destx,  desty;//  destination(negatif si immobile)
char  production;  // production d'une ruche ou d'un nid et RECOLTE pour la recolte de pollen
int temps; //nombres de tours total pour cette production
int  toursrestant; //  tours restant pour cette production
struct  unite  *usuiv,  *uprec;  //  liste des unites affiliees a une ruche ou un nid
struct  unite  *colsuiv,  *colprec;  //liste des autres ruches ou nids(colonies) du meme camp
struct  unite  *vsuiv,  *vprec;        //liste des autres unites sur la meme case
}Unite,*UListe;

//  La  structure  Case  :
typedef  struct  {
Unite  *colonie;   //  S'il  y  a  une  ruche  ou  un  nid  sur  la  case
UListe  occupant;  //  les  autres  occupants  de  la  case
}  Case;

//  La  structure  Grille  :
typedef  struct  {
Case  plateau[COLONNES][LIGNES];
UListe  abeille,  frelon;
int  tour;  //  Numero  du  tour
int  ressourcesAbeille,   ressourcesFrelon;
char campActuel;// le camp actuel.
Unite *uniteEnaction;// le unit en action
int fin;// calculer le tour.
}  Grille;


/*==============fonction==================*/

//Trouver la force correspondante
int force(char type){ 
    switch (type)
    {
    case REINE:
        return FREINE;
        break;
    case OUVRIERE :
        return FOUVRIERE;
        break ;
    case GUERRIERE:
        return FGUERRIERE;
        break;
    case ESCADRON:
        return FESCADRON;
        break;
    case TYFRELON:
        return FFRELON;
        break;
    default:
        break;
    }
    return 0;
}

//unité de création
Unite *createUnite(char camp, char type,int force) {
    Unite *newUnit = (Unite *)malloc(sizeof(Unite));
    if (newUnit == NULL) {
        //Traitement des échecs d'allocation de mémoire
        return NULL;
    }
    newUnit->camp = camp;
    newUnit->type = type;
    newUnit->force = force;
    newUnit->production = ' ';
    newUnit->temps = 0;
    newUnit->toursrestant = 0;
    // Position initiale
    newUnit->posx = newUnit->posy = -1;
    newUnit->destx = newUnit->desty = -1;
    // Initialisation du pointeur de liste chaînée
    newUnit->usuiv = newUnit->uprec = NULL;
    newUnit->colsuiv = newUnit->colprec = NULL;
    newUnit->vsuiv = newUnit->vprec = NULL;
    return newUnit;
}

//placer l'unité
void placerUnite(Grille *g, Unite *u, int x, int y) {
    // Vérifier la validité des coordonnées
    if (x < 0 || x >= COLONNES || y < 0 || y >= LIGNES) {
        printf("Invalid position.\n");
        return;
    }
    // Actualiser les données de localisation de l'unité
    u->posx = x;
    u->posy = y;
    // Traitement particulier de nid ou de ruche
    if (u->type == RUCHE || u->type == NID) {
        if (g->plateau[x][y].colonie != NULL) {
            // Si un nid de guêpes ou de frelons est déjà présent, il est interdit de placer
            printf("Il déjà exist une Ruche ou un Nid.\n");
            return;
        }
        //Insérer colonie dans la case
        g->plateau[x][y].colonie = u;
    }
    // Ajouter à la liste V
    if (g->plateau[x][y].occupant != NULL) {
        u->vsuiv = g->plateau[x][y].occupant;
        g->plateau[x][y].occupant->vprec = u;
    }
    g->plateau[x][y].occupant = u;
}

//Mettre à jour la liste
void updateUnitLists(Grille *g, Unite *u) {
    // Mettre à jour les listes globales d'abeilles et de frelons, en triant les unités dans l'ordre.
    UListe *list=(u->camp ==ABEILLE)?&g->abeille:&g->frelon;
    // Pointeurs pour parcourir la liste
    UListe prev = NULL;
    UListe courant = *list;
    // Parcourir la liste actuelle
    while (courant != NULL){
        // Si le type de l'unité correspond à celui de l'élément courant
        if(courant->type == u->type){
            prev = courant;
        }
        courant = courant->usuiv;
    }    
    // Si prev est NULL, cela signifie qu'aucun élément de même type n'a été trouvé dans la liste
    if(prev == NULL){
        u->usuiv = *list;// le probleme avec (*list) ？？
        if(*list != NULL){
            (*list)->uprec =u;
        }
        *list =u;
        
    }else{
        u->usuiv = prev->usuiv;
        if(prev->usuiv !=NULL){
            prev->usuiv->uprec = u;
        }
        prev->usuiv =u;
        u->uprec =prev;
    }
}


//bouton
int button(int x,int y,int buttonX,int buttonY,int buttonWidth,int buttonHeight){
    return (x>=buttonX && x<=buttonX + buttonWidth && y>=buttonY && y <=buttonY + buttonHeight);
}

//Ordre d'action des abeilles
Unite* prochaineUniteAbeille(Grille *g){
    char types[]={RUCHE,REINE,GUERRIERE,OUVRIERE,ESCADRON};
    Unite *Point = (g->uniteEnaction == NULL)?g->abeille:g->uniteEnaction->usuiv;
    //Rechercher l'unité suivante du même type
    for (int indexType = 0 ;indexType <5 ; indexType++){
        for(Unite *courant = Point ; courant != NULL;courant = courant->usuiv){    
            if(courant->type == types[indexType]){      
                g->uniteEnaction = courant;     
                return courant;
            }
        }
    }
    return NULL;
    }

//Ordre d'action des frelons
Unite* prochaineUniteFrelon(Grille *g) {
    char types[] = {NID, REINE, TYFRELON};   
    Unite *Point = (g->uniteEnaction == NULL)?g->frelon:g->uniteEnaction->usuiv;
    //Rechercher l'unité suivante du même type
    for (int indexType = 0 ;indexType <3 ; indexType++){
        for(Unite *courant = Point ; courant != NULL;courant = courant->usuiv){    
            if(courant->type == types[indexType]){     
                g->uniteEnaction = courant;    
                return courant;
            }
        }
    }
    return NULL;
    }

//"Initialiser le plateau de jeu
void initialiserPlateau(Grille *g) {
    int i, j;
    for (j = 0; j < COLONNES; j++) {
        for (i = 0; i < LIGNES; i++) {
            g->plateau[j][i].colonie = NULL;
            g->plateau[j][i].occupant = NULL;
        }
    }
    g->abeille = NULL;
    g->frelon = NULL;
    g->tour = 1;
    g->ressourcesAbeille = 10;
    g->ressourcesFrelon = 10;
    g->campActuel = (rand()%2) == 0 ? ABEILLE : FRELON;
    g->uniteEnaction = NULL;
    g->fin = 0;
     //Créer et placer l'unité d'abeille
    Unite *ouvriere = createUnite(ABEILLE,OUVRIERE,FOUVRIERE);//Créer une ouvrière
    placerUnite(g,ouvriere,0,0);
    updateUnitLists(g,ouvriere);
    Unite *guerriere = createUnite(ABEILLE,GUERRIERE,FGUERRIERE);//Créer un guerrier
    placerUnite(g,guerriere,0,0);
    updateUnitLists(g,guerriere);
    Unite *guerriere1 = createUnite(ABEILLE,GUERRIERE,FGUERRIERE);//Créer un guerrier
    placerUnite(g,guerriere1,10,10);
    updateUnitLists(g,guerriere1);
    Unite *reineA = createUnite(ABEILLE,REINE,FREINE);//Créer un reine
    placerUnite(g,reineA,0,0);
    updateUnitLists(g,reineA);
    Unite *ruche = createUnite(ABEILLE, RUCHE,0);  // Créer une ruche
    placerUnite(g, ruche, 0, 0);                     // Placer une ruche à la position (0,0)
    updateUnitLists(g, ruche);
    Unite *frelonA = createUnite(FRELON,TYFRELON,FFRELON);//Créer un frelon
    placerUnite(g,frelonA,11,17);
    updateUnitLists(g,frelonA);
    Unite *frelonB = createUnite(FRELON,TYFRELON,FFRELON);
    placerUnite(g,frelonB,10,10);
    updateUnitLists(g,frelonB);  
    Unite *renineB = createUnite(FRELON,REINE,FREINE);//Créer une reine frelon
    placerUnite(g,renineB,11,17);
    updateUnitLists(g,renineB);
    Unite *nid = createUnite(FRELON,NID,0);//Créer un nid
    placerUnite(g,nid,11,17);
    updateUnitLists(g,nid); 
    g->uniteEnaction = g->campActuel == ABEILLE ? prochaineUniteAbeille(g) : prochaineUniteFrelon(g);
}

//Suppression des unités sélectionnées
void supprimer_unite(Grille *g,char type,int posx,int posy,Unite *u,int recolte){
    // Si l'unité est une ruche ou un nid, changer son camp et supprimer toutes les unités associées
    if(u->type == RUCHE || u->type == NID){
        u->camp =(u->camp ==ABEILLE)? FRELON : ABEILLE;
        Unite *courant=u->usuiv;
        while(courant != NULL){
            Unite *suivant = courant->usuiv;
            supprimer_unite(g,courant->type,courant->posx,courant->posy,courant,1);
            courant = suivant;
        }
        u->usuiv = NULL;
    }
     // Supprimer les références à l'unité dans différentes listes
    if(u->usuiv){
        u->usuiv->uprec = u->uprec;
    }
    if(u->uprec){
        u->uprec->usuiv = u->usuiv;
    }
    if(u->colsuiv){
        u->colsuiv->colprec=u->colprec;
    }
    if(u->colprec){
        u->colprec->colsuiv=u->colsuiv;
    }
    if(u->vsuiv){
        u->vsuiv->vprec=u->vprec;
    }
    if(u->vprec){
        u->vprec->vsuiv=u->vsuiv;
    }else if(u->vsuiv){
        g->plateau[posx][posy].occupant = u->vsuiv;
    }else {
        g->plateau[posx][posy].occupant = NULL;
    }
    // Supprimer l'unité de la liste globale correspondante
    UListe list = (u->camp == ABEILLE)?g->abeille:g->frelon;
    if(list == u){
        list = u->usuiv;
    }
    else {
        Unite *prev = list;
        while(prev!=NULL && prev->usuiv !=u){
            prev = prev->usuiv;
        }
        if(prev!=NULL){
            prev->usuiv = u->usuiv;
        }
    }
    // Libérer la mémoire occupée par l'unité
    free (u);
     // Si recolte est égal à 2, ajouter des ressources en fonction du type d'unité et du camp
    if (recolte == 2){
        if(u->camp == ABEILLE){
            switch(type){
                case REINE:
                g->ressourcesFrelon += CREINEA;
                break;
                case OUVRIERE:
                g->ressourcesFrelon += COUVRIERE;
                break;
                case GUERRIERE:
                g->ressourcesFrelon += CGUERRIERE;
                break;
                case ESCADRON:
                g->ressourcesFrelon += CESCADRON;
                break;
            }
        }
    }
}




// Traverse les cases aux coordonnées spécifiées, en vérifiant la présence d'unités dans la grille.
// g : le plateau de jeu
// posx, posy : coordonnées de la case du plateau
// 0 - il y a des unités d'un côté ou aucune des autres côtés dans la case
// 1 - il y a des unités de différents côtés dans la case
int look_campunit_incase(Grille *g, int posx, int posy) {
    int abeille = 0;
    int frelon = 0;
    
    UListe tmp = g->plateau[posx][posy].occupant;
    
    // Parcourir les occupants de la case
    for (; tmp; tmp = tmp->vsuiv) {
        if (tmp->camp == ABEILLE) {
            abeille += 1;
        } else if (tmp->camp == FRELON) {
            frelon += 1;
        }
    }

    // Vérifier s'il y a deux camps dans une case
    if (abeille > 0 && frelon > 0) {
        return 1; // Deux camps dans une case
    } else if ((abeille > 0 && frelon == 0 && g->plateau[posx][posy].colonie == NULL) || 
               (abeille == 0 && frelon > 0 && g->plateau[posx][posy].colonie == NULL)) {
        return 0; // Un seul camp dans la case
    } else if ((abeille > 0 && frelon == 0 && g->plateau[posx][posy].colonie) || 
               (abeille == 0 && frelon > 0 && g->plateau[posx][posy].colonie)) {
        int pos_x = posx;
        int pos_y = posy;

        // Vérifier s'il y a une colonie sur la case
        if (g->plateau[pos_x][pos_y].colonie) {
            // Vérifier la présence d'une colonie d'abeilles et d'un frelon dans la case
            if (g->plateau[pos_x][pos_y].colonie->camp == ABEILLE && g->plateau[pos_x][pos_y].occupant->camp == FRELON) {
                // Supprimer toutes les unités associées à la colonie d'abeilles
                while (g->plateau[pos_x][pos_y].colonie->usuiv) {
                    supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                                     g->plateau[pos_x][pos_y].colonie->usuiv->posx,
                                     g->plateau[pos_x][pos_y].colonie->usuiv->posy, g->plateau[pos_x][pos_y].colonie->usuiv, 2);
                }
                
                // Mettre à jour la colonie pour représenter un frelon
                g->plateau[pos_x][pos_y].colonie->camp = FRELON;
                g->plateau[pos_x][pos_y].colonie->type = NID;

                // Mettre à jour les liens de la colonie dans la liste des frelons
                if (g->plateau[pos_x][pos_y].colonie->colsuiv) {
                    g->plateau[pos_x][pos_y].colonie->colsuiv->colprec = g->plateau[pos_x][pos_y].colonie->colprec;
                }
                if (g->plateau[pos_x][pos_y].colonie->colprec) {
                    g->plateau[pos_x][pos_y].colonie->colprec->colsuiv = g->plateau[pos_x][pos_y].colonie->colsuiv;
                }

                // Mettre à jour les liens de la colonie dans la liste des frelons
                g->plateau[pos_x][pos_y].colonie->colsuiv = g->frelon->colsuiv;
                g->plateau[pos_x][pos_y].colonie->colprec = g->frelon;
                g->frelon->colsuiv = g->plateau[pos_x][pos_y].colonie;
            } else if (g->plateau[pos_x][pos_y].colonie->camp == FRELON && g->plateau[pos_x][pos_y].occupant->camp == ABEILLE) {
                // Supprimer toutes les unités associées à la colonie de frelons
                while (g->plateau[pos_x][pos_y].colonie->usuiv) {
                    supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                                     g->plateau[pos_x][pos_y].colonie->usuiv->posx,
                                     g->plateau[pos_x][pos_y].colonie->usuiv->posy,
                                     g->plateau[pos_x][pos_y].colonie->usuiv, 2);
                }
                
                // Supprimer la colonie de frelons
                supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->type, g->plateau[pos_x][pos_y].colonie->posx,
                                 g->plateau[pos_x][pos_y].colonie->posy, g->plateau[pos_x][pos_y].colonie, 0);
            }
        }
        return 0;
    }

    return 0;
}


// Fonction pour les combats sur une case 
void war_combat(Grille *g, int pos_x, int pos_y) {
    int random1;
    int random2;
    
    // Parcourir tous les unités sur la case
    Unite *tmp = g->plateau[pos_x][pos_y].occupant;
    Unite *adver = g->plateau[pos_x][pos_y].occupant;

    // Effectuer les combats tant que des unités de camps différents sont présentes
    while (look_campunit_incase(g, pos_x, pos_y)) {
        if (adver->camp == tmp->camp) {
            adver = adver->vsuiv;
            continue;
        } else {
            if (tmp->toursrestant > 0) {
                tmp = tmp->vsuiv;
                continue;
            }
            if (adver->toursrestant > 0) {
                adver = adver->vsuiv;
                continue;
            }
            srand((unsigned)time(NULL));
            random1 = rand() % 61;
            random2 = rand() % 61;

            // Comparaison de la force et de nombres aléatoires pour déterminer le résultat du combat
            if ((tmp->force * random1) > adver->force * random2) {
                int recolte = 0;
                if (adver->camp == ABEILLE) {
                    recolte = 2;
                }
                Unite *deladver = adver;
                adver = adver->vsuiv;
                supprimer_unite(g, deladver->type, deladver->posx, deladver->posy, deladver, recolte);
            } else if ((tmp->force * random1) < adver->force * random2) {
                int recolte = 0;
                if (tmp->camp == ABEILLE) {
                    recolte = 2;
                }
                Unite *deladver = tmp;
                tmp = tmp->vsuiv;
                supprimer_unite(g, deladver->type, deladver->posx, deladver->posy, deladver, recolte);
            }
        }
    }

    // Traitement des changements de ruche après le combat
    if (g->plateau[pos_x][pos_y].colonie) {
        if (g->plateau[pos_x][pos_y].colonie->camp == ABEILLE && g->plateau[pos_x][pos_y].occupant->camp == FRELON) {
            // Transformer toutes les unités associées à la ruche en unités de frelons
            while (g->plateau[pos_x][pos_y].colonie->usuiv) {
                supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                                g->plateau[pos_x][pos_y].colonie->usuiv->posx,
                                g->plateau[pos_x][pos_y].colonie->usuiv->posy,
                                g->plateau[pos_x][pos_y].colonie->usuiv, 2);
            }

            // Modifier les attributs de la ruche pour devenir un nid de frelons et ajuster les pointeurs de la liste
            g->plateau[pos_x][pos_y].colonie->camp = FRELON;
            g->plateau[pos_x][pos_y].colonie->type = NID;
            if (g->plateau[pos_x][pos_y].colonie->colsuiv) {
                g->plateau[pos_x][pos_y].colonie->colsuiv->colprec = g->plateau[pos_x][pos_y].colonie->colprec;
            }
            if (g->plateau[pos_x][pos_y].colonie->colprec) {
                g->plateau[pos_x][pos_y].colonie->colprec->colsuiv = g->plateau[pos_x][pos_y].colonie->colsuiv;
            }
            g->plateau[pos_x][pos_y].colonie->colsuiv = g->frelon->colsuiv;
            g->plateau[pos_x][pos_y].colonie->colprec = g->frelon;
            g->frelon->colsuiv = g->plateau[pos_x][pos_y].colonie;
        } else if (g->plateau[pos_x][pos_y].colonie->camp == FRELON &&
                   g->plateau[pos_x][pos_y].occupant->camp == ABEILLE) {
            // Si la ruche est un nid de frelons mais qu'il y a des abeilles sur la case, détruire la ruche et ses unités associées
            while (g->plateau[pos_x][pos_y].colonie->usuiv) {
                supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                                g->plateau[pos_x][pos_y].colonie->usuiv->posx,
                                g->plateau[pos_x][pos_y].colonie->usuiv->posy,
                                g->plateau[pos_x][pos_y].colonie->usuiv, 2);
            }
            // Supprimer la ruche
            supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->type,
                             g->plateau[pos_x][pos_y].colonie->posx,
                             g->plateau[pos_x][pos_y].colonie->posy,
                             g->plateau[pos_x][pos_y].colonie, 0);
        }
    }
}


//Préparation d'un nouveau tour
void debutTour(Grille *g){
    Unite *courant;
    //Collecter toutes les ouvrières collectrices ; si le nombre de tours est écoulé, elles meurent
    for(courant=g->abeille;courant !=NULL;courant=courant->usuiv){
        if(courant->type == OUVRIERE && courant->toursrestant !=0){
            g->ressourcesAbeille += 1;
            courant->toursrestant -= 1;
            if(courant->toursrestant == 0){
                supprimer_unite(g,courant->type,courant->posx,courant->posy,courant,0);
            }
        }
    }
     // Gérer la production des ruches et les reines abeilles
    for (courant=g->abeille;courant != NULL;courant=courant->usuiv){
        if(courant->type == RUCHE && courant->toursrestant != 0 ){
           courant->toursrestant -= 1;
           if(courant->toursrestant == 0){
             Unite *production = createUnite(ABEILLE,courant->production,force(courant->production));
             placerUnite(g,production,courant->posx,courant->posy);
             updateUnitLists(g,production);
           }
        }
        if (courant->type == REINE && courant->toursrestant != 0 ){
            courant->toursrestant -=1;
            if(courant->toursrestant ==0){
                Unite *production = createUnite(ABEILLE,courant->production,0);
                placerUnite(g,production,courant->posx,courant->posy);
                updateUnitLists(g,production);
            }
        }
    }
    // Gérer la production des nids de frelons et les reines frelons
    for (courant=g->frelon;courant != NULL;courant=courant->usuiv){
        if(courant->type == NID && courant->toursrestant != 0 ){
           courant->toursrestant -= 1;
           if(courant->toursrestant == 0){
             Unite *production = createUnite(FRELON,courant->production,force(courant->production));
             placerUnite(g,production,courant->posx,courant->posy);
             updateUnitLists(g,production);
            
           }
        }
        if (courant->type == REINE && courant->toursrestant != 0 ){
            courant->toursrestant -=1;
            if(courant->toursrestant ==0){
                Unite *production = createUnite(FRELON,courant->production,0);
                placerUnite(g,production,courant->posx,courant->posy);
                updateUnitLists(g,production);
            }
        }
    }
// le combat(pas reussi)
   // for(int i=0;i<LIGNES;++i){
   //    for(int j=0;j<COLONNES;++j){
   //     if(look_campunit_incase(g,i,j)==0){
   //         continue;
   //     }else{
   //         war_combat(g,i,j);
   //     }
   //    }
   // }
    g->campActuel = (rand()%2) == 0 ? ABEILLE : FRELON;   
    g->tour++;
    g->fin = 0;
    g->uniteEnaction = NULL;
    g->uniteEnaction = g->campActuel == ABEILLE ? prochaineUniteAbeille(g) : prochaineUniteFrelon(g);
}


//Afficher le plateau de jeu
void afficherPlateau(Grille *g) {
    int i, j;
    for (i = 0; i < LIGNES; i++) {
        for (j = 0; j < COLONNES; j++) {
            int x = j*60;
            int y = i*60;
            //Sélection de la zone en fonction du type d'unité et de la position
            int width =0,height = 0;
            int offsetX=0,offsetY=0;
            //Dessiner l'arrière-plan des cases
            MLV_draw_filled_rectangle(x,y,60,60,MLV_COLOR_GREY);
            //Obtenir l'unité actuelle de la case
            Unite *unit = g->plateau[j][i].occupant;
            //Dessiner en fonction du type
            while(unit!=NULL){
                MLV_Color color = MLV_COLOR_WHITE;//Couleur par défaut
                if(unit->camp == ABEILLE){
                    //ABEILLE
                    if (unit->type == REINE){
                        offsetX = 0,offsetY = 0;
                        color = MLV_COLOR_RED;
                        width = 30;
                        height = 20;
                    }
                    else if(unit->type == OUVRIERE){
                        offsetX = 0,offsetY = 20;
                        color = MLV_COLOR_ORANGE;
                        width = 20;
                        height = 20;
                    }
                    else if(unit->type == RUCHE){
                        offsetX = 20,offsetY = 20;
                        color = MLV_COLOR_YELLOW;
                        width = 20;
                        height = 20;
                    }
                    else if(unit->type == GUERRIERE){
                        offsetX = 0,offsetY = 40;
                        color = MLV_COLOR_GREEN;
                        width = 30;
                        height = 20;
                    }
                    else if(unit->type == ESCADRON){
                        offsetX = 30, offsetY = 40;
                        color = MLV_COLOR_TAN;
                        width = 30;
                        height = 20;
                    }
                } else if(unit->camp == FRELON){
                    //FRELON
                    if (unit ->type == REINE){
                        offsetX = 30,offsetY = 0;
                        color = MLV_COLOR_BLUE;
                        width = 30;
                        height = 20;
                    }
                    else if(unit->type == NID){
                        offsetX = 20,offsetY = 20;
                        color = MLV_COLOR_PURPLE;
                        width = 20;
                        height = 20;
                    }
                    else if(unit->type == TYFRELON){
                        offsetX = 40,offsetY = 20;
                        color = MLV_COLOR_WHITE;
                        width = 20;
                        height = 20;
                    }
                }
                //Dessiner l'unité
                if (unit == g->uniteEnaction){
                    //Mettre en surbrillance l'unité d'action actuelle
                    color = MLV_COLOR_BLACK;
                }
                MLV_draw_filled_rectangle(x+offsetX,y+offsetY,width,height,color);
                unit = unit->vsuiv;
            }
            //Dessiner la bordure
            MLV_draw_rectangle(x,y,60,60,MLV_COLOR_BLACK);
        }
    }
}

//Catalogue d'actions
void Ordre(Unite *unit){
    //Dessiner l'arrière-plan du ordre
    MLV_draw_filled_rectangle(720,0,900,1080,MLV_COLOR_GREY);
    //Unité en cours d'action
    char Text[50];
    sprintf(Text,"Unit %c : x= %d, y=%d",unit->type,unit->posx,unit->posy);
    MLV_draw_text(750,30,Text,MLV_COLOR_BLACK);
    if (unit->type == TYFRELON || unit->type == GUERRIERE || unit->type == ESCADRON){
        //deplacement
        MLV_draw_filled_rectangle(720,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(720,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(780,90,"Deplcaement",MLV_COLOR_BLACK);
        //detruire
        MLV_draw_filled_rectangle(1080,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1080,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1140,90,"Detruire",MLV_COLOR_BLACK);
        //passer
        MLV_draw_filled_rectangle(1260,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1260,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1320,90,"Passer",MLV_COLOR_BLACK);
    }
    else if(unit->type == REINE){
        MLV_draw_filled_rectangle(720,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(720,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(780,90,"Deplcaement",MLV_COLOR_BLACK);
        //produire la ruche
        MLV_draw_filled_rectangle(900,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(900,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(960,90,"Produire",MLV_COLOR_BLACK);
        MLV_draw_filled_rectangle(1080,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1080,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1140,90,"Detruire",MLV_COLOR_BLACK);
        MLV_draw_filled_rectangle(1260,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1260,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1320,90,"Passer",MLV_COLOR_BLACK);
    }
    else if(unit->type == RUCHE || unit->type == NID){
        if(unit->toursrestant == 0){
        MLV_draw_filled_rectangle(900,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(900,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(960,90,"Produire",MLV_COLOR_BLACK);
        MLV_draw_filled_rectangle(1260,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1260,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1320,90,"Passer",MLV_COLOR_BLACK);
        }
        else {
            //Si il en cours de produire
            MLV_draw_text(900,30,"En cours de produire",MLV_COLOR_BLACK);
            MLV_draw_filled_rectangle(1260,60,180,60,MLV_COLOR_RED);
            MLV_draw_rectangle(1260,60,180,60,MLV_COLOR_BLACK);
            MLV_draw_text(1320,90,"Passer",MLV_COLOR_BLACK);
        }
    }
    else if(unit->type == OUVRIERE){
        if(unit->toursrestant == 0 ){
        MLV_draw_filled_rectangle(720,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(720,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(780,90,"Deplcaement",MLV_COLOR_BLACK);
        //recolter
        MLV_draw_filled_rectangle(1440,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1440,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1500,90,"Recolter",MLV_COLOR_BLACK);
        MLV_draw_filled_rectangle(1080,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1080,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1140,90,"Detruire",MLV_COLOR_BLACK);
        MLV_draw_filled_rectangle(1260,60,180,60,MLV_COLOR_RED);
        MLV_draw_rectangle(1260,60,180,60,MLV_COLOR_BLACK);
        MLV_draw_text(1320,90,"Passer",MLV_COLOR_BLACK);
        }
        else {
            //Si il en cours de recolter
            MLV_draw_text(900,30,"En cours de recolter",MLV_COLOR_BLACK);
            MLV_draw_filled_rectangle(1260,60,180,60,MLV_COLOR_RED);
            MLV_draw_rectangle(1260,60,180,60,MLV_COLOR_BLACK);
            MLV_draw_text(1320,90,"Passer",MLV_COLOR_BLACK);
        }
    }
}

//Menu
void Menu(Grille *g){
    //Bouton de sauvegarde
    MLV_draw_filled_rectangle(720,300,180,60,MLV_COLOR_BLUE);
    MLV_draw_rectangle(720,300,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(780,330,"Sauve",MLV_COLOR_BLACK);
    //pas reussi
    MLV_draw_filled_rectangle(900,300,180,60,MLV_COLOR_BLUE);
    MLV_draw_rectangle(900,300,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(960,330,"Charge",MLV_COLOR_BLACK);
    //Quitter
    MLV_draw_filled_rectangle(1080,300,180,60,MLV_COLOR_BLUE);
    MLV_draw_rectangle(1080,300,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1140,330,"Quitter",MLV_COLOR_BLACK);
    //Fin de tour
    MLV_draw_filled_rectangle(1260,300,180,60,MLV_COLOR_BLUE);
    MLV_draw_rectangle(1260,300,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1320,330,"Fin de tour",MLV_COLOR_BLACK);
    //Afficher les informations de la faction en cours d'action
    char info[100];
    sprintf(info,"Tour : %d, Camp: %c, Ressources: %d",g->tour,g->campActuel,g->campActuel == ABEILLE ? g->ressourcesAbeille : g->ressourcesFrelon);
    MLV_draw_text(730,180,info,MLV_COLOR_BLACK);
    UListe courant = (g->campActuel == ABEILLE)?g->abeille:g->frelon;
    int y_offset = 600;
    while(courant != NULL){
        if(courant->toursrestant > 0 ){
            char production[100];
            sprintf(production,"En cours de produire %c dans (%d,%d),il reste %d tours",courant->production,courant->posx,courant->posy,courant->toursrestant);
            MLV_draw_text(720,y_offset,production,MLV_COLOR_BLACK);
            y_offset += 20;
        }
        courant = courant->usuiv;
    }
}


int finDetour(Grille *g) {
    //Nouveau tour lorsque les deux parties ont pris des actions
    if (g->fin == 1){
        debutTour(g);
    }
    //Changer de camp active
    else if (g->campActuel == ABEILLE) {
        g->campActuel = FRELON;
        g->uniteEnaction = NULL;
        g->uniteEnaction = prochaineUniteFrelon(g);
        g->fin = 1;
    } else {
        g->campActuel = ABEILLE;
        g->uniteEnaction = NULL;
        g->uniteEnaction =prochaineUniteAbeille(g);
        g->fin = 1;
    }
    return 1;
}

//Deplacement
int deplacement(Grille *g,Unite *unit){
    MLV_draw_filled_rectangle(720,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(720,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(780,390,"Nord",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(900,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(900,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(960,390,"Nord-Est",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(1080,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(1080,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1140,390,"Est",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(1260,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(1260,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1320,390,"Sud-Est",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(720,420,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(720,420,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(780,450,"Sud",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(900,420,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(900,420,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(960,450,"Sud-Ouest",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(1080,420,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(1080,420,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1140,450,"Ouest",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(1260,420,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(1260,420,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1320,450,"Nord-Ouest",MLV_COLOR_BLACK);
    MLV_update_window();
    int mouseX= 0 ,mouseY = 0;
    MLV_wait_mouse(&mouseX, &mouseY);
    //Nord
       if (button(mouseX,mouseY,720,360,180,60)){
            unit->destx=unit->posx;
            unit->desty=unit->posy-1;
        }
        //Nord-Est
        if (button(mouseX,mouseY,900,360,180,60)){
            unit->destx=unit->posx+1;
            unit->desty=unit->posy-1;
        }
        //Est
        if(button(mouseX,mouseY,1080,360,180,60)){
            unit->destx=unit->posx+1;
            unit->desty=unit->posy;
        }
        //Sud-Est
        if(button(mouseX,mouseY,1260,360,180,60)){
            unit->destx=unit->posx+1;
            unit->desty=unit->posy+1;
        }
        //Sud
        if(button(mouseX,mouseY,720,420,180,60)){
            unit->destx=unit->posx;
            unit->desty=unit->posy+1;
        }
        //Sud-Ouest
        if(button(mouseX,mouseY,900,420,180,60)){
            unit->destx=unit->posx-1;
            unit->desty=unit->posy+1;
        }
        //Ouest
        if(button(mouseX,mouseY,1080,420,180,60)){
            unit->destx=unit->posx-1;
            unit->desty=unit->posy;
        }
        //Nord-Ouest
        if(button(mouseX,mouseY,1260,420,180,60)){
            unit->destx=unit->posx-1;
            unit->desty=unit->posy-1;
        }
    //Dans le plateau
    if((unit->destx<0||unit->destx>=COLONNES)||(unit->desty<0||unit->desty>=LIGNES)){
        unit->destx=-1;
        unit->desty=-1;
        return 1;
    }
    //Modifier la liste de chaine V
    else {
         if(unit->vsuiv != NULL){
            unit->vsuiv->vprec=unit->vprec;
         }
         if(unit->vprec != NULL){
            unit->vprec->vsuiv=unit->vsuiv;
         }
        if(g->plateau[unit->posx][unit->posy].occupant == unit){
            g->plateau[unit->posx][unit->posy].occupant = unit->vsuiv;
        }
        unit->vprec=NULL;
        unit->vsuiv = g->plateau[unit->destx][unit->desty].occupant;
        if(g->plateau[unit->destx][unit->desty].occupant !=NULL){
            g->plateau[unit->destx][unit->desty].occupant->vprec =unit;
        }
        g->plateau[unit->destx][unit->desty].occupant=unit;
        //Modifier la position
         unit->posx = unit->destx;
         unit->posy=unit->desty;
         return 0;
    }
}

//La production de ruche
int produit_ruche(Grille *g,Unite *unit){
    MLV_draw_filled_rectangle(720,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(720,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(780,390,"Reine",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(900,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(900,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(960,390,"Ouvriere",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(1080,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(1080,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1140,390,"Guerriere",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(1260,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(1260,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(1320,390,"Escadron",MLV_COLOR_BLACK);
    MLV_update_window();
    int mouseX= 0 ,mouseY = 0;
    MLV_wait_mouse(&mouseX, &mouseY);
    if(button(mouseX,mouseY,720,360,180,60)){
        //Produire le reine
        if (g->ressourcesAbeille < CREINEA){
            MLV_draw_text(730,450,"Pas de ressources",MLV_COLOR_BLACK);
            MLV_update_window();
            return 1;
        }
        else {
            g->ressourcesAbeille -= CREINEA;
            unit->production = REINE;
            unit->toursrestant = TREINEA;
        }
    }
    if(button(mouseX,mouseY,900,360,180,60)){
        //Produire le ouvriere
        if (g->ressourcesAbeille < COUVRIERE){
            MLV_draw_text(730,450,"Pas de ressources",MLV_COLOR_BLACK);
            MLV_update_window();
            return 1;
        }
        else{
            g->ressourcesAbeille -= COUVRIERE;
            unit->production = OUVRIERE;
            unit->toursrestant = TOUVRIERE;
        }
    }
    if(button(mouseX,mouseY,1080,360,180,60)){
        //Produire le guerriere
        if (g->ressourcesAbeille < CGUERRIERE){
            MLV_draw_text(730,450,"Pas de ressources",MLV_COLOR_BLACK);
            MLV_update_window();
            return 1;
        }
        else {
            g->ressourcesAbeille -= CGUERRIERE;
            unit->production = GUERRIERE;
            unit->toursrestant = TGUERRIERE;
        }
    }
    if(button(mouseX,mouseY,1260,360,180,60)){
        //Produire le escadron
        if (g->ressourcesAbeille < CESCADRON){
            MLV_draw_text(730,450,"Pas de ressources",MLV_COLOR_BLACK);
            MLV_update_window();
            return 1;
        }
        else {
            g->ressourcesAbeille -=CESCADRON;
            unit->production = ESCADRON;
            unit->toursrestant = TESCADRON;
        }
    }
    return 0;
}

//La production de ruche
int produit_nid(Grille *g,Unite *unit){
    MLV_draw_filled_rectangle(720,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(720,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(780,390,"Reine",MLV_COLOR_BLACK);
    MLV_draw_filled_rectangle(900,360,180,60,MLV_COLOR_RED);
    MLV_draw_rectangle(900,360,180,60,MLV_COLOR_BLACK);
    MLV_draw_text(960,390,"Frelon",MLV_COLOR_BLACK);
    MLV_update_window();
    int mouseX= 0 ,mouseY = 0;
    MLV_wait_mouse(&mouseX, &mouseY);
    if(button(mouseX,mouseY,720,360,180,60)){
        //Produire le reine
        if (g->ressourcesFrelon < CREINEF){
            MLV_draw_text(730,450,"Pas de ressources",MLV_COLOR_BLACK);
            MLV_update_window();
            return 1;
        }
        else {
            g->ressourcesFrelon -=CREINEF;
            unit->production = REINE;
            unit->toursrestant = TREINEF;
        }
    }
    if(button(mouseX,mouseY,900,360,180,60)){
        //Produire le frelon
        if (g->ressourcesAbeille < CFRELON){
            MLV_draw_text(730,450,"Pas de ressources",MLV_COLOR_BLACK);
            MLV_update_window();
            return 1;
        }
        else {
            g->ressourcesFrelon -=CFRELON;
            unit->production = TYFRELON;
            unit->toursrestant = TFRELON;
        }
    }
    return 0;
}

//Produire la ruche ou le nid
void produit_reine(Grille *g,Unite *u){
    if(g->campActuel==ABEILLE){
        g->ressourcesAbeille -= CRUCHE;
        u->production = RUCHE;
        u->toursrestant = 1;
    }
    else{
        g->ressourcesFrelon -= CNID;
        u->production =NID;
        u->toursrestant =1;
    }
}

//Recolter par ouvriere
void ouvriere_recolte(Grille *g,Unite *unit){
    unit->production = RECOLTE;
    unit->temps = TRECOLTE;
    unit->toursrestant = TRECOLTE;
}



// Fonction pour sauvegarder l'état du jeu dans un fichier
void save_game(Grille *g, char camp) {
    FILE *file = fopen("projet_final_save", "w");
    int resA = 0, resB = 0;

    // Déterminer la quantité de ressources en fonction du camp actuel
    if (camp == ABEILLE) {
        resA = g->ressourcesAbeille;
        resB = g->ressourcesFrelon;
    } else if (camp == FRELON) {
        resA = g->ressourcesFrelon;
        resB = g->ressourcesAbeille;
    }

    // Écrire les informations sur les ressources dans le fichier
    fprintf(file, "%c %d %d\n", camp, resA, resB);

    UListe colon_c = g->abeille->colsuiv;
    UListe unit_c = NULL;

    // Parcourir les nids et les unités associées du camp des abeilles, puis écrire dans le fichier
    while (colon_c) {
        fprintf(file, "%c %c %d %d %c %d\n", colon_c->camp, colon_c->type, colon_c->posx, colon_c->posy, colon_c->production, colon_c->toursrestant);

        unit_c = colon_c->usuiv;
        while (unit_c) {
            fprintf(file, "%c %c %d %d %c %d\n", unit_c->camp, unit_c->type, unit_c->posx, unit_c->posy, unit_c->production, unit_c->toursrestant);
            unit_c = unit_c->usuiv;
        }

        colon_c = colon_c->colsuiv;
    }

    // Parcourir les nids et les unités associées du camp des frelons, puis écrire dans le fichier
    colon_c = g->frelon->colsuiv;
    while (colon_c) {
        fprintf(file, "%c %c %d %d %c %d\n", colon_c->camp, colon_c->type, colon_c->posx, colon_c->posy, colon_c->production, colon_c->toursrestant);

        unit_c = colon_c->usuiv;
        while (unit_c) {
            fprintf(file, "%c %c %d %d %c %d\n", unit_c->camp, unit_c->type, unit_c->posx, unit_c->posy, unit_c->production, unit_c->toursrestant);
            unit_c = unit_c->usuiv;
        }

        colon_c = colon_c->colsuiv;
    }

    // Fermer le fichier
    fclose(file);
}



//Verifier le bouton
int Verifier(int mouseX,int mouseY,Grille *g,Unite *u){
     if (button(mouseX,mouseY,1080,300,180,60)){ //quitter
        return 0;
     }
     if (button(mouseX,mouseY,1260,60,180,60)){//passer
        if (g->campActuel == ABEILLE) {
            prochaineUniteAbeille(g);
        } else if (g->campActuel == FRELON) {
            prochaineUniteFrelon(g);
        }
     }
     if (button(mouseX,mouseY,1260,300,180,60)){//fin de tour
        finDetour(g);
     }
     if(button(mouseX,mouseY,720,60,180,60)){//Deplacement
        int i = 1 ;
        while (i==1){
            i = deplacement(g,u);
        }
        if (g->campActuel == ABEILLE) {
            prochaineUniteAbeille(g);
        } else if (g->campActuel == FRELON) {
            prochaineUniteFrelon(g);
        }
     }
     if(button(mouseX,mouseY,1080,60,180,60)){ // detuire
        supprimer_unite(g,u->type,u->posx,u->posy,u,0);
        if (g->campActuel == ABEILLE) {
            prochaineUniteAbeille(g);
        } else if (g->campActuel == FRELON) {
            prochaineUniteFrelon(g);
        }
     }
     if(button(mouseX,mouseY,1440,60,180,60)){ // recolter par ouvriere
        ouvriere_recolte(g,u);
        if (g->campActuel == ABEILLE) {
            prochaineUniteAbeille(g);
        } else if (g->campActuel == FRELON) {
            prochaineUniteFrelon(g);
        }
     }
     if(g->uniteEnaction->type == RUCHE){ // produire les insectes de abeille
        if(button(mouseX,mouseY,900,60,180,60)){
            int i = 1;
            while (i == 1)
            {
                i = produit_ruche(g,u);
            }        
        
        if (g->campActuel == ABEILLE) {
            prochaineUniteAbeille(g);
        } else if (g->campActuel == FRELON) {
            prochaineUniteFrelon(g);
        }
        }
     }
     if(g->uniteEnaction->type == NID){ // produire les insectes de frelon
        if(button(mouseX,mouseY,900,60,180,60)){
            int i = 1;
            while (i == 1)
            {
                i = produit_nid(g,u);
            }        
        
        if (g->campActuel == ABEILLE) {
            prochaineUniteAbeille(g);
        } else if (g->campActuel == FRELON) {
            prochaineUniteFrelon(g);
        }
        }
     }
     // Produire la ruche ou le nid
    if (g->uniteEnaction->type == REINE){
        if(button(mouseX,mouseY,900,60,180,60)){
            if(g->plateau[u->posx][u->posy].colonie == NULL){
                if ((g->campActuel==ABEILLE && g->ressourcesAbeille >= CRUCHE)||(g->campActuel == FRELON && g->ressourcesFrelon>=CNID)){
                produit_reine(g,u);            
                if (g->campActuel == ABEILLE) {
                    prochaineUniteAbeille(g);
                } else if (g->campActuel == FRELON) {
                    prochaineUniteFrelon(g);
                }
                }
            }
        }
    }
    // Sauvegarder
    if (button(mouseX,mouseY,720,300,180,60)){
        save_game(g,g->campActuel);
    }
    return 1;
}


//Conditions de victoire pour la fin du jeu
int gagner_ou_pas(Grille *g){
    int abeille = 0;
    int frelon = 0;
    for (int i = 0; i < COLONNES; i++)
    {
        for (int j = 0; j < LIGNES; j++)
        {
            if (g->plateau[i][j].colonie && g->plateau[i][j].colonie->camp == ABEILLE)
            {
                abeille += 1;
            }
            if (g->plateau[i][j].colonie && g->plateau[i][j].colonie->camp == FRELON)
            {
                frelon += 1;
            }
        }
    }
    if (abeille == 0)
    {
        printf("Les frelons ont gagné.\n");
        return 0;
    }
    if (frelon == 0)
    {
        printf("Les abeilles ont gagné.\n");
        return 0; 
    }
    return 1;
}


//Libérer de l'espace
void freeList(UListe list){
    Unite *courant = list;
    while(courant !=NULL){
        Unite *next = courant->usuiv;
        free(courant);
        courant = next;
    }
}





/*===========================main===============================*/


/*Fonction main pour l'essai des terminaux


int main(int argc, char *argv[]){

    FILE *newfile;
    Grille *g = malloc(sizeof(Grille));
    srand(time(NULL));


//直接开始游戏不读取游戏的情况
    if (argc == 1)
    {
    //显示棋盘
    printf("welcome, game start !!!\n");
    initialiserPlateau(g);
    afficherPlateau(g);


    g ->tour = 1;
    int result = gagner_ou_pas(g);
    int save_or_no;


    while (gagner_ou_pas(g) == 0)
    {
        //显示游戏菜单和选项
        printf("==========Maintenant, c'est tour %d==========\n",g->tour);
        int nombre = rand()%2;
        printf("le random nombre est %d\n",nombre);
        clean_Buffer();

        if (nombre == 0)
        {  
            char camp = ABEILLE;
            grille_change(g);
            printf("tour de abeille!!!\n");
            printf("Pollen reste : %d\n",g->ressourcesAbeille);
            printf("pouvez vous souhaiter save??? Y:1,N:0\n");
            scanf("%d",&save_or_no);
            if (save_or_no == 1)
            {
                save_game(g,ABEILLE);
                printf("reussir a save dans project_final_save!!!\n");
                return 0;
            }

            gamestart_et_choix(g,camp);
            if(gagner_ou_pas(g) != 0){
                break;
            }
            grille_change(g);
            printf("tour de frelon!!!\n");
            printf("resource reste : %d\n",g->ressourcesFrelon);
            gamestart_et_choix(g,FRELON);
            look_case(g);
            if (gagner_ou_pas(g)!=0)
            {
                break;
            }
            
        }else{
            grille_change(g);
            char camp = FRELON;
            printf("tour de frelon!!!\n");
            printf("ressource reste : %d\n",g->ressourcesAbeille);
            printf("pouvez vous souhaiter save??? Y:1,N:0\n");
            scanf("%d",&save_or_no);
            if (save_or_no == 1)
            {
                save_game(g,camp);
                printf("reussir a save dans project_final!!!\n");
                return 0;
            }

            gamestart_et_choix(g,camp);
            if(gagner_ou_pas(g) != 0){
                break;
            }
            grille_change(g);
            printf("tour de abeille!!!\n");
            printf("pollen reste : %d\n",g->ressourcesAbeille);
            gamestart_et_choix(g,ABEILLE);

            look_case(g);
            if (gagner_ou_pas(g)!=0)
            {
                break;
            }
        }
        
        g->tour++;

    }
    return 0;
    }
    
     if (argc == 2)
     {
        printf("difficile!\n");
     }
     
    
}

*/

int main(int argc,char *argv[]){
    srand(time(NULL));
    Grille g;
    initialiserPlateau(&g);
    MLV_create_window("Game",NULL,1620,1080);
    int function = 1;
    int gagner = 1;
    int mouseX= 0 ,mouseY = 0;
    //Répéter jusqu'à la fin
    while (function && gagner){
    afficherPlateau(&g);
    Ordre(g.uniteEnaction);
    Menu(&g);
    MLV_actualise_window();
    MLV_Event event = MLV_get_event(NULL,NULL,NULL,NULL,NULL,&mouseX,&mouseY,NULL,NULL);
    if (event == MLV_MOUSE_BUTTON){
      function = Verifier(mouseX,mouseY,&g,g.uniteEnaction);
    }
    gagner = gagner_ou_pas(&g);
    MLV_update_window();
    }
    freeList(g.abeille);
    freeList(g.frelon);
    MLV_free_window();
    return 0 ;
}



