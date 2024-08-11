/*

Programa de Alinhamento de Genoma Versao 27/07/2024.

Programa "sequencial" baseado no metodo Needleman-Wunsch, desenvolvido por Saul
B. Needleman e Christian D. Wunsch, publicado em 1970, conforme segue:

Needleman, Saul B. & Wunsch, Christian D. (1970)."A general method applicable to
the search for similarities in the amino acid sequence of two proteins". Journal
of Molecular Biology.48 (3):443-53.

Este programa NAO pode ser usado, da forma original ou modificada, completa ou
parcial, para finalidade comercial ou lucrativa, sem a previa autorizacao dos
seus desenvolvedores, os quais detem os direitos autorais.

Este programa PODE ser livremente e gratuitamente usado, da forma original ou
modificada, completa ou parcial, como apoio ao processo de ensino-aprendizagem,
nao comercial e nao lucrativo, por qualquer pessoa, desde que os resultados
obtidos ou os produtos/subprodutos gerados tambem possam ser usados com a mesma
liberdade e gratuidade.

Todos os desenvolvedores responsaveis pelo codigo original e futuras modificacoes
devem ser informados na lista a seguir, apos o ultimo, antes da distribuicao do
codigo modificado, informando quais foram as modificacoes realizadas.

Lista de Desenvolvedores:

Desenvolvedor: Ronaldo Augusto de Lara Goncalves
Data da ultima atualizacao: 27/07/2024
eMail: ralgonca@gmail.com
Whatsapp: 55(44)99159-2310
Modificacoes realizadas: desenvolvimento do codigo original, composto pelos
modulos leTamMaior(), leTamMenor(), lePenalidade(), menuOpcao(void), trataOpcao(),
geraSequencias(), leMatrizPesos(), mostraMatrizPesos(), leGrauMutacao(),
geraMatrizEscores(), mostraMatrizEscores(), leSequencias(), geraSequencias(),
mostraSequencias(), traceBack(), mostraAlinhamentoGlobal() e main().



======================================
BREVE DESCRICAO:

======================================

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define A 0 // representa uma base Adenina
#define T 1 // representa uma base Timina
#define G 2 // representa uma base Guanina
#define C 3 // representa uma base Citosina
#define X 4 // representa um gap

#define sair 11

#define maxSeq 1000 // tamanho maximo de bases em uma sequencia genomica

/* mapaBases mapeia indices em caracteres que representam as bases, sendo 0='A',
1='T', 2='G', 3='C' e 4='-' representando gap */

char mapaBases[5]={'A','T','G','C','-'};

/* seqMaior e seqMenor representam as duas sequencias de bases de entrada, a
   serem comparadas, inicializadas conforme segue. Elas conterao os indices aos
   inves dos proprios caracteres. seqMenor deve ser menor ou igual a seqMaior. */

int  seqMaior[maxSeq]={A,A,C,T,T,A},
     seqMenor[maxSeq]={A,C,T,T,G,A};

/* alinhaGMaior representa a sequencia maior ja alinhada, assim como alinhaGMenor,
   ambas obtidas no traceback. As duas juntas, pareadas, formam o alinhamento
   global. Tal alinhamento global pode ser obtido de duas formas: a partir do
   primeiro maior escore ou a partir do ultimo maior escore */

int  alinhaGMaior[maxSeq],
     alinhaGMenor[maxSeq];

/* matrizEscores representa a matriz de escores que sera preenchida pelo metodo.
   A matriz, ao final de seu preenchimento, permitira obter o melhor alinhamento
   global entre as sequencias seqMaior e seqMenor, por meio de uma operacao
   denominada TraceBack. Uma linha e uma coluna extras sao adicionadas na matriz
   para inicializar as pontuacoes/escores. Trata-se da linha 0 e coluna 0. A
   matriz de escores tera tamSeqMenor+1 linhas e tamSeqMaior+1 colunas.
   Considera-se a primeira dimensao da matriz como linhas e a segunda como colunas.*/

int matrizEscores[maxSeq+1][maxSeq+1];

