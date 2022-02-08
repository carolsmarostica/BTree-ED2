//Exercicio 8
//Caio de Assis Ribeiro e Carolina Silva Marostica
//exercicio de Arvore-B de ordem 4

//IMPORTS
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

//DEFINICOES
#define MAXKEYS 3
#define MINKEYS (MAXKEYS-1)/2 //a chave que e alterada caso de overflow e a (0a >1b< 2c 3d)
#define NIL (-1)
#define NOKEY -1
#define NO 0
#define YES 1

//MENU
typedef enum m{ //organizar o menu
	insercao=1, busca_geral, busca_ind, carregar, sair=0 
}MENU;

//STRUCTS
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
typedef struct{
    int keycount; //numero de chaves na pagina
    int key[MAXKEYS]; //a chave atual
    int child[MAXKEYS+1]; //ponteiros para os rrn de descendentes
}BTPAGE;
#define PAGESIZE sizeof(BTPAGE);

//VARIAVEIS GLOBAIS
FILE *btree; //cria o arquivo da arvore como sendo global
FILE *inseridos;
INSERIR add[30];
BUSCAR busca[30];

//PROTOTIPOS
void carrega_arquivos();
void btclose();
int btopen();
int btread(int rrn, BTPAGE *page_ptr);
int btwrite(int rrn, BTPAGE *page_ptr);
int create_root(int key, int left, int right);
int create_tree();
int getpage();
int getroot();
int insert(int rrn, int key, int *promo_r_child, int *promo_key);
void ins_in_page(int key,int r_child, BTPAGE *p_page);
void pageinit(BTPAGE *p_page);
void putroot(int root);
int search_node(int key, BTPAGE *p_page, int *pos);
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage);
int pegakey();

