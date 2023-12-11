/*
Gabriel Kury Fonseca
32153848

Gabriel Marques Gonçalves Almeida
32066724
Integra��o do analisador l�xico e analisador sint�tico
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// ########### TIPOS DOS ANALISADOR LEXICO E SINTATICO
typedef enum{
    ERRO,
    IDENTIFICADOR,
    NUMERO,
    OP_SOMA,
    OP_MULT,
    PONTO_VIRGULA,
    PONTO,
    ALGORITMO,
    SE,
    ENQUANTO,
    VERDADEIRO,
    FALSO,
    VARIAVEL,
    DOIS_PONTOS,
    VIRGULA,
    INTEIRO,
    LOGICO,             //Definicao dos tipos de atomo
    INICIO,
    FIM,
    DOIS_PONTOS_IGUAL,
    ABRE_PARENTESES,
    FECHA_PARENTESES,
    ENTAO,
    SENAO,
    FACA,
    LEIA,
    ESCREVA,
    MENOR_QUE,
    MENOR_IGUAL_QUE,
    IGUAL,
    HASHTAG,
    MAIOR_QUE,
    MAIOR_IGUAL_QUE,
    OP_SUBT,
    OU,
    OP_DIVI,
    E,
    EOS
}TAtomo;

typedef struct{
  TAtomo atomo;
  int linha;
  float atributo_numero;
  char atributo_ID[16];
}TInfoAtomo;

// #######################
// variavel global e funcoes DECLARADA NO ANALISADOR LEXICO

char *buffer;
int contaLinha=1;
int rotulo = 0;
int nVariaveis = 0;
char **tabelaSimbolos;
int flagUsoVariaveis = -1; //flag para saber se as variaveis estao sendo declaradas ou chamadas

TInfoAtomo obter_atomo();
TInfoAtomo reconhece_numero();
TInfoAtomo reconhece_id();
TInfoAtomo exponencial();
void bloco();
void declaracaoDeVariaveis();
void listaVariavel();
void tipo();
void comandoComposto();
void comando();
void comandoAtribuicao();
void comandoSe();
void comandoEnquanto();
void comandoEntrada();
void comandoSaida();
void expressao();
void relacional();
void expressaoSimples();
void termo();
void fator();
int proximo_rotulo();

// ###########
// variaveis globais e funcoes DO ANALISADOR SINTATICO

//correlacao entre numero do tipo do atomo e string do tipo do atomo
char strAtomo[][30] = { "Erro lexico", "IDENTIFICADOR","NUMERO","Op de Soma","Op Multiplicao",";", ".","algoritmo","se", "enquanto", "verdadeiro", "falso","variavel","dois pontos","virgula", "inteiro","logico", "inicio", "fim", ":=","(", ")","entao", "senao", "faca", "leia", "escreva", "<", "<=", "=", "#", ">", ">=", "OP de subt", "ou", "OP de divisao", "e", "Fim Analise Sintatica"};

TInfoAtomo InfoAtomo;
TAtomo lookahead;// lookahead = obter_atomo()

void programa(); // prototipacao de funcao
void consome( TAtomo atomo );
int main(){
  FILE *arquivo;
  long tamanho_arquivo;

  // Abre o arquivo no modo de leitura ("r")
  arquivo = fopen("entrada.txt", "r");

  // Verifica se o arquivo foi aberto corretamente
  if (arquivo == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo.\n");
      return 1;
  }

  // Obtém o tamanho do arquivo
  fseek(arquivo, 0, SEEK_END); // Move para o final do arquivo
  tamanho_arquivo = ftell(arquivo); // Obtém a posição atual, que é o tamanho do arquivo
  rewind(arquivo); // Retorna para o início do arquivo

  // Aloca memória para armazenar o conteúdo do arquivo
  buffer = (char *)malloc(tamanho_arquivo + 1); // +1 para o caractere nulo de terminação

  // Verifica se a alocação de memória foi bem-sucedida
  if (buffer == NULL) {
      fprintf(stderr, "Erro ao alocar memória.\n");
      fclose(arquivo); // Fecha o arquivo antes de sair
      return 1;
  }

  // Lê o conteúdo do arquivo para o ponteiro de char
  fread(buffer, 1, tamanho_arquivo, arquivo);

  // Adiciona um caractere nulo de terminação ao final do texto
  buffer[tamanho_arquivo] = '\x0';

  fclose(arquivo);


   //printf("Analisando:\n\n%s\n\n",buffer);
  InfoAtomo = obter_atomo();
  lookahead = InfoAtomo.atomo;

  programa(); // chama o simbolo inicial da gramatica
  consome(EOS);

  //printf("analise sintatica finalizada com sucesso.");

  /*for(int i = 0; i < nVariaveis; i++){
    free(tabelaSimbolos[i]);
  }*/

  //printf("\n%s", tabelaSimbolos[1]);

  for(int i = 0;i < nVariaveis; i++){
    free(tabelaSimbolos[i]);
  }
                                        //Libera memoria
  free(tabelaSimbolos);

  return 0;
}

