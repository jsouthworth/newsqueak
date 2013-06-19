#include "store.h"
#include "node.h"
#include "nodenames.h"
#include "typenames.h"
#include "symbol.h"
#include "ydefs.h"
#include <u.h>
#include <lib9.h>
#include "errjmp.h"
#include <stdarg.h>

#include "fns.h"

extern	int	eflag;

void
lerror(Node *n, char *s, ...)
{
	va_list va;
	char buf[4096];

	va_start(va, s);
	vseprint(buf, buf+sizeof buf, s, va);
	va_end(va);
	fprint(2, "%Z%s\n", n->line, buf);
	executeinhibit();
	if(eflag)
		abort();
	errflush();
	errjmp();
}

void
error(char *s, ...)
{
	va_list va;
	char buf[4096];

	va_start(va, s);
	vseprint(buf, buf+sizeof buf, s, va);
	va_end(va);
	fprint(2, "%z%s\n", buf);
	executeinhibit();
	if(eflag)
		abort();
	errflush();
	errjmp();
}

void
rerror(char *s, ...)
{
	va_list va;
	char buf[4096];

	va_start(va, s);
	vseprint(buf, buf+sizeof buf, s, va);
	va_end(va);
	fprint(2, "%z%s\n", buf);
	executeinhibit();
	processes(0);
	if(eflag)
		abort();
	errflush();
	errjmp();
}

void
warn(char *s, ...)
{
	va_list va;
	char buf[4096];

	va_start(va, s);
	vseprint(buf, buf+sizeof buf, s, va);
	va_end(va);
	fprint(2, "warning:%z%s\n", buf);
	executeinhibit();
	errflush();
	errjmp();
}

void
panic(char *s, ...)
{
	va_list va;
	char buf[4096];

	va_start(va, s);
	vseprint(buf, buf+sizeof buf, s, va);
	va_end(va);
	fprint(2, "internal error:%z%s\n", buf);
	abort();
}

void
rpanic(char *s, ...)
{
	va_list va;
	char buf[4096];

	processes(0);
	va_start(va, s);
	vseprint(buf, buf+sizeof buf, s, va);
	va_end(va);
	fprint(2, "internal error:%z%s\n", buf);
	abort();
}

int
bconv(Fmt *f)
{
	int o;
	char buf[4096];
	int printcol = 0;
	o = va_arg(f->args, int);
	while(printcol < o) {
		sprint(buf, "  ");
		printcol += 2;
	}
	return fmtprint(f, buf);
}

int
nconv(Fmt *f)
{
	Node *n;
	char buf[4096];
	int t;
	n = va_arg(f->args, Node*);
	t = n->t;
	if(t<0 || sizeof(Ntypename)/sizeof(Ntypename[0])<=t){
		sprint(buf, "mystery node(%d)", t);
	}else
		sprint(buf, Ntypename[t]);
	return fmtprint(f, buf);
}

int
tconv(Fmt *f)
{
	int t;
	Node *n;
	char buf[4096];
	n = va_arg(f->args, Node*);
	t = n->o.t;
	if(t<0 || sizeof(Ttypename)/sizeof(Ttypename[0])<=t){
		sprint(buf, "mystery type(%d)", t);
	}else if(t==TArray)
		sprint(buf, "TArray of %t", n->r);
	else if(t==TChan)
		sprint(buf, "TChan of %t", n->r);
	else
		sprint(buf, Ttypename[t]);
	return fmtprint(f, buf);
}

int
econv(Fmt *f)
{
	char buf[16], *x;
	int t;
	Node *n;
	n = va_arg(f->args, Node*);
	t = n->o.i;
	if(t<128 && strchr("+-*/%|&^~?!><=", t)){
		sprint(buf, "%c", t);
		return fmtprint(f, buf);
	}
	switch(t){
	case GE:
		x=">=";
		break;
	case LE:
		x="<=";
		break;
	case NE:
		x="!=";
		break;
	case EQ:
		x="==";
		break;
	case ANDAND:
		x="&&";
		break;
	case OROR:
		x="||";
		break;
	case CAT:
		x="cat";
		break;
	case DEL:
		x="del";
		break;
	case REF:
		x="ref";
		break;
	case LEN:
		x="len";
		break;
	case UMINUS:
		x="unary -";
		break;
	case RCV:
		x="rcv";
		break;
	case SND:
		x="send";
		break;
	case LSH:
		x="<<";
		break;
	case RSH:
		x=">>";
		break;
	case DEC:
		x="--";
		break;
	case INC:
		x="++";
		break;
	case PRINT:
		x="print";
		break;
	default:
		x="mystery expression";
		break;
	}
	sprint(buf, "%s",  x);
	return fmtprint(f, buf);
}

int
mconv(Fmt *f)
{
	char buf[4096];
	Node *n;
	n = va_arg(f->args, Node*);
	switch(n->t){
	case NID:
		sprint(buf, n->o.s->name);
		break;
	case NArrayref:
		while(n->t==NArrayref)
			n=n->l;
		sprint(buf, "%m[]", n);
		break;
	case NCall:
		sprint(buf, "%m()", n->l);
		break;
	case NExpr:
		if(n->o.i=='=')
			sprint(buf, "(assignment to %m)", n->l);
		else
			sprint(buf, "(%e expr)", n);
		break;
	case NMk:
		sprint(buf, "mk()");
		break;
	case NNum:
		sprint(buf, "%ld", n->o.l);
		break;
	case NProg:
		sprint(buf, "prog(){}");
		break;
	case NString:
		sprint(buf, "\"%s\"", (char *)n->o.st->data);
		break;
	case NStructref:
		sprint(buf, "%m.%s", n->l, n->r->t==NID? n->r->o.s->name : "tag");
		break;
	case NUnit:
		sprint(buf, "unit");
		break;
	default:
		sprint(buf, "(expression)");
		break;
	}
	return fmtprint(f, buf);
}
