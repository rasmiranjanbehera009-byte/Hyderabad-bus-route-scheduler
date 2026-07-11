#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_STOPS        20
#define MAX_NAME_LEN     50
#define BUS_ID_LEN       12
#define INF              INT_MAX
#define DEFAULT_CAPACITY 52
#define CQ_SIZE          15
#define PATH_LEN         MAX_STOPS

/*
 * ============================================================
 *  HYDERABAD / SECUNDERABAD BUS ROUTE SCHEDULER
 *  Real stops around Secunderabad area, Hyderabad
 *
 *  Stop Index Map:
 *   0  Secunderabad Bus Stand (Central)
 *   1  Paradise Circle
 *   2  Begumpet
 *   3  Ameerpet
 *   4  SR Nagar
 *   5  Balkampet
 *   6  Trimulgherry
 *   7  Mettuguda
 *   8  Tarnaka
 *   9  Uppal
 *  10  ECIL X Roads
 *  11  Habsiguda
 *  12  Musheerabad
 *  13  Gandhi Hospital
 *  14  Koti
 * ============================================================
 */

/* ===================================================
   1. GRAPH -- Adjacency List
   =================================================== */
typedef struct Edge { int dest, travel_min, route_no; struct Edge *next; } Edge;
typedef struct { char name[MAX_NAME_LEN]; Edge *head; } Stop;
typedef struct { int num_stops; Stop stops[MAX_STOPS]; } Graph;

void graph_add_edge(Graph *g, int src, int dest, int mins, int route) {
    Edge *e = malloc(sizeof(Edge));
    e->dest=dest; e->travel_min=mins; e->route_no=route;
    e->next=g->stops[src].head; g->stops[src].head=e;
}
void graph_add_bidir(Graph *g,int a,int b,int m,int r){
    graph_add_edge(g,a,b,m,r);
    graph_add_edge(g,b,a,m,r);
}

void graph_print(const Graph *g) {
    printf("\n  ADJACENCY LIST -- Hyderabad Bus Stop Network\n");
    printf("  %-4s %-24s  Connections\n","Idx","Stop Name");
    printf("  ---------------------------------------------------------------\n");
    for(int i=0;i<g->num_stops;i++){
        printf("  [%2d] %-24s ->",i,g->stops[i].name);
        Edge *e=g->stops[i].head;
        if(!e){printf(" (none)\n");continue;}
        while(e){
            printf("  %s (%dmin,R%d)",g->stops[e->dest].name,e->travel_min,e->route_no);
            e=e->next; if(e) printf(" |");
        }
        printf("\n");
    }
}

/* ===================================================
   2. CIRCULAR QUEUE -- Vehicle Arrivals
   =================================================== */
typedef enum{ONTIME,DELAYED,DEPARTED}BusStatus;
typedef struct{
    char bus_id[BUS_ID_LEN];
    int  route_no,arrival_min,capacity,passengers;
    BusStatus status;
}Bus;
typedef struct{Bus data[CQ_SIZE];int front,rear,size;}CircQueue;

void cq_init(CircQueue*q){q->front=q->rear=q->size=0;}
int  cq_full(CircQueue*q){return q->size==CQ_SIZE;}
int  cq_empty(CircQueue*q){return q->size==0;}
int  cq_enqueue(CircQueue*q,Bus b){
    if(cq_full(q))return 0;
    q->data[q->rear]=b; q->rear=(q->rear+1)%CQ_SIZE; q->size++; return 1;
}
Bus  cq_dequeue(CircQueue*q){
    Bus b=q->data[q->front]; q->front=(q->front+1)%CQ_SIZE; q->size--; return b;
}
Bus *cq_peek(CircQueue*q){return cq_empty(q)?NULL:&q->data[q->front];}

