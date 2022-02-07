//Exercicio 8
//Caio de Assis Ribeiro e Carolina Silva Marostica
//exercicio de Arvore-B de ordem 4
//IMPORTS
#include<stdio.h>

//DEFINICOES
FILE *btfd; //cria o arquivo da arvore como sendo global
///#define MAXKEYS 3
///#define MINKEYS (MAXKEYS-1)/2 //a chave que e alterada caso de overflow e a (0a >1b< 2c 3d)
#define MAXKEYS 4 //teste inicial
#define MINKEYS MAXKEYS/2 //teste inicial
#define NIL (-1)
#define NOKEY -1
#define NO 0
#define YES 1

//ESTRUTURA DA PAGINA
typedef struct{
    int keycount; //numero de chaves na pagina
    int key[MAXKEYS]; //a chave atual
    int child[MAXKEYS+1]; //ponteiros para os rrn de descendentes
}BTPAGE;
#define PAGESIZE sizeof(BTPAGE);
//PROTOTIPOS
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

//SUBROTINAS
void btclose(){
    fclose(btfd);
}
int btopen(){
    btfd = fopen("btree.bin", "w+b"); //cria o arquivo btfd
    return 0; //retorna verdadeiro, o arquivo foi criado
}
int btread (int rrn, BTPAGE *page_ptr){
    int addr;
    addr = rrn * (sizeof(BTPAGE)); //-------------------
    fseek(btfd, addr, 0);
    return (fread(page_ptr,sizeof(BTPAGE), 1, btfd));
}
int btwrite (int rrn, BTPAGE *page_ptr){
    int addr;
    addr = rrn * (sizeof(BTPAGE)); //--------------------------
    fseek(btfd, addr, 0);
    return(fwrite(page_ptr, sizeof(BTPAGE), 1, btfd));
}
int create_root (int key, int left, int right){
    BTPAGE page;
    int rrn;
    rrn=getpage();
    pageinit(&page);
    page.key[0]=key;
    page.child[0]=left;
    page.child[1]=right;
    page.keycount=1;
    btwrite(rrn, &page);
    putroot(rrn);
    return(rrn);
}
int create_tree(){
    int key;
    btfd = fopen("btree.bin", "w+b"); //criar e abrir para escrita o arquivo
    //escrever -1 no header
    printf("chave");
    scanf("%d", &key);//pega a chave
    return (create_root(key, NIL, NIL)); //criar raiz sem folhas
}
int getpage(){
    int addr;
    rewind(btfd); //volta no comeco do arquivo
    addr = fseek(btfd, 0, 2); //colocar cabecalho-----------------------------------
    ftell(btfd); //----------------------------------------------------
    return addr/PAGESIZE; //retorna o numero da pagina
}
int getroot(){
    int root;
    fseek(btfd, 0, 0); //voltar ao comeco do arquivo --------------pular o header depois
    if (fread(&root, sizeof(int), 2, btfd) == 0){ //ler um char de tamanho 2 para pegar raiz
        printf("Nao foi possivel achar a raiz\n");
        return 1;
    }
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
void pageinit (BTPAGE *p_page){
    int j;
    for(j=0;j<MAXKEYS;j++){
    // @|@|@|@|-1|-1|-1|-1
        p_page->key[j]=NOKEY;
        p_page->child[j]=NIL;
    }
    // @|@|@|@|-1|-1|-1|-1|-1
    p_page->child[MAXKEYS]=NIL;
    return;
}
void putroot(int root){
    fseek(btfd,0,0);
    fwrite(&root, sizeof(root), 1, btfd);
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
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage){
    int j;
    short mid;
    char workkeys[MAXKEYS+1];
    int workchil[MAXKEYS+2];
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

//MAIN
int main(){
    int infd; //
//MANIPULANDO O ARQUIVO DA ARVORE
    int promoted; // boolean: tells if a promotion from below
    int root, //rrn da raiz da pagina
    promo_rrn; // rrn promoted from below
    int promo_key, // key promoted from below
    key; // next key to insert in tree
    if (btopen(btfd)){
        root = getroot(btfd);
    }else{
        root = create_tree(btfd);
    }
    while ((key = getchar()) != '0'){
        promoted = insert(root, key, &promo_rrn, &promo_key);
        if (promoted)
            root = create_root(promo_key, root, promo_rrn);
    }
    btclose(btfd);
    return 0;
}
