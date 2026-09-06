

/*Il s'agit d'une version test pour les terminaux et non de la version finale.*/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//  Dimensions  de  la  grille  en  nombre  de  cases  (origine  en  haut  a  gauche)  :
#define  COLONNES  10
#define  LIGNES  10

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
Case  plateau[LIGNES][COLONNES];
UListe  abeille,  frelon;
int  tour;  //  Numero  du  tour
int  ressourcesAbeille,   ressourcesFrelon;
char campActuel;
Unite *uniteEnaction;
}  Grille;


/*=====================================开始定义棋盘==============================================*/


// Créer une nouvelle unité avec les paramètres spécifiés
Unite *createUnite(char camp, char type, int force) {
    Unite *newUnit = (Unite *)malloc(sizeof(Unite));
    
    // Vérifier si l'allocation de mémoire a réussi
    if (newUnit == NULL) {
        // Gérer le cas d'échec d'allocation de mémoire
        return NULL;
    }
    
    // Initialiser les attributs de l'unité
    newUnit->camp = camp;
    newUnit->type = type;
    newUnit->force = force;
    newUnit->production = ' ';
    newUnit->temps = 0;
    newUnit->toursrestant = 0;
    
    // Initialiser les positions et les destinations
    newUnit->posx = newUnit->posy = -1;
    newUnit->destx = newUnit->desty = -1;
    
    // Initialiser les pointeurs de la liste chaînée
    newUnit->usuiv = newUnit->uprec = NULL;
    newUnit->colsuiv = newUnit->colprec = NULL;
    newUnit->vsuiv = newUnit->vprec = NULL;
    
    return newUnit;
}


void placerUnite(Grille *g, Unite *u, int x, int y) {
    // Vérifier si les coordonnées sont valides
    if (x < 0 || x >= LIGNES || y < 0 || y >= COLONNES) {
        printf("Position invalide.\n");
        return;
    }
    
    // Mettre à jour les informations de position de l'unité
    u->posx = x;
    u->posy = y;
    
    // Si cette position a déjà une unité, insérer la nouvelle unité au début de la liste chaînée
    if (g->plateau[x][y].occupant != NULL) {
        u->vsuiv = g->plateau[x][y].occupant;
        g->plateau[x][y].occupant->vprec = u;
    }
    g->plateau[x][y].occupant = u;
    
    // Gérer spécifiquement les cas de ruche ou nid
    if (u->type == RUCHE || u->type == NID) {
        if (g->plateau[x][y].colonie != NULL) {
            // Si une ruche ou un nid existe déjà, l'emplacement n'est pas autorisé
            printf("Il existe déjà une Ruche ou un Nid.\n");
            return;
        }
        g->plateau[x][y].colonie = u;
    }
}



void updateUnitLists(Grille *g, Unite *u) {
    int x = u->posx;
    int y = u->posy;
    // 更新全局abeille和frelon链表
    if (u->camp == ABEILLE) {
        if (g->abeille != NULL) {
            g->abeille->vprec = u;
        }
        u->vsuiv = g->abeille;
        g->abeille = u;
    } else if (u->camp == FRELON) {
        if (g->frelon != NULL) {
            g->frelon->vprec = u;
        }
        u->vsuiv = g->frelon;
        g->frelon = u;
    }
    // 更新colsuiv和colprec链表
    if (u->type == RUCHE || u->type == NID) {
        // 相应阵营的蜂巢或巢穴链表
        UListe *colList = (u->camp == ABEILLE) ? &g->abeille : &g->frelon;
        if (*colList != NULL) {
            (*colList)->colprec = u;
        }
        u->colsuiv = *colList;
        *colList = u;
    }
    // 更新usuiv和uprec链表
    else {
        Unite *colonie = g->plateau[x][y].colonie;
        if (colonie != NULL) {
            if (colonie->usuiv != NULL) {
                colonie->usuiv->uprec = u;
            }
            u->usuiv = colonie->usuiv;
            colonie->usuiv = u;
            u->uprec = colonie;
        }
    }
}

//初始化棋盘
void initialiserPlateau(Grille *g) {
    int i, j;
    for (i = 0; i < LIGNES; i++) {
        for (j = 0; j < COLONNES; j++) {
            g->plateau[i][j].colonie = NULL;
            g->plateau[i][j].occupant = NULL;
        }
    }
    g->abeille = NULL;
    g->frelon = NULL;
    g->tour = 0;
    g->ressourcesAbeille = 10;
    g->ressourcesFrelon = 10;
     // 创建并放置蜜蜂单位
    Unite *ruche = createUnite(ABEILLE, RUCHE,0);  // 创建蜂巢
    placerUnite(g, ruche, 0, 0);                     // 放置蜂巢在(0,0)
    updateUnitLists(g, ruche);  
    Unite *reineA = createUnite(ABEILLE,REINE,FREINE);//创建蜂后
    placerUnite(g,reineA,0,0);
    updateUnitLists(g,reineA);
    Unite *ouvriere = createUnite(ABEILLE,OUVRIERE,FOUVRIERE);//创建工蜂
    placerUnite(g,ouvriere,0,0);
    updateUnitLists(g,ouvriere);
    Unite *guerriere = createUnite(ABEILLE,GUERRIERE,FGUERRIERE);//创建战士
    placerUnite(g,guerriere,0,0);
    updateUnitLists(g,guerriere);
    Unite *nid = createUnite(FRELON,NID,0);//创建巢穴
    placerUnite(g,nid,9,9);
    updateUnitLists(g,nid);
    Unite *renineB = createUnite(FRELON,REINE,FREINE);//创建黄蜂蜂后
    placerUnite(g,renineB,9,9);
    updateUnitLists(g,renineB);
    Unite *frelonA = createUnite(FRELON,TYFRELON,FFRELON);//创建黄蜂
    placerUnite(g,frelonA,9,9);
    updateUnitLists(g,frelonA);
    Unite *frelonB = createUnite(FRELON,TYFRELON,FFRELON);
    placerUnite(g,frelonB,9,9);
    updateUnitLists(g,frelonB);
}



   
//显示初始化棋盘
void afficherPlateau(Grille *g) {
    int i, j;
    for (i = 0; i < LIGNES; i++) {
        for (j = 0; j < COLONNES; j++) {
            char symbols[10] = "     -    "; // 初始化为5个空格-4个空格
            if (g->plateau[i][j].colonie != NULL) {
                // 如果有colonie，根据类型放置字符
                symbols[g->plateau[i][j].colonie->camp == ABEILLE ? 0 : 6] = g->plateau[i][j].colonie->type;
            }
            UListe unit = g->plateau[i][j].occupant;
            while (unit != NULL) {
                int index = -1;
                if (unit->camp == ABEILLE) {
                    // 根据蜜蜂单位的类型确定数组中的位置
                    switch (unit->type) {
                        case REINE: index = 1; break;
                        case OUVRIERE: index = 2; break;
                        case GUERRIERE: index = 3; break;
                        case ESCADRON: index = 4; break;
                    }
                } else if (unit->camp == FRELON) {
                    // 根据黄蜂单位的类型确定数组中的位置
                    switch (unit->type) {
                        case REINE: index = 7; break;
                        case TYFRELON: index = 8; break;
                    }
                }
                if (index != -1) {
                    symbols[index] = unit->type;
                }
                unit = unit->vsuiv;
            }
            for (int k = 0; k < 10; k++) {
                printf("\033[4m%c", symbols[k]);
           }
            printf("\033[0m|"); // 重置样式
        }
        printf("\n"); // 换行
    }
}