//SUBROTINAS
void carrega_arquivos(){//---------------------------------------------------------------CARREGA OS ARQUIVOS BASE
    FILE *arq;
    int i=0;
	//carrega vetor de struct add
	    arq = fopen("CasosTeste//insere.bin", "r+b");//abre o arquivo insere
	    rewind(arq);//volta ao comeco do arquivo por garantia
	    //salva os registros de insercao em um vetor auxiliar add
	    while(!feof(arq)){
		    fread(&add[i], sizeof(INSERIR), 1, arq);
            i++;
	    }
	    fclose(arq);//fecha o arquivo insere
    //carrega vetor de struct busca
	    arq = fopen("CasosTeste//busca.bin", "r+b");//abre o arquivo busca
	    rewind(arq);//volta ao comeco do arquivo por garantia
	    //salva os registros de busca em um vetor auxiliar busca
	    i=0;
        while(!feof(arq)){
		    fread(&busca[i], sizeof(BUSCAR), 1, arq);
            i++;
        }

	printf("----Upload realizado----\n");
	return;
}
void btclose(){//------------------------------------------------------------------------FECHA OS ARQUIVOS ABERTOS
    fclose(btree);
    fclose(inseridos);
}
int btopen(){//--------------------------------------------------------------------------CRIA OS ARQUIVOS BASE
    int tam_reg, ind_add, ind_busca, dist;

    if(access("inseridos.bin", F_OK)!=0){ //se o arquivo nao existe
       btree = fopen("btree.bin", "w+b"); //cria o arquivo de indice da arvore
        putroot(-1); 
        //------------header, numero da pagina da raiz?------------
        inseridos = fopen("inseridos.bin", "w+b"); //cria o arquivo dos registros
        tam_reg=sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(char)+
        sizeof(char)+sizeof(char)+sizeof(char); //(20)
        fwrite(&tam_reg,sizeof(int),1,inseridos); //tamanho do header
        fwrite("#",sizeof(char),1,inseridos);
        fwrite(&ind_add,sizeof(int),1,inseridos); //indice de insercoes ja realizadas
        fwrite("#",sizeof(char),1,inseridos);
        fwrite(&ind_busca,sizeof(int),1,inseridos); //indice de buscas ja realizadas
        fwrite("#",sizeof(char),1,inseridos);
        fwrite(&dist,sizeof(int),1,inseridos); //distancia do header
        fwrite("#",sizeof(char),1,inseridos);
        return NO; //os arquivos foram criados
    }else{
        //--------------------fazer criar novos arquivos
        inseridos = fopen("inseridos.bin", "r+b"); //le o arquivo dos registros
        btree = fopen("btree.bin", "r+b"); //le o arquivo da arvore
        return YES; //os arquivos ja existiam
    }    
}
int btread (int rrn, BTPAGE *page_ptr){//------------------------------------------------LE O ARQUIVO DA ARVORE
    int addr;
    addr = (rrn * (sizeof(BTPAGE)))+ 4; //endereco_geral = (num_pagina * tam_Pag) +4 do header
    fseek(btree, addr, 0); //vai ate o endereco geral recuperado
    return (fread(page_ptr,sizeof(BTPAGE), 1, btree)); //le a pagina toda esse valor
}
int btwrite (int rrn, BTPAGE *page_ptr){//-----------------------------------------------ESCREVE A PAGINA NO ARQUIVO DA ARVORE
    int addr;
    addr = (rrn * (sizeof(BTPAGE)))+4; //endereco_geral = (num_pagina * tam_Pag) +4 do header
    fseek(btree, addr, 0);//vai ate o endereco geral recuperado
    return(fwrite(page_ptr, sizeof(BTPAGE), 1, btree)); //escrevea pagina toda
}
int create_root (int key, int left, int right){//----------------------------------------CRIA UMA NOVA PAGINA
    BTPAGE page;
    int rrn;
    rrn=getpage(); //pega o numero da pagina
    pageinit(&page); //inicializa a pagina com os valores default
    page.key[0]=key; 
    page.child[0]=left;
    page.child[1]=right;
    page.keycount=1;
    btwrite(rrn, &page);//escreve no arquivo btree
    putroot(rrn);
    return(rrn);
}
int create_tree(){//---------------------------------------------------------------------CRIA A ARVORE
    int key=0;
    return (create_root(key, NIL, NIL)); //criar raiz sem folhas
}
int getpage(){//-------------------------------------------------------------------------PEGA O NUMERO DA PAGINA
    int addr;
    rewind(btree); //volta no comeco do arquivo
    addr = fseek(btree, 0, 4)-4; //menos o int do header
    ftell(btree);
    return addr/PAGESIZE; //retorna o numero da pagina
}
int getroot(){//-------------------------------------------------------------------------PEGA A RAIZ
    int root;
    fseek(btree, 0, 4); //voltar ao comeco do arquivo pulando header
    fread(&root, sizeof(int), 1, btree); //ler para pegar raiz
    return (root);
}
int insert (int rrn, int key, int *promo_r_child, int *promo_key){
    BTPAGE page, // pagina atual
    newpage; // pagina temporaria
    int found, promoted, // valores booleanos
    pos, //posicao
    p_b_rrn; // rrn promoted from below
    int p_b_key; // key promoted from below

    //chave inserida sem divisao e sem promocao
    if (rrn == NIL){ //se cabe
        *promo_key = key;
        *promo_r_child = NIL;
        return(YES);  //a chave e inserida
    }
    btread(rrn, &page);
    //chave duplicada
    found = search_node(key, &page, &pos);
    if (found){ //a chave ja existe?
        printf ("Chave %d duplicada.", key);
        return(0);
    }

    promoted = insert(page.child[pos], key, &p_b_rrn, &p_b_key);
    if (!promoted){ //precisou promover?
        return(NO);
    }

    if(page.keycount < MAXKEYS){ //se ainda cabem nessa pagina
        ins_in_page(p_b_key, p_b_rrn, &page);
        btwrite(rrn, &page);
        return(NO);
    }else{ //se nao cabem na pagina
        split(p_b_key, p_b_rrn, &page, promo_key, promo_r_child, &newpage); //divide
        btwrite(rrn, &page);
        btwrite(*promo_r_child, &newpage);
        return(YES); //foi inserido
    }
}
void ins_in_page (int key,int r_child, BTPAGE *p_page){
    int j;
    for(j = p_page-> keycount; key < p_page->key[j-1] && j > 0; j--){
        p_page->key[j] = p_page->key[j-1];
        p_page->child[j+1] = p_page->child[j];
    }
    p_page->keycount++;
    p_page->key[j] = key;
    p_page->child[j+1] = r_child;
    return;
}
void pageinit (BTPAGE *p_page){//--------------------------------------------------------INICIALIZA A PAGINA
    int j;
    for(j=0;j<MAXKEYS;j++){
        p_page->key[j]=NOKEY;//atribui -1 pras chaves
        p_page->child[j]=NIL;//atribui -1 pros ponteiros de descendentes
    }
    p_page->child[MAXKEYS]=NIL;
    return;
}
void putroot(int root){//----------------------------------------------------------------ATUALIZA O CABECALHO COM A NOVA RAIZ ???
    fseek(btree,0,0);
    fwrite(&root, sizeof(root), 1, btree);
}
int search_node(int key, BTPAGE *p_page, int *pos){
    int i;
    for (i = 0; i < p_page->keycount && key > p_page->key[i]; i++){
        *pos = i;
    }
    if (*pos < p_page->keycount && key == p_page->key[*pos]){
        return(YES);
    }
    return(NO);
}
int pegakey(int indice){
    int ind_add, tam_reg, item;
    if(indice==1){
        fseek(inseridos, 5 , SEEK_SET);
        fread(&ind_add, sizeof(int), 1, inseridos);
    }else{
        ind_add=0;
    }
    int aux1=atol(add[ind_add].CodCli),
    aux2=atol(add[ind_add].CodCli);
    item = aux1 + aux2;
    return 3;
}
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key,
            int *promo_r_child, BTPAGE *p_newpage){//------------------------------------DIVIDE A PAGINA
    int j;
    short mid;
    char workkeys[MAXKEYS+1]; //
    int workchil[MAXKEYS+2];
    //a pagina temporaria recebe os dados da pagina que teve overflow
    for (j = 0; j < MAXKEYS; j++){
        workkeys[j] = p_oldpage->key[j];
        workchil[j] = p_oldpage->child[j];
    }
    workchil[j] = p_oldpage->child[j];
    for (j = MAXKEYS; key < workkeys[j-1] && j > 0; j--){
        workkeys[j] = workkeys[j-1];
        workchil[j+1] = workchil[j];
    }
    workkeys[j] = key;
    workchil[j+1] = r_child;
    *promo_r_child = getpage();
    pageinit(p_newpage);
    //escreve a nova pagina com os valores na struct
    for (j = 0; j < MINKEYS; j++){
        p_oldpage->key[j] = workkeys[j];
        p_oldpage->child[j] = workchil[j];
        p_newpage->key[j] = workkeys[j+1+MINKEYS];
        p_newpage->child[j] = workchil[j+1+MINKEYS];
        p_oldpage->key[j+MINKEYS] = NOKEY;
        p_oldpage->child[j+1+MINKEYS] = NIL;
    }
    p_oldpage->child[MINKEYS] = workchil[MINKEYS];
    p_newpage->child[MINKEYS] = workchil[j+1+MINKEYS];
    p_newpage->keycount = MAXKEYS - MINKEYS;
    p_oldpage->keycount = MINKEYS;
    *promo_key = workkeys[MINKEYS];
}

//PRINCIPAL
int main(){//-------------------------------------------------------------------------------------MAIN
    int promoted, root, promo_rrn, promo_key, key, op;    
    do{
        printf("\n1-Insere\n2-Listar os dados de todos os clientes\n3-Listar os dados de um cliente especÃ­fico\n4-Carrega Arquivos\n0-Sair\nEscolha:");
        scanf(" %d", &op);
        switch(op){
            case insercao://1
                if (btopen()==NO){//NO = os arquivos ja existiam
                    pegakey(1);
                    root = getroot(btree);
                }else{//os arquivos foram criados agora
                    pegakey(0);
                    root = create_root(key, NIL, NIL);
                }
                promoted = insert(root, key, &promo_rrn, &promo_key);
                if (promoted)
                    root = create_root(promo_key, root, promo_rrn);
                break;
            case busca_geral: //2
                //busca_todos();     
                break;
            case busca_ind: //3
                //busca_um();     
                break;
            case carregar: //4
                carrega_arquivos();
                break;
            default: //sair 0
                break;
        }        
    }while(op!=0);

    btclose();
    return 0;
}