int tamSeqMaior=6,  /* tamanho da sequencia maior, inicializado como 6 */
    tamSeqMenor=6,  /* tamanho da sequencia menor, inicializado como 6 */
    tamAlinha,      /* tamanho do alinhamento global obtido */
    penalGap=0,     /* penalidade de gap, a ser descontada no escore acumulado
                       quando um gap eh encontrado */
    grauMuta=0,     /* porcentagem maxima de mutacao na geracao aleatoria da
                       sequencia menor, a qual eh copiada da maior e sofre algumas
                       trocas de bases */
    escoreDiag,      /* escore da diagonal anterior da matriz de escores */
    escoreLin,       /* escore da linha anterior da matriz de escores */
    escoreCol;       /* escore da coluna anterior da matriz de escores */

/*  matrizPesos contem os pesos do pareamento de bases. Estruturada e inicializada
    conforme segue, onde cada linha ou coluna se refere a uma das bases A, T, G
    ou C. Considera-se a primeira dimensao da matriz como linhas e a segunda como
    colunas. Na configuracao default, o peso de bases iguais eh 1 e o peso de bases
    diferentes eh 0, mas pode ser alterado. Outra configuracao usual eh 2 para
    bases iguais e -1 para bases diferentes

       0 1 2 3
       A T G C
   0 A 1 0 0 0
   1 T 0 1 0 0
   2 G 0 0 1 0
   3 C 0 0 0 1
*/
int matrizPesos[4][4]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

int indRef=-1,  // indice da sequencia maior a partir do qual extrai a sequencia
                // menor, no caso de geracao aleatoria
    nTrocas=-1, // quantidade de trocas na geracao automatica da sequencia menor,
                // a partir de um segmento da sequencia maior
    linPMaior, colPMaior, PMaior, // suporte para deteccao do primeiro maior escore
    linUMaior, colUMaior, UMaior; // suporte para deteccao do ultimo maior escore

/* leitura do tamanho da sequencia maior */
int leTamMaior(void)
{
  printf("\nLeitura do Tamanho da Sequencia Maior:");
  do{ 
    printf("\nDigite 0 < valor < %d = ", maxSeq);
    scanf("%d", &tamSeqMaior);
  }while ((tamSeqMaior<1)||(tamSeqMaior>maxSeq));
}

/* leitura do tamanho da sequencia menor */
int leTamMenor(void)
{
  printf("\nLeitura do Tamanho da Sequencia Menor:");
  do{ 
     printf("\nDigite 0 < valor <= %d = ", tamSeqMaior);
     scanf("%d", &tamSeqMenor);
  }while ((tamSeqMenor<1)||(tamSeqMenor>tamSeqMaior));
}

/* leitura do valor da penalidade de gap */
int lePenalidade(void)
{   int penal;

  printf("\nLeitura da Penalidade de Gap:");
  do{
    printf("\nDigite valor >= 0 = ");
    scanf("%d", &penal);
  }while (penal<0);

  return penal;
}

/* leitura da matriz de pesos */
void leMatrizPesos()
{ int i,j;

  printf("\nLeitura da Matriz de Pesos:\n");
  for (i=0; i<4; i++)
  {
    for (j=0; j<4; j++)
    {
      printf("Digite valor %c x %c = ", mapaBases[i], mapaBases[j]);
      scanf("%d",&(matrizPesos[i][j]));
    }
    printf("\n");
  }
}

/* mostra da matriz de pesos */
void mostraMatrizPesos(void)
{ int i,j;

  printf("\nMatriz de Pesos Atual:");
  printf("\n%4c%4c%4c%4c%4c\n",' ','A','T','G','C');
  for (i=0; i<4; i++)
  {
    printf("%4c",mapaBases[i]);
    for (j=0; j<4; j++)
      printf("%4d",matrizPesos[i][j]);
    printf("\n");
  }
}

