%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

  typedef struct _item{
    char *val;
    int l;
    int number;
    struct _item *next;
  } item;

  static int print_display(char*,char*);
  static int print_goto(char*);
  static int item_add(char*);
  static int label_add(char*);
  static int search_label(char*);
  static int print_all();
  static int push_lp(int);
  static int pop_lp();
  static int lnumber = 1;
  static int mail_l = -1;
  static int schedule_l = -1;
  int lines = 1;
#ifdef DEBUG
  static int i = 1;
#endif
  static item *item_head,*item_tail,*labels,*lp_stack,*loop_l;
  char tmp[BUFSIZ];
  FILE *out;

%}

%union {
  char *val;
}

%token <val> LP DP GT ML SC LABEL NUMBER
%start all
%%

all: statements events { print_all(); }
;

statements:
          | statement statements  { print_all(); }
          | '{' statements '}'  { print_all(); }
;

statement: DISPLAY ';'
         | GOTO ';'
         | LABEL ':' { label_add($1); }
         | loop_event
;

m_event:
       | MAIL statement { sprintf(tmp,"GOTO %d",mail_l);item_add(tmp); }
       | MAIL '{' statements '}'  { sprintf(tmp,"GOTO %d",mail_l);item_add(tmp); }
;

s_event:
       | SCHEDULE statement { sprintf(tmp,"GOTO %d",schedule_l);item_add(tmp); }
       | SCHEDULE '{' statements '}'  { sprintf(tmp,"GOTO %d",schedule_l);item_add(tmp); }
;

events: m_event s_event
      | s_event m_event 
;

loop_event:
       | LOOP statement { pop_lp(loop_l);sprintf(tmp,"GOTO %d %d",loop_l->l,loop_l->number);item_add(tmp); }
       | LOOP '{' statements '}'  { pop_lp(loop_l);sprintf(tmp,"GOTO %d %d",loop_l->l,loop_l->number);item_add(tmp); } 
;
       

DISPLAY: DP '(' LABEL ',' NUMBER ')' { print_display($3,$5); }
       | DP '(' LABEL ')' { print_display($3,"-1"); }
;

GOTO: GT LABEL  { print_goto($2); }
;

LOOP: LP '(' NUMBER ')' { push_lp(atoi($3)); }

MAIL: ML { item_add("GOTO 1\nMAIL"); lnumber++;mail_l = lnumber;}
;

SCHEDULE: SC { item_add("GOTO 1\nSCHEDULE"); lnumber++;schedule_l = lnumber;}
;


%%

int yyerror(char *s)
{
  fprintf(stderr,"%s in %d\n",s,lines);
  return 0;
}
  
extern FILE *yyin;

int main(int argc,char** argv)
{

  if(argc >1){
    if((yyin = fopen(argv[1],"r")) == NULL){
      fprintf(stderr,"can't open input file:%s\n",argv[1]);
      return 1;
    }

    if(argc > 2){
      if((out = fopen(argv[2],"w")) == NULL){
	fprintf(stderr,"can't open output file:%s\n",argv[2]);
	return 1;
      }
    } else {
      out = stdout;
    }

  } else {
    fprintf(stderr,"usage: %s input_file [output file] \n",argv[0]);
    return 1;
  }

  item_head = item_tail = labels = lp_stack = NULL;

  loop_l = malloc(sizeof(item));

  fprintf(out,"XHisho Animation File\n");
  while(!feof(yyin)){
    yyparse();
  }
  fclose(yyin);
  return 0;
}

static int print_display(char* label,char* number)
{
  sprintf(tmp,"%s %s",label,number);
  item_add(tmp);
  return;
}

static int print_goto(char* label)
{
  int l;
  if((l = search_label(label)) == 0){
    fprintf(stderr,"not defined label:%s\n",label);
    exit(1);
  }
  sprintf(tmp,"GOTO %d",l);
  item_add(tmp);
  return;
}

static int item_add(char* value)
{
  item *tmp;

  lnumber++;
  tmp = (item*)malloc(sizeof(item));
  tmp->val = malloc(strlen(value) + 1);
  tmp->next = NULL;
  strcpy(tmp->val,value);

  if(!item_head){
    item_head = item_tail = tmp;
  } else {
    item_tail->next = tmp;
    item_tail = tmp;
  }

  return 0;
}

static int print_all()
{
  item *it_p,*tmp;

  for(it_p = item_head;it_p;){
#ifdef DEBUG
    fprintf(out,"%d:",i);
    i++;
#endif
    fprintf(out,"%s\n",it_p->val);
    tmp = it_p->next;
    free(it_p->val);
    free(it_p);
    it_p = tmp;
  }
  
  item_head = item_tail = NULL;
  return 0;
}

static int label_add(char* label)
{
  item *tmp,*l_p;

  for(l_p = labels;l_p;){
    if(!strcmp(label,l_p->val)){
      fprintf(stderr,"duplicate label %s\n",label);
      exit(1);
    }
    l_p = l_p->next;
  }

  tmp = (item*)malloc(sizeof(item));
  tmp->val = malloc(strlen(label) + 2);
  tmp->next = NULL;
  strcpy(tmp->val,label);
  tmp->l = lnumber;

  tmp->next = labels;
  labels = tmp;
  return 0;
}

static int search_label(char* label)
{
  item *l_p;

  for(l_p = labels;l_p;){
    if(!strcmp(label,l_p->val)) return l_p->l;
    l_p = l_p->next;
  }
  return 0;
}

static int push_lp(int n)
{
  item *tmp;

  tmp = (item*)malloc(sizeof(item));
  tmp->l = lnumber;
  tmp->number = n;

  tmp->next = lp_stack;
  lp_stack = tmp;
  return 0;
}

static int pop_lp(item *rt)
{
  item *tmp;
  if(!lp_stack){
    fprintf(stderr,"LOOP ERROR\n");
    exit(1);
  }

  rt->l = lp_stack->l;
  rt->number = lp_stack->number;

  tmp = lp_stack->next;
  free(lp_stack);
  lp_stack = tmp;
  return 0;
}