void cq_print(CircQueue*q){
    printf("\n  CIRCULAR QUEUE -- Vehicle Arrivals at Secunderabad Bus Stand\n");
    printf("  Ring buffer: size=%d  front=%d  rear=%d  used=%d/%d\n",
           CQ_SIZE,q->front,q->rear,q->size,CQ_SIZE);
    printf("  ---------------------------------------------------------------\n");
    printf("  %-10s %-7s %-8s %-10s %-10s %s\n",
           "Bus ID","Route","Arrival","Capacity","Passengers","Status");
    printf("  ---------------------------------------------------------------\n");
    for(int i=0;i<q->size;i++){
        Bus*b=&q->data[(q->front+i)%CQ_SIZE];
        const char*ss=(b->status==DELAYED)?"** DELAYED **":(b->status==DEPARTED)?"DEPARTED":"ON TIME";
        printf("  %-10s %-7d %02d:%02d     %-10d %-10d %s\n",
               b->bus_id,b->route_no,
               b->arrival_min/60,b->arrival_min%60,
               b->capacity,b->passengers,ss);
    }
    printf("\n  [FRONT] -> slot %d   [REAR] -> slot %d\n",
           q->front,(q->rear-1+CQ_SIZE)%CQ_SIZE);
}

/* ===================================================
   3. PASSENGER LINKED LIST
   =================================================== */
typedef struct Passenger{
    int id,dest_stop;
    char name[MAX_NAME_LEN];
    struct Passenger*next;
}Passenger;
typedef struct{Passenger*front,*rear;int count;}PassQueue;

void pq_init(PassQueue*pq){pq->front=pq->rear=NULL;pq->count=0;}
void pq_enqueue(PassQueue*pq,int id,const char*name,int dest){
    Passenger*p=malloc(sizeof(Passenger));
    p->id=id; p->dest_stop=dest; p->next=NULL;
    strncpy(p->name,name,MAX_NAME_LEN-1); p->name[MAX_NAME_LEN-1]='\0';
    if(!pq->rear) pq->front=pq->rear=p;
    else{pq->rear->next=p; pq->rear=p;}
    pq->count++;
}
Passenger*pq_dequeue(PassQueue*pq){
    if(!pq->front) return NULL;
    Passenger*p=pq->front;
    pq->front=pq->front->next;
    if(!pq->front) pq->rear=NULL;
    pq->count--; return p;
}
void pq_print(PassQueue*pq,const Graph*g){
    printf("\n  PASSENGER LINKED LIST -- Waiting at Secunderabad Bus Stand\n");
    printf("  Total passengers: %d\n",pq->count);
    printf("  -------------------------------------------------\n");
    if(!pq->front){printf("  (empty)\n");return;}
    Passenger*p=pq->front; int pos=1;
    while(p){
        printf("  [%d] %-22s -> %s\n",pos++,p->name,g->stops[p->dest_stop].name);
        if(p->next) printf("   |\n   v\n");
        p=p->next;
    }
    printf("  [NULL]\n");
}
void pq_free(PassQueue*pq){
    Passenger*p=pq->front;
    while(p){Passenger*t=p->next;free(p);p=t;}
    pq_init(pq);
}

/* ===================================================
   4. MIN-HEAP
   =================================================== */
typedef struct{int stop,dist;}HNode;
typedef struct{HNode data[MAX_STOPS*MAX_STOPS];int size;}MinHeap;

static void hswap(MinHeap*h,int a,int b){
    HNode t=h->data[a]; h->data[a]=h->data[b]; h->data[b]=t;
}
void heap_push(MinHeap*h,int stop,int dist){
    int i=h->size++; h->data[i]=(HNode){stop,dist};
    while(i&&h->data[(i-1)/2].dist>h->data[i].dist){hswap(h,i,(i-1)/2);i=(i-1)/2;}
}
HNode heap_pop(MinHeap*h){
    HNode top=h->data[0]; h->data[0]=h->data[--h->size]; int i=0;
    while(1){
        int l=2*i+1,r=2*i+2,s=i;
        if(l<h->size&&h->data[l].dist<h->data[s].dist)s=l;
        if(r<h->size&&h->data[r].dist<h->data[s].dist)s=r;
        if(s==i)break; hswap(h,i,s); i=s;
    }
    return top;
}