/* leitura manual das sequencias de entrada seqMaior e seqMenor */
void leSequencias(void)
{ int i, erro;
  char seqMaiorAux[maxSeq], seqMenorAux[maxSeq];

  indRef=-1;
  nTrocas=-1;
  printf("\nLeitura das Sequencias:\n");

  /* lendo a sequencia maior */
  do
  {
    printf("\nPara a Sequencia Maior,");
    printf("\nDigite apenas caracteres 'A', 'T', 'G' e 'C'");
    do
    { printf("\n> ");
      fgets(seqMaiorAux,maxSeq,stdin);
      tamSeqMaior=strlen(seqMaiorAux)-1; /* remove o enter */
    } while (tamSeqMaior<1);
    printf("\ntamSeqMaior = %d\n",tamSeqMaior);
    i=0;
    erro=0;
    do
    {
      switch (seqMaiorAux[i])
      {
        case 'A': seqMaior[i]=A;
                  break;
        case 'T': seqMaior[i]=T;
                  break;
        case 'G': seqMaior[i]=G;
                  break;
        case 'C': seqMaior[i]=C;
                  break;
        default: erro=1;  /* nao eh permitido qquer outro caractere */
      }
      i++;
    } while ((erro==0)&&(i<tamSeqMaior));
  }while (erro==1);

  /* lendo a sequencia menor */
  do
  {
    printf("\nPara a Sequencia Menor, ");
    printf("\nDigite apenas caracteres 'A', 'T', 'G' e 'C'");
    do
    { printf("\n> ");
      fgets(seqMenorAux,maxSeq,stdin);
      tamSeqMenor=strlen(seqMenorAux)-1; /* remove o enter */
    } while ((tamSeqMenor<1)||(tamSeqMenor>tamSeqMaior));
    printf("\ntamSeqMenor = %d\n",tamSeqMenor);

    i=0;
    erro=0;
    do
    {
      switch (seqMenorAux[i])
      {
        case 'A': seqMenor[i]=A;
                  break;
        case 'T': seqMenor[i]=T;
                  break;
        case 'G': seqMenor[i]=G;
                  break;
        case 'C': seqMenor[i]=C;
                  break;
        default: erro=1;
      }
      i++;
    } while ((erro==0)&&(i<tamSeqMenor));
  }while (erro==1);
}

void leArquivoSeq(){
    FILE *arqPonteiro;
    arqPonteiro = fopen("sequencias.txt", "r");

    printf("\nVerifique se as sequencias estao nas linhas corretas!");
    printf("\nIniciando leitura de arquivo!\n");

    char seqMaiorAux[maxSeq], seqMenorAux[maxSeq], ch;
    int i = 0;

    //Lê sequencia maior e menor para strings auxiliares
    do{ 
        ch = fgetc(arqPonteiro);

        if(ch == 'A' || ch == 'T' || ch == 'G' || ch == 'C')
            strncat(seqMaiorAux, &ch, 1);

        if(ch == '\n'){
            do{
                ch = fgetc(arqPonteiro);

                if(ch == 'A' || ch == 'T' || ch == 'G' || ch == 'C')
                  strncat(seqMenorAux, &ch, 1);

            }while(ch != EOF);
        }

    }while(ch != EOF);

    tamSeqMaior = strlen(seqMaiorAux);
    tamSeqMenor = strlen(seqMenorAux);

    int erro = 0;

    do
    {
      switch (seqMaiorAux[i])
      {
        case 'A': seqMaior[i]=A;
                  break;
        case 'T': seqMaior[i]=T;
                  break;
        case 'G': seqMaior[i]=G;
                  break;
        case 'C': seqMaior[i]=C;
                  break;
        default: erro=1;  /* nao eh permitido qquer outro caractere */
      }
      i++;
    } while ((erro==0)&&(i<tamSeqMaior));

    erro = 0;
    i = 0;
    
    do
    {
      switch (seqMenorAux[i])
      {
        case 'A': seqMenor[i]=A;
                  break;
        case 'T': seqMenor[i]=T;
                  break;
        case 'G': seqMenor[i]=G;
                  break;
        case 'C': seqMenor[i]=C;
                  break;
        default: erro=1;  /* nao eh permitido qquer outro caractere */
      }
      i++;
    } while ((erro==0)&&(i<tamSeqMenor));

    fclose(arqPonteiro);

    printf("Sequencias lidas do arquivo!\n");
}

