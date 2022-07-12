#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFSIZE_INFO 60
#define PATH "/proc" 

/*-------------------------define struct body--------------------------------*/
struct Process{
	pid_t pid;
	pid_t ppid;
	char *name[BUFFSIZE_INFO];
};

typedef struct Node {
	struct Node *Parent;
	struct Node *Child;
	struct Process P;
       	struct Node *next;
	int ChildNum;	
}*PNode;

typedef struct qnode
{
	PNode p;
	struct qnode *next;
}QNode,*QueuePtr;

typedef struct queue
{
	QueuePtr front;
	QueuePtr rear;
}LinkQueue;


/*-------------------------define global variable-------------------------------*/
struct Process ProcessList[500];
int Process_Number = 0;
PNode Tree;
LinkQueue Q;

/*-------------------------declare function--------------------------------*/
pid_t GetPPid(char *filename);
void Set_Pid_PPid();
void PrintList();
void GetTree();
void PrintPsTree(PNode p,int layer);
PNode LevelOrder(pid_t pid);
void InitQueue();
void EnQueue(PNode p);
void DeQueue(PNode *p_ptr);
void ClearQueue();

/*-------------------------main--------------------------------*/
int main()
{
	Tree = (PNode) malloc(sizeof(struct Node));
	Set_Pid_PPid();
	//PrintList();	
	InitQueue();
	GetTree();
	PrintPsTree(Tree,-1);
	return 0;
}


/*-------------------------define function--------------------------------*/
int GetPPid(char *filename){
	pid_t ppid = -100;
	char *right = NULL;
	FILE *fp = fopen(filename,"r");
	char info[BUFFSIZE_INFO+1];info[BUFFSIZE_INFO] = '\0';

	if(fp == NULL){
		fprintf(stderr,"open file %s error!" , filename);
		return -1;
	}
	if(fgets(info,BUFFSIZE_INFO,fp) == NULL){
		fprintf(stderr,"fgets error!");
		exit(0);
	}
	right = strrchr(info,')');
	if(right == NULL)
		printf("Not find \')\'\n");
	right += 3;
	sscanf(right,"%d",&ppid);
	return ppid;

}

void Set_Pid_PPid(){
	pid_t pid,ppid;
	DIR *dir_ptr;
	struct dirent *direntp;
	char process_path[51] = "/proc/";
	char stat[6] = "/stat";
	char pid_str[20];

	dir_ptr = opendir(PATH);
	if(dir_ptr == NULL)
	{
		fprintf(stderr,"can not open /proc\n");
		exit(0);
	}

	while(direntp=readdir(dir_ptr))
	{
		pid = atoi(direntp->d_name);
		if(pid!=0)
		{
			//concat string : process_path
			//examples : /proc/pid/stat
			ProcessList[Process_Number].pid = pid;
			sprintf(pid_str,"%d",pid);
			strcat(process_path,pid_str);
			strcat(process_path,stat);

			ppid = GetPPid(process_path);
			if(ppid!=-1)
				ProcessList[Process_Number++].ppid = ppid;
			else
				Process_Number++;
			process_path[6] = '\0';
		}
	}
}

void PrintList()
{
	printf("pid\t\tppid\n");
	for(int i=0;i<Process_Number;i++){
		printf("%d\t\t%d\n",ProcessList[i].pid,ProcessList[i].ppid);
	}
	return;
}

void GetTree(){
	Tree->ChildNum = 0;Tree->P.pid = 0;Tree->P.ppid = -1;Tree->Parent=NULL;Tree->Child = NULL;Tree->next = NULL;
	for(int i=0;i<Process_Number;i++){
		struct Process p = ProcessList[i];
		PNode Parent = LevelOrder(p.ppid);
		PNode p_ptr = (PNode)malloc(sizeof(struct Node));
		p_ptr->P = p;p_ptr->ChildNum = 0;p_ptr->Parent = Parent;p_ptr->next = NULL;
		if(Parent->Child ==NULL){
			Parent->Child = p_ptr;
		}else{
			PNode Temp = Parent->Child;
			while(Temp->next!=NULL){
				Temp = Temp->next;
			}
			Temp->next = p_ptr;
			
		}
	}
}

PNode LevelOrder(pid_t pid){
	EnQueue(Tree);
	PNode p,p_temp;
	while(Q.front != Q.rear && Q.rear != NULL){
		DeQueue(&p_temp);
		if(p_temp->P.pid == pid){
			p = p_temp;
			ClearQueue();
			return p;
		}else{
			PNode child_ptr = p_temp->Child;
			while(child_ptr!=NULL){
				EnQueue(child_ptr);
				child_ptr = child_ptr->next;
			}
		}		
	}
	ClearQueue();
	printf("Not Find Process %d\n",pid);
	return NULL;
}

void InitQueue()
{
	QueuePtr Head_Node;
	Head_Node = (QueuePtr)malloc(sizeof(QNode));
	Head_Node->next = NULL;
	Q.front = Head_Node;
	Q.rear = Q.front;
	return;
}

void EnQueue(PNode p)
{
	QueuePtr new_node;
	new_node = (QueuePtr)malloc(sizeof(QNode));
	new_node->p = p;
	new_node->next = Q.rear->next;
	Q.rear->next = new_node;
	Q.rear = new_node;
	return;
}

void DeQueue(PNode *p_ptr)
{
	QueuePtr node;
	if(Q.front == Q.rear||Q.rear == NULL)
	{
		printf("队列是空的，不能出列了！！！");
		return;
	}
	*p_ptr = Q.front->next->p;
	node = Q.front->next;
	Q.front->next = node->next;
	if(node==Q.rear)
		Q.rear = Q.front;
	free(node);
	node = NULL;
	return;
}

void ClearQueue(){
	QueuePtr node = Q.front->next;
	while(node != NULL){
		Q.front->next = node->next;
		free(node);
		node = Q.front->next;
	}
	Q.front->next = NULL;
	Q.rear = Q.front;
}

void PrintPsTree(PNode p,int layer){
	if(p!=NULL){
		if(p->P.pid!=0){
			for(int i=0;i<layer;i++)
				printf("--");
			printf("pid : %d , ppid : %d\n",p->P.pid,p->P.ppid);
		}
		PNode child = p->Child;
		while(child!=NULL){
			PrintPsTree(child, layer+1);
			child = child->next;
		}
	}
}
