#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "ad.h"



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
Token *lastToken, *tokens, *iTk, *consumedTk;
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

void EliberareMemorie(){
    Token *p;
    while(tokens!=NULL){
        p=tokens;
        tokens=tokens->next;
        free(p);
    }
}

void err(const char *fmt,...)
{
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    EliberareMemorie();
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
    EliberareMemorie();
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
const char * tkCodeName(int code){
    return a[code];
}

bool structDef();
bool fnDef();
bool varDef();
bool typeBase();
bool arrayDecl();
bool expr();
bool fnParam();
bool stmCompound();
bool stm();
bool exprAssign();
bool exprUnary();
bool exprOr();
bool exprOrPrim();
bool exprAnd();
bool exprAndPrim();
bool exprEq();
bool exprEqPrim();
bool exprRel();
bool exprRelPrim();
bool exprAdd();
bool exprAddPrim();
bool exprMul();
bool exprMulPrim();
bool exprCast();
bool exprPostfix();
bool exprPostfixPrim();
bool exprPrimary();


/*
bool consume(int code){
    if(iTk->code==code)
    {
        consumedTk=iTk;
        iTk=iTk->next;
        return true;
    }
    return false;
}
*/

bool consume(int code){
    printf("consume(%s)",tkCodeName(code));
    if(iTk->code==code)
    {
        consumedTk=iTk;
        iTk=iTk->next;
        printf(" => consumed\n");
        return true;
    }
    printf(" => found %s\n",tkCodeName(iTk->code));
    return false;
}

bool unit(){
    Token *start=iTk;
    for(;;){
        if(structDef()){}
        else if(fnDef()){}
        else if(varDef()){}
        else break;
    }
    if(consume(END)){
        return true;
    }
    iTk = start;
    return false;
}
bool structDef(){
    Token *start=iTk;
    if (consume(STRUCT)){
        if(consume(ID)){
            if(consume(LACC)){
                while(varDef()){}
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        return true;
                    }
                    else tkerr(iTk, "missing ;");
                }
                else tkerr(iTk, "missing } after last struct field");
            }
        }
        else tkerr(iTk, "missing identifier after struct");
    }
    iTk = start;
    return false;
}
bool varDef(){
    Token *start=iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){}
            if(consume(SEMICOLON)){
                return true;
            }
            else tkerr(iTk, "missing ;");

        }
        else tkerr(iTk, "missing identifier after type declaration");
    }
    iTk = start;
    return false;
}
bool fnDef(){
    Token *start=iTk;
    if(typeBase()||consume(VOID)){
        if(consume(ID)){
            if(consume(LPAR)){
                if(fnParam()){
                    while(consume(COMMA)){
                        if(fnParam()){
                        }
                        else{
                          tkerr(iTk, "there is a comma but no parameter after");
                          //iTk = start;
                          //return false;
                        }
                    }
                }
                if(consume(RPAR)){
                    if(stmCompound()){
                        return true;
                    }
                    else tkerr(iTk, "missing function implementation");
                }
                else tkerr(iTk, "missing )");
            }
        }
        else tkerr(iTk, "missing function identifier");
    }
    iTk = start;
    return false;
}
bool typeBase(){
    Token *start=iTk;
    if(consume(INT)){
        return true;
    }
    if(consume(DOUBLE)){
        return true;
    }
    if(consume(CHAR)){
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            return true;
        }
        else tkerr(iTk, "missing struct identifier");
    }
    iTk = start;
    return false;
}
bool arrayDecl(){
    Token *start=iTk;
    if(consume(LBRACKET)){
        if(consume(CT_INT)){}
        if(consume(RBRACKET)){
            return true;
        }
        else tkerr(iTk, "missing ]");
    }
    iTk = start;
    return false;
}
bool fnParam(){
    Token *start=iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){}
            return true;
        }
        else tkerr(iTk, "missing function parameter type identifier");
    }
    iTk = start;
    return false;
}
bool stm(){
    Token *start=iTk;
    if(stmCompound()){
        return true;
    }
    iTk=start;
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(stm()){
                                return true;
                            }
                            else tkerr(iTk, "missing else branch code block");
                            //iTk = start;
                            //return false;
                        }
                        return true;
                    }
                    else tkerr(iTk, "missing if branch code block");
                }
                else tkerr(iTk, "missing )");
            }
            else tkerr(iTk, "missing if condition");
        }
        else tkerr(iTk, "missing (");
    }
    iTk=start;
    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        return true;
                    }
                    else tkerr(iTk, "missing while code block");
                }
                else tkerr(iTk, "missing )");
            }
            else tkerr(iTk, "missing while condition");
        }
        else tkerr(iTk, "missing (");
    }
    iTk=start;
    if(consume(FOR)){
        if(consume(LPAR)){
            if(expr()){}
            if(consume(SEMICOLON)){
                if(expr()){}
                if(consume(SEMICOLON)){
                    if(expr()){}
                    if(consume(RPAR)){
                        if(stm()){
                            return true;
                        }
                        else tkerr(iTk, "missing for code block");
                    }
                    else tkerr(iTk, "missing )");
                }
                else tkerr(iTk, "missing ;");
            }
            else tkerr(iTk, "missing ;");
        }
        else tkerr(iTk, "missing (");
    }
    iTk=start;
    if(consume(BREAK)){
        if(consume(SEMICOLON)){
            return true;
        }
        else tkerr(iTk, "missing ;");
    }
    iTk=start;
    if(consume(RETURN)){
        if(expr()){}
        if(consume(SEMICOLON)){
            return true;
        }
        else tkerr(iTk, "missing ;");
    }
    iTk=start;
    if(expr()){}
    if(consume(SEMICOLON)){
        return true;
    }
    iTk = start;
    return false;
}
bool stmCompound(){
    Token *start=iTk;
    if(consume(LACC)){
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            return true;
        }
        else tkerr(iTk, "missing }");
    }
    iTk = start;
    return false;
}
bool expr(){
    Token *start=iTk;
    if(exprAssign()){
        return true;
    }
    iTk = start;
    return false;
}
bool exprAssign(){
    Token *start=iTk;
    if(exprUnary()){
        if(consume(ASSIGN)){
            if(exprAssign()){
                return true;
            }
            else tkerr(iTk, "missing right expression term");
        }
    }
    iTk=start;
    if(exprOr()){
        return true;
    }
    iTk = start;
    return false;
}
bool exprUnary(){
    Token *start=iTk;
    if(consume(SUB)||consume(NOT)){
        if(exprUnary()){
            return true;
        }
        else tkerr(iTk, "missing expresion after unary - or unary !");
    }
    iTk=start;
    if(exprPostfix()){
        return true;
    }
    iTk = start;
    return false;
}
bool exprOr(){
    Token *start=iTk;
    if(exprAnd()){
        if(exprOrPrim()){
            return true;
        }
        //else tkerr(iTk, "missing ||");
    }
    iTk = start;
    return false;
}
bool exprOrPrim(){
    Token *start=iTk;
    if(consume(OR)){
        if(exprAnd()){
            if(exprOrPrim()){
                return true;
            }
            //else tkerr(iTk, "missing ||");
        }
        else tkerr(iTk, "missing expression after ||");
    }
    iTk = start;
    return true;
}
bool exprAnd(){
    Token *start=iTk;
    if(exprEq()){
        if(exprAndPrim()){
            return true;
        }
        //else tkerr(iTk, "missing &&");
    }
    iTk = start;
    return false;
}
bool exprAndPrim(){
    Token *start=iTk;
    if(consume(AND)){
        if(exprEq()){
            if(exprAndPrim()){
                return true;
            }
            //else tkerr(iTk, "missing &&");
        }
        else tkerr(iTk, "missing expression after &&");
    }
    iTk = start;
    return true;
}
bool exprEq(){
    Token *start=iTk;
    if(exprRel()){
        if(exprEqPrim()){
            return true;
        }
        //else tkerr(iTk, "missing == or !=");
    }
    iTk = start;
    return false;
}
bool exprEqPrim(){
    Token *start=iTk;
    if(consume(EQUAL)||consume(NOTEQ)){
        if(exprRel()){
            if(exprEqPrim()){
                return true;
            }
            //else tkerr(iTk, "missing == or !=");
        }
        else tkerr(iTk, "missing expression after == or !=");
    }
    iTk = start;
    return true;
}
bool exprRel(){
    Token *start=iTk;
    if(exprAdd()){
        if(exprRelPrim()){
            return true;
        }
        //else tkerr(iTk, "missing <, <=, > or >=");

    }
    iTk = start;
    return false;
}
bool exprRelPrim(){
    Token *start=iTk;
    if(consume(LESS)||consume(LESSEQ)||consume(GREATER)||consume(GREATEREQ)){
        if(exprAdd()){
            if(exprRelPrim()){
                return true;
            }
            //else tkerr(iTk, "missing <, <=, > or >=");
        }
        else tkerr(iTk, "missing expression after <, <=, > or >=");
    }
    iTk = start;
    return true;
}
bool exprAdd(){
    Token *start=iTk;
    if(exprMul()){
        if(exprAddPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}
bool exprAddPrim(){
    Token *start=iTk;
    char sym;
    if(start->code == ADD) {sym='+';}
        else {sym='-';}
    if(consume(ADD)||consume(SUB)){
        if(exprMul()){
            if(exprAddPrim()){
                return true;
            }
        }
        else tkerr(iTk, "missing expression after %c",sym);
    }
    iTk = start;
    return true;
}
bool exprMul(){
    Token *start=iTk;
    if(exprCast()){
        if(exprMulPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}
bool exprMulPrim(){
    Token *start=iTk;
    char sym;
    if(start->code == MUL) {sym='*';}
        else {sym='/';}
    if(consume(MUL)||consume(DIV)){
        if(exprCast()){
            if(exprMulPrim()){
                return true;
            }
        }
        else tkerr(iTk, "missing expression after %c",sym);
    }
    iTk = start;
    return true;
}
bool exprCast(){
    Token *start=iTk;
    if(consume(LPAR)){
        if(typeBase()){
            if(arrayDecl()){}
            if(consume(RPAR)){
                if(exprCast()){
                    return true;
                }
            }
            else tkerr(iTk, "missing )");
        }
        //else tkerr(iTk, "missing cast type declaration");
    }
    iTk=start;
    if(exprUnary()){
        return true;
    }
    iTk = start;
    return false;
}
bool exprPostfix(){
    Token *start=iTk;
    if(exprPrimary()){
        if(exprPostfixPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}
bool exprPostfixPrim(){
    Token *start=iTk;
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfixPrim()){
                    return true;
                }
            }
            else tkerr(iTk, "missing ]");
        }
        else tkerr(iTk, "missing expression between []");
    }
    iTk=start;
    if(consume(DOT)){
        if(consume(ID)){
            if(exprPostfixPrim()){
                return true;
            }
        }
        else tkerr(iTk, "missing identifier after .");
    }
    iTk = start;
    return true;
}
bool exprPrimary(){
    Token *start=iTk;
    if(consume(ID)){
        if(consume(LPAR)){
            if(expr()){
                while(consume(COMMA)){
                    if(expr()){}
                    else{
                        tkerr(iTk, "missing expression after ,");
                        //iTk=start;
                        //return false;
                    }
                }
            }
            if(consume(RPAR)){
                return true;
            }
            else tkerr(iTk, "missing )");
            //iTk=start;
            //return false;
        }
        return true;
    }
    iTk=start;
    if(consume(CT_INT)){
        return true;
    }
    if(consume(CT_REAL)){
        return true;
    }
    if(consume(CT_CHAR)){
        return true;
    }
    if(consume(CT_STRING)){
        return true;
    }
    if(consume(LPAR)){
        if(expr()){
            if(consume(RPAR)){
                return true;
            }
            else tkerr(iTk, "missing )");
        }
        else tkerr(iTk, "missing expression after (");
    }
    iTk = start;
    return false;
}

int main()
{
    FILE *fis;

    fis=fopen("2.txt", "rb");
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
    //showAtoms();
    iTk = tokens;
    bool start = unit();
    printf("%d\n", start);

    EliberareMemorie();
    return 0;
}
