#include<stdio.h>
#include <stdlib.h>
#include<time.h>

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
#define  FRELON  'f'
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
}  Grille;

void initCase(Case *c) {
    c->colonie = NULL;
    c->occupant = NULL;
}

void initGrille(Grille *g) {
    int i, j;
    for (i = 0; i < COLONNES; i++) {
        for (j = 0; j < LIGNES; j++) {
            initCase(&(g->plateau[i][j]));
        }
    }
    g->abeille = NULL;
    g->frelon = NULL;
    g->tour = 0;
    g->ressourcesAbeille = 0; // 初始化蜜蜂资源
    g->ressourcesFrelon = 0; // 初始化黄蜂资源
}

void displayBoard(const Grille *g) {
    int i, j;
    for (i = 0; i < LIGNES; i++) {
        for (j = 0; j < COLONNES; j++) {
            Case currentCase = g->plateau[j][i];

            // 检查是否有单位在当前单元格
            if (currentCase.colonie != NULL) {
                printf("%c ", currentCase.colonie->type);
            } else if (currentCase.occupant != NULL) {
                printf("%c ", currentCase.occupant->type);
            } else {
                printf("%c ", VIDE); // 无单位时显示空格
            }
        }
        printf("\n");
    }
}


Unite* createUnite(char camp, char type, int force, int x, int y) {
    Unite* newUnite = (Unite*)malloc(sizeof(Unite));
    if (newUnite == NULL) {
        perror("Allocation error");
        exit(EXIT_FAILURE);
    }
    newUnite->camp = camp;
    newUnite->type = type;
    newUnite->force = force;
    newUnite->posx = x;
    newUnite->posy = y;
    newUnite->destx = -1;  // 初始状态设置为不移动
    newUnite->desty = -1;
    newUnite->production = VIDE;  // 初始状态无生产
    newUnite->temps = 0;
    newUnite->toursrestant = 0;
    newUnite->usuiv = NULL;
    newUnite->uprec = NULL;
    newUnite->colsuiv = NULL;
    newUnite->colprec = NULL;
    newUnite->vsuiv = NULL;
    newUnite->vprec = NULL;
    return newUnite;
}

void setUnite(Grille *g, Unite *u) {
    int x = u->posx;
    int y = u->posy;

    if (x >= 0 && x < COLONNES && y >= 0 && y < LIGNES) {
        // 将单位添加到游戏板的相应位置
        g->plateau[x][y].occupant = u;
    }
}

void moveUnite(Grille *g, Unite *u, int newX, int newY) {
    // 检查新位置是否有效
    if (newX < 0 || newX >= COLONNES || newY < 0 || newY >= LIGNES) {
        return; // 无效位置，不移动
    }
    
    // 清除原位置
    g->plateau[u->posx][u->posy].occupant = NULL;

    // 更新位置
    u->posx = newX;
    u->posy = newY;
    g->plateau[newX][newY].occupant = u;
}


void combat(Grille *g, int x, int y) {
    Unite *u1 = g->plateau[x][y].occupant;

    // 检查当前位置是否有两个不同阵营的单位
    if (u1 != NULL && u1->vsuiv != NULL && u1->camp != u1->vsuiv->camp) {
        Unite *u2 = u1->vsuiv;

        // 比较两个单位的力量
        if (u1->force >= u2->force) {
            // u1 胜利，移除 u2
            removeUnite(g, u2);
            // 可以在这里处理胜利方的奖励，如资源获取等
        } else {
            // u2 胜利，移除 u1
            removeUnite(g, u1);
            // 处理胜利方的奖励
        }
    }
}

void removeUnite(Grille *g, Unite *u) {
    // 首先，从游戏板上移除单位
    g->plateau[u->posx][u->posy].occupant = NULL;

    // 接着，处理链表中的前一个和后一个元素
    if (u->uprec != NULL) {
        u->uprec->usuiv = u->usuiv;
    }
    if (u->usuiv != NULL) {
        u->usuiv->uprec = u->uprec;
    }

    // 如果这个单位是某个链表的头部，需要更新头部指针
    if (g->abeille == u) {
        g->abeille = u->usuiv;
    } else if (g->frelon == u) {
        g->frelon = u->usuiv;
    }

    // 最后，释放分配给单位的内存
    free(u);
}