/* ===================================================
   5. DIJKSTRA
   =================================================== */
void dijkstra(const Graph*g,int src,int dist[],int prev[]){
    MinHeap heap={.size=0};
    for(int i=0;i<g->num_stops;i++) dist[i]=INF,prev[i]=-1;
    dist[src]=0; heap_push(&heap,src,0);
    while(heap.size){
        HNode cur=heap_pop(&heap); int u=cur.stop;
        if(cur.dist>dist[u]) continue;
        for(Edge*e=g->stops[u].head;e;e=e->next){
            if(dist[u]+e->travel_min<dist[e->dest]){
                dist[e->dest]=dist[u]+e->travel_min;
                prev[e->dest]=u;
                heap_push(&heap,e->dest,dist[e->dest]);
            }
        }
    }
}
static int build_path(int prev[],int d,int path[]){
    if(prev[d]==-1){path[0]=d;return 1;}
    int len=build_path(prev,prev[d],path);
    path[len]=d; return len+1;
}
void print_path(const Graph*g,int prev[],int dst){
    int path[PATH_LEN];
    int len=build_path(prev,dst,path);
    for(int i=0;i<len;i++){if(i)printf(" -> ");printf("%s",g->stops[path[i]].name);}
}

/* ===================================================
   HYDERABAD / SECUNDERABAD NETWORK SETUP
   =================================================== */
void setup_network(Graph*g){
    g->num_stops=15;
    const char*names[15]={
        "Secunderabad Stand", /* 0  — main hub                  */
        "Paradise Circle",   /* 1  — iconic landmark            */
        "Begumpet",          /* 2  — airport road area          */
        "Ameerpet",          /* 3  — major junction             */
        "SR Nagar",          /* 4                               */
        "Balkampet",         /* 5                               */
        "Trimulgherry",      /* 6  — cantonment area            */
        "Mettuguda",         /* 7                               */
        "Tarnaka",           /* 8                               */
        "Uppal",             /* 9  — east Hyderabad             */
        "ECIL X Roads",      /* 10                              */
        "Habsiguda",         /* 11                              */
        "Musheerabad",       /* 12                              */
        "Gandhi Hospital",   /* 13 — city centre                */
        "Koti"               /* 14 — old city edge              */
    };
    for(int i=0;i<15;i++){
        strncpy(g->stops[i].name,names[i],MAX_NAME_LEN-1);
        g->stops[i].head=NULL;
    }

    /*
     * ROUTE 1  — Secunderabad <-> Ameerpet  (via Paradise, Begumpet)
     *   Sec Stand -> Paradise  7 min
     *   Paradise  -> Begumpet  8 min
     *   Begumpet  -> Ameerpet  9 min
     */
    graph_add_bidir(g, 0, 1,  7, 1);
    graph_add_bidir(g, 1, 2,  8, 1);
    graph_add_bidir(g, 2, 3,  9, 1);

    /*
     * ROUTE 2  — Secunderabad <-> Uppal  (via Mettuguda, Tarnaka)
     *   Sec Stand  -> Mettuguda  10 min
     *   Mettuguda  -> Tarnaka     8 min
     *   Tarnaka    -> Uppal      12 min
     */
    graph_add_bidir(g, 0, 7, 10, 2);
    graph_add_bidir(g, 7, 8,  8, 2);
    graph_add_bidir(g, 8, 9, 12, 2);

    /*
     * ROUTE 3  — Paradise <-> ECIL X Roads  (via Musheerabad, Habsiguda)
     *   Paradise    -> Musheerabad  6 min
     *   Musheerabad -> Habsiguda   10 min
     *   Habsiguda   -> ECIL X Roads 9 min
     */
    graph_add_bidir(g, 1, 12,  6, 3);
    graph_add_bidir(g,12, 11, 10, 3);
    graph_add_bidir(g,11, 10,  9, 3);

    /*
     * ROUTE 4  — Ameerpet <-> Balkampet <-> SR Nagar
     *   Ameerpet -> SR Nagar   5 min
     *   SR Nagar -> Balkampet  7 min
     */
    graph_add_bidir(g, 3, 4,  5, 4);
    graph_add_bidir(g, 4, 5,  7, 4);

    /*
     * ROUTE 5  — Secunderabad <-> Gandhi Hospital <-> Koti
     *   Sec Stand      -> Gandhi Hospital 12 min
     *   Gandhi Hospital-> Koti             8 min
     */
    graph_add_bidir(g, 0, 13, 12, 5);
    graph_add_bidir(g,13, 14,  8, 5);

    /*
     * ROUTE 6  — Cross links
     *   Secunderabad -> Trimulgherry  9 min  (cantonment route)
     *   Tarnaka      -> ECIL X Roads 11 min  (east link)
     *   Koti         -> Ameerpet     15 min  (old city to junction)
     */
    graph_add_bidir(g, 0,  6,  9, 6);
    graph_add_bidir(g, 8, 10, 11, 6);
    graph_add_bidir(g,14,  3, 15, 6);
}

