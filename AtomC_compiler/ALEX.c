#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>



enum{
/* IDENTIFICATORI SI CONSTANTE */
    ID, CT_INT, CT_REAL, CT_CHAR, CT_STRING,
/* CUVINTE CHEIE*/
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE,
/* DELIMITATORI */
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, END,
/* OPERATORI*/
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ
};

typedef struct _Token{
    int code; // codul (numele)
    union{
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
    int line; // linia din fisierul de intrare
    struct _Token *next; // inlantuire la urmatorul AL
}Token;

int line=1;
Token *lastToken, *tokens;
char buffin[30001];
char *pCrtCh;
char *a[]={"ID", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", "BREAK",
            "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT", "RETURN",
            "STRUCT", "VOID", "WHILE", "COMMA", "SEMICOLON", "LPAR",
            "RPAR","LBRACKET","RBRACKET", "LACC", "RACC", "END", "ADD", "SUB", "MUL", "DIV", "DOT", "AND",
            "OR", "NOT", "ASSIGN", "EQUAL","NOTEQ", "LESS",
            "LESSEQ", "GREATER", "GREATEREQ"};

int getNextToken();

char *createString(const char *pStartCh, char *pCrtCh){

    int size = pCrtCh-pStartCh;
    char *value;
    value=(char *)malloc((size+1)*sizeof(char));
    if(value == NULL){
        printf("alocare esuata");
        exit(EXIT_FAILURE);
    }
    strncpy(value, pStartCh, size);
    value[size]='\0';

    return value;
}

void err(const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk,const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");
Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    }
        else{
            tokens=tk;
        }
    lastToken=tk;
    return tk;
}


