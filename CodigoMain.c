// insertioning a key on a B-tree in C

#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

#define MAX 3
#define MIN (MAX-1)/2 //a chave que e alterada caso de overflow e a (0a >1b< 2c 3d)
typedef enum m{ //organizar o menu
	insercao=1, busca_geral, busca_ind, carregar, sair=0 
}MENU;

struct btreeNode {
  int item[MAX + 1], count;
  struct btreeNode *link[MAX + 1];
};

struct btreeNode *root;

typedef struct inserir{ //struct para a funcao carrega_arquivos
	char CodCli[3];
  char CodF[3];
  char NomeCli[50];
  char NomeF[50];
  char Genero[50];
}INSERIR;

typedef struct buscar{ //struct para a funcao carrega_arquivos
	char CodCli[3];
  char CodF[3];
}BUSCAR;

// Carrega Arquivos
void carrega_arquivos(INSERIR *add, BUSCAR *busca){
  FILE *arq;
	int i;
	/*__________CARREGA INSERIR__________*/
	i=0;
	arq = fopen("teste//insere.bin", "r+b");//cria e abre o arquivo insere
	rewind(arq);//volta ao comeÃ§o do arquivo por garantia
	//salva os registros de insercao em um vetor auxiliar add
	while(!feof(arq)){
		fread(&add[i], sizeof(INSERIR), 1, arq);
		i++;
	}
	fclose(arq);//fecha o arquivo insere
  /*__________CARREGA BUSCAR__________*/
	i=0;
	arq = fopen("teste//busca.bin", "r+b");//cria e abre o arquivo insere
	rewind(arq);//volta ao comeÃ§o do arquivo por garantia
	//salva os registros de insercao em um vetor auxiliar busca
	while(!feof(arq)){
		fread(&busca[i], sizeof(BUSCAR), 1, arq);
		i++;
	}
	printf("----Upload realizado----\n");
	
	return;
}

// Node creation
struct btreeNode *createNode(int item, struct btreeNode *child) {
  struct btreeNode *newNode;
  newNode = (struct btreeNode *)malloc(sizeof(struct btreeNode));
  newNode->item[1] = item;
  newNode->count = 1;
  newNode->link[0] = root;
  newNode->link[1] = child;
  return newNode;
}

// Insert
void insertValue(int item, int pos, struct btreeNode *node, struct btreeNode *child) {
  int j = node->count;
  while (j > pos) {
    node->item[j + 1] = node->item[j];
    node->link[j + 1] = node->link[j];
    j--;
  }
  node->item[j + 1] = item;
  node->link[j + 1] = child;
  node->count++;
}

// Split node
void splitNode(int item, int *pval, int pos, struct btreeNode *node, struct btreeNode *child, struct btreeNode **newNode) {
  int median, j;

  if (pos > MIN)
    median = MIN + 1;
  else
    median = MIN;

  *newNode = (struct btreeNode *)malloc(sizeof(struct btreeNode));
  j = median + 1;
  while (j <= MAX) {
    (*newNode)->item[j - median] = node->item[j];
    (*newNode)->link[j - median] = node->link[j];
    j++;
  }
  node->count = median;
  (*newNode)->count = MAX - median;

  if (pos <= MIN) {
    insertValue(item, pos, node, child);
  } else {
    insertValue(item, pos - median, *newNode, child);
  }
  *pval = node->item[node->count];
  (*newNode)->link[0] = node->link[node->count];
  node->count--;
}

// Set the value of node
int setNodeValue(int item, int *pval, struct btreeNode *node, struct btreeNode **child) {
  int pos;
  if (!node) {
    *pval = item;
    *child = NULL;
    return 1;
  }

  if (item < node->item[1]) {
    pos = 0;
  } else {
    for (pos = node->count;
       (item < node->item[pos] && pos > 1); pos--)
      ;
    if (item == node->item[pos]) {
      printf("Chave %d duplicada\n", item);
      return 0;
    }
  }
  if (setNodeValue(item, pval, node->link[pos], child)) {
    if (node->count < MAX) {
      insertValue(*pval, pos, node, *child);
    } else {
      splitNode(*pval, pval, pos, node, *child, child);
      return 1;
    }
  }
  return 0;
}

// Insert the value
void inserir(INSERIR *add, FILE *inseridos) {
  int flag, item, tam_reg=0, ind_add, i=0, aux;
  
  struct btreeNode *child;
	char registro[155];

  fseek(inseridos, 5 , SEEK_SET);
	fread(&ind_add, sizeof(int), 1, inseridos);
	
	/*__________ESCREVE em INSERIDOS__________*/
	sprintf(registro,"#%s#%s#%s#%s#%s", add[ind_add].CodCli, add[ind_add].CodF, 
	add[ind_add].NomeCli, add[ind_add].NomeF, add[ind_add].Genero);
	tam_reg = strlen(registro);//pega apenas o tamanho valido de registro
	tam_reg++;//\0
  aux=tam_reg;
  fseek(inseridos,0, SEEK_END);
  fwrite(&tam_reg, sizeof(int),1,inseridos);
  fwrite(registro, sizeof(char), tam_reg, inseridos); //escreve o registro
  /*__________ESCREVE em ARVORE__________*/
  // P2|@|@|@|@|-1|-1|-1|-1|-1
  int aux1=atol(add[ind_add].CodCli),
  aux2=atol(add[ind_add].CodCli);
  item = aux1 + aux2;
  fseek(inseridos, 5 , 0);
  ind_add++;
  fwrite(&ind_add, sizeof(int), 1, inseridos);
  flag = setNodeValue(item, &i, root, &child);
  if (flag)
    root = createNode(i, child);
}