/* leitura da porcentagem maxima (grau) de mutacao aleatoria. Essa porcentagem eh
   usada na geracao aleatoria da seqMenor. A seqMenor eh obtida a partir da seqMaior, para se parecer com ela, se diferenciando
   por um certo grau de alteracoes em suas bases, fornecida pelo usuario. Esse
   metodo evita a gera��o aleatoria de sequencias totalmente diferentes. A
   quantidade de trocas realizadas eh no maximo a porcentagem aqui informada. */

int leGrauMutacao(void)
{ int prob;

  printf("\nLeitura da Porcentagem Maxima de Mutacao Aleatoria:\n");
  do
  { printf("\nDigite 0 <= valor <= 100 = ");
    scanf("%d", &prob);
  } while ((prob<0)||(prob>100));

  return prob;
}

/* geracao das sequencias aleatorias, conforme tamanho. Gera-se numeros aleatorios
   de 0 a 3 representando as bases 'A', 'T', 'G' e 'C'. Gera-se primeiramente a
   maior sequencia e desta extrai a menor sequencia. A menor sequencia eh obtida
   da maior por meio de algumas trocas de bases (mutacoes), de acordo com o grau
   de mutacao informado. A ideia eh gerar sequencias parecidas, mas com certo grau
   de diferenca. */


int gr_seq_numThreads; /*Quantidade de threads para geração aleatória das sequências maior e menor, e ainda da mutação*/

/*Geração aleatória da sequência maior*/
void *geraSequenciaMaior(void *arg)
{
    int id = *(int *)arg;
    int inicio = (id * tamSeqMaior) / gr_seq_numThreads;
    int fim = ((id + 1) * tamSeqMaior) / gr_seq_numThreads;

    for (int i = inicio; i < fim; i++) {
        char base = rand() % 4;
        seqMaior[i] = base;
    }

    pthread_exit(NULL);
}

/*Geração da sequência menor.  Copia trecho da sequencia maior,
  a partir de um indice aleatorio que nao ultrapasse os limites do
  vetor maior */
void *geraSequenciaMenor(void *arg)
{
    int id = *(int *)arg;
    int inicio = (id * tamSeqMenor) / gr_seq_numThreads;
    int fim = ((id + 1) * tamSeqMenor) / gr_seq_numThreads;

    for (int i = inicio; i < fim; i++) {
        seqMenor[i] = seqMaior[indRef + i];
    }

    pthread_exit(NULL);
}

/*Paralelização da geração aleatória da sequência menor*/
void *aplicaMutacoes(void *arg)
{
    int id = *(int *)arg;
    int inicio = (id * tamSeqMenor) / gr_seq_numThreads;
    int fim = ((id + 1) * tamSeqMenor) / gr_seq_numThreads;

    for (int i = inicio; i < fim; i++) {
        int probAux = rand() % 100 + 1;
        if (probAux <= grauMuta && nTrocas < ((grauMuta * tamSeqMenor) / 100)) {
            seqMenor[i] = (seqMenor[i] + (rand() % 3) + 1) % 4;
            nTrocas++;
        }
    }

    pthread_exit(NULL);
}

void geraSequencias(void)
{   int dif;

    printf("\nDigitite a quantidade de threads que deseja utilizar na geração aleatoria de sequencias: ");
    scanf("%d", &gr_seq_numThreads);

    pthread_t threads[gr_seq_numThreads];
    int id_thread[gr_seq_numThreads];
    int iret;

    printf("\nGeracao Aleatoria das Sequencias:\n");

    /* gerando a sequencia maior em paralelo */
    for (int i = 0; i < gr_seq_numThreads; i++) {
        id_thread[i] = i;
        iret = pthread_create(&threads[i], NULL, geraSequenciaMaior, (void *)&id_thread[i]);

        if (iret) {
            printf("ERRO: código de retorno de pthread_create() é %d\n", iret);
            exit(-1);
        }
    }

    for (int i = 0; i < gr_seq_numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    dif=tamSeqMaior-tamSeqMenor; /* diferenca entre os tamanhos das sequencias */

    indRef=0;
    if (dif>0)
      indRef=rand()%dif; /* produz um indice aleatorio para indexar a sequencia maior,
                         para a partir dele extrair a primeira versao da sequencia
                         menor */

    /* gerando a sequencia menor a partir da maior em paralelo */
    for (int i = 0; i < gr_seq_numThreads; i++) {
        id_thread[i] = i;
        iret = pthread_create(&threads[i], NULL, geraSequenciaMenor, (void *)&id_thread[i]);

        if (iret) {
            printf("ERRO: código de retorno de pthread_create() é %d\n", iret);
            exit(-1);
        }
    }

    for (int i = 0; i < gr_seq_numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    /* causa mutacoes aleatorias na sequencia menor para gerar "gaps",
       sobre cada base, de acordo com o grau (porcentagem) informado.
       A mutacao causa a troca da base original por outra base aleatoria
       necessariamente diferente. Gera-se uma probabilidade aleatoria
       ateh 100 e se ela estiver dentro do grau informado, a mutacao
       ocorre na base, caso contrario, mantem a base copiada. */

    nTrocas=0;

    for (int i = 0; i < gr_seq_numThreads; i++) {
        id_thread[i] = i;
        iret = pthread_create(&threads[i], NULL, aplicaMutacoes, (void *)&id_thread[i]);

        if (iret) {
            printf("ERRO: código de retorno de pthread_create() é %d\n", iret);
            exit(-1);
        }
    }

    printf("\nSequencias Geradas: Dif = %d, IndRef = %d, NTrocas = %d\n ",dif, indRef, nTrocas);
}

/* mostra das sequencias seqMaior e seqMenor */
void mostraSequencias(void)
{   int i;

  printf("\nSequencias Atuais:\n");
  printf("\nSequencia Maior, Tam = %d\n", tamSeqMaior);
  for (i=0; i<tamSeqMaior; i++)
    printf("%c",mapaBases[seqMaior[i]]);
  printf("\n");

  for (i=0; i<tamSeqMaior; i++)
    if (i!=indRef)
      printf(" ");
    else printf("^");
  printf("\nIndice de Referencia = %d\n", indRef);

  printf("\nSequencia Menor, Tam = %d\n", tamSeqMenor);
  for (i=0; i<tamSeqMenor; i++)
    printf("%c",mapaBases[seqMenor[i]]);
  printf("\n");

  for (i=0; i<tamSeqMenor; i++)
      if (seqMenor[i]!=seqMaior[indRef+i])
           printf("^");
      else printf(" ");
  printf("\nQuantidade de trocas = %d\n", nTrocas);

}

/* geraMatrizEscores gera a matriz de escores. A matriz de escores tera
   tamSeqMenor+1 linhas e tamSeqMaior+1 colunas. A linha 0 e a coluna
   0 s�o adicionadas para representar gaps e conter penalidades. As
   demais linhas e colunas sao associadas as bases da seqMenor e da
   SeqMaior, respectivamente. */

int score_numThreads;
pthread_mutex_t mutex;

typedef struct {
    int thread_id;
    int num_threads;
} thread_pack;

void *calculaEscores(void *arg)
{
    thread_pack *pack = (thread_pack *)arg;
    int thread_id = pack->thread_id;
    int num_threads = pack->num_threads;

    for (int lin = 1 + thread_id; lin <= tamSeqMenor; lin += num_threads) {
        for (int col = 1; col <= tamSeqMaior; col++) {
            int peso = matrizPesos[(seqMenor[lin-1])][(seqMaior[col-1])];
            int escoreDiag = matrizEscores[lin-1][col-1] + peso;
            int escoreLin = matrizEscores[lin][col-1] - penalGap;
            int escoreCol = matrizEscores[lin-1][col] - penalGap;
            
            int maxEscore;

            if ((escoreDiag > escoreLin) && (escoreDiag > escoreCol))
                maxEscore = escoreDiag;
            else if (escoreLin > escoreCol)
                maxEscore = escoreLin;
            else
                maxEscore= escoreCol;

          pthread_mutex_lock(&mutex);
          matrizEscores[lin][col] = maxEscore;
          pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(NULL);
}

void geraMatrizEscores(void)
{ int lin, col, peso;

  printf("\nDigite a quantidade de threads que deseja utilziar na geração da matriz score: ");
  scanf("%d", &score_numThreads);

  pthread_t threads[score_numThreads];
  thread_pack thread_pack[score_numThreads];
  int iret;

  printf("\nGeracao da Matriz de escores:\n");
  /*  A matriz sera gerada considerando que ela representa o cruzamento
      da seqMenor[] associada as linhas e a seqMaior[] associada as
      colunas. */

  /* inicializando a linha de penalidades/gaps */
  for (col=0; col<=tamSeqMaior; col++)
    matrizEscores[0][col]=-1*(col*penalGap);

  /* inicializando a coluna de penalidades/gaps */
  for (lin=0; lin<=tamSeqMenor; lin++)
    matrizEscores[lin][0]=-1*(lin*penalGap);

  /* calculando os demais escores, percorrendo todas as posicoes
     da matriz, linha por linha, coluna por coluna, aplicando
     a seguinte formula:
                             / f(lin-1,col-1)+matrizPesos[lin,col]
     f(lin,col) = m�ximo de {  f(lin,col-1)-penalGap
                             \ f(lin-1,col)-penalGap
  */

  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < score_numThreads; i++) {
        thread_pack[i].thread_id = i;
        thread_pack[i].num_threads = score_numThreads;
        iret = pthread_create(&threads[i], NULL, calculaEscores, (void *)&thread_pack[i]);

        if (iret) {
            printf("ERRO: Código de retorno de pthread_create() é %d\n", iret);
            exit(-1);
        }
  }

  for (int i = 0; i < score_numThreads; i++) {
        pthread_join(threads[i], NULL);
  }  

  pthread_mutex_destroy(&mutex);

  /* localiza o primeiro e o ultimo maior escores e suas posicoes. */

  linPMaior=1;
  colPMaior=1;
  PMaior=matrizEscores[1][1];

  linUMaior=1;
  colUMaior=1;
  UMaior=matrizEscores[1][1];

  for (lin=1; lin<=tamSeqMenor; lin++)
  {
    for (col=1; col<=tamSeqMaior; col++)
    {
      if (PMaior<matrizEscores[lin][col])
      {
        linPMaior=lin;
        colPMaior=col;
        PMaior=matrizEscores[lin][col];
      }
      if (UMaior<=matrizEscores[lin][col])
      {
        linUMaior=lin;
        colUMaior=col;
        UMaior=matrizEscores[lin][col];
      }
    }
  }
  printf("\nMatriz de escores Gerada.");
  printf("\nPrimeiro Maior escore = %d na celula [%d,%d]", PMaior, linPMaior, colPMaior);
  printf("\nUltimo Maior escore = %d na celula [%d,%d]", UMaior, linUMaior, colUMaior);
}


/* imprime a matriz de escores de acordo */
void mostraMatrizEscores()
{ int i, lin, col;

  printf("\nMatriz de escores Atual:\n");

  printf("%4c%4c",' ',' ');
  for (i=0; i<=tamSeqMaior; i++)
    printf("%4d",i);
  printf("\n");

  printf("%4c%4c%4c",' ',' ','-');
  for (i=0; i<tamSeqMaior; i++)
    printf("%4c",mapaBases[(seqMaior[i])]);
  printf("\n");

  printf("%4c%4c",'0','-');
  for (col=0; col<=tamSeqMaior; col++)
    printf("%4d",matrizEscores[0][col]);
  printf("\n");

  for (lin=1;lin<=tamSeqMenor;lin++)
  {
    printf("%4d%4c",lin,mapaBases[(seqMenor[lin-1])]);
    for (col=0;col<=tamSeqMaior;col++)
    {
      printf("%4d",matrizEscores[lin][col]);
    }
    printf("\n");
  }
}

void arquivaMatrizEscores() {

    char nomeArq[100];
    int i, lin, col;
    int larguraCampo = 4;

    printf("\n\nDigite o nome do arquivo que deseja criar ou modificar para escrever a matriz escore: ");
    scanf("%s", &nomeArq);
    
    FILE *arquivo = fopen(nomeArq, "w");
    
    fprintf(arquivo, "Matriz de Escores:\n\n");

    // Cabeçalho da coluna com os índices
    fprintf(arquivo, "    ");
    for (i = 0; i <= tamSeqMaior; i++) {
        fprintf(arquivo, "%*d", larguraCampo, i);
    }
    fprintf(arquivo, "\n");

    // Linha das bases da seqMaior
    fprintf(arquivo, "    -");
    for (i = 0; i < tamSeqMaior; i++) {
        fprintf(arquivo, "%*c", larguraCampo, mapaBases[(seqMaior[i])]);
    }
    fprintf(arquivo, "\n");

    // Linha inicial (0, e seqMaior)
    fprintf(arquivo, "%*d -", larguraCampo, 0);
    for (col = 0; col <= tamSeqMaior; col++) {
        fprintf(arquivo, "%*d", larguraCampo, matrizEscores[0][col]);
    }
    fprintf(arquivo, "\n");

    // Restante da matriz (seqMenor e valores)
    for (lin = 1; lin <= tamSeqMenor; lin++) {
        fprintf(arquivo, "%*d%*c", larguraCampo, lin, larguraCampo, mapaBases[(seqMenor[lin - 1])]);
        for (col = 0; col <= tamSeqMaior; col++) {
            fprintf(arquivo, "%*d", larguraCampo, matrizEscores[lin][col]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
}

/* gera o alinhamento global por meio do percurso de retorno na Matriz de escores,
   em duas formas possiveis, conforme o parametro tipo: 1) a partir da celula do
   primeiro maior escore [linPMaior,colPMaior] ou 2) a partir da celula de ultimo
   maior escore [linUMaior,colUMaior], ambos em direcao a celula inicial [0,0],
   uma celula por vez. O caminho de retorno deve ser feito seguindo o mesmo caminho
   inverso que gerou a celular final a partir da celula inicial. O alinhamento
   global eh composto por duas sequencias alinhaGMenor e alinhaGMaior.

   Note que outros alinhamentos globais podem ser encontrados se o percurso do
   traceback for ramificado em celulas que foram "escoreadas/pontuadas" por meio
   de uma estrategia de desempate, pelo fato de ter havido empate na pontuacao
   dos nohs vizinhos. Alem disso, alinhamentos parciais tambem podem ser obtidos
   com traceback iniciado a partir de qualquer c�lula */
void traceBack(int tipo)
{ int tbLin, tbCol, peso, pos, posAux, aux, i;

  if (tipo==1)
  { printf("\nGeracao do Primeiro Maior Alinhamento Global:\n");
    tbLin=linPMaior;
    tbCol=colPMaior;
  }
  else {
    printf("\nGeracao do Ultimo Maior Alinhamento Global:\n");
    tbLin=linUMaior;
    tbCol=colUMaior;
  }

  pos=0;
  do
  {
    // a seguir verifica o escore do elemento [tbLin, tbCol]

    peso=matrizPesos[(seqMenor[tbLin-1])][(seqMaior[tbCol-1])];
    escoreDiag = matrizEscores[tbLin-1][tbCol-1]+peso;
    escoreLin = matrizEscores[tbLin][tbCol-1]-penalGap;
    escoreCol = matrizEscores[tbLin-1][tbCol]-penalGap;

      if ((escoreDiag>escoreLin)&&(escoreDiag>escoreCol)) // trocado >= por >
      {
        if (seqMenor[tbLin-1]!=seqMaior[tbCol-1])
        {   /* O escore da diagonal venceu, mas os elementos correspondentes entre
               as sequencias menor e maior sao diferentes. Nesse caso, surge um
               gap duplo */

            printf("\nALERTA no TraceBack: Pos = %d Lin = %d e Col = %d\n", pos, tbLin, tbCol);

            alinhaGMenor[pos]=X;
            alinhaGMaior[pos]=seqMaior[tbCol-1];
            tbCol--;
            pos++;

            alinhaGMenor[pos]=seqMenor[tbLin-1];
            alinhaGMaior[pos]=X;
            tbLin--;
            pos++;

        }
        else {
            alinhaGMenor[pos]=seqMenor[tbLin-1];
            tbLin--;
            alinhaGMaior[pos]=seqMaior[tbCol-1];
            tbCol--;
            pos++;
        }
      }
      else if (escoreLin>=escoreCol)
           {
              alinhaGMenor[pos]=X;
              alinhaGMaior[pos]=seqMaior[tbCol-1];
              tbCol--;
              pos++;
           }
           else
           {
              alinhaGMenor[pos]=seqMenor[tbLin-1];
              alinhaGMaior[pos]=X;
              tbLin--;
              pos++;
           }

  } while ((tbLin!=0)&&(tbCol!=0));

  /* descarrega o restante de gaps da linha 0, se for o caso */
  while (tbLin>0)
  {
    alinhaGMenor[pos]=seqMenor[tbLin-1];
    alinhaGMaior[pos]=X;
    tbLin--;
    pos++;
  }

  /* descarrega o restante de gaps da coluna 0, se for o caso */
  while (tbCol>0)
  {
    alinhaGMenor[pos]=X;
    alinhaGMaior[pos]=seqMaior[tbCol-1];
    tbCol--;
    pos++;
  }

  tamAlinha=pos;

  /* o alinhamento foi feito de tras para frente e deve ser
     invertido, conforme segue */
  for (i=0;i<(tamAlinha/2);i++)
  {
    aux=alinhaGMenor[i];
    alinhaGMenor[i]=alinhaGMenor[tamAlinha-i-1];
    alinhaGMenor[tamAlinha-i-1]=aux;

    aux=alinhaGMaior[i];
    alinhaGMaior[i]=alinhaGMaior[tamAlinha-i-1];
    alinhaGMaior[tamAlinha-i-1]=aux;
  }

  printf("\nAlinhamento Global Gerado.");
}

/* menu de opcoes fornecido para o usuario */
int menuOpcao(void)
{ int op;
  char enter;

  do
  {
    printf("\nMenu de Opcao:");
    printf("\n<01> Ler Matriz de Pesos");
    printf("\n<02> Mostrar Matriz de Pesos");
    printf("\n<03> Ler Penalidade de Gap");
    printf("\n<04> Mostrar Penalidade");
    printf("\n<05> Definir Sequencias Genomicas");
    printf("\n<06> Mostrar Sequencias");
    printf("\n<07> Gerar Matriz de Escores");
    printf("\n<08> Mostrar Matriz de Escores");
    printf("\n<09> Gerar Alinhamento Global");
    printf("\n<10> Mostrar Alinhamento Global");
    printf("\n<11> Sair");
    printf("\nDigite a opcao => ");
    scanf("%d",&op);
    scanf("%c",&enter);
  } while ((op<1)||(op>sair));

  return (op);
}

/* trata a opcao fornecida pelo usuario, executando o modulo pertinente */
void trataOpcao(int op)
{ int resp;
  char enter;

  switch (op)
  {
    case 1: leMatrizPesos();
            break;
    case 2: mostraMatrizPesos();
            break;
    case 3: penalGap=lePenalidade();
            break;
    case 4: printf("\nPenalidade = %d",penalGap);
            break;
    case 5: printf("\nDeseja Definicao: <1>MANUAL, <2>ARQUIVO ou <3>ALEATORIA? = ");
            scanf("%d",&resp);
            scanf("%c",&enter); /* remove o enter */
            if (resp==1)
            {
              leSequencias();
            }
            else if(resp==2){
                leArquivoSeq();
            }
            else
            { leTamMaior();
              leTamMenor();
              grauMuta=leGrauMutacao();
              geraSequencias();
            }
            break;
    case 6: mostraSequencias();
            break;
    case 7: geraMatrizEscores();
            arquivaMatrizEscores();
            break;
    case 8: mostraMatrizEscores();
            break;
    case 9: printf("\nDeseja: <1> Primeiro Maior ou <2> Ultimo Maior? = ");
            scanf("%d",&resp);
            scanf("%c",&enter); /* remove o enter */
            traceBack(resp);
            break;
    case 10: mostraAlinhamentoGlobal();
            break;
  }
}

/* programa principal */
void main(void)
{ int opcao;

  srand(time(NULL));

  do
  {
    printf("\n\nPrograma Needleman-Wunsch Sequencial\n");
    opcao=menuOpcao();
    trataOpcao(opcao);

  } while (opcao!=sair);

}