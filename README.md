## Comentários sobre as atualizações referentes à essa pasta:

### Atualização1
- [x] Arquivo inseridos não está sendo escrito (nem o cabeçalho).
- [x] A função pegakey() foi arrumada.
- [x] É possível fazer a inserção de 1 item na árvore.
  - Se tenta inserir 1 item e sai o cabeçalho fica: -1|item|filhos(-1)
  - Se tenta inserir 2 itens o cabeçalho fica: 1|item|filhos
- [x] Ainda não entendi a função insert direito. Na main o primeiro argumento é a raiz (root), mas o parâmetro que recebe é o rrn.

### Atualização2
- [x] Esqueleto da função busca_um();
- [x] Como saber quantas paginas ja foram escritas? É o rrn?

### Atuaização3
- Não está escrevendo os filhos depois do split
- Parece que só escreve 1 pagina a mais depois do split
- Quando da uma pesquisa(3) ele nao escreve o ultimo dado que era pra ser escrito em btree 
- Não está inserindo mais chaves depois que encontra uma duplicada.
