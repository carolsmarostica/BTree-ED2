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
void busca_um();
int inopen();
void inwrite();
//SUBROTINAS
void carrega_arquivos(){//---------------------------------------------------------------CARREGA OS ARQUIVOS BASE
    FILE *arq;
    int i=0,j;
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
int btopen(){//--------------------------------------------------------------------------CRIA O ARQUIVO DA ARVORE
    if(access("btree.bin", F_OK)!=0){ //se o arquivo nao existe
        btree = fopen("btree.bin", "w+b"); //cria o arquivo de indice da arvore
        putroot(-1);
        create_tree();
        return NO;
    }else{
        btree = fopen("btree.bin", "r+b");
        //chama funcao que reescreve o arquivo
        return YES;
    }
}
int inopen(){//--------------------------------------------------------------------------CRIA O ARQUIVO DE INSERÇÃO
    int tam_reg, ind_add=0, ind_busca=0, dist=0;

    if(access("inseridos.bin", F_OK)!=0){ //se o arquivo nao existe
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
        inseridos = fopen("inseridos.bin", "r+b"); //le o arquivo dos registros
        //chama funcao que reescreve o arquivo
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
    page.keycount=0;
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
    addr = fseek(btree, 0, 0); //menos o int do header
    ftell(btree);
    return addr/PAGESIZE; //retorna o numero da pagina
}
int getroot(){//-------------------------------------------------------------------------PEGA A RAIZ
    int root;
    fseek(btree, 0, 0); //voltar ao comeco do arquivo pulando header
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
        return(-1);
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
        printf("Divisao de no.\n");
        printf("Chave %d promovida.\n", &promo_key);
        btwrite(rrn, &page);
        btwrite(*promo_r_child, &newpage);
        return(YES); //foi inserido
    }
}
void ins_in_page (int key,int r_child, BTPAGE *p_page){//--------------------------------INSERE NA PAGINA
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
void putroot(int root){//----------------------------------------------------------------ATUALIZA A PAGINA DA RAIZ
    fseek(btree,0,0);
    fwrite(&root, sizeof(root), 1, btree);//numero pagina
}
int search_node(int key, BTPAGE *p_page, int *pos){
    int i;
    for (i = 0; i <= p_page->keycount && key >= p_page->key[i]; i++){
        *pos = i;
    }
    if (*pos <= p_page->keycount && key == p_page->key[*pos]){
        return(YES);
    }
    return(NO);
}
int pegakey(){
    int ind_add, item;

    fseek(inseridos, 5 , SEEK_SET);
    fread(&ind_add, sizeof(int), 1, inseridos);

     //variavel auxiliar
    char codf[3], codc[3];
    strcpy(codc,add[ind_add].CodCli);
    strcpy(codf,add[ind_add].CodF); 

    //char em int
    int aux1=atoi(codf),
    aux2=atoi(codc);
    item = aux1 + aux2;
    return item;
}
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key,
            int *promo_r_child, BTPAGE *p_newpage){//------------------------------------DIVIDE A PAGINA
    int j;
    short mid;
    char workkeys[MAXKEYS+1];
    int workchil[MAXKEYS+2];
    printf("ERRO");
    //os vetores temporarios recebem os dados da pagina que teve overflow
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
void inwrite(){
    int ind_add,distanciap_origem,tam_reg, aux;
    char registro[155];

    fseek(inseridos, 5, 0);
    fread(&ind_add, sizeof(int),1, inseridos);
    
    sprintf(registro,"#%s#%s#%s#%s#%s", add[ind_add].CodCli, add[ind_add].CodF, 
	add[ind_add].NomeCli, add[ind_add].NomeF, add[ind_add].Genero);
	tam_reg = strlen(registro);//pega apenas o tamanho valido de registro
	tam_reg++;//\0
    fseek(inseridos,0, SEEK_END);
    fwrite(&tam_reg, sizeof(int),1,inseridos);
    fwrite(registro, sizeof(char), tam_reg, inseridos); //escreve o registro

    fseek(inseridos, 5, 0);
    ind_add++;
    fwrite(&ind_add, sizeof(int), 1, inseridos);
    fseek(inseridos,15,0);   
}
void busca_um(){
    int ind_busca, item, *pos=0, found, i;
    BTPAGE *pag=0;
    fseek(inseridos, 10 , SEEK_SET);
    fread(&ind_busca, sizeof(int), 1, inseridos);
    
    //variavel auxiliar
    char codf[3], codc[3];
    strcpy(codc,busca[ind_busca].CodCli);
    strcpy(codf,busca[ind_busca].CodF); 
    
    //char em int
    int aux1=atoi(codf),
    aux2=atoi(codc);

    //chave que deve ser buscada
    item = aux1 + aux2;
    
    found=search_node(item, pag, pos);     
    if(found){
        printf("Chave %d encontrada, pagina %d, posicao %d", item, pag, pos);
        return;
    }
    printf("Chave %d nao encontrada", item);
}


//PRINCIPAL
int main(){//----------------------------------------------------------------------------MAIN
    int promoted, root, promo_rrn, promo_key, key, op, aux;
    //inicializa os arquivos base
    aux=btopen();
    inopen();
    //menu    
    do{
        printf("\n1-Insere\n2-Listar os dados de todos os clientes\n3-Listar os dados de um cliente especÃ­fico\n4-Carrega Arquivos\n0-Sair\nEscolha:");
        scanf(" %d", &op);
        switch(op){
            case insercao://1
                fseek(btree,0,0);
                fread(&root,sizeof(int),1,btree);
                key=pegakey();
                promoted = insert(root, key, &promo_rrn, &promo_key);//primeiro argumento é a raiz, mas ele recebe na funcao como o rrn ???
                if (promoted){
                    root = create_root(promo_key, root, promo_rrn);
                }
                if(promoted!=-1){
                    //escreve no arquivo inseridos
                    inwrite();                    
                    printf("Chave %d inserida com sucesso.\n", key);
                }
                break;
            case busca_geral: //2
                //busca_todos();     
                break;
            case busca_ind: //3
                busca_um();     
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