/*
 * TSRTC buses operating around Secunderabad
 * Real TSRTC route numbers used in Hyderabad
 */
void setup_buses(CircQueue*cq){
    Bus fleet[]={
        {"HYD-5K",   5, 360, DEFAULT_CAPACITY, 0, ONTIME},   /* 06:00 Sec->Ameerpet    */
        {"HYD-10C",  2, 375, DEFAULT_CAPACITY, 0, ONTIME},   /* 06:15 Sec->Uppal       */
        {"HYD-216",  3, 390, DEFAULT_CAPACITY, 0, DELAYED},  /* 06:30 Paradise->ECIL   */
        {"HYD-49M",  4, 405, DEFAULT_CAPACITY, 0, ONTIME},   /* 06:45 Ameerpet->Balkam */
        {"HYD-8J",   5, 420, DEFAULT_CAPACITY, 0, ONTIME},   /* 07:00 Sec->Koti        */
        {"HYD-127",  1, 435, DEFAULT_CAPACITY, 0, ONTIME},   /* 07:15 Sec->Ameerpet    */
        {"HYD-73G",  6, 450, DEFAULT_CAPACITY, 0, ONTIME},   /* 07:30 Trimulgherry     */
    };
    for(int i=0;i<7;i++) cq_enqueue(cq,fleet[i]);
}

/* Passengers with common Hyderabad names */
void setup_passengers(PassQueue*pq){
    pq_enqueue(pq, 1, "Srinivas Reddy",    9);  /* -> Uppal          */
    pq_enqueue(pq, 2, "Kavitha Sharma",    3);  /* -> Ameerpet       */
    pq_enqueue(pq, 3, "Mohammed Farhan",  10);  /* -> ECIL X Roads   */
    pq_enqueue(pq, 4, "Priya Nair",       14);  /* -> Koti           */
    pq_enqueue(pq, 5, "Rajesh Yadav",      2);  /* -> Begumpet       */
    pq_enqueue(pq, 6, "Sunita Kumari",    13);  /* -> Gandhi Hospital*/
    pq_enqueue(pq, 7, "Arjun Teja",        6);  /* -> Trimulgherry   */
}

/* ===================================================
   UI & MENUS
   =================================================== */
void press_enter(void){
    printf("\n  [Press Enter to continue]");
    while(getchar()!='\n');
}

void print_header(void){
    printf("\n");
    printf("  +--------------------------------------------------------------+\n");
    printf("  |   HYDERABAD BUS ROUTE SCHEDULER  --  DSA Project             |\n");
    printf("  |   Area: Secunderabad & Surroundings, Hyderabad, Telangana     |\n");
    printf("  |   DS: Graph | Circular Queue | Linked List | Min-Heap         |\n");
    printf("  +--------------------------------------------------------------+\n");
}