void collectResources(Grille *g, Unite *u) {
    // 假设u是一个工蜂或者其他可以收集资源的单位
    if (u->type == OUVRIERE && u->camp == ABEILLE) {
        // 假设每个工蜂每回合可以收集1单位的花粉
        g->ressourcesAbeille += 1;
    }
    // 可以根据需要添加其他收集逻辑
}

int useResources(Grille *g, char camp, int amount) {
    if (camp == ABEILLE) {
        if (g->ressourcesAbeille >= amount) {
            g->ressourcesAbeille -= amount;
            return 1; // 资源足够，使用成功
        }
    } else if (camp == FRELON) {
        if (g->ressourcesFrelon >= amount) {
            g->ressourcesFrelon -= amount;
            return 1; // 资源足够，使用成功
        }
    }
    return 0; // 资源不足
}
void updateResources(Grille *g) {
    // 遍历游戏板，对于每个可以收集资源的单位，调用 collectResources
    for (int i = 0; i < COLONNES; i++) {
        for (int j = 0; j < LIGNES; j++) {
            Unite *u = g->plateau[i][j].occupant;
            if (u != NULL) {
                collectResources(g, u);
            }
        }
    }
    // 可以添加其他资源更新逻辑
}

void produceUnits(Grille *g) {
    // 遍历游戏板，查找可以生产单位的游戏单元
    for (int x = 0; x < COLONNES; x++) {
        for (int y = 0; y < LIGNES; y++) {
            Unite *unite = g->plateau[x][y].colonie;

            if (unite != NULL && (unite->type == RUCHE || unite->type == NID)) {
                // 检查是否满足生产条件（比如资源是否足够）
                if (canProduceUnit(g, unite)) {
                    // 创建新单位
                    Unite *newUnit = createUnite(unite->camp, determineUnitType(unite), unite->force, x, y);
                    // 放置新单位
                    placeUnite(g, newUnit);
                    // 扣除资源
                    useResources(g, unite->camp, determineResourceCost(newUnit->type));
                }
            }
        }
    }
}

int canProduceUnit(const Grille *g, const Unite *unite) {
    // 实现检查是否可以生产单位的逻辑
    // 例如，检查资源是否足够，是否满足生产时间等
    return 1; // 示例代码，根据实际情况修改
}

char determineUnitType(const Unite *unite) {
    // 根据生产单位的类型决定生产什么单位
    // 示例代码，根据实际情况修改
    if (unite->type == RUCHE) {
        return OUVRIERE; // 巢穴生产工蜂
    } else if (unite->type == NID) {
        return FRELON; // 巢生产黄蜂
    }
    return VIDE;
}

int determineResourceCost(char type) {
    // 根据单位类型确定资源消耗
    // 示例代码，根据实际情况修改
    switch (type) {
        case OUVRIERE: return COUVRIERE;
        case FRELON: return CFRELON;
        // 添加其他单位类型的情况
        default: return 0;
    }
}

int checkGameOver(const Grille *g) {
    int hasAbeille = 0;
    int hasFrelon = 0;

    // 遍历游戏板，检查每个阵营是否至少有一个单位
    for (int x = 0; x < COLONNES; x++) {
        for (int y = 0; y < LIGNES; y++) {
            Unite *unite = g->plateau[x][y].occupant;
            if (unite != NULL) {
                if (unite->camp == ABEILLE) {
                    hasAbeille = 1;
                }
                if (unite->camp == FRELON) {
                    hasFrelon = 1;
                }
            }
        }
    }

    // 检查是否有一方已无单位
    if (!hasAbeille) {
        printf("Frelons win!\n");
        return 1;  // 蜜蜂输，游戏结束
    }
    if (!hasFrelon) {
        printf("Abeilles win!\n");
        return 1;  // 黄蜂输，游戏结束
    }

    // 可以添加其他游戏结束的条件
    // 比如达到特定回合数，或者特定资源被耗尽

    return 0;  // 游戏继续
}



void saveGame(const Grille *g, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return;
    }

    // 写入基本游戏信息
    fprintf(file, "%d %d\n", g->tour, g->ressourcesAbeille, g->ressourcesFrelon);

    // 遍历游戏板，保存每个单位的信息
    for (int i = 0; i < COLONNES; i++) {
        for (int j = 0; j < LIGNES; j++) {
            Unite *u = g->plateau[i][j].occupant;
            if (u != NULL) {
                fprintf(file, "%c %c %d %d %d %d %d\n", u->camp, u->type, u->force, u->posx, u->posy, u->temps, u->toursrestant);
                // 添加其他需要保存的单位属性
            }
        }
    }

    fclose(file);
}

void loadGame(Grille *g, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file for reading");
        return;
    }

    // 读取基本游戏信息
    fscanf(file, "%d %d %d\n", &g->tour, &g->ressourcesAbeille, &g->ressourcesFrelon);

    // 初始化游戏板
    initGrille(g);

    // 读取单位信息
    char camp, type;
    int force, posx, posy, temps, toursrestant;
    while (fscanf(file, "%c %c %d %d %d %d %d\n", &camp, &type, &force, &posx, &posy, &temps, &toursrestant) != EOF) {
        Unite *u = createUnite(camp, type, force, posx, posy);
        u->temps = temps;
        u->toursrestant = toursrestant;
        // 设置其他单位属性
        setUnite(g, u);
    }

    fclose(file);
}

void processInput(Grille *g) {
    int unitX, unitY, targetX, targetY;
    char action;

    // 获取玩家的输入：操作类型、单位位置和目标位置
    printf("Enter action (m for move, a for attack): ");
    scanf(" %c", &action);
    printf("Enter unit position (x y): ");
    scanf("%d %d", &unitX, &unitY);
    printf("Enter target position (x y): ");
    scanf("%d %d", &targetX, &targetY);

    // 检查选择的单位是否有效
    if (unitX < 0 || unitX >= COLONNES || unitY < 0 || unitY >= LIGNES || g->plateau[unitX][unitY].occupant == NULL) {
        printf("Invalid unit position!\n");
        return;
    }

    Unite *selectedUnit = g->plateau[unitX][unitY].occupant;

    switch(action) {
        case 'm': // 移动单位
            moveUnite(g, selectedUnit, targetX, targetY);
            break;
        case 'a': // 攻击
            // 假设攻击就是把单位移到目标位置并进行战斗
            moveUnite(g, selectedUnit, targetX, targetY);
            combat(g, targetX, targetY);
            break;
        default:
            printf("Invalid action!\n");
            break;
    }
}

void updateGameRound(Grille *g) {
    // 增加回合数
    g->tour++;

    // 1. 处理单位的移动
    // 遍历所有单位，根据它们的目标位置进行移动
    for (int x = 0; x < COLONNES; x++) {
        for (int y = 0; y < LIGNES; y++) {
            Unite *u = g->plateau[x][y].occupant;
            if (u != NULL && u->destx >= 0 && u->desty >= 0) {
                moveUnite(g, u, u->destx, u->desty);
                u->destx = -1; // 重置目标位置
                u->desty = -1;
            }
        }
    }

    // 2. 处理所有单位的战斗
    for (int x = 0; x < COLONNES; x++) {
        for (int y = 0; y < LIGNES; y++) {
            if (g->plateau[x][y].occupant != NULL) {
                combat(g, x, y);  // 处理单元格上的战斗
            }
        }
    }

    // 3. 更新资源
    updateResources(g);

    // 4. 生产新单位
    produceUnits(g);

}

void freeUniteList(UListe list) {
    while (list != NULL) {
        Unite *temp = list;
        list = list->usuiv;
        free(temp);
    }
}

void cleanup(Grille *g) {
    // 释放所有蜜蜂单位
    freeUniteList(g->abeille);

    // 释放所有黄蜂单位
    freeUniteList(g->frelon);
}


int main() {
    Grille g;
    char input;
    int gameRunning = 1;

    // 初始化游戏状态
    initGrille(&g);

    // 主循环
    while (gameRunning) {
        // 渲染游戏界面
        displayBoard(&g);

        // 获取用户输入 (这里需要您根据游戏的实际情况来设计和实现)
        printf("Enter your move (or 'q' to quit): ");
        scanf(" %c", &input);
        if (input == 'q') {
            gameRunning = 0;
            continue;
        }

        // 根据输入更新游戏状态
        // 例如：processInput(&g, input);

        // 更新游戏状态
        updateGameRound(&g);

        // 检查游戏是否结束
        if (checkGameOver(&g)) {
            gameRunning = 0;
        }
    }

    // 游戏结束后的清理工作
    // 例如：cleanup(&g);

    return 0;
}