int getNextToken()
{
    int state=0,nCh;
    char ch;
    const char *pStartCh;
    Token *tk;

    for(;;)
    { // bucla infinita
        ch=*pCrtCh;
        switch(state)
        {
            case 0:
                if(isalpha(ch)||ch=='_')
                {
                    pStartCh=pCrtCh;
                    pCrtCh++;
                    state=27;
                }
                else if(ch=='+')
                {
                    pCrtCh++;
                    state=1;
                }
                else if(ch=='-')
                {
                    pCrtCh++;
                    state=2;
                }
                else if(ch=='*')
                {
                    pCrtCh++;
                    state=3;
                }
                else if(ch=='/')
                {
                    pCrtCh++;
                    state=4;
                }
                else if(ch=='.')
                {
                    pCrtCh++;
                    state=5;
                }
                else if(ch=='&')
                {
                    pCrtCh++;
                    state=6;
                }
                else if(ch=='|')
                {
                    pCrtCh++;
                    state=8;
                }
                else if(ch=='!')
                {
                    pCrtCh++;
                    state=10;
                }
                else if(ch=='=')
                {
                    pCrtCh++;
                    state=12;
                }
                else if(ch=='<')
                {
                    pCrtCh++;
                    state=14;
                }
                else if(ch=='>')
                {
                    pCrtCh++;
                    state=16;
                }
                else if(ch==',')
                {
                    pCrtCh++;
                    state=18;
                }
                else if(ch==';')
                {
                    pCrtCh++;
                    state=19;
                }
                else if(ch=='(')
                {
                    pCrtCh++;
                    state=20;
                }
                else if(ch==')')
                {
                    pCrtCh++;
                    state=21;
                }
                else if(ch=='[')
                {
                    pCrtCh++;
                    state=22;
                }
                else if(ch==']')
                {
                    pCrtCh++;
                    state=23;
                }
                else if(ch=='{')
                {
                    pCrtCh++;
                    state=24;
                }
                else if(ch=='}')
                {
                    pCrtCh++;
                    state=25;
                }
                else if(ch>='0'&&ch<='9')
                {
                    pStartCh=pCrtCh;
                    pCrtCh++;
                    state=29;
                }
                else if(ch=='\'')
                {
                    pCrtCh++;
                    state=37;
                }
                else if(ch=='"')
                {
                    pStartCh=pCrtCh;
                    pCrtCh++;
                    state=40;
                }
                else if(ch==' '||ch=='\t'||ch=='\r')
                {
                    pCrtCh++;
                }
                else if(ch=='\n')
                { // tratat separat pentru a actualiza linia curenta
                    line++;
                    pCrtCh++;
                }
                else if(ch=='\0')
                { // sfarsit de sir
                    addTk(END);
                    return END;
                }
                else tkerr(addTk(END),"caracter invalid");
                break;

            case 1:
                addTk(ADD);
                return ADD;
            case 2:
                addTk(SUB);
                return SUB;
            case 3:
                addTk(MUL);
                return MUL;
            case 4:
                if(ch=='/')
                {
                    pCrtCh++;
                    state=43;
                }
                else state=44;
                break;
            case 5:
                addTk(DOT);
                return DOT;
            case 6:
                if(ch=='&')
                {
                    pCrtCh++;
                    state=7;
                }
                else tkerr(addTk(END),"caracter invalid");
                break;
            case 7:
                addTk(AND);
                return AND;
            case 8:
                if(ch=='|')
                {
                    pCrtCh++;
                    state=9;
                }
                else tkerr(addTk(END),"caracter invalid");
                break;
            case 9:
                addTk(OR);
                return OR;
            case 10:
                if(ch=='=')
                {
                    pCrtCh++;
                    state=11;
                }
                else state=45; //to NOT
                break;
            case 11:
                addTk(NOTEQ);
                return NOTEQ;
            case 12:
                if(ch=='=')
                {
                    pCrtCh++;
                    state=13;
                }
                else state=46; //to ASSIGN
                break;
            case 13:
                addTk(EQUAL);
                return EQUAL;
            case 14:
                if(ch=='=')
                {
                    pCrtCh++;
                    state=15;
                }
                else state=47; //to LESS
                break;
            case 15:
                addTk(LESSEQ);
                return LESSEQ;
            case 16:
                if(ch=='=')
                {
                    pCrtCh++;
                    state=17;
                }
                else state=48; //to GREAT
                break;
            case 17:
                addTk(GREATEREQ);
                return GREATEREQ;
            case 18:
                addTk(COMMA);
                return COMMA;
            case 19:
                addTk(SEMICOLON);
                return SEMICOLON;
            case 20:
                addTk(LPAR);
                return LPAR;
            case 21:
                addTk(RPAR);
                return RPAR;
            case 22:
                addTk(LBRACKET);
                return LBRACKET;
            case 23:
                addTk(RBRACKET);
                return RBRACKET;
            case 24:
                addTk(LACC);
                return LACC;
            case 25:
                addTk(RACC);
                return RACC;
            case 44:
                addTk(DIV);
                return DIV;
            case 45:
                addTk(NOT);
                return NOT;
            case 46:
                addTk(ASSIGN);
                return ASSIGN;
            case 47:
                addTk(LESS);
                return LESS;
            case 48:
                addTk(GREATER);
                return GREATER;
            case 27:
                if(isalnum(ch)||ch=='_')pCrtCh++;
                else state=28;
                break;
            case 28:
                nCh=pCrtCh-pStartCh; // lungimea cuvantului gasit
                // teste cuvinte cheie
                if(nCh==5&&!memcmp(pStartCh,"break",5))tk=addTk(BREAK);
                else if(nCh==4&&!memcmp(pStartCh,"char",4))tk=addTk(CHAR);
                else if (nCh==6&&!memcmp(pStartCh,"double",6))tk=addTk(DOUBLE);
                else if (nCh==4&&!memcmp(pStartCh,"else",4))tk=addTk(ELSE);
                else if (nCh==3&&!memcmp(pStartCh,"for",3))tk=addTk(FOR);
                else if (nCh==2&&!memcmp(pStartCh,"if",2))tk=addTk(IF);
                else if (nCh==3&&!memcmp(pStartCh,"int",3))tk=addTk(INT);
                else if (nCh==6&&!memcmp(pStartCh,"return",6))tk=addTk(RETURN);
                else if (nCh==6&&!memcmp(pStartCh,"struct",6))tk=addTk(STRUCT);
                else if (nCh==4&&!memcmp(pStartCh,"void",4))tk=addTk(VOID);
                else if (nCh==5&&!memcmp(pStartCh,"while",5))tk=addTk(WHILE);

                else{ // daca nu este un cuvant cheie, atunci e un ID
                    tk=addTk(ID);
                    tk->text=createString(pStartCh,pCrtCh);
                }
                return tk->code;
            case 29:
               if(ch>='0'&&ch<='9') pCrtCh++;
               else if (ch=='.'){ pCrtCh++; state=31; }
               else state=30;
               break;
            case 30:
                tk=addTk(CT_INT);
                tk->i=strtol(pStartCh, NULL, 10);
                return tk->code;
            case 31:
                if(ch>='0'&&ch<='9')
                {
                    pCrtCh++;
                    state=32;
                }

                else tkerr(addTk(END),"caracter invalid");
                break;
            case 32:
                if(ch>='0'&&ch<='9') pCrtCh++;
                else if (ch=='e'|| ch=='E') state=33;
                else state=36;
                break;
            case 33:
                if(ch=='+'||ch=='-')
                {
                    pCrtCh++;
                    state=34;
                }
                else
                {
                    pCrtCh++;
                    state=34;
                }
                break;
            case 34:
                if(ch>='0'&&ch<='9')
                {
                    pCrtCh++;
                    state=35;
                }

                else tkerr(addTk(END),"caracter invalid");
                break;
            case 35:
                if(ch>='0'&&ch<='9') pCrtCh++;
                else state=36;
                break;
            case 36:
                tk=addTk(CT_REAL);
                tk->r=atof(pStartCh);
                return tk->code;
            case 37:
                if(ch!='\'')
                {
                    pCrtCh++;
                    state=38;
                }
                else tkerr(addTk(END),"caracter invalid");
                break;
            case 38:
                if(ch=='\'')
                {
                    pCrtCh++;
                    state=39;
                }
                else tkerr(addTk(END),"caracter invalid");
                break;
            case 39:
                tk=addTk(CT_CHAR);
                tk->i=*(pCrtCh-2);
                return tk->code;
            case 40:
                if(ch!='"')
                {
                    pCrtCh++;
                    state=40;
                }
                else
                {
                    pCrtCh++;
                    state=41;
                }
                break;
            case 41:
                tk=addTk(CT_STRING);
                tk->text=createString(pStartCh+1,pCrtCh-1);
                return tk->code;
            case 43:
                if(!(ch == '\n' || ch == '\r' || ch == '\0')) {pCrtCh++;}
                else
                {
                    pCrtCh++;
                    state=0;
                }
                break;
            default:
                printf("stare necunoscuta\n");
                break;
        }
    }
}
void showAtoms()
{
    Token *p;
    while(tokens!=NULL)
    {
        printf("%d\t%s",tokens->line, a[tokens->code]);
        if(tokens->code == ID || tokens->code == CT_STRING){ printf(":%s",tokens->text); }
        else if(tokens->code == CT_INT){ printf(":%d",tokens->i); }
        else if(tokens->code == CT_CHAR){ printf(":%c",tokens->i); }
        else if(tokens->code == CT_REAL){ printf(":%f",tokens->r); }
        printf("\n");
        p=tokens;
        tokens=tokens->next;
        free(p);
    }
}
int main()
{
    FILE *fis;

    fis=fopen("1.txt", "rb");
    if(fis==NULL){
        printf("nu s-a putut deschide fisierul");
        return -1;
    }

    int n=fread(buffin,1,30000,fis);
    buffin[n]='\n';
    fclose(fis);

    pCrtCh=buffin;
    while(getNextToken()!=END){
        }
    showAtoms();
    return 0;
}