//蜜蜂行动顺序??
Unite* prochaineUniteAbeille(Grille *g){
    char types[]={RUCHE,REINE,ESCADRON,GUERRIERE,OUVRIERE};
    Unite *Point = (g->uniteEnaction == NULL)?g->abeille:g->uniteEnaction->usuiv;
    //查找当前类型的下一个单位
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

//黄蜂行动顺序??
Unite* prochaineUniteFrelon(Grille *g) {
    char types[] = {NID, REINE, TYFRELON};   
    Unite *Point = (g->uniteEnaction == NULL)?g->frelon:g->uniteEnaction->usuiv;
    //查找当前类型的下一个单位
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

//删除报废的单位
int supprimer_unite(Grille *g, char type, int posx, int posy, Unite *u, int recolte)
{
    int success = 0;
    Unite * del_colchose = u;
    //
    if(u->type == RUCHE){
        del_colchose = u;
            while(del_colchose->usuiv){
                supprimer_unite(g, del_colchose->usuiv->type,del_colchose->usuiv->posx,del_colchose->usuiv->posy,del_colchose->usuiv,1);
            }
    }
    if(u->type == NID){
        del_colchose = u;
        while(del_colchose->usuiv){
            supprimer_unite(g, del_colchose->usuiv->type,del_colchose->usuiv->posx,del_colchose->usuiv->posy,del_colchose->usuiv,1);
        }
    }
    if(u->usuiv){
        u->usuiv->uprec = u->uprec;
    }
    if(u->uprec){
        u->uprec->usuiv = u->usuiv;
    }
    if(u->colsuiv){
        u->colsuiv->colprec = u->colprec;
    }
    if(u->colprec){
        printf("战狼的荣耀\n");
        if(u->colsuiv == NULL){
            printf("吴京还是昊京？？？\n");
        }
        u->colprec->colsuiv = u->colsuiv;
    }
    if(u->vsuiv){
        u->vsuiv->vprec = u->vprec;
    }
    if(u->vprec){
        u->vprec->vsuiv = u->vsuiv;
    }else if(u->vsuiv){
        g->plateau[posx][posy].occupant = u->vsuiv;
    }else{
        g->plateau[posx][posy].occupant = NULL;
    }

    free(u);

    if (recolte == 1)
    {
        printf("66666666666666666666666\n");


        if (type == RUCHE){
            g->ressourcesAbeille += CRUCHE;
        }
        else if (type == NID){

            printf("ressourcesFrelon:%d + %d\n",g->ressourcesFrelon, CNID);
            g->ressourcesFrelon += CNID;
        }
    }else if(recolte == 2){
        if (u->camp == ABEILLE){
            switch (type) {
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

    printf("Unité type: %c position (%d ,%d) supprimée.\n", type, posx, posy);
    return success;
}


//显示改进的格子
char *case_change(Case caseCourante)
{
    char *resultat = malloc(sizeof(char) * 10); // 使用动态内存分配

    memset(resultat, ' ', 9);                   // 将数组初始化为空格
    if (!caseCourante.occupant && !caseCourante.colonie)
    {
        return resultat;
    }

    if (caseCourante.colonie && caseCourante.colonie->type == RUCHE && caseCourante.colonie->toursrestant == 0)
    {
        resultat[0] = 'R';
    }
    if (caseCourante.colonie && caseCourante.colonie->type == NID && caseCourante.colonie->toursrestant == 0)
    {
        resultat[6] = 'N';
    }

    // 遍历占领单位链表，根据类型设置相应字符
    for (UListe tmp = caseCourante.occupant; tmp; tmp = tmp->vsuiv)
    {
        if (tmp->type == REINE && tmp->camp == FRELON && tmp->toursrestant == 0)
        {
            resultat[7] = 'r';
        }
        if (tmp->type == REINE && tmp->camp == ABEILLE && tmp->toursrestant == 0)
        {
            resultat[1] = 'r';
        }
        if (tmp->type == OUVRIERE && tmp->toursrestant == 0)
        {
            resultat[2] = 'o';
        }
        if (tmp->type == GUERRIERE && tmp->toursrestant == 0)
        {
            resultat[3] = 'g';
        }
        if (tmp->type == ESCADRON && tmp->toursrestant == 0)
        {
            resultat[4] = 'e';
        }
        if (tmp->type == FRELON && tmp->toursrestant == 0)
        {
            resultat[8] = 'f';
        }
    }
    resultat[5] = '-';
    // 添加字符串结束符
    resultat[9] = '\0';

    return resultat;
}



//显示不断改进的棋盘
void grille_change(Grille *g)
{
    int i, j;

    for (i = 0; i < LIGNES; i++)
    {
        printf("-------------------------------------------------------------------------------------------------------------------------\n");

        printf("|");
        for (j = 0; j < COLONNES; j++)
        {
            char *caseAffichee = case_change(g->plateau[i][j]);
            printf("%s|", caseAffichee);
            free(caseAffichee);
        }
        printf("\n");
    }
    printf("-------------------------------------------------------------------------------------------------------------------------\n");
}


/*------------------遍历网格中的所有方格并查看方格中是否有单元存在----------------------------------*/


int look_campunit_incase(Grille *g, int posx,int posy){
    int abeille = 0;
    int frelon = 0;

    UListe tmp = g->plateau[posx][posy].occupant;
    for (;tmp;tmp = tmp->vsuiv){
        printf("******  tmp->camp:%c   ***** \n",tmp->camp);
        if (tmp->camp == ABEILLE){
            abeille += 1;
        }else if (tmp->camp == FRELON){
            frelon += 1;
        }
    }

    if (abeille > 0 && frelon > 0) {
        printf("cuixiang写不出来啊\n");
   
        return 1; // deux camps dans une case

    }else if ((abeille > 0 && frelon == 0 && g->plateau[posx][posy].colonie == NULL) || (abeille == 0 && frelon > 0 && g->plateau[posx][posy].colonie == NULL) ) {
        printf("dans case (%d,%d) il y a un seul camp\n",posx,posy);
        return 0; // un seul camp
    }else if ((abeille > 0 && frelon == 0 && g->plateau[posx][posy].colonie) || (abeille == 0 && frelon > 0 && g->plateau[posx][posy].colonie) ){
        int pos_x = posx;
        int pos_y = posy;
        printf("//////////////////////////////////////////////////////\n");
        if (g->plateau[pos_x][pos_y].colonie) {
            printf("======g->plateau[pos_x][pos_y].colonie->camp:%c============\n",g->plateau[pos_x][pos_y].colonie->camp);
            printf("======g->plateau[pos_x][pos_y].OCCUPANT->camp:%c============\n",g->plateau[pos_x][pos_y].occupant->camp);

            if (g->plateau[pos_x][pos_y].colonie->camp == ABEILLE &&
                g->plateau[pos_x][pos_y].occupant->camp == FRELON) {
                printf("真的好难啊   崔翔写不出来\n");
                while (g->plateau[pos_x][pos_y].colonie->usuiv) {
                    supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                             g->plateau[pos_x][pos_y].colonie->usuiv->posx,
                             g->plateau[pos_x][pos_y].colonie->usuiv->posy, g->plateau[pos_x][pos_y].colonie->usuiv, 2);
                }
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
                printf("--------------------------------------------");
                while (g->plateau[pos_x][pos_y].colonie->usuiv) {
                    supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                             g->plateau[pos_x][pos_y].colonie->usuiv->posx,
                             g->plateau[pos_x][pos_y].colonie->usuiv->posy,
                             g->plateau[pos_x][pos_y].colonie->usuiv, 2);
                }
                supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->type, g->plateau[pos_x][pos_y].colonie->posx,
                         g->plateau[pos_x][pos_y].colonie->posy, g->plateau[pos_x][pos_y].colonie, 0);
            }
        }
        return 0;
    }
    return 0;
}

int war_combat(Grille *g,int pos_x,int pos_y);


int look_case(Grille *g) {
    for (int i = 0; i < LIGNES; ++i) {
        for (int j = 0; j < COLONNES; ++j) {
            if (look_campunit_incase(g, i, j) == 0) {
                //un seul camp dans la case
                continue;
            } else {
                printf("case (%d,%d) il y a enemy camps\n", i, j);
                war_combat(g, i, j);
            }
        }
    }
}






/*========================combat,construire,move,gagner=======================*/



//蜂后进行新蜂巢的建造
int construire_colonie(Grille *g, Unite *Reine_now, char production, int posx,int posy)
{
    if (g->plateau[posx][posy].colonie != NULL)
    {
        printf("Il y a deja une colonie dans cette case.\n");
        return 0;
    }

    if (production == RUCHE)
    {
        if (g->ressourcesAbeille < CRUCHE) {

            printf("Vous n'avez pas assez de ressource.\n");
            return 0;
        }
        g->plateau[posx][posy].colonie = malloc(sizeof(Unite));
        Unite *temp = g->plateau[posx][posy].colonie;
        temp->type = RUCHE;
        temp->camp = ABEILLE;
        temp->force = 0;
        temp->destx = -1;
        temp->desty = -1;
        temp->posx = posx;
        temp->posy = posy;
        temp->temps = 0;
        temp->toursrestant = 0;
        temp->production = 'i';
        temp->usuiv = Reine_now;
        temp->uprec = NULL;

        //reine的affliation更改
        if(Reine_now->uprec != NULL){
            Reine_now->uprec->usuiv = Reine_now->usuiv;
        }

        if(Reine_now->usuiv != NULL){
            Reine_now->usuiv->uprec = Reine_now->uprec;
        }
        Reine_now->usuiv = NULL;
        Reine_now->uprec = temp;
        //改col的affiliation
        //对于colsuiv的使用方法,初始化1,0的时候,其为第一个,后面的根据他来.
        Unite *colonie_courant = g->abeille;
        while(colonie_courant->colsuiv){
            colonie_courant = colonie_courant->colsuiv;
        }
        colonie_courant->colsuiv = temp;
        temp->colprec = colonie_courant;
        temp->colsuiv = NULL;
        g->ressourcesAbeille -= CRUCHE;
    }


    if (production == NID)
    {
        if (g->ressourcesFrelon < CNID)
        {
            printf("Vous n'avez pas assez de ressource.\n");
            return 0;
        }

        
        g->plateau[posx][posy].colonie = malloc(sizeof(Unite));
        Unite *temp = g->plateau[posx][posy].colonie;
        temp->type = NID;
        temp->camp = FRELON;
        temp->force = 0;
        temp->destx = -1;
        temp->desty = -1;
        temp->posx = posx;
        temp->posy = posy;
        temp->temps = 0;
        temp->toursrestant = 0;
        temp->production = 'i';
        temp->usuiv = Reine_now;
        temp->uprec = NULL;
        //改reine的affliation后面没有

        if(Reine_now->uprec){
        Reine_now->uprec->usuiv = Reine_now->usuiv;}
        if(Reine_now->usuiv){
        Reine_now->usuiv->uprec = Reine_now->uprec;}
        Reine_now->usuiv = NULL;
        Reine_now->uprec = temp;
        //改col的affliation
        //对于colsuiv的使用方法,初始化1,0的时候,其为第一个,后面的根据他来.
        Unite *colonie_courant = g->frelon;
        while(colonie_courant->colsuiv){
            colonie_courant = colonie_courant->colsuiv;
        }
        colonie_courant->colsuiv = temp;
        temp->colprec = colonie_courant;
        temp->colsuiv = NULL;
        g->ressourcesAbeille -= CNID;
    }
    Reine_now->production = 'C';
    return 1;
}

//战斗
int war_combat(Grille *g,int pos_x,int pos_y){
    int random1;
    int random2;

    //遍历g 的所有case，如果有两个不同阵营的单位在同一个case，就进行战斗确保每个case只有一个阵营的单位。
    Unite *tmp = g->plateau[pos_x][pos_y].occupant;
    Unite *adver = g->plateau[pos_x][pos_y].occupant;

    while(look_campunit_incase(g,pos_x,pos_y)){
        printf("combat\n");
        if(adver->camp == tmp->camp){
            printf("eeeeeeeee\n");
            adver = adver->vsuiv;
            continue;
        }else{
            if(tmp->toursrestant>0){
                tmp = tmp->vsuiv;
                printf("dddddd\n");
                continue;
            }
            if(adver->toursrestant>0){
                adver = adver->vsuiv;
                printf("ccccccc\n");
                continue;

            }
            srand((unsigned) time(NULL));
            random1 = rand() % 61;
            
            random2 = rand() % 61;
            printf("%d,%d random1,2,%d,%d",random1,random2,tmp->force,adver->force);
            if((tmp->force * random1) > adver->force * random2){
                printf("combat tmp %d , adver %d type GG ==> %c\n",tmp->force*random1, adver->force*random2, adver->type);
                int recolte = 0;
                if (adver->camp == ABEILLE){
                    recolte = 2;
                }
                Unite * deladver = adver;
                adver = adver->vsuiv;
                supprimer_unite(g,deladver->type,deladver->posx,deladver->posy,deladver,recolte);

            }else if((tmp->force * random1) < adver->force * random2){
                printf("**********************combat tmp %d , adver %d type GG ==> %c\n",tmp->force*random1, adver->force*random2,tmp->type);
                int recolte = 0;
                if (tmp->camp == ABEILLE){
                    recolte = 2;
                }
                Unite * deladver = tmp;
                tmp = tmp->vsuiv;
                supprimer_unite(g,deladver->type,deladver->posx,deladver->posy,deladver,recolte);

            }
        }
    }
    if(g->plateau[pos_x][pos_y].colonie){
        if(g->plateau[pos_x][pos_y].colonie->camp == ABEILLE && g->plateau[pos_x][pos_y].occupant->camp == FRELON){
            while(g->plateau[pos_x][pos_y].colonie->usuiv){
                supprimer_unite(g,g->plateau[pos_x][pos_y].colonie->usuiv->type,g->plateau[pos_x][pos_y].colonie->usuiv->posx,g->plateau[pos_x][pos_y].colonie->usuiv->posy,g->plateau[pos_x][pos_y].colonie->usuiv,2);
            }
            g->plateau[pos_x][pos_y].colonie->camp = FRELON;
            if(g->plateau[pos_x][pos_y].colonie->colsuiv){
            g->plateau[pos_x][pos_y].colonie->colsuiv->colprec = g->plateau[pos_x][pos_y].colonie->colprec;}
            if(g->plateau[pos_x][pos_y].colonie->colprec){
                g->plateau[pos_x][pos_y].colonie->colprec->colsuiv = g->plateau[pos_x][pos_y].colonie->colsuiv;
            }
            g->plateau[pos_x][pos_y].colonie->colsuiv = g->frelon->colsuiv;
            g->plateau[pos_x][pos_y].colonie->colprec = g->frelon;
            g->frelon->colsuiv = g->plateau[pos_x][pos_y].colonie;
        }else if(g->plateau[pos_x][pos_y].colonie->camp == FRELON && g->plateau[pos_x][pos_y].occupant->camp == ABEILLE){
            while(g->plateau[pos_x][pos_y].colonie->usuiv) {
                supprimer_unite(g, g->plateau[pos_x][pos_y].colonie->usuiv->type,
                         g->plateau[pos_x][pos_y].colonie->usuiv->posx, g->plateau[pos_x][pos_y].colonie->usuiv->posy,
                         g->plateau[pos_x][pos_y].colonie->usuiv, 2);
            }
            supprimer_unite(g,g->plateau[pos_x][pos_y].colonie->type,g->plateau[pos_x][pos_y].colonie->posx,g->plateau[pos_x][pos_y].colonie->posy,g->plateau[pos_x][pos_y].colonie,0);
        }
    }

}


//移动
int move(Grille *g, Unite *unit)
{
    char dest[3] = {'\0', '\0','\0'};

    int prcx, prcy;
    prcx = unit->posx;
    prcy = unit->posy;
    printf("déplacer : N (nord), NE (nord-est), E (est), SE (sud-est), S (sud), SO (sud-ouest), O (ouest), NO (nord-ouest).\n");
    printf("avant commande : posx and posy %d %d\n",unit->posx,unit->posy);
    printf("avant commande : destx and desty %d %d\n",unit->destx,unit->desty);

    scanf("%s", &dest);
    printf("%s",dest);
    printf("apres commande : posx and posy %d %d\n",unit->posx,unit->posy);
    printf("apres commande : destx and desty %d %d\n",unit->destx,unit->desty);
    if (unit->destx == -1 && unit->desty == -1)
    {
        if (unit->type == RUCHE || unit->type == NID){
            printf("Les ruches et les nids ne peuvent pas se déplacer.\n");
        } else {
            printf("L'unite est déjà en mouvement.\n");
            return -1;
        }
    }
    else
    {	
    	printf(" in  posx and posy %d %d\n",unit->posx,unit->posy);
        printf("in  destx and desty %d %d\n",unit->destx,unit->desty);
        if (dest[0] == 'N')
        {
            if (dest[1] == 'E')
            {
                unit->destx = unit->posx - 1;
                unit->desty = unit->posy + 1;
            }
            else if (dest[1] == 'O')
            {	
            	//printf("posx and posy %d %d\n",prcx,prcy);
            	printf("destx and desty %d %d\n",unit->destx,unit->desty);
                unit->destx = unit->posx - 1;
                unit->desty = unit->posy - 1;
            }
            else
            {
            	printf("seul N posx and posy %d %d\n",unit->posx,unit->posy);
            	printf("N destx and desty %d %d\n",unit->destx,unit->desty);
                unit->destx = unit->posx - 1;
                unit->desty = unit->posy;
            }
        }
        else if (dest[0] == 'S')
        {
            if (dest[1] == 'E')
            {
                unit->destx = unit->posx + 1;
                unit->desty = unit->posy + 1;
            }
            else if (dest[1] == 'O')
            {
                unit->destx = unit->posx + 1;
                unit->desty = unit->posy - 1;
            }
            else
            {
                unit->destx = unit->posx + 1;
                unit->desty = unit->posy;
            }
        }
        else if (dest[0] == 'E')
        {
            unit->destx = unit->posx;
            unit->desty = unit->posy + 1;
        }
        else if (dest[0] == 'O')
        {
            unit->destx = unit->posx;
            unit->desty = unit->posy - 1;
        }
        else
        {
            printf("Erreur de saisie.\n");
            return 0;
        }
        if ((unit->destx < 0 || unit->destx > 17) || (unit->desty < 0 || unit->desty > 11))
        {	
       		printf("%d +++ %d",unit->destx,unit->desty);	
            printf("22--Erreur de saisie.\n");
            unit->destx = prcx;
            unit->desty = prcy;
            return 0;
        }
        else
        {
            // 删除

            //reine
            if (unit->vsuiv ){
            	printf("123456789\n");
                unit->vsuiv->vprec = unit->vprec;
            }
            if (unit->vprec){
            	printf("123456789999999999999\n");
                unit->vprec->vsuiv = unit->vsuiv;
            }
            if(unit->vprec == NULL && unit->vsuiv == NULL){
            	g->plateau[unit->posx][unit->posy].occupant = NULL;
            	}
            if(unit->vprec == NULL && unit->vsuiv != NULL){
            	g->plateau[unit->posx][unit->posy].occupant = unit->vsuiv;
            }
//            printf("-------------这是指针%c-----\n",g->plateau[unit->destx][unit->desty].occupant->type);
            if(g->plateau[unit->destx][unit->desty].occupant==NULL){
                g->plateau[unit->destx][unit->desty].occupant = malloc(sizeof(Unite));
                unit->vsuiv = NULL;
                unit->vprec = NULL;
                printf("LAasdasd");
                g->plateau[unit->destx][unit->desty].occupant = unit;
                printf("---zhizheng???%p-----\n",g->plateau[unit->destx][unit->desty].occupant);
            }else{
                printf("-------------zhizheng???%c-----\n",g->plateau[unit->destx][unit->desty].occupant->type);
                Unite *tmp = g->plateau[unit->destx][unit->desty].occupant;
                while(tmp->vsuiv){
                    tmp = tmp->vsuiv;
                }
                unit->vsuiv = NULL;
                unit->vprec = tmp;
                tmp->vsuiv= unit;
            }
            printf("avatar deplace: %d %d --> ", unit->posx, unit->posy);
            unit->posx = unit->destx;
            unit->posy = unit->desty;

            printf("après déplacement: %d %d\n", unit->posx, unit->posy);
            printf("depalcement réussi.\n");

            return 1;
        }
    }
}

//游戏结束胜利条件
int gagner_ou_pas(Grille *g){
    int abeille = 0;
    int frelon = 0;
    for (int i = 0; i < LIGNES; i++)
    {
        for (int j = 0; j < COLONNES; j++)
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
        return 11; // 11frelons赢了
    }
    if (frelon == 0)
    {
        printf("Les abeilles ont gagné.\n");
        return 22; // 22abeilles赢了
    }
    printf("Le jeu continue.\n");
    return 0;


}


//清空缓冲区
int clean_Buffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    return 1;
}

//tmp->camp:F



/*---------------------------choisir action-------------------------------------------*/
//收获花粉
void recolte(Grille *g, Unite *u)
{
    printf("temps a besoin de %d\n", u->temps);
    if (u->production == RECOLTE && u->temps < 4)
    {
        if (u->camp == ABEILLE)
        {
            printf("La recolte est en cours. temps deja :  %d\n", u->temps);
            u->temps++;
            g->ressourcesAbeille += 1;
        }
    }
    if (u->production == RECOLTE && u->temps >= 4)
    {
        printf("La recolte terminee.\n");
        supprimer_unite(g, RECOLTE, u->posx, u->posy, u, 0);

    }
}

//蜂巢创造新的单位
int produit_unite(Grille *g, Unite *colonie,char produ, char camp)
{
    int pos_x, pos_y;
    pos_x = colonie->posx;
    pos_y = colonie->posy;
    printf("position : (%d,%d) ? ?\n",pos_y,pos_x);
    Unite *temp = g->plateau[pos_x][pos_y].occupant; // 同一个case???
    Unite *colonie_courant = colonie;               // ruche????
    int force_courant = 0;
    char production_courant =' ';
    int cout_courant;
    int temp_courant;
    int tourrestant_courant;
    switch (produ)
    {
    case REINE:
        force_courant = FREINE;
        if (camp == ABEILLE )
        {
            cout_courant = CREINEA;
            temp_courant = TREINEA;
            tourrestant_courant = TREINEA; 
            // 如果建造完之后这个ruche就变成'/0'
        }
        else
        {
            cout_courant = CREINEF;
            temp_courant = TREINEF;
            tourrestant_courant = TREINEF;
        }

        break;
    case OUVRIERE:
        force_courant = FOUVRIERE;

        temp_courant = 0;
        tourrestant_courant = TOUVRIERE;
        cout_courant = COUVRIERE;
        break;
    case GUERRIERE:
        force_courant = FGUERRIERE;
        temp_courant = TGUERRIERE;
        cout_courant = CGUERRIERE;
        tourrestant_courant = TGUERRIERE;
        break;
    case ESCADRON:
        force_courant = FESCADRON;
        temp_courant = TESCADRON;
        cout_courant = CESCADRON;
        tourrestant_courant = TESCADRON;
        break;
    case FRELON:
        force_courant = FFRELON;
        temp_courant = TFRELON;
        cout_courant = CFRELON;
        tourrestant_courant = TFRELON;
        break;
    default:
        force_courant = 0;
    }
    if (camp == ABEILLE && cout_courant < g->ressourcesAbeille){
        g->ressourcesAbeille -= cout_courant;
    }else if(camp == FRELON && cout_courant < g->ressourcesFrelon) {
        g->ressourcesFrelon -= cout_courant;
    }else{
        printf("pas assez de ressources\n");
        return 0;
    }
    while (colonie_courant->usuiv)
    {
        colonie_courant = colonie_courant->usuiv;
    }
    colonie_courant->usuiv = malloc(sizeof(Unite));
    colonie_courant->usuiv->type = produ;
    colonie_courant->usuiv->camp = camp;
    colonie_courant->usuiv->force = force_courant;
    colonie_courant->usuiv->posx = pos_x;
    colonie_courant->usuiv->posy = pos_y;
    colonie_courant->usuiv->destx = pos_x;
    colonie_courant->usuiv->desty = pos_y;
    colonie_courant->usuiv->production = production_courant;
    colonie_courant->usuiv->temps = temp_courant;
    colonie_courant->usuiv->toursrestant = tourrestant_courant+1;
    colonie_courant->usuiv->usuiv = NULL;
    colonie_courant->usuiv->uprec = colonie_courant;
    colonie_courant->usuiv->colsuiv = NULL;
    colonie_courant->usuiv->colprec = NULL;
    colonie_courant->usuiv->vsuiv = NULL;
    if (temp == NULL)
    {
        colonie_courant->usuiv->vprec = NULL;
        temp = colonie_courant->usuiv;
        g->plateau[pos_x][pos_y].occupant = colonie_courant->usuiv;


        printf(" camp: %c \n",temp->camp);

    }
    else if (temp)
    { 
        // 需要排序? 以及,战斗逻辑其实是先遍历occupant的内容,然后再看colonie的内容,unite中的usuiv其实不包含ruche和nid
        while (temp->vsuiv)
        {
            temp = temp->vsuiv;
        }
        temp->vsuiv = colonie_courant->usuiv;
        colonie_courant->usuiv->vprec = temp;
    }

}
//工蜂的收获和连锁反应，对于资源的管控
void ouvriere_recolte(Grille *g, Unite *unit_affilie) {
    unit_affilie->destx = -1;
    unit_affilie->desty = -1;
    unit_affilie->production = RECOLTE;
    unit_affilie->temps = 0;
}
//对于蜂后行动的选择
void choisir_reine(Grille *g,Unite *unit_affilie) {
    int choix;
    if (unit_affilie->production == ' ') {
        printf("Choix de la reine en (%d, %d).\n", unit_affilie->posx, unit_affilie->posy);
        printf("1. deplacer\n");
        printf("2. construire une ruche.\n");
        printf("3. passer le tour.\n");
        scanf("%d", &choix);
        while (choix < 1 || choix > 3) {
            printf("incorresct choix!!!");
            printf("Choix de la reine en (%d, %d).\n", unit_affilie->posx, unit_affilie->posy);
            printf("1. deplacer\n");
            printf("2. construire une ruche.\n");
            printf("3. passer le tour.\n");
            scanf("%d", &choix);
        }
        int succ;
        switch (choix) {
            case 1:
                do {
                    succ = move(g, unit_affilie);
                } while (succ == 0);
                if (succ == -1) {
                    printf("deplacement impossible\n");
                    break;
                }
                break;
            case 2:
                if(unit_affilie->camp == ABEILLE) {
                    construire_colonie(g, unit_affilie, RUCHE, unit_affilie->posx, unit_affilie->posy);
                }else{
                    construire_colonie(g, unit_affilie, NID, unit_affilie->posx, unit_affilie->posy);
                }
                break;
            case 3:
                break;

        }
    }else{
        printf("reine deja construire une RUCHE ou NID.\n");
        printf("Choix de la reine en (%d, %d).\n", unit_affilie->posx, unit_affilie->posy);
        printf("1. deplacer\n");
        printf("2. passer le tour.\n");
        scanf("%d", &choix);
        while (choix < 1 || choix > 2) {
            printf("reine deja construire une RUCHE ou NID.\n");
            printf("incorresct choix!!!");
            printf("Choix de la reine en (%d, %d).\n", unit_affilie->posx, unit_affilie->posy);
            printf("1. deplacer\n");
            printf("2. passer le tour.\n");
            scanf("%d", &choix);
        }
        int succ;
        switch (choix) {
            case 1:
                do {
                    succ = move(g, unit_affilie);
                } while (succ == 0);
                if (succ == -1){
                    printf("deplacement impossible\n");
                    break;
                }

                break;
            case 2:
                break;

        }
    }
}
//战斗组织行动的选择
void choisir_guerriere_escadron_frelon(Grille *g, Unite *unit_affilie) {
    int choix;
    printf("Choix %c en (%d, %d).\n",unit_affilie->type, unit_affilie->posx, unit_affilie->posy);
    printf("1. deplacer\n");
    printf("2. passer le tour.\n");
    scanf("%d", &choix);
    while (choix < 1 || choix > 2){
        printf("incorresct choix!!!");
        printf("Choix de la production de la ruche en (%d, %d).\n", unit_affilie->posx, unit_affilie->posy);
        printf("1. deplacer\n");
        printf("2. passer le tour.\n");
        scanf("%d", &choix);
    }
    int succ;
    switch (choix) {
        case 1:
            do {
                succ = move(g, unit_affilie);
            } while (succ == 0);
            if (succ == -1){
                printf("deplacement impossible\n");
                break;
            }

            break;
        case 2:
            break;

    }
}
//工蜂行动的选择
void choisir_ouvriere(Grille *g, Unite *unit_affilie) {
    int choix;
    printf("Choix %c en (%d, %d).\n",unit_affilie->type, unit_affilie->posx, unit_affilie->posy);
    printf("1. deplacer\n");
    printf("2. Recolter du pollen\n");
    printf("3. passer le tour\n");
    scanf("%d", &choix);
    while (choix < 1 || choix > 2){
        printf("incorresct choix!!!");
        printf("Choix de la production de la ruche en (%d, %d).\n", unit_affilie->posx, unit_affilie->posy);
        printf("1.deplacer\n");
        printf("2.Recolter du pollen\n");
        printf("3. passer le tour\n");
        scanf("%d", &choix);
    }
    int succ;
    switch (choix) {
        case 1:
            do {
                succ = move(g, unit_affilie);
            } while (succ == 0);
            if (succ == -1){
                printf("deplacement impossible\n");
                break;
            }

            break;
        case 2:
            ouvriere_recolte(g, unit_affilie);
            break;
        case 3:
        break;

    }
}





//游戏的开始和用户的选择
void gamestart_et_choix(Grille *g,char camp){
    // 需要遍历所有蜜蜂蜂巢吗？？


    if (camp == ABEILLE) {
        //Unite *colonie_temp = g->abeille->colsuiv;

        Unite *colonie_temp = g->plateau[0][0].colonie->colsuiv;

        if (colonie_temp != NULL) {
        colonie_temp->colprec = NULL;  // 将当前节点的前驱节点设为 NULL
        g->plateau[9][9].colonie->colsuiv = NULL;  // 将当前节点的后继节点设为 NULL
}
      
  
        for (;colonie_temp;colonie_temp = colonie_temp->colsuiv){
                int choix;
                printf("Choix de la production de la ruche en (%d, %d).\n", colonie_temp->posx, colonie_temp->posy);
                printf("1. Produire une reine(%d pollen, %d tours).\n", CREINEA, TREINEA);
                printf("2. Produire une Ouvriere(%d pollen, %d tours).\n", COUVRIERE, TOUVRIERE);
                printf("3. Produire une Guerriere(%d pollen, %d tours).\n", CGUERRIERE, TGUERRIERE);
                printf("4. Produire une escadron(%d pollen, %d tours).\n", CESCADRON, TESCADRON);
                printf("5. Passer de tour.\n");
                printf("6. Detruire une unite.\n");
                scanf("%d", &choix);
                while (choix < 1 || choix > 6) {
                    printf("Choix incorrect!!!");//选择的不对：
                    printf("Choix de la production de la ruche en (%d, %d).\n", colonie_temp->posx, colonie_temp->posy);
                    printf("1. Produire une reine(%d pollen, %d tours).\n", CREINEA, TREINEA);
                    printf("2. Produire une Ouvriere(%d pollen, %d tours).\n", COUVRIERE, TOUVRIERE);
                    printf("3. Produire une Guerriere(%d pollen, %d tours).\n", CGUERRIERE, TGUERRIERE);
                    printf("4. Produire une escadron(%d pollen, %d tours).\n", CESCADRON, TESCADRON);
                    printf("5. Passer de tour.\n");
                    printf("6. Detruire une unite.\n");
                    scanf("%d", &choix);
                }

                UListe temp_supprimer = NULL;

                switch (choix) {
                    case 1:
                        produit_unite(g, colonie_temp, REINE, camp);// 产生reine 函数
        
                        break;
                    case 2:
                        produit_unite(g, colonie_temp, OUVRIERE, camp);
                        break;// 产生ouvriere 函数
                    case 3:
                        produit_unite(g, colonie_temp, GUERRIERE, camp);
                        break;// 产生guerriere 函数
                    case 4:
                        produit_unite(g, colonie_temp, ESCADRON, camp);
                        break;// 产生escadron 函数
                    case 5:
                        break;// passer de tour
                    case 6:
                        // detruire une unite
                        temp_supprimer = colonie_temp->usuiv;

                        supprimer_unite(g, colonie_temp->type, colonie_temp->posx, colonie_temp->posy, colonie_temp, 1);

                        break;
                }
                if(colonie_temp){
                    printf("type:%c,camp:%c,posx:%d,posy:%d  \n",colonie_temp->type,colonie_temp->camp,colonie_temp->posx,colonie_temp->posy);
                    printf("-----1.colonie_temp est full------\n");
                }
                if(colonie_temp->usuiv){
                    printf("-----2.colonie_temp->usuiv est full------\n");
                }
                Unite *unit_affilie = colonie_temp->usuiv;

                if (temp_supprimer){
                    printf("--------temp pour supprimer est full -----------\n");
                    unit_affilie = temp_supprimer;
                }

                for (;unit_affilie;unit_affilie = unit_affilie->usuiv) {

                

                    printf("avant command temp_srestant:%d\n",unit_affilie->toursrestant);
                    if(unit_affilie->type == OUVRIERE && unit_affilie->production == RECOLTE){
                        recolte(g, unit_affilie);
                    }
                    if (unit_affilie->toursrestant > 0) {
                        printf("temps_sub\n");
                        unit_affilie->toursrestant -= 1;
                    }
                    printf("apres command temps_restant:%d\n",unit_affilie->toursrestant);

  

                    if(unit_affilie->toursrestant == 0 ){
                        switch (unit_affilie->type) {
                            case REINE:
                                choisir_reine(g, unit_affilie);
                                break;
                            case OUVRIERE:
                                choisir_ouvriere(g, unit_affilie);
                                break;// 产生ouvriere 函数
                            case GUERRIERE:
                                choisir_guerriere_escadron_frelon(g, unit_affilie);
                                break;// 产生guerriere 函数
                            case ESCADRON:
                                choisir_guerriere_escadron_frelon(g, unit_affilie);
                                break;// 产生escadron 函数

                        }
                    }

                }
        }
    }else{
        //Unite *colonie_temp = g->frelon->colsuiv;
        Unite *colonie_temp = g->plateau[9][9].colonie->colsuiv;
        if (colonie_temp != NULL) {
    colonie_temp->colprec = NULL;  // 将当前节点的前驱节点设为 NULL
    g->plateau[9][9].colonie->colsuiv = NULL;  // 将当前节点的后继节点设为 NULL
}


        for (;colonie_temp;colonie_temp = colonie_temp->colsuiv){
            int choix;
            printf("Choix de la production de la NID en (%d, %d).\n", colonie_temp->posx, colonie_temp->posy);
            printf("1. Produire une reine(%d pollen, %d tours).\n", CREINEF, TREINEF);
            printf("2. Produire une Frelon(%d pollen, %d tours).\n", CFRELON, TFRELON);
            printf("3. Passer de tour.\n");
            printf("4. Detruire l'unite.\n");
            scanf("%d", &choix);
            while (choix < 1 || choix > 4) {
                printf("Choix incorrect!!!");//选择错误法语：
                printf("Choix de la production de la NID en (%d, %d).\n", colonie_temp->posx, colonie_temp->posy);
                printf("1. Produire une reine(%d pollen, %d tours).\n", CREINEF, TREINEF);
                printf("2. Produire une Frelon(%d pollen, %d tours).\n", CFRELON, TFRELON);
                printf("3. Passer de tour.\n");
                printf("4. Detruire l'unite.\n");
                scanf("%d", &choix);
            }
            UListe temp_supprimer = NULL;

            switch (choix) {
                case 1:
                    produit_unite(g, colonie_temp, REINE, camp);// 产生reine 函数
                    break;
                case 2:
                    produit_unite(g, colonie_temp, FRELON, camp);
                    break;// 产生ouvriere 函数
                case 3:
                    break;
                case 4:
                    // detruire une unite
                    printf("detruire une unite %c\n",colonie_temp->type);
                    temp_supprimer = colonie_temp->usuiv;

                    supprimer_unite(g, colonie_temp->type, colonie_temp->posx, colonie_temp->posy, colonie_temp, 1);
                    break;
            }
            if(colonie_temp){
                printf("type:%c,camp:%c,posx:%d,posy:%d\n",colonie_temp->type,colonie_temp->camp,colonie_temp->posx,colonie_temp->posy);
                printf("-----1.colonie_temp est full-------\n");
            }
            if(colonie_temp->usuiv){
                printf("-----2.colonie_temp->usuiv est full-----\n");
            }



            Unite *unit_affilie = colonie_temp->usuiv;

            if (temp_supprimer){
                printf("--------temp pour supprimer est full -----------\n");
                unit_affilie = temp_supprimer;
            }

            for (;unit_affilie;unit_affilie = unit_affilie->usuiv) {
  
                printf("avant commande tempsrestant:%d\n",unit_affilie->toursrestant);


                if (unit_affilie->toursrestant > 0) {
                    printf("subscribe_tours\n");
                    unit_affilie->toursrestant -= 1;
                }
                printf("apres commande tempsestant:%d\n",unit_affilie->toursrestant);
    
                if(unit_affilie->toursrestant == 0) {
                    switch (unit_affilie->type) {
                        case REINE:
                            choisir_reine(g, unit_affilie);
                            break;
                        case FRELON:
                            choisir_guerriere_escadron_frelon(g, unit_affilie);
                            break;
                    }
                }
            }
        }
    }
}




/*----------------------------------------save-----------------------------------------------------*/



void save_game(Grille *g,char camp){
    FILE *file = fopen("projet_final_save","w");
    int resA=0,resB=0;
    if(camp == ABEILLE){
        resA = g->ressourcesAbeille;
        resB = g->ressourcesFrelon;
    }
    else if(camp == FRELON){
        resA= g->ressourcesFrelon;
        resB = g->ressourcesAbeille;
    }
    fprintf(file,"%c %d %d\n",camp,resA,resB);


    UListe colon_c = g->abeille->colsuiv;
    UListe unit_c = NULL;


    while(colon_c){
        fprintf(file,"%c %c %d %d %c %d\n",colon_c->camp,colon_c->type,colon_c->posx,colon_c->posy,colon_c->production,colon_c->toursrestant);
        unit_c = colon_c->usuiv;
        while(unit_c){
            printf("%c asdfasdfasdasdas",unit_c->type);
            fprintf(file,"%c %c %d %d %c %d\n",unit_c->camp,unit_c->type,unit_c->posx,unit_c->posy,unit_c->production,unit_c->toursrestant);
            unit_c = unit_c->usuiv;
        }
        colon_c = colon_c->colsuiv;
    }
    colon_c = g->frelon->colsuiv;

    while(colon_c){
        fprintf(file,"%c %c %d %d %c %d\n",colon_c->camp,colon_c->type,colon_c->posx,colon_c->posy,colon_c->production,colon_c->toursrestant);
        unit_c = colon_c->usuiv;
        while(unit_c){
            fprintf(file,"%c %c %d %d %c %d\n",unit_c->camp,unit_c->type,unit_c->posx,unit_c->posy,unit_c->production,unit_c->toursrestant);
            unit_c = unit_c->usuiv;
        }
        colon_c = colon_c->colsuiv;
    }
}





/*-------------------------------------main--------------------------------------------------*/


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