//###############################
// ANALISADOR LEXICO
//###############################
TInfoAtomo obter_atomo(){
    TInfoAtomo infoAtomo;
    infoAtomo.linha = contaLinha;
    // descartando os delimitadores
    while( *buffer==' ' || *buffer=='\n' || *buffer=='\t' || *buffer== '\r' || (*buffer == '/' && *(buffer + 1) == '/') || (*buffer == '/' && *(buffer + 1) == '*')){
        if(*buffer == '/' && *(buffer + 1) == '/'){
          //printf("#%03d:comentario\n", infoAtomo.linha);
          while(*buffer != '\n'){
            buffer++;
          }
        } else if(*buffer == '/' && *(buffer + 1) == '*'){
          //printf("#%03d:comentario\n", infoAtomo.linha);
          while(!(*buffer == '*' && *(buffer + 1) == '/')){
            if(*buffer == '\n'){
              contaLinha++;
            }
            buffer++;
          }
          buffer++;
          buffer++;
        } else{
          if(*buffer == '\n'){
            contaLinha++;
          }
          buffer++;
        }
    }


    // reconhece numero
    if(isdigit(*buffer)){
        infoAtomo = reconhece_numero();
    }
    // reconhece identificador
    else if(isalpha(*buffer)){
        infoAtomo = reconhece_id();
    }
    else if(*buffer == '+'){
        infoAtomo.atomo = OP_SOMA;
        buffer++;
    }
    else if(*buffer == '*'){
        infoAtomo.atomo = OP_MULT;
        buffer++;
    }
    else if(*buffer == ';'){
        infoAtomo.atomo = PONTO_VIRGULA;
        buffer++;
    }
    else if(*buffer == ':' && *(buffer + 1) == '='){
        infoAtomo.atomo = DOIS_PONTOS_IGUAL;
        buffer+=2;
    }
    else if(*buffer == ':'){
        infoAtomo.atomo = DOIS_PONTOS;
        buffer++;
    }
    else if(*buffer == '.'){
        infoAtomo.atomo = PONTO;
        buffer++;
    }
    else if(*buffer == '-'){
        infoAtomo.atomo = OP_SUBT;
        buffer++;
    }
    else if(*buffer == ','){
        infoAtomo.atomo = VIRGULA;
        buffer++;
    }
    else if(*buffer == '('){
        infoAtomo.atomo = ABRE_PARENTESES;
        buffer++;
    }
    else if(*buffer == '<' && *buffer + 1 == '='){
        infoAtomo.atomo = MENOR_IGUAL_QUE;
        buffer+=2;
    }
    else if(*buffer == '>' && *buffer + 1 == '='){
        infoAtomo.atomo = MAIOR_IGUAL_QUE;
        buffer+=2;
    }
    else if(*buffer == '<'){
        infoAtomo.atomo = MENOR_QUE;
        buffer++;
    }
    else if(*buffer == '='){
        infoAtomo.atomo = IGUAL;
        buffer++;
    }
    else if(*buffer == '#'){
        infoAtomo.atomo = HASHTAG;
        buffer++;
    }
    else if(*buffer == '>'){
        infoAtomo.atomo = MAIOR_QUE;
        buffer++;
    }
    else if(*buffer == ')'){
        infoAtomo.atomo = FECHA_PARENTESES;
        buffer++;
    }
    else if(*buffer == '\x0')
        infoAtomo.atomo = EOS;
    else
        infoAtomo.atomo = ERRO;

    infoAtomo.linha = contaLinha;
    return infoAtomo;
}
// funcao reconhe o atomo NUMERO
TInfoAtomo reconhece_numero(){
    char *pIniNum = buffer;
    TInfoAtomo infoAtomo;
    infoAtomo.atomo = ERRO;

    while(isdigit(*buffer)){
        buffer++;
    }
    if(isalpha(*buffer) && (*buffer != 'e' || *buffer != 'E')){
        return infoAtomo;
    }

    if(*buffer == 'e' || *buffer == 'E'){
        buffer++;
        if(*buffer == '+' || *buffer == '-'){
            buffer++;
        }
        if(! isdigit(*buffer)){
            return infoAtomo;
        }
        while(isdigit(*buffer)){
            buffer++;
        }
    }


    // qualquer outra coisa
    //https://www.tutorialspoint.com/c_standard_library/c_function_strncpy.htm
    strncpy(infoAtomo.atributo_ID,pIniNum,buffer-pIniNum);
    infoAtomo.atributo_ID[buffer-pIniNum] = '\x0';
    infoAtomo.atributo_numero = atof(infoAtomo.atributo_ID);
    infoAtomo.atomo = NUMERO;
    return infoAtomo;
}

