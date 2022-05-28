#include "ad.h"
#include "at.h"
#include "mv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>



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
Symbol *owner = NULL;

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
bool typeBase(Type *t);
bool arrayDecl(Type *t);
bool expr();
bool fnParam();
bool stmCompound(bool newDomain);
bool stm();
bool exprAssign(Ret *r);
bool exprUnary(Ret *r);
bool exprOr(Ret *r);
bool exprOrPrim(Ret *r);
bool exprAnd(Ret *r);
bool exprAndPrim(Ret *r);
bool exprEq(Ret *r);
bool exprEqPrim(Ret *r);
bool exprRel(Ret *r);
bool exprRelPrim(Ret *r);
bool exprAdd(Ret *r);
bool exprAddPrim(Ret *r);
bool exprMul(Ret *r);
bool exprMulPrim(Ret *r);
bool exprCast(Ret *r);
bool exprPostfix(Ret *r);
bool exprPostfixPrim(Ret *r);
bool exprPrimary(Ret *r);


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

// numele structurii trebuie sa fie unic in domeniu
// in interiorul structurii nu pot exista doua variabile cu acelasi nume
bool structDef(){
    Token *start=iTk;
    if (consume(STRUCT)){
        if(consume(ID)){
            Token *tkName=consumedTk;
            if(consume(LACC)){
                Symbol *s=findSymbolInDomain(symTable,tkName->text);
                if(s)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;
                while(varDef()){}
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        owner=NULL;
                        dropDomain();
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

// numele variabilei trebuie sa fie unic in domeniu
// variabilele de tip vector trebuie sa aiba dimensiunea data (nu se accepta: int v[])
bool varDef(){
    Token *start=iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName=consumedTk;
            if(arrayDecl(&t)){
                if(t.n==0)tkerr(iTk,"a vector variable must have a specified dimension");
            }
            if(consume(SEMICOLON)){
                Symbol *var=findSymbolInDomain(symTable,tkName->text);
                if(var)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                var=newSymbol(tkName->text,SK_VAR);
                var->type=t;
                var->owner=owner;
                addSymbolToDomain(symTable,var);
                if(owner){
                    switch(owner->kind){
                        case SK_FN:
                            var->varIdx=symbolsLen(owner->fn.locals);
                            addSymbolToList(&owner->fn.locals,dupSymbol(var));
                            break;
                        case SK_STRUCT:
                            var->varIdx=typeSize(&owner->type);
                            addSymbolToList(&owner->structMembers,dupSymbol(var));
                            break;
                        }
                    }else{
                        var->varIdx=allocInGlobalMemory(typeSize(&t));
                    }
                return true;
            }
            else tkerr(iTk, "missing ;");

        }
        else tkerr(iTk, "missing identifier after type declaration");
    }
    iTk = start;
    return false;
}

// numele functiei trebuie sa fie unic in domeniu
// domeniul local functiei incepe imediat dupa LPAR
// corpul functiei {...} nu defineste un nou subdomeniu in domeniul local functiei
bool fnDef(){
    Type t;
    Token *start=iTk;
    if(typeBase(&t)||consume(VOID)){
        if(consumedTk->code == VOID)
        {
            t.tb=TB_VOID;
        }
        if(consume(ID)){
            Token *tkName=consumedTk;
            if(consume(LPAR)){
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr(iTk,"symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
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
                    if(stmCompound(false)){
                        dropDomain();
                        owner=NULL;
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

// daca tipul de baza este o structura, ea trebuie sa fie deja definita anterior
bool typeBase(Type *t){
    Token *start=iTk;
    t->n=-1;
    if(consume(INT)){
        t->tb=TB_INT;
        return true;
    }
    if(consume(DOUBLE)){
        t->tb=TB_DOUBLE;
        return true;
    }
    if(consume(CHAR)){
        t->tb=TB_CHAR;
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName=consumedTk;
            t->tb=TB_STRUCT;
            t->s=findSymbol(tkName->text);
            if(!t->s)tkerr(iTk,"structura nedefinita: %s",tkName->text);
            return true;
        }
        else tkerr(iTk, "missing struct identifier");
    }
    iTk = start;
    return false;
}
bool arrayDecl(Type *t){
    Token *start=iTk;
    if(consume(LBRACKET)){
        if(consume(CT_INT)){
            Token *tkSize=consumedTk;
            t->n = tkSize ->i;
        }
            else {t->n=0;}
        if(consume(RBRACKET)){
            return true;
        }
        else tkerr(iTk, "missing ]");
    }
    iTk = start;
    return false;
}

// numele parametrului trebuie sa fie unic in domeniu
// parametrii pot fi vectori cu dimensiune data, dar in acest caz li se sterge
//dimensiunea ( int v[10] -> int v[] )
bool fnParam(){
    Type t;
    Token *start=iTk;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName=consumedTk;
            if(arrayDecl(&t)){
                t.n=0;
            }
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr(iTk,"symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->paramIdx=symbolsLen(owner->fn.params);
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        }
        else tkerr(iTk, "missing function parameter type identifier");
    }
    iTk = start;
    return false;
}

// corpul compus {...} al instructiunilor defineste un nou domeniu
// IF - conditia trebuie sa fie scalar
// WHILE, FOR - conditia trebuie sa fie scalar
// RETURN - expresia trebuie sa fie scalar
// RETURN - functiile void nu pot returna o valoare
// RETURN - functiile non-void trebuie sa aiba o expresie returnata, a carei tip sa fie
//convertibil la tipul returnat de functie
bool stm(){
    Token *start=iTk;
    Ret rInit,rCond,rStep,rExpr;
    if(stmCompound(true)){
        return true;
    }
    iTk=start;
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr(&rCond)){
                if(!canBeScalar(&rCond))tkerr(iTk,"the if condition must be a scalar value");
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
            if(expr(&rCond)){
                if(!canBeScalar(&rCond))tkerr(iTk,"the while condition must be a scalar value");
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
            if(expr(&rInit)){}
            if(consume(SEMICOLON)){
                if(expr(&rCond)){
                    if(!canBeScalar(&rCond))tkerr(iTk,"the for condition must be a scalar value");
                }
                if(consume(SEMICOLON)){
                    if(expr(&rStep)){}
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
        if(expr(&rExpr)){
            if(owner->type.tb==TB_VOID)tkerr(iTk,"a void function cannot return a value");
            if(!canBeScalar(&rExpr))tkerr(iTk,"the return value must be a scalar value");
            if(!convTo(&rExpr.type,&owner->type))tkerr(iTk,"cannot convert the return expression type to the function return type");
        }
        else{
            if(owner->type.tb!=TB_VOID)tkerr(iTk,"a non-void function must return a value");
        }

        if(consume(SEMICOLON)){
            return true;
        }
        else tkerr(iTk, "missing ;");
    }
    iTk=start;
    if(expr(&rExpr)){}
    if(consume(SEMICOLON)){
        return true;
    }
    iTk = start;
    return false;
}

// se defineste un nou domeniu doar la cerere
bool stmCompound(bool newDomain){
    Token *start=iTk;
    if(consume(LACC)){
        if(newDomain)pushDomain();
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            if(newDomain)dropDomain();
            return true;
        }
        else tkerr(iTk, "missing }");
    }
    iTk = start;
    return false;
}
bool expr(Ret *r){
    Token *start=iTk;
    if(exprAssign(r)){
        return true;
    }
    iTk = start;
    return false;
}

// Destinatia trebuie sa fie left-value
// Destinatia nu trebuie sa fie constanta
// Ambii operanzi trebuie sa fie scalari
// Sursa trebuie sa fie convertibila la destinatie
// Tipul rezultat este tipul sursei
bool exprAssign(Ret *r){
    Ret rDst;
    Token *start=iTk;
    if(exprUnary(&rDst)){
        if(consume(ASSIGN)){
            if(exprAssign(r)){
                if(!rDst.lval)tkerr(iTk,"the assign destination must be a left-value");
                if(rDst.ct)tkerr(iTk,"the assign destination cannot be constant");
                if(!canBeScalar(&rDst))tkerr(iTk,"the assign destination must be scalar");
                if(!canBeScalar(r))tkerr(iTk,"the assign source must be scalar");
                if(!convTo(&r->type,&rDst.type))tkerr(iTk,"the assign source cannot be converted to destination");
                r->lval=false;
                r->ct=true;
                return true;
            }
            else tkerr(iTk, "missing right expression term");
        }
    }
    iTk=start;
    if(exprOr(r)){
        return true;
    }
    iTk = start;
    return false;
}
// Minus unar si Not trebuie sa aiba un operand scalar
// Rezultatul lui Not este un int
bool exprUnary(Ret *r){
    Token *start=iTk;
    if(consume(SUB)||consume(NOT)){
        if(exprUnary(r)){
            if(!canBeScalar(r))tkerr(iTk,"unary - must have a scalar operand");
            r->lval=false;
            r->ct=true;
            return true;
        }
        else tkerr(iTk, "missing expresion after unary - or unary !");
    }
    iTk=start;
    if(exprPostfix(r)){
        return true;
    }
    iTk = start;
    return false;
}

// Ambii operanzi trebuie sa fie scalari si sa nu fie structuri
// Rezultatul este un int
bool exprOr(Ret *r){
    Token *start=iTk;
    if(exprAnd(r)){
        if(exprOrPrim(r)){
            return true;
        }
        //else tkerr(iTk, "missing ||");
    }
    iTk = start;
    return false;
}
bool exprOrPrim(Ret *r){
    Token *start=iTk;
    if(consume(OR)){
        Ret right;
        if(exprAnd(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr(iTk,"invalid operand type for ||");
            *r=(Ret){{TB_INT,NULL,-1},false,true};
            if(exprOrPrim(r)){
                return true;
            }
            //else tkerr(iTk, "missing ||");
        }
        else tkerr(iTk, "missing expression after ||");
    }
    iTk = start;
    return true;
}
// Ambii operanzi trebuie sa fie scalari si sa nu fie structuri
// Rezultatul este un int
bool exprAnd(Ret *r){
    Token *start=iTk;
    if(exprEq(r)){
        if(exprAndPrim(r)){
            return true;
        }
        //else tkerr(iTk, "missing &&");
    }
    iTk = start;
    return false;
}
bool exprAndPrim(Ret *r){
    Token *start=iTk;
    if(consume(AND)){
        Ret right;
        if(exprEq(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr(iTk,"invalid operand type for &&");
            *r=(Ret){{TB_INT,NULL,-1},false,true};
            if(exprAndPrim(r)){
                return true;
            }
            //else tkerr(iTk, "missing &&");
        }
        else tkerr(iTk, "missing expression after &&");
    }
    iTk = start;
    return true;
}
// Ambii operanzi trebuie sa fie scalari si sa nu fie structuri
// Rezultatul este un int
bool exprEq(Ret *r){
    Token *start=iTk;
    if(exprRel(r)){
        if(exprEqPrim(r)){
            return true;
        }
        //else tkerr(iTk, "missing == or !=");
    }
    iTk = start;
    return false;
}
bool exprEqPrim(Ret *r){
    Token *start=iTk;
    if(consume(EQUAL)||consume(NOTEQ)){
        Ret right;
        if(exprRel(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr(iTk,"invalid operand type for == or !=");
            *r=(Ret){{TB_INT,NULL,-1},false,true};
            if(exprEqPrim(r)){
                return true;
            }
            //else tkerr(iTk, "missing == or !=");
        }
        else tkerr(iTk, "missing expression after == or !=");
    }
    iTk = start;
    return true;
}
// Ambii operanzi trebuie sa fie scalari si sa nu fie structuri
// Rezultatul este un int
bool exprRel(Ret *r){
    Token *start=iTk;
    if(exprAdd(r)){
        if(exprRelPrim(r)){
            return true;
        }
        //else tkerr(iTk, "missing <, <=, > or >=");

    }
    iTk = start;
    return false;
}
bool exprRelPrim(Ret *r){
    Token *start=iTk;
    if(consume(LESS)||consume(LESSEQ)||consume(GREATER)||consume(GREATEREQ)){
        Ret right;
        if(exprAdd(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr(iTk,"invalid operand type for <, <=, >, >=");
            *r=(Ret){{TB_INT,NULL,-1},false,true};
            if(exprRelPrim(r)){
                return true;
            }
            //else tkerr(iTk, "missing <, <=, > or >=");
        }
        else tkerr(iTk, "missing expression after <, <=, > or >=");
    }
    iTk = start;
    return true;
}
// Ambii operanzi trebuie sa fie scalari si sa nu fie structuri
bool exprAdd(Ret *r){
    Token *start=iTk;
    if(exprMul(r)){
        if(exprAddPrim(r)){
            return true;
        }
    }
    iTk = start;
    return false;
}
bool exprAddPrim(Ret *r){
    Token *start=iTk;
    char sym;
    if(start->code == ADD) {sym='+';}
        else {sym='-';}
    if(consume(ADD)||consume(SUB)){
        Ret right;
        if(exprMul(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr(iTk,"invalid operand type for + or -");
            *r=(Ret){tDst,false,true};
        }
            if(exprAddPrim(r)){
                return true;
            }
                else tkerr(iTk, "missing expression after %c",sym);
    }
    iTk = start;
    return true;
}
// Ambii operanzi trebuie sa fie scalari si sa nu fie structuri
bool exprMul(Ret *r){
    Token *start=iTk;
    if(exprCast(r)){
        if(exprMulPrim(r)){
            return true;
        }
    }
    iTk = start;
    return false;
}
bool exprMulPrim(Ret *r){
    Token *start=iTk;
    char sym;
    if(start->code == MUL) {sym='*';}
        else {sym='/';}
    if(consume(MUL)||consume(DIV)){
        Ret right;
        if(exprCast(&right)){
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr(iTk,"invalid operand type for * or /");
            *r=(Ret){tDst,false,true};
            if(exprMulPrim(r)){
                return true;
            }
        }
        else tkerr(iTk, "missing expression after %c",sym);
    }
    iTk = start;
    return true;
}

// Structurile nu se pot converti
// Tipul la care se converteste nu poate fi structura
// Un array se poate converti doar la alt array
// Un scalar se poate converti doar la alt scalar
bool exprCast(Ret *r){
    Token *start=iTk;
    if(consume(LPAR)){
        Ret op;
        Type t;
        if(typeBase(&t)){
            if(arrayDecl(&t)){}
            if(consume(RPAR)){
                if(exprCast(&op)){
                    if(t.tb==TB_STRUCT)tkerr(iTk,"cannot convert to a struct type");
                    if(op.type.tb==TB_STRUCT)tkerr(iTk,"cannot convert a struct");
                    if(op.type.n>=0&&t.n<0)tkerr(iTk,"an array can be converted only to another array");
                    if(op.type.n<0&&t.n>=0)tkerr(iTk,"a scalar can be converted only to another scalar");
                    *r=(Ret){t,false,true};
                    return true;
                }
            }
            else tkerr(iTk, "missing )");
        }
        //else tkerr(iTk, "missing cast type declaration");
    }
    iTk=start;
    if(exprUnary(r)){
        return true;
    }
    iTk = start;
    return false;
}
bool exprPostfix(Ret *r){
    Token *start=iTk;
    if(exprPrimary(r)){
        if(exprPostfixPrim(r)){
            return true;
        }
    }
    iTk = start;
    return false;
}
bool exprPostfixPrim(Ret *r){
    Token *start=iTk;
    if(consume(LBRACKET)){
        Ret idx;
        if(expr(&idx)){
            if(consume(RBRACKET)){
                if(r->type.n<0)tkerr(iTk,"only an array can be indexed");
                Type tInt={TB_INT,NULL,-1};
                if(!convTo(&idx.type,&tInt))tkerr(iTk,"the index is not convertible to int");
                r->type.n=-1;
                r->lval=true;
                r->ct=false;
                if(exprPostfixPrim(r)){
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
            Token *tkName=consumedTk;
            if(r->type.tb!=TB_STRUCT)tkerr(iTk,"a field can only be selected from a struct");
            Symbol *s=findSymbolInList(r->type.s->structMembers,tkName->text);
            if(!s)tkerr(iTk,"the structure %s does not have a field %s",r->type.s->name,tkName->text);
            *r=(Ret){s->type,true,s->type.n>=0};
            if(exprPostfixPrim(r)){
                return true;
            }
        }
        else tkerr(iTk, "missing identifier after .");
    }
    iTk = start;
    return true;
}

// ID-ul trebuie sa existe in TS
// Doar functiile pot fi apelate
// O functie poate fi doar apelata
// Apelul unei functii trebuie sa aiba acelasi numar de argumente ca si numarul de parametri de la definitia ei
// Tipurile argumentelor de la apelul unei functii trebuie sa fie convertibile la tipurile parametrilor functiei
bool exprPrimary(Ret *r){
    Token *start=iTk;
    if(consume(ID)){
        Token *tkName=consumedTk;
        Symbol *s=findSymbol(tkName->text);
        if(!s)tkerr(iTk,"undefined id: %s",tkName->text);
        if(consume(LPAR)){
            if(s->kind!=SK_FN)tkerr(iTk,"only a function can be called");
            Ret rArg;
            Symbol *param=s->fn.params;
            if(expr(&rArg)){
                if(!param)tkerr(iTk,"too many arguments in function call");
                if(!convTo(&rArg.type,&param->type))tkerr(iTk,"in call, cannot convert the argument type to the parameter type");
                param=param->next;
                while(consume(COMMA)){
                    if(expr(&rArg)){
                        if(!param)tkerr(iTk,"too many arguments in function call");
                        if(!convTo(&rArg.type,&param->type))tkerr(iTk,"in call, cannot convert the argument type to the parameter type");
                        param=param->next;
                    }
                    else{
                        tkerr(iTk, "missing expression after ,");
                        //iTk=start;
                        //return false;
                    }
                }
            }
            if(consume(RPAR)){
                if(param)tkerr(iTk,"too few arguments in function call");
                *r=(Ret){s->type,false,true};
                return true;
            }
            else tkerr(iTk, "missing )");
            //iTk=start;
            //return false;
        }
        if(s->kind==SK_FN)tkerr(iTk,"a function can only be called");
        *r=(Ret){s->type,true,s->type.n>=0};
        return true;
    }
    iTk=start;
    if(consume(CT_INT)){
        *r=(Ret){{TB_INT,NULL,-1},false,true};
        return true;
    }
    if(consume(CT_REAL)){
        *r=(Ret){{TB_DOUBLE,NULL,-1},false,true};
        return true;
    }
    if(consume(CT_CHAR)){
        *r=(Ret){{TB_CHAR,NULL,-1},false,true};
        return true;
    }
    if(consume(CT_STRING)){
        *r=(Ret){{TB_CHAR,NULL,0},false,true};
        return true;
    }
    if(consume(LPAR)){
        if(expr(r)){
            if(consume(RPAR)){
                return true;
            }
            else tkerr(iTk, "missing )");
        }
        //else tkerr(iTk, "missing expression after (");
    }
    iTk = start;
    return false;
}

int main()
{
    FILE *fis;

    fis=fopen("4.txt", "rb");
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
    pushDomain();
    mvInit();
    iTk = tokens;
    bool start = unit();
    printf("%d\n", start);

    //showDomain(symTable,"global");
    genTestProgram2();
    run();
    dropDomain();
    EliberareMemorie();
    return 0;
}