void print_stop_list(const Graph*g){
    printf("\n  STOP REFERENCE\n");
    printf("  -----------------------------------------------\n");
    for(int i=0;i<g->num_stops;i++)
        printf("  [%2d] %s\n",i,g->stops[i].name);
    printf("  -----------------------------------------------\n");
}

void print_menu(void){
    printf("\n  +------------------------------------------------------+\n");
    printf("  | MENU                                                 |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  | 1  View Network Graph    (Adjacency List)            |\n");
    printf("  | 2  View Arrival Schedule (Circular Queue)            |\n");
    printf("  | 3  View Passenger Queue  (Linked List)               |\n");
    printf("  | 4  Find Shortest Path    (Dijkstra + Min-Heap)       |\n");
    printf("  | 5  Simulate Bus Boarding                             |\n");
    printf("  | 6  Schedule a New Bus    (Enqueue)                   |\n");
    printf("  | 7  Add a Passenger       (Enqueue)                   |\n");
    printf("  | 8  Add a Route           (Graph Edge)                |\n");
    printf("  | 9  Dequeue Next Bus      (Process Departure)         |\n");
    printf("  | 0  Exit                                              |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  Choice: ");
}

int pick_stop(const Graph*g,const char*prompt){
    printf("\n  %s\n",prompt);
    for(int i=0;i<g->num_stops;i++)
        printf("    [%2d] %s\n",i,g->stops[i].name);
    printf("  Enter (0-%d): ",g->num_stops-1);
    int x; scanf("%d",&x); getchar();
    if(x<0||x>=g->num_stops){printf("  Invalid stop.\n");return -1;}
    return x;
}

/* ===================================================
   MENU ACTIONS
   =================================================== */
void do_shortest_path(const Graph*g){
    int src=pick_stop(g,"SOURCE stop:"); if(src<0)return;
    int dst=pick_stop(g,"DESTINATION stop:"); if(dst<0)return;
    if(src==dst){printf("  Already at destination!\n");return;}
    int dist[MAX_STOPS],prev[MAX_STOPS];
    dijkstra(g,src,dist,prev);
    printf("\n  DIJKSTRA SHORTEST PATH RESULT\n");
    printf("  ------------------------------------------------------\n");
    printf("  From : %s\n",g->stops[src].name);
    printf("  To   : %s\n",g->stops[dst].name);
    if(dist[dst]==INF) printf("  Result : No path found!\n");
    else{
        printf("  Path   : "); print_path(g,prev,dst);
        printf("\n  Time   : %d minutes\n",dist[dst]);
    }
    printf("\n  All distances from %s:\n",g->stops[src].name);
    printf("  %-24s %s\n","Stop","Travel Time");
    printf("  ----------------------------------------\n");
    for(int i=0;i<g->num_stops;i++){
        if(i==src)continue;
        if(dist[i]==INF) printf("  %-24s unreachable\n",g->stops[i].name);
        else             printf("  %-24s %d min\n",g->stops[i].name,dist[i]);
    }
}

void do_boarding(PassQueue*pq,CircQueue*cq,const Graph*g){
    if(cq_empty(cq)){printf("\n  No buses in schedule!\n");return;}
    if(!pq->front){printf("\n  No passengers waiting.\n");return;}
    Bus*b=cq_peek(cq);
    int avail=b->capacity-b->passengers;
    printf("\n  BOARDING SIMULATION\n");
    printf("  Bus %-10s | Route %d | %02d:%02d | %d seats available\n\n",
           b->bus_id,b->route_no,b->arrival_min/60,b->arrival_min%60,avail);
    int n=0;
    while(pq->count&&n<avail){
        Passenger*p=pq_dequeue(pq);
        b->passengers++; n++;
        printf("  [BOARDED] %-22s -> %s\n",p->name,g->stops[p->dest_stop].name);
        free(p);
    }
    if(pq->count) printf("\n  %d passenger(s) remain (bus full).\n",pq->count);
    else          printf("\n  All passengers boarded!\n");
    printf("  Bus occupancy: %d / %d\n",b->passengers,b->capacity);
}