// funcao reconhe o atomo IDENTIFICADOR
TInfoAtomo reconhece_id(){
    char *pIniID = buffer;
    TInfoAtomo infoAtomo; //aA
    infoAtomo.atomo = ERRO;

    if(islower(*buffer) || isupper(*buffer)){
        buffer++;
        goto q1;
    }
    return infoAtomo;
q1:
    if(islower(*buffer)||isupper(*buffer) || isdigit(*buffer) || *buffer == '_'){
        buffer++;
        goto q1;
    }
              //https://www.tutorialspoint.com/c_standard_library/c_function_strncpy.htm
    if(buffer-pIniID > 15){
        return infoAtomo;
    }
    strncpy(infoAtomo.atributo_ID,pIniID,buffer-pIniID);
    infoAtomo.atributo_ID[buffer-pIniID] = '\x0';
    //printf("\n\n%s\n\n", infoAtomo.atributo_ID);
    if(strcmp(infoAtomo.atributo_ID,"algoritmo")==0)
        infoAtomo.atomo  = ALGORITMO;
    else if(strcmp(infoAtomo.atributo_ID,"se")==0)
        infoAtomo.atomo  = SE;
    else if(strcmp(infoAtomo.atributo_ID,"enquanto")==0)
        infoAtomo.atomo  = ENQUANTO;
    else if(strcmp(infoAtomo.atributo_ID,"verdadeiro")==0)
        infoAtomo.atomo  = VERDADEIRO;
    else if(strcmp(infoAtomo.atributo_ID,"falso")==0)
        infoAtomo.atomo  = FALSO;
    else if(strcmp(infoAtomo.atributo_ID,"variavel")==0)
        infoAtomo.atomo = VARIAVEL;
    else if(strcmp(infoAtomo.atributo_ID,"inteiro")==0)
        infoAtomo.atomo = INTEIRO;
    else if(strcmp(infoAtomo.atributo_ID,"logico")==0)
        infoAtomo.atomo = LOGICO;
    else if(strcmp(infoAtomo.atributo_ID,"inicio")==0)
        infoAtomo.atomo = INICIO;
    else if(strcmp(infoAtomo.atributo_ID,"fim")==0)
        infoAtomo.atomo = FIM;
    else if(strcmp(infoAtomo.atributo_ID,"entao")==0)
        infoAtomo.atomo = ENTAO;
    else if(strcmp(infoAtomo.atributo_ID,"senao")==0)
        infoAtomo.atomo = SENAO;
    else if(strcmp(infoAtomo.atributo_ID,"faca")==0)
        infoAtomo.atomo = FACA;
    else if(strcmp(infoAtomo.atributo_ID,"leia")==0)
        infoAtomo.atomo = LEIA;
    else if(strcmp(infoAtomo.atributo_ID,"escreva")==0)
        infoAtomo.atomo = ESCREVA;
    else if(strcmp(infoAtomo.atributo_ID,"ou")==0)
        infoAtomo.atomo = OU;
    else if(strcmp(infoAtomo.atributo_ID,"div")==0)
        infoAtomo.atomo = OP_DIVI;
    else if(strcmp(infoAtomo.atributo_ID,"e")==0)
        infoAtomo.atomo = E;
    else{
      infoAtomo.atomo = IDENTIFICADOR;
      if(flagUsoVariaveis == 0 ){
      nVariaveis++;

      if(nVariaveis == 1){
        tabelaSimbolos = (char **)malloc(nVariaveis * sizeof(char *));
        tabelaSimbolos[nVariaveis-1] = (char *)malloc((strlen(infoAtomo.atributo_ID) + 1) * sizeof(char));
        strcpy(tabelaSimbolos[nVariaveis-1], infoAtomo.atributo_ID);

      } else{
        for(int i = 0;i < nVariaveis-1;i++){
          if(strcmp(tabelaSimbolos[i], infoAtomo.atributo_ID)==0){
            printf("#%03d: Erro semântico: variável repetida [%s]\n", InfoAtomo.linha, infoAtomo.atributo_ID);
            exit(1);
          }
        }

        tabelaSimbolos = (char **)realloc(tabelaSimbolos, nVariaveis * sizeof(char *));
        tabelaSimbolos[nVariaveis-1] = (char *)malloc((strlen(infoAtomo.atributo_ID) + 1) * sizeof(char));
        strcpy(tabelaSimbolos[nVariaveis-1], infoAtomo.atributo_ID);
      }
      }

      if(flagUsoVariaveis == 1){
        int flagExisteVar = 0;
        for(int i = 0;i < nVariaveis;i++){
          if(strcmp(tabelaSimbolos[i], infoAtomo.atributo_ID)==0){
            flagExisteVar = 1;
          }
        }
        if(flagExisteVar == 0){
          printf("#%03d: Erro semântico: variável não declarada [%s]\n", InfoAtomo.linha, infoAtomo.atributo_ID);
          exit(1);
        }
      }
    }

    return infoAtomo;
}
//###############################
// ANALISADOR SINTATICO
//###############################
void consome( TAtomo atomo ){
    if( lookahead == atomo ){
      /*if(lookahead == IDENTIFICADOR){
        printf("#%03d:identificador - atributo:%s\n", InfoAtomo.linha, InfoAtomo.atributo_ID);
      } else{
        printf("#%03d:%s\n", InfoAtomo.linha,strAtomo[lookahead]);
      }*/
        //    lookahead = *buffer++;
        InfoAtomo = obter_atomo();
        lookahead = InfoAtomo.atomo;

    }
    else{
        //printf("erro sintatico: esperado [%c] encontrado [%c]\n",atomo,lookahead);
       // printf("#%03d: Erro sintatico: esperado [%s] encontrado [%s]\n", InfoAtomo.linha,strAtomo[atomo],strAtomo[lookahead]);
      printf("#%03d: Erro sintatico: esperado [%s] encontrado [%s]\n", InfoAtomo.linha, strAtomo[atomo] ,strAtomo[lookahead]);
        exit(1);
    }
}

//<programa>::= algoritmo identificador �;� <bloco> �.�
void programa(){
    printf("\tINPP\n");
    consome(ALGORITMO);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    bloco();
    consome(PONTO);
    printf("\tPARA");

}

//============================ Minhas Funcs ============================================

void bloco(){
  if(lookahead == VARIAVEL){
    declaracaoDeVariaveis();
    printf("\tAMEM %d\n", nVariaveis);
  }
  comandoComposto();
}

void declaracaoDeVariaveis(){
  flagUsoVariaveis = 0;
  consome(VARIAVEL);
  do{
    listaVariavel();
    consome(DOIS_PONTOS);
    tipo();
    consome(PONTO_VIRGULA);
  } while (lookahead == IDENTIFICADOR);
}

void listaVariavel(){
  consome(IDENTIFICADOR);
  while (lookahead == VIRGULA){
    consome(VIRGULA);
    consome(IDENTIFICADOR);
  }
}

void tipo(){
  if(lookahead == INTEIRO){
    consome(INTEIRO);
  } else{
    consome(LOGICO);
  }
}

void comandoComposto(){
  consome(INICIO);
  flagUsoVariaveis = 1;
  comando();
  while(lookahead == PONTO_VIRGULA){
    consome(PONTO_VIRGULA);
    comando();
  }
  consome(FIM);
}

void comando(){
  if(lookahead == IDENTIFICADOR){
    comandoAtribuicao();
  } else if (lookahead == SE){
    comandoSe();
  } else if (lookahead == ENQUANTO){
    comandoEnquanto();
  } else if (lookahead == LEIA){
    comandoEntrada();
  } else if (lookahead == ESCREVA){
    comandoSaida();
  } else{
    comandoComposto();
  }
}

void comandoAtribuicao(){
  int endereco;
  for(int i = 0; i < nVariaveis;i++){
    if(strcmp(tabelaSimbolos[i], InfoAtomo.atributo_ID)==0){
      endereco = i;
    }
  }
  consome(IDENTIFICADOR);
  consome(DOIS_PONTOS_IGUAL);
  expressao();
  printf("\tARMZ %d\n", endereco);

}

void comandoSe(){
  int L1 = proximo_rotulo();
  int L2 = proximo_rotulo();
  consome(SE);
  consome(ABRE_PARENTESES);
  expressao();
  consome(FECHA_PARENTESES);
  printf("\tDSVF L%d\n", L1);
  consome(ENTAO);
  comando();
  if(lookahead == SENAO){
    printf("\tDSVS L%d\n", L2);
  }
  printf("L%d:\tNADA\n", L1);
  if(lookahead == SENAO){
    consome(SENAO);
    comando();
    printf("L%d:\tNADA\n", L2);
  }
}

void comandoEnquanto(){
  int L1 = proximo_rotulo();
  int L2 = proximo_rotulo();
  consome(ENQUANTO);
  printf("L%d:\tNADA\n", L1);
  consome(ABRE_PARENTESES);
  expressao();
  consome(FECHA_PARENTESES);
  printf("\tDSVF L%d\n", L2);
  consome(FACA);
  comando();
  printf("\tDSVS L%d\n", L1);
  printf("L%d:\tNADA\n", L2);
}

void comandoEntrada(){
  consome(LEIA);
  printf("\tLEIT\n");
  consome(ABRE_PARENTESES);
  for(int i = 0;i < nVariaveis;i++){
    if(strcmp(tabelaSimbolos[i], InfoAtomo.atributo_ID)==0){
      printf("\tARMZ %d\n", i);
    }
  }
  listaVariavel();
  consome(FECHA_PARENTESES);
}

void comandoSaida(){
  consome(ESCREVA);
  consome(ABRE_PARENTESES);
  expressao();
  while(lookahead == VIRGULA){
    consome(VIRGULA);
    expressao();
  }
  consome(FECHA_PARENTESES);
  printf("\tIMPR\n");
}

void expressao(){
  expressaoSimples();
  if(lookahead == MENOR_QUE){
    relacional();
    expressaoSimples();
    printf("\tCMME\n");
  } else if (lookahead == MENOR_IGUAL_QUE){
    relacional();
    expressaoSimples();
    printf("\tCMEG\n");
  } else if (lookahead == IGUAL){
    relacional();
    expressaoSimples();
    printf("\tCMIG\n");
  } else if (lookahead == HASHTAG){
    relacional();
    expressaoSimples();
    printf("\tCMDG\n");
  } else if (lookahead == MAIOR_QUE){
    relacional();
    expressaoSimples();
    printf("\tCMMA\n");
  } else if (lookahead == MAIOR_IGUAL_QUE){
    relacional();
    expressaoSimples();
    printf("\tCMAG\n");
  }
}

void relacional(){
  if(lookahead == MENOR_QUE){
    consome(MENOR_QUE);
  } else if (lookahead == MENOR_IGUAL_QUE){
    consome(MENOR_IGUAL_QUE);
  } else if (lookahead == IGUAL){
    consome(IGUAL);
  } else if (lookahead == HASHTAG){
    consome(HASHTAG);
  } else if (lookahead == MAIOR_QUE){
    consome(MAIOR_QUE);
  } else{
    consome(MAIOR_IGUAL_QUE);
  }
}

void expressaoSimples(){
  if(lookahead == OP_SOMA){
    consome(OP_SOMA);
    termo();
  } else if(lookahead == OP_SUBT){
    consome(OP_SUBT);
    termo();
    printf("\tINVR\n");
  } else{
    termo();
  }
  while(lookahead == OP_SOMA || lookahead == OP_SUBT || lookahead == OU){
    if(lookahead == OP_SOMA){
      consome(OP_SOMA);
      termo();
      printf("\tSOMA\n");
    } else if(lookahead == OP_SUBT){
      consome(OP_SUBT);
      termo();
      printf("\tSUBT\n");
    } else{
      consome(OU);
      termo();
      printf("\tDISJ\n");
    }
  }
}

void termo(){ //Mudei a estrutura no Trab2, pode dar erro
  fator();
  while(lookahead == OP_MULT || lookahead == OP_DIVI || lookahead == E){
    if(lookahead == OP_MULT){
      consome(OP_MULT);
      fator();
      printf("\tMULT\n");
    } else if(lookahead == OP_DIVI){
      consome(OP_DIVI);
      printf("\tDIVI\n");
      fator();
    } else{
      consome(E);
      fator();
      printf("\tCONJ\n");
    }
  }
}

void fator(){
  if(lookahead == IDENTIFICADOR){
    for(int i = 0; i < nVariaveis;i++){
      if(strcmp(tabelaSimbolos[i], InfoAtomo.atributo_ID)==0){
        printf("\tCRVL %d\n", i);
      }
    }
    consome(IDENTIFICADOR);
  } else if(lookahead == NUMERO){
    printf("\tCRCT %.0f\n", InfoAtomo.atributo_numero);
    consome(NUMERO);
  } else if(lookahead == VERDADEIRO){
    consome(VERDADEIRO);
  } else if (lookahead == FALSO){
    consome(FALSO);
  } else{
    consome(ABRE_PARENTESES);
    expressao();
    consome(FECHA_PARENTESES);
  }
}

int proximo_rotulo(){
  rotulo++;
  return rotulo;
}