// Copy the successor
void copySuccessor(struct btreeNode *myNode, int pos) {
  struct btreeNode *dummy;
  dummy = myNode->link[pos];

  for (; dummy->link[0] != NULL;)
    dummy = dummy->link[0];
  myNode->item[pos] = dummy->item[1];
}

// Do rightshift
void rightShift(struct btreeNode *myNode, int pos) {
  struct btreeNode *x = myNode->link[pos];
  int j = x->count;

  while (j > 0) {
    x->item[j + 1] = x->item[j];
    x->link[j + 1] = x->link[j];
  }
  x->item[1] = myNode->item[pos];
  x->link[1] = x->link[0];
  x->count++;

  x = myNode->link[pos - 1];
  myNode->item[pos] = x->item[x->count];
  myNode->link[pos] = x->link[x->count];
  x->count--;
  return;
}

// Do leftshift
void leftShift(struct btreeNode *myNode, int pos) {
  int j = 1;
  struct btreeNode *x = myNode->link[pos - 1];

  x->count++;
  x->item[x->count] = myNode->item[pos];
  x->link[x->count] = myNode->link[pos]->link[0];

  x = myNode->link[pos];
  myNode->item[pos] = x->item[1];
  x->link[0] = x->link[1];
  x->count--;

  while (j <= x->count) {
    x->item[j] = x->item[j + 1];
    x->link[j] = x->link[j + 1];
    j++;
  }
  return;
}

// Merge the nodes
void mergeNodes(struct btreeNode *myNode, int pos) {
  int j = 1;
  struct btreeNode *x1 = myNode->link[pos], *x2 = myNode->link[pos - 1];

  x2->count++;
  x2->item[x2->count] = myNode->item[pos];
  x2->link[x2->count] = myNode->link[0];

  while (j <= x1->count) {
    x2->count++;
    x2->item[x2->count] = x1->item[j];
    x2->link[x2->count] = x1->link[j];
    j++;
  }

  j = pos;
  while (j < myNode->count) {
    myNode->item[j] = myNode->item[j + 1];
    myNode->link[j] = myNode->link[j + 1];
    j++;
  }
  myNode->count--;
  free(x1);
}

// Adjust the node
void adjustNode(struct btreeNode *myNode, int pos) {
  if (!pos) {
    if (myNode->link[1]->count > MIN) {
      leftShift(myNode, 1);
    } else {
      mergeNodes(myNode, 1);
    }
  } else {
    if (myNode->count != pos) {
      if (myNode->link[pos - 1]->count > MIN) {
        rightShift(myNode, pos);
      } else {
        if (myNode->link[pos + 1]->count > MIN) {
          leftShift(myNode, pos + 1);
        } else {
          mergeNodes(myNode, pos);
        }
      }
    } else {
      if (myNode->link[pos - 1]->count > MIN)
        rightShift(myNode, pos);
      else
        mergeNodes(myNode, pos);
    }
  }
}

// Traverse the tree
void traversal(struct btreeNode *myNode) {
  int i;
  if (myNode) {
    for (i = 0; i < myNode->count; i++) {
      traversal(myNode->link[i]);
      printf("Chave %d inserida\n", myNode->item[i + 1]);
    }
    traversal(myNode->link[i]);
  }
}

// Busca por cliente específico
void busca_um(BUSCAR *busca){
  int i;
  for(i=0; i<3;i++){
      printf("cod cliente: %s | ", busca[i].CodCli);
      printf("cod filme: %s \n", busca[i].CodF);
  }
}

// Busca por todos os clientes
void busca_todos(){

}
int main() {
  int item, op, tam_reg, ind_add=0, ind_busca=0;
  INSERIR add[30];
  BUSCAR busca[30];
  FILE *inseridos;
  if(access("inseridos.bin", F_OK)!=0){ //se o arquivo nÃ£o existe
    inseridos = fopen("inseridos.bin", "w+b"); //cria o arquivo
    tam_reg=sizeof(int)+sizeof(int)+sizeof(int)+sizeof(char)+sizeof(char)+sizeof(char); //(15)
    fwrite(&tam_reg,sizeof(int),1,inseridos); //tamanho do header
    fwrite("#",sizeof(char),1,inseridos);
    fwrite(&ind_add,sizeof(int),1,inseridos); //indice de insercoes ja realizadas
    fwrite("#",sizeof(char),1,inseridos);
    fwrite(&ind_busca,sizeof(int),1,inseridos); //indice de buscas ja realizadas
    fwrite("#",sizeof(char),1,inseridos);
  }else{
    inseridos = fopen("inseridos.bin", "r+b"); //le o arquivo
  }
  do{
		printf("\n1-Insere\n2-Listar os dados de todos os clientes\n3-Listar os dados de um cliente específico\n4-Carrega Arquivos\n0-Sair\nEscolha:");
    scanf(" %d", &op);
    switch(op){
      case insercao://1
        inserir(add, inseridos);
        break;
      case busca_geral: //2
        busca_todos();     
        break;
      case busca_ind: //3
        busca_um(busca);     
        break;
      case carregar: //4
        carrega_arquivos(add, busca);
        break;
      default: //sair 0
        break;
    }        
  }while(op!=0);

  traversal(root);
}