void do_add_bus(CircQueue*cq){
    if(cq_full(cq)){printf("\n  Queue full!\n");return;}
    Bus b;
    printf("\n  Bus ID      : "); scanf("%11s",b.bus_id); getchar();
    printf("  Route #     : "); scanf("%d",&b.route_no); getchar();
    int hh,mm;
    printf("  Time (HH MM): "); scanf("%d %d",&hh,&mm); getchar();
    b.arrival_min=hh*60+mm; b.capacity=DEFAULT_CAPACITY;
    b.passengers=0; b.status=ONTIME;
    cq_enqueue(cq,b);
    printf("  Bus %s scheduled at %02d:%02d. Queue size: %d\n",
           b.bus_id,hh,mm,cq->size);
}

void do_add_passenger(PassQueue*pq,const Graph*g){
    static int nid=100;
    char name[MAX_NAME_LEN];
    printf("\n  Passenger name: "); fgets(name,MAX_NAME_LEN,stdin);
    name[strcspn(name,"\n")]='\0';
    int dest=pick_stop(g,"Destination stop:"); if(dest<0)return;
    pq_enqueue(pq,nid++,name,dest);
    printf("  Added %s -> %s. Total waiting: %d\n",
           name,g->stops[dest].name,pq->count);
}

void do_add_route(Graph*g){
    int a=pick_stop(g,"Stop A:"); if(a<0)return;
    int b=pick_stop(g,"Stop B:"); if(b<0)return;
    int mins,route;
    printf("  Travel time (min): "); scanf("%d",&mins); getchar();
    printf("  Route number     : "); scanf("%d",&route); getchar();
    graph_add_bidir(g,a,b,mins,route);
    printf("  Route added: %s <-> %s (%d min, Route %d)\n",
           g->stops[a].name,g->stops[b].name,mins,route);
}

void do_dequeue_bus(CircQueue*cq){
    if(cq_empty(cq)){printf("\n  Queue empty!\n");return;}
    Bus b=cq_dequeue(cq);
    printf("\n  BUS DEPARTED\n");
    printf("  %-12s | Route %-4d | Departed %02d:%02d | Load %d/%d\n",
           b.bus_id,b.route_no,b.arrival_min/60,b.arrival_min%60,
           b.passengers,b.capacity);
    printf("  Remaining in queue: %d bus(es)\n",cq->size);
}

/* ===================================================
   MAIN
   =================================================== */
int main(void){
    Graph     g;
    CircQueue cq; cq_init(&cq);
    PassQueue pq; pq_init(&pq);

    setup_network(&g);
    setup_buses(&cq);
    setup_passengers(&pq);

    print_header();
    print_stop_list(&g);
    printf("\n  Loaded: %d stops | 6 routes | %d buses | %d passengers\n",
           g.num_stops,cq.size,pq.count);

    int choice;
    while(1){
        print_menu();
        if(scanf("%d",&choice)!=1){getchar();continue;}
        getchar();
        switch(choice){
            case 1: graph_print(&g);                   break;
            case 2: cq_print(&cq);                     break;
            case 3: pq_print(&pq,&g);                  break;
            case 4: do_shortest_path(&g);              break;
            case 5: do_boarding(&pq,&cq,&g);           break;
            case 6: do_add_bus(&cq);                   break;
            case 7: do_add_passenger(&pq,&g);          break;
            case 8: do_add_route(&g);                  break;
            case 9: do_dequeue_bus(&cq);               break;
            case 0:
                printf("\n  Goodbye!\n\n");
                pq_free(&pq);
                for(int i=0;i<g.num_stops;i++){
                    Edge*e=g.stops[i].head;
                    while(e){Edge*t=e->next;free(e);e=t;}
                }
                return 0;
            default: printf("  Invalid choice. Enter 0-9.\n");
        }
        press_enter();
    }
}
