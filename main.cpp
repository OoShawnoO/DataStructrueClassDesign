#include <iostream>
#include <cstring>


const int Total_lines = 528;
const int Total_stations =3431;
const int  MAX_DISTANCE=9999999;


/*
 * 将车站作为节点v,公交线路则为关系e.
 * 使用领接表这种数据结构
 * */

enum Mode{
    Most_save = 1, /*最经济模式  （考虑很多车的全程都是2元 单线乘坐超过10站 每100m+0.1元） */
    Most_fast,     /*最快速模式*/
    Least_change,   /*最少换乘模式*/
    Wanna_Pass,     /*期望经过模式*/
    Synthesize,     /*综合模式*/
};

typedef struct station_edge{
    int next_adj_station_id;
    int pre_adj_station_id;
    int next_distance;
    int pre_distance;
    int line_id;
    station_edge* next;
}Station_Edge;

typedef struct station{
    int station_id;
    char name[512];
    Station_Edge *first;
}Station;

typedef struct node{
    int station_id;
    int next_distance;
    int pre_distance;
    node* next;
    node* pre;
}Node;

typedef struct line{
    int line_id;
    char name[512];
    node* head;
}Line;

typedef struct {
    Station stations[Total_stations];
    Line lines[Total_lines];
}Nets;

Station* Create_Station(char* name,int station_id,station_edge* first){
    auto *station = (Station*)malloc(sizeof(Station));
    strcpy(station->name,name);
    station->station_id = station_id;
    station->first = first;
    return station;
}

static Station stations[Total_stations];
static Line lines[Total_lines];

Line* Create_Line(char* name,int line_id,Node* head){
    auto *line = (Line*)malloc(sizeof(Line));
    strcpy(line->name,name);
    line->line_id = line_id;
    line->head = head;
    return line;
}

void Add_Edge(station_edge* head,Node *n,int line_id){
    auto* stationEdge = (station_edge*)malloc(sizeof(station_edge));
    station_edge *p;
    p = head;
    if(n->pre== nullptr){
        stationEdge->pre_adj_station_id = -1;
        stationEdge->pre_distance = MAX_DISTANCE;
    }
    else{
        stationEdge->pre_adj_station_id = n->pre->station_id;
        stationEdge->pre_distance = n->pre_distance;
    }
    if(n->next==nullptr){
        stationEdge->next_adj_station_id = -1;
        stationEdge->next_distance = MAX_DISTANCE;
    }
    else{
        stationEdge->next_adj_station_id = n->next->station_id;
        stationEdge->next_distance = n->next_distance;
    }
    stationEdge->line_id = line_id;
    stationEdge->next = nullptr;
    while(p->next!=nullptr){p=p->next;}
    p->next = stationEdge;
}

Nets Initialize_Nets(){
    Nets *nets = (Nets *)malloc(sizeof(Nets));

    int stations_num=0,lines_num=0;

    FILE *fp_station = fopen("D://C_learn/DSCD/stations.txt","r");
    FILE *fp_line = fopen("D://C_learn/DSCD/lines.txt","r");
    FILE *fp_distance = fopen("D://C_learn/DSCD/distance.txt","r");

    /* 初始化站点 */
    for(;stations_num<Total_stations;stations_num++){
        char str[1024]={'\0'};
        fscanf(fp_station,"%s",str);
        auto* head = (station_edge*)malloc(sizeof(station_edge));
        head->next = nullptr;
        head->next_adj_station_id = -1;
        head->next_distance = 0;
        head->line_id = -1;
        head->pre_adj_station_id = -1;
        head->pre_distance = 0;
        Station *station = Create_Station(str,stations_num,head);
        stations[stations_num] = *station;
    }

    /* 初始化路线 */

    for(;lines_num<Total_lines;lines_num++){
        char sta[100],str[100];
        int dis;
        int* distance = &dis;
        Node *head =(Node *)malloc(sizeof(Node));
        Node *p = head,*pre = nullptr;

        fscanf(fp_line,"%s",sta);
        fscanf(fp_distance,"%s",str);


        Line *line = Create_Line(sta,lines_num,p);
        lines[lines_num] = *line;
        while(strcmp(sta,"end") != 0){
            fscanf(fp_line,"%s",sta);
            fscanf(fp_distance,"%s",str);
            *distance = strtol(str,nullptr,10);

            for(int i=0;i<Total_stations;i++){
                if(strcmp(sta,stations[i].name)==0){
                    head->station_id = i;
                    head->next_distance = *distance;
                    head->pre_distance = MAX_DISTANCE;
                    if(pre!=nullptr){
                        head->next_distance = *distance;
                        head->pre_distance = head->pre->next_distance;
                    }

                    Node *q = (Node*)malloc(sizeof(Node));
                    q->next = nullptr;
                    q->pre = head;
                    q->station_id = -1;
                    head->next = q;
                    head->pre = pre;
                    pre = head;
                    head = q;

                    break;
                }
            }
        }
    }


    for(int i=0;i<Total_lines;i++) {
        Node *n = lines[i].head;
        while(n->next!=nullptr){
            for(auto & station : stations){
                if(station.station_id == n->station_id){
                    Add_Edge(station.first,n,i);
                }
            }
            n = n->next;
        }
    }

    for(int i=0;i<Total_lines;i++){
        nets->lines[i] = lines[i];
    }
    for(int j=0;j<Total_stations;j++){
        nets->stations[j] = stations[j];
    }
    fclose(fp_line);
    fclose(fp_distance);
    fclose(fp_station);
    return *nets;
}

void Show_Lines(Nets nets){
    for(int i=0;i<Total_lines;i++){
       Node* n = nets.lines[i].head;
        printf("%s ",nets.lines[i].name);
        while(n->next!=nullptr){
            printf("%s->%d->",nets.stations[n->station_id].name,n->next_distance);
            n = n->next;
        }
        printf("\n");
        n = n->pre;
        while(n!=nullptr){
            printf("%s=>",nets.stations[n->station_id].name);
            station_edge* q = nets.stations[n->station_id].first;
            while(q->line_id!=i){q=q->next;}
            printf("%d=>",q->pre_distance);
            n = n->pre;
        }
        printf("\n");
    }

//    for(int j=0;j<Total_stations;j++){
//        printf("%d %s ",j,nets.stations[j].name);
////        printf("%s",nets.stations[nets.stations[j].first->next->next_adj_station_id].name);
//        Station_Edge* p = nets.stations[j].first->next;
//        while(p!= nullptr){
//            printf("(%s %s)",nets.stations[p->pre_adj_station_id].name,nets.lines[p->line_id].name);
//            p = p->next;
//        }
//        printf("\n");
//    }

}

void Go(int station_1,int station_2,Mode mode,...){

}

/*
 * Nets -> stations[] / lines[] {交通网 -> 站数组 / 线数组
 *
 * Station -> station_id        {车站 -> 车站id / 车站名称 / 车站领接表}
 *          / name
 *          / Station_Edge
 *
 * Station_Edge -> next_adj_station_id  {车站领接表 -> 下一站车站id / 上一站车站id / 下一站距离 / 上一站距离 / 所在线路id / 下一个车站领接关系}
 *              / pre_adj_station_id
 *              / next_instance
 *              / pre_instance
 *              / line_id
 *              / station_edge* next
 *
 * Line -> line_id {线路 -> 线路id / 线路名称 / 线路首站头结点}
 *      / name
 *      / Node* head
 *
 * Node -> station_id   {车站节点 -> 车站id / 下一站距离 / 上一站距离 / 上一站节点 / 下一站节点}
 *  / next_distance
 *  / pre_distance
 *  / Node* next
 *  / Node* pre
 *
 * */


int main() {
    system("chcp 65001");
    Nets nets = Initialize_Nets();
    Show_Lines(nets);
    Go(1,2,Most_fast);
}
