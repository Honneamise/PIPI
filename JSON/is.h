#ifndef _IS_H
#define _IS_H

#define UNIX 	0
#define MAC 	1
#define WIN 	2

typedef struct Is
{
	char *input;
	int size;

	int pos;
	int line;
	int col;

	int plat;
	int tab;
	
}Is;

/*********/
Is IsInit(char *input, int input_size, int plat, int tab);

char IsNext(Is *is);

char IsPeek(Is *is);

int IsAccept(Is *is, char c);

int IsExpect(Is *is, char c);

void IsSkipBlanks(Is *is);

int IsEnd(Is *is);

#ifdef IS

/**********/
Is IsInit(char *input, int input_size, int plat, int tab)
{
	Is is= {0};

	is.input = input;
	is.size  = input_size;

	is.pos  = 0;
	is.line = 1;
	is.col  = 0;

	is.plat = plat;
	is.tab  = tab;

	return is;
}

/**********/
char IsNext(Is *is)//caller must check !IsEnd()
{	
	char ch = is->input[is->pos];

	if(is->plat==UNIX && is->input[is->pos]=='\n')
	{
		is->line++;
		is->col=0;
	}
	else if(is->plat==MAC && is->input[is->pos]=='\r')
	{
		is->line++;
		is->col=0;
	}
	else if(is->plat==WIN && is->input[is->pos]=='\n' && is->input[is->pos-1]=='\r')
	{
			is->line++;
			is->col=0;
	}
	else if(ch=='\t')
	{
		is->col += is->tab;
	}
	else
	{
		is->col++;
	}

	is->pos++;

	return ch;
}

/**********/
char IsPeek(Is *is)
{
	return is->input[is->pos];
}

/**********/
int IsAccept(Is *is, char c)
{
	if(IsEnd(is)){ return 0; }

	if(IsPeek(is) == c) 
	{
		IsNext(is);
        return 1;
    }
    return 0;
}

/**********/
int IsExpect(Is *is, char c) 
{
	if(IsEnd(is)){ return 0; }

    if(IsAccept(is,c)) { return 1; }
    
    return 0;//error
}

/**********/
void IsSkipBlanks(Is *is)
{
	if(IsEnd(is)){ return; }

	while(
		!IsEnd(is) &&
		(
		IsAccept(is,' ') || 
		IsAccept(is,'\f') || 
		IsAccept(is,'\n') || 
		IsAccept(is,'\r') || 
		IsAccept(is,'\t') ||
		IsAccept(is,'\v') 
		))
		{ };
}

/**********/
int IsEnd(Is *is)
{
	return (is->pos == is->size);
}

#endif

#endif