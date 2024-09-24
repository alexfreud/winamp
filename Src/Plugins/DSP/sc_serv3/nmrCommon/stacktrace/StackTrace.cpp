#ifdef PLATFORM_LINUX
#if !defined(__APPLE__) && !defined(_GNU_SOURCE)
#  define _GNU_SOURCE // enable dladdr and getline
#endif

#include "StackTrace.h"
#include <unistd.h>

#if defined(__APPLE__) // need atos
#  if defined(STACKTRACE_USE_BACKTRACE)
#    include <execinfo.h> // to record the backtrace addresses
#  endif
// for display:
#  include </usr/include/util.h> // forkpty to make a pseudo-terminal for atos
#  include <termios.h> // set pseudo-terminal to raw mode
#  include <sys/select.h> // test whether atos has responded yet

#elif defined(STACKTRACE_USE_BACKTRACE)
#  include <execinfo.h> // to record the backtrace addresses
// for display:
#  include <dlfcn.h> // dladdr()
#  include <sys/param.h> // realpath()
#  include <errno.h>
#  if defined(__GNUC__) && defined(__cplusplus)
#    include <cxxabi.h> // demangling
#    include <string>
#  endif
#  if defined(__linux__)
#    include <sys/stat.h>
#  endif
#endif

#ifdef __cplusplus
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#else
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#endif

#ifdef ST_UNUSED 
#elif defined(__GNUC__) && __GNUC__>3
//! portable access to compiler hint not to warn if a function argument is ignored (goes in argument list)
# define ST_UNUSED(x) UNUSED_##x __attribute__((unused)) 
//! portable access to compiler hint not to warn if a function argument is ignored (goes at beginning of function body)
# define ST_BODY_UNUSED(x) /*no body necessary*/

#elif defined(__LCLINT__) 
//! portable access to compiler hint not to warn if a function argument is ignored (goes in argument list)
# define ST_UNUSED(x) /*@unused@*/ x 
//! portable access to compiler hint not to warn if a function argument is ignored (goes at beginning of function body)
# define ST_BODY_UNUSED(x) /*no body necessary*/

#else 
//! portable access to compiler hint not to warn if a function argument is ignored (goes in argument list)
# define ST_UNUSED(x) UNUSED_##x 
//! portable access to compiler hint not to warn if a function argument is ignored (goes at beginning of function body)
# define ST_BODY_UNUSED(x) (void)UNUSED_##x /* ugly hack to avoid warning */
#endif

#ifdef __cplusplus
namespace stacktrace {
#endif /* __cplusplus */

int unrollStackFrame(struct StackFrame* curFrame, struct StackFrame* nextFrame) {
	if(curFrame==NULL)
		return 0;
	curFrame->caller=NULL;
	if(nextFrame==NULL)
		return 0;
	

#ifdef STACKTRACE_USE_BACKTRACE
	
	if(curFrame->packedRA==NULL)
		return 0; // don't have current frame
	if(*curFrame->packedRAUsed<=curFrame->depth+1) {
		// last element
		if(curFrame!=nextFrame) {
			nextFrame->packedRA = NULL;
			nextFrame->packedRAUsed = nextFrame->packedRACap = NULL;
		}
	return 0;
	}
	if(curFrame!=nextFrame) {
		nextFrame->packedRA = curFrame->packedRA;
		nextFrame->packedRAUsed = curFrame->packedRAUsed;
		nextFrame->packedRACap = curFrame->packedRACap;
		nextFrame->depth = curFrame->depth; // will be incremented below
	}
	nextFrame->ra = (*nextFrame->packedRA)[++(nextFrame->depth)];
	curFrame->caller=nextFrame;
	return 1;
	
#else

	void* nsp=NULL;
	machineInstruction * nra=NULL;

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
	if(curFrame->sp==NULL)
		return 0;
	if(((void**)curFrame->sp)-1==NULL)
		return 0;
	nsp=((void***)curFrame->sp)[-1];
	if(nsp==NULL)
		return 0;
	nsp=(void**)nsp+1; //move from frame pointer to stack pointer of previous frame
	nra=*((machineInstruction**)curFrame->sp);
	if(nsp<=curFrame->sp) {
		fprintf(stderr,"stacktrace::unrollStackFrame(sp=%p,ra=%p) directed to invalid next frame: (sp=%p,ra=%p)\n",curFrame->sp,curFrame->ra,nsp,nra);
		return 0;
	}
#  ifdef DEBUG_STACKTRACE
	if(curFrame->debug)
		printf("( %p %p ) -> { %p %p }\n",curFrame->sp,curFrame->ra,nsp,nra);
	nextFrame->debug=curFrame->debug;
#  endif
	nextFrame->sp=nsp;
	nextFrame->ra=nra;
	curFrame->caller=nextFrame;
	return 1;
#endif
#ifdef __POWERPC__
	if(curFrame->sp==NULL)
		return 0;
	if(*(void**)curFrame->sp==NULL)
		return 0;
	nsp=*(void**)curFrame->sp;
	nra=((machineInstruction**)nsp)[2];
	if(nsp<=curFrame->sp) {
		fprintf(stderr,"stacktrace::unrollStackFrame(sp=%p,ra=%p) directed to invalid next frame: (sp=%p,ra=%p)\n",curFrame->sp,curFrame->ra,nsp,nra);
		return 0;
	}
#  ifdef DEBUG_STACKTRACE
	if(curFrame->debug)
		printf("( %p %p ) -> { %p %p }\n",curFrame->sp,curFrame->ra,nsp,nra);
	nextFrame->debug=curFrame->debug;
#  endif
	nextFrame->sp=nsp;
	nextFrame->ra=nra;
	curFrame->caller=nextFrame;
	return 1;
#endif
#if defined(__MIPSEL__) || defined(__MIPS__) /* we're running on PLATFORM_APERIOS */
	if(curFrame->sp==NULL)
		return 0;
	/* Have to scan through intructions being executed because stack pointer is not stored directly on the stack */
	machineInstruction * ins;
	const machineInstruction * INS_BASE=(const machineInstruction *)0x2000; // lowest valid memory address?
	
#ifdef __PIC__
	ins = reinterpret_cast<machineInstruction*>(curFrame->gp-curFrame->ra);
#else
	ins = curFrame->ra;
#endif
	// find previous return address
	for(; ins>=INS_BASE; ins--) {
		// gcc will always save the return address with the instruction
		//     sw ra, offset(sp)
		// 
		// the high word in this case is sw sp ra
		if ( ( *ins & 0xffff0000 ) == 0xafbf0000 )
		{
			// the low word is the offset from sp
			int offset = *ins & 0x000ffff;
			
			// in case things went horribly awry, don't deref the non-aligned ptr
			if (offset & 0x3)
				return 0;
			
			nra = *reinterpret_cast<machineInstruction**>((char*)curFrame->sp + offset);
			break; // now search for stack pointer
		}
		
		//it appears the aperios stub entry functions always begin with "ori  t0,ra,0x0"
		//if we hit one of these, return 0, because we can't unroll any more
		//(or at least, I don't know how it returns from these... there's no return statements!)
		if ( *ins  == 0x37e80000 ) {
#  ifdef DEBUG_STACKTRACE
			if(curFrame->debug)
				printf("( %p %p %p ) -> { kernel? }\n",curFrame->sp,curFrame->ra,curFrame->gp);
#  endif
			return 0;
		}
	}
	// find previous stack pointer
	for(; ins>=INS_BASE; ins--) {
		// gcc will always change the stack frame with the instruction
		//     addiu sp,sp,offset
		//
		// at the beginning of the function the offset will be negative since the stack grows 
		// from high to low addresses
		//
		// first check the high word which will be instruction + regs in this case (I-type)
		if ( ( *ins & 0xffff0000 ) == 0x27bd0000 ) {
			// the offset is in the low word. since we're finding occurrence at the start of the function,
			// it will be negative (increase stack size), so sign extend it
			int offset = ( *ins & 0x0000ffff ) | 0xffff0000;

			// in case things went horribly awry, don't deref the non-aligned ptr
			if (offset & 0x3)
				return 0;
			
			nsp = (char*)curFrame->sp - offset;
			break;
		}
	}
	
	
	if(ins>=INS_BASE) {
		if(nsp<=curFrame->sp) {
#ifdef __PIC__
			fprintf(stderr,"stacktrace::unrollStackFrame(sp=%p,ra=%p,gp=%p) directed to invalid next frame: (sp=%p,ra=%p,gp=%p)\n",curFrame->sp,(void*)curFrame->ra,(void*)curFrame->gp,nsp,nra,(void*)(reinterpret_cast<size_t*>(nsp)[4]));
#else
			fprintf(stderr,"stacktrace::unrollStackFrame(sp=%p,ra=%p) directed to invalid next frame: (sp=%p,ra=%p)\n",curFrame->sp,(void*)curFrame->ra,nsp,nra);
#endif
			return 0;
		}

#ifdef __PIC__
#  ifdef DEBUG_STACKTRACE
		if(curFrame->debug)
			printf("( %p %p %p ) -> { %p %p %p }\n",curFrame->sp,curFrame->ra,curFrame->gp,nsp,nra,reinterpret_cast<size_t*>(nsp)[4]);
		nextFrame->debug=curFrame->debug;
#  endif
		// I'm not actually sure this is a valid stop criteria, but in testing,
		// after this it seems to cross into some kind of kernel code.
		// (We get a really low gp (0x106), although a fairly normal nra, and then go bouncing
		// around in memory until we hit sp=0x80808080, ra=0x2700, which seems to be the 'real' last frame)
		//if(reinterpret_cast<size_t>(nra)>reinterpret_cast<size_t*>(nsp)[4])
		//return 0;
		//instead of this however, now we check for the ori t0,ra,0 statement, and reuse previous gp below
		
		nextFrame->sp=nsp;
		//not sure how valid this is either:
		if(reinterpret_cast<size_t>(nra)>reinterpret_cast<size_t*>(nsp)[4]) {
			nextFrame->gp = curFrame->gp;
		} else {
			nextFrame->gp = reinterpret_cast<size_t*>(nsp)[4]; // gp is stored 4 words from stack pointer
		}
		nextFrame->ra = nextFrame->gp-reinterpret_cast<size_t>(nra);
#else
#  ifdef DEBUG_STACKTRACE
		if(curFrame->debug)
			printf("( %p %p ) -> { %p %p }\n",curFrame->sp,curFrame->ra,nsp,nra);
		nextFrame->debug=curFrame->debug;
#  endif
		nextFrame->sp=nsp;
		nextFrame->ra=nra;
#endif /* __PIC__ */
		curFrame->caller=nextFrame;
		return 1;
	}
#ifdef __PIC__
#  ifdef DEBUG_STACKTRACE
	if(curFrame->debug)
		printf("( %p %p %p ) -> { %p %p --- }\n",curFrame->sp,curFrame->ra,curFrame->gp,nsp,nra);
#  endif
#else
#  ifdef DEBUG_STACKTRACE
	if(curFrame->debug)
		printf("( %p %p ) -> { %p %p }\n",curFrame->sp,curFrame->ra,nsp,nra);
#  endif
#endif
	return 0;
#endif
#endif /* backtrace */
}

#ifdef STACKTRACE_USE_BACKTRACE
static int growAlloc(struct StackFrame* frame) {
	void** r = (void**)realloc(*frame->packedRA, *frame->packedRACap * sizeof(void*) * 2);
	if(r==NULL)
		return 0;
	*frame->packedRACap *= 2;
	*frame->packedRA = r;
	return 1;
}
#endif

#ifdef STACKTRACE_USE_BACKTRACE
const size_t MIN_CAP=50;
static void allocBacktraceRegion(struct StackFrame* frame, size_t /*cap*/) {
	/*if(cap < MIN_CAP)
		cap=MIN_CAP;*/
	frame->packedRACap = (size_t*)malloc(sizeof(size_t));
	*frame->packedRACap = MIN_CAP;
	frame->packedRAUsed = (size_t*)malloc(sizeof(size_t));
	*frame->packedRAUsed = 0;
	frame->packedRA = (void***)malloc(sizeof(void**));
	*frame->packedRA = (void**)malloc(*frame->packedRACap * sizeof(void*));
	//printf("Allocated %p\n",*frame->packedRA);
}
	
static void freeBacktraceRegion(struct StackFrame* frame) {
	//printf("Freeing %p\n",*frame->packedRA);
	free(frame->packedRACap);
	frame->packedRACap = NULL;
	free(frame->packedRAUsed);
	frame->packedRAUsed = NULL;
	free(*frame->packedRA);
	*frame->packedRA=NULL;
	free(frame->packedRA);
	frame->packedRA=NULL;
}
#endif

void getCurrentStackFrame(struct StackFrame* frame) {
#ifdef STACKTRACE_USE_BACKTRACE

	// first call?  allocate the storage area
	if(frame->packedRA==NULL)
		allocBacktraceRegion(frame,MIN_CAP);
	
	// call backtrace, if we hit the capacity, grow the buffer and try again
	do {
		*frame->packedRAUsed = backtrace(*frame->packedRA, *frame->packedRACap);
	} while(*frame->packedRAUsed==*frame->packedRACap && growAlloc(frame));
	
	// if we used an oversized buffer, shrink it back down
	if(*frame->packedRACap > *frame->packedRAUsed*2 && *frame->packedRACap>MIN_CAP) {
		unsigned int newsize = *frame->packedRAUsed * 3 / 2;
		if(newsize < MIN_CAP)
			newsize = MIN_CAP;
		void** r = (void**)realloc(*frame->packedRA, newsize * sizeof(void*) * 2);
		if(r!=NULL) {
			*frame->packedRACap = newsize;
			*frame->packedRA = r;
		}
	}
	frame->depth=1;
	
#else
	
	void** csp=NULL;
	machineInstruction* cra=NULL;

#ifdef __POWERPC__
	__asm __volatile__ ("mr %0,r1" : "=r"(csp) ); // get the current stack pointer
	__asm __volatile__ ("mflr %0" : "=r"(cra) );  // get the current return address
#endif /* __POWERPC__ */
	
#if defined(__MIPSEL__) || defined(__MIPS__)
#ifdef __PIC__
	size_t cgp=0;
	__asm __volatile__ ("move %0,$gp" : "=r"(cgp) ); //get the gp register so we can compute link addresses
#endif /* __PIC__ */
	__asm __volatile__ ("move %0,$sp" : "=r"(csp) ); // get the current stack pointer
	__asm __volatile__ ("jal readepc; nop; readepc: move %0,$ra" : "=r"(cra) ); // get the current return address
#endif /* __MIPSEL__ */
	
#if defined(__i386__)
	__asm __volatile__ ("movl %%ebp,%0" : "=m"(csp) ); // get the caller's stack pointer
	csp++; //go back one to really be a stack pointer
	//__asm __volatile__ ("movl (%%esp),%0" : "=r"(cra) ); // get the caller's address
	cra=*((machineInstruction**)csp);
	csp=((void***)csp)[-1]+1;
#endif /* __i386__ */

// basically the same as i386, but movq instead of movl, and rbp instead of ebp
#if defined(__x86_64__) || defined(__amd64__)
	__asm __volatile__ ("movq %%rbp,%0" : "=m"(csp) ); // get the caller's stack pointer
	csp++; //go back one to really be a stack pointer
	//__asm __volatile__ ("movq (%%rsp),%0" : "=r"(cra) ); // get the caller's address
	cra=*((machineInstruction**)csp);
	csp=((void***)csp)[-1]+1;
#endif /* amd64/x86_64 */

	frame->sp=csp;
#if defined(__PIC__) && (defined(__MIPSEL__) || defined(__MIPS__))
	frame->ra=cgp-reinterpret_cast<size_t>(cra);
	frame->gp=cgp;
#else
	frame->ra=cra;
#endif /* __PIC__ */

#if !defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
	//with ia-32 it was more convenient to directly provide caller, so don't need to unroll
	//otherwise we actually want to return *caller's* frame, so unroll once
	unrollStackFrame(frame,frame);
#endif /* not __i386__ */
#endif /* backtrace */
}

void freeStackTrace(struct StackFrame* frame) {
#ifdef STACKTRACE_USE_BACKTRACE
	if(frame!=NULL && frame->packedRA!=NULL) {
		freeBacktraceRegion(frame);
	}
#endif
	while(frame!=NULL) {
		struct StackFrame * next=frame->caller;
		free(frame);
		if(frame==next)
			return;
		frame=next;
	}
}

struct StackFrame* allocateStackTrace(unsigned int size) {
	struct StackFrame * frame=NULL;
	while(size--!=0) {
		struct StackFrame * prev = (struct StackFrame *)malloc(sizeof(struct StackFrame));
		memset(prev, 0, sizeof(*prev));
#ifdef STACKTRACE_USE_BACKTRACE
		if(frame==NULL) {
			allocBacktraceRegion(prev,size);
		} else {
			prev->packedRA = frame->packedRA;
			prev->packedRAUsed = frame->packedRAUsed;
			prev->packedRACap = frame->packedRACap;
		}
		prev->depth = size-1;
#endif
		prev->caller=frame;
		frame=prev;
	}
	return frame;
}


struct StackFrame * recordStackTrace(unsigned int limit/*=-1U*/, unsigned int skip/*=0*/) {
	if(limit==0)
		return NULL;
	struct StackFrame * cur = allocateStackTrace(1);
#ifdef DEBUG_STACKTRACE
	cur->debug=0;
#endif
	getCurrentStackFrame(cur);
	for(; skip!=0; skip--)
		if(!unrollStackFrame(cur,cur)) {
			freeStackTrace(cur);
			return NULL;
		}
	struct StackFrame * prev = (struct StackFrame *)malloc(sizeof(struct StackFrame));
	memset(prev, 0, sizeof(*prev));
#ifdef DEBUG_STACKTRACE
	prev->debug=0;
#endif
	
	unrollStackFrame(cur,prev); //unroll once more for the current frame
#ifdef STACKTRACE_USE_BACKTRACE
	memset(cur,0,sizeof(*cur)); // clear cur, prev is now responsible for packedRA allocation
#endif
	freeStackTrace(cur);
	cur=prev;
	
	for(--limit; limit!=0; limit--) {
		struct StackFrame * next = (struct StackFrame *)malloc(sizeof(struct StackFrame));
		memset(next, 0, sizeof(*next));
#ifdef DEBUG_STACKTRACE
		next->debug=0;
#endif
		if(!unrollStackFrame(prev,next)) {
			// reached end of trace
			free(next);
			prev->caller=NULL; //denotes end was reached
			return cur;
		}
		prev=next;
	}
	// reaching here implies limit was reached
	prev->caller=prev; //denotes limit was reached
	return cur;
}

struct StackFrame * recordOverStackTrace(struct StackFrame* frame, unsigned int skip) {
	struct StackFrame * cur = allocateStackTrace(1);
#ifdef DEBUG_STACKTRACE
	cur->debug=0;
#endif
	if(frame==NULL)
		return frame;
	getCurrentStackFrame(cur);
	for(; skip!=0; skip--)
		if(!unrollStackFrame(cur,cur)) {
			freeStackTrace(cur);
			return frame;
	}
#ifdef STACKTRACE_USE_BACKTRACE
	if(frame->packedRA!=NULL)
		freeBacktraceRegion(frame);
#endif
	unrollStackFrame(cur,frame); //unroll once more for the current frame
#ifdef STACKTRACE_USE_BACKTRACE
	memset(cur,0,sizeof(*cur)); // clear cur, frame is now responsible for packedRA allocation
#endif
	freeStackTrace(cur);

	for(; frame->caller!=NULL && frame->caller!=frame; frame=frame->caller) {
		struct StackFrame *ans=frame->caller; //don't lose remainder of free list if we hit the end
		if(!unrollStackFrame(frame,frame->caller)) {
			return ans; // reached end of trace
		}
	}
	// reaching here implies limit was reached
	frame->caller=frame; //denotes limit was reached
	return NULL;
}
		
		
#ifdef __APPLE__
// use atos to do symbol lookup, can lookup non-dynamic symbols and also line numbers
/*! This function is more complicated than you'd expect because atos doesn't flush after each line,
 *  so plain pipe() or socketpair() won't work until we close the write side.  But the whole point is
 *  we want to keep atos around so we don't have to reprocess the symbol table over and over.
 *  What we wind up doing is using forkpty() to make a new pseudoterminal for atos to run in,
 *  and thus will use line-buffering for stdout, and then we can get each line. */
static void atosLookup(unsigned int depth, void* ra) {
	static int fd=-1;
	int isfirst=0;
	
	if(fd==-1) {
		struct termios opts;
		cfmakeraw(&opts); // have to set this first, otherwise queries echo until child kicks in
		pid_t child = forkpty(&fd,NULL,&opts,NULL);
		if(child<0) {
			perror("Could not forkpty for atos call");
			return;
		}
		if(child==0) {
			//sleep(3);
			char pidstr[50];
			snprintf(pidstr,50,"%d",getppid());
			execlp("atos","atos","-p",pidstr,(char*)0);
			//snprintf(pidstr,50,"atos -p %d",getppid());
			//execlp("sh","sh","-i","-c",pidstr,(char*)0);
			fprintf(stderr,"Could not exec atos for stack trace!\n");
			_exit(1);
		}
		isfirst=1;
		}

		{
		char q[50];
		size_t qlen = snprintf(q,50,"%p\n",ra);
		//printf("query: %.*s",50,q);
		write(fd,q,qlen);
	}
	
	if(isfirst) {
		// atos can take a while to parse symbol table on first request, which is why we leave it running
		// if we see a delay, explain what's going on...
		int err;
		struct timeval tv = {3,0};
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd,&fds);
		err = select(fd+1,&fds,NULL,NULL,&tv);
		if(err<0)
			perror("select for atos output");
		if(err==0) // timeout
			printf("Generating... first call takes some time for 'atos' to cache the symbol table.\n");
		}

				{
		const unsigned int MAXLINE=1024;
		char line[MAXLINE];
		size_t nread=0;
		char c='x';
		while(c!='\n' && nread<MAXLINE) {
			if(read(fd,&c,1)<=0) {
				fprintf(stderr,"Lost atos connection for stacktrace\n");
				close(fd);
				fd=-1;
				break;
				}
			//printf("Read %d\n",c);
			if(c!='\r')
				line[nread++]=c;
		}
		if(nread<MAXLINE)
			line[nread++]='\0';
		fprintf(stderr,"%4u %.*s",depth,MAXLINE,line);
		}
}
	
#elif defined(STACKTRACE_USE_BACKTRACE)
	
#  if defined(__GNUC__) && defined(__cplusplus)
	static std::string demangle(const std::string& name)
	{
		int status = 0;
		char *d = 0;
		std::string ret = name;
		try { if ((d = abi::__cxa_demangle(name.c_str(), 0, 0, &status))) ret = d; }
		catch(...) {  }
		std::free(d);
		return ret;
	}
#  endif
	
#  ifndef __linux__
// on bsd based systems, we have fgetln() instead of getline()
static int getline(char** s, size_t* len, FILE* f) {
	size_t _len;
	char * _s = fgetln(f,&_len);
	if(_s==NULL)
		return -1;
	if(*len<_len+1) {
		char * ns = (char*)realloc(*s,_len+1);
		if(ns==NULL)
			return -1;
		*s=ns;
	}
	memcpy(*s,_s,_len);
	(*s)[_len]='\0';
	return _len;
}
#  endif

static int addr2lineLookup(const char* const ex, const void* const off, char** srcfile, size_t* srcfilelen, char** func, size_t* funclen) {
	const char * cmdfmt = "addr2line -fe '%s' %p";
	const size_t cmdlen = snprintf(NULL,0,cmdfmt,ex,off)+1;
	char * cmd = (char*)malloc(cmdlen*sizeof(char));
	if(cmd==NULL) {
		fprintf(stderr,"[ERR Could not malloc addr2line command]\n");
		return -1;
	}
	const int cmdused = snprintf(cmd,cmdlen,cmdfmt,ex,off);
	if(cmdused<0) {
		perror("snprintf for addr2line command");
		free(cmd);
		return -1;
	}
	if((size_t)cmdused>=cmdlen) {
		fprintf(stderr, "[ERR addr2line command grew? %d vs %lu]\n",cmdused,(unsigned long)cmdlen);
		free(cmd);
		return -1;
	}
	FILE* look=popen(cmd,"r");
	free(cmd);
	if(look==NULL) {
		fprintf(stderr, "[Missing addr2line]\n");
		return -1;
	}
	if(getline(func,funclen,look)<=0) {
		pclose(look);
		return -1;
	}
	if(getline(srcfile,srcfilelen,look)<=0) {
		pclose(look);
		return -1;
	}
	pclose(look);
	char* nl = strrchr(*func,'\n');
	if(nl!=NULL)
		*nl='\0';
	nl = strrchr(*srcfile,'\n');
	if(nl!=NULL)
		*nl='\0';
	return 0;
}

static void displayRelPath(FILE* os, const char * wd, const char* path) {
	unsigned int same=0,i=0;
	for(i=0; path[i]!='\0'; ++i) {
		if(wd[i]=='/')
			same=i+1;
		else if(wd[i]=='\0') {
			same=i;
			break;
		} else if(wd[i]!=path[i])
			break;
	}
	if(wd[same]=='\0')
		++same;
	else if(same>1) {
		// really want to be relative to source tree root, don't bother with ..'s
		/*for(i=same; wd[i]!='\0'; ++i)
			if(wd[i]=='/')
				fprintf(os,"../");
		fprintf(os,"../");*/
	}
	fprintf(os,"%s",&path[same]);
}
#endif

//! attempts to read symbol information and displays stack trace header
static void beginDisplay() {
#ifdef STACKTRACE_USE_BACKTRACE
	//fprintf(stderr,"Stack Trace:\n");
#elif defined(PLATFORM_APERIOS)
	fprintf(stderr,"Run trace_lookup:");
#elif defined(__APPLE__)
	fprintf(stderr,"backtrace_symbols() unavailable, try 'atos' to make human-readable backtrace (-p %d):",getpid());
#else
	fprintf(stderr,"backtrace_symbols() unavailable, try addr2line or tools/trace_lookup to make human-readable backtrace:");
#endif
}

#ifdef __APPLE__
static void displayStackFrame(unsigned int depth, const struct StackFrame* frame) {
	atosLookup(depth,(void*)frame->ra);
}
#elif defined(STACKTRACE_USE_BACKTRACE)
static void displayStackFrame(unsigned int depth, const struct StackFrame* frame) {
	void* ra = (void*)frame->ra;
	Dl_info sym;
	memset(&sym,0,sizeof(Dl_info));
	int dlres = dladdr(frame->ra, &sym);
	
	int isExe = (sym.dli_fname==NULL); // if lib unknown, assume static linkage, implies executable
#  ifdef __linux__
	// detect if /proc/self/exe points to sym.dli_fname
	if(!isExe && sym.dli_fname[0]!='\0') {
		struct stat exeStat;
		struct stat libStat;
		if(stat("/proc/self/exe",&exeStat)!=0) {
			perror(" stat /proc/self/exe");
		} else if(stat(sym.dli_fname,&libStat)!=0) {
			perror(" stat lib");
		} else {
			isExe = (exeStat.st_dev==libStat.st_dev && exeStat.st_ino==libStat.st_ino);
		}
	}
#  endif

	if(dlres==0 || sym.dli_sname==NULL) {
		if(sym.dli_fname==NULL || sym.dli_fname[0]=='\0') {
			fprintf(stderr,"%4u  [non-dynamic symbol @ %p]",depth,ra);
			fprintf(stderr," (has offset %p in unknown library)\n",sym.dli_fbase);
		} else {
			// use addr2line for static function lookup
			const void* const off = (isExe) ? ra : (void*)((size_t)ra-(size_t)sym.dli_fbase);
			char* srcfile=NULL, *func=NULL;
			size_t srcfilelen=0, funclen=0;
			if(addr2lineLookup(sym.dli_fname,off,&srcfile,&srcfilelen,&func,&funclen)==0) {
				fprintf(stderr,"%4u  %s",depth,(strlen(func)==0) ? "[unknown symbol]" : func);
				if(!isExe) {
					const char * base = strrchr(sym.dli_fname,'/');
					fprintf(stderr," (%s)",(base==NULL)?sym.dli_fname:base+1);
				}
				if(strcmp(srcfile,"??:0")!=0) {
					fprintf(stderr," ");
					char * wd = getcwd(NULL,0);
					if(wd==NULL) {
						perror("getcwd");
						return;
					}
					fprintf(stderr,"(");
					displayRelPath(stderr,wd,srcfile);
					fprintf(stderr,")");
					free(wd);
				}
				fprintf(stderr,"\n");
			}
			free(srcfile);
			free(func);
		}
		return;
	} else {
		const char * dispFmt="%4d  %s +%#lx";
#  ifdef __cplusplus
		fprintf(stderr,dispFmt,depth,demangle(sym.dli_sname).c_str(),(size_t)ra-(size_t)sym.dli_saddr);
#  else
		fprintf(stderr,dispFmt,depth,sym.dli_sname,(size_t)ra-(size_t)sym.dli_saddr);
#  endif
	}

	if(sym.dli_fname==NULL || sym.dli_fname[0]=='\0') {
		fprintf(stderr," (%p, offset %p in unknown lib)\n",ra,sym.dli_fbase);
		return;
	} else if(isExe) {
		// don't bother listing executable name, just show return address
		// ...or since the symbol name and offset imply the return address, just skip it
		//fprintf(stderr," (%p)",ra);
	} else {
		// ra is meaningless in dynamically loaded libraries... address space layout randomization (ASLR)
		// just show library name
		const char * base = strrchr(sym.dli_fname,'/');
		fprintf(stderr," (%s)",(base==NULL)?sym.dli_fname:base+1);
	}
	
	// now do file and line number lookup of function via addr2line
	const void* const off = (isExe) ? ra : (void*)((size_t)ra-(size_t)sym.dli_fbase);
	char* srcfile=NULL, *func=NULL;
	size_t srcfilelen=0, funclen=0;
	if(addr2lineLookup(sym.dli_fname,off,&srcfile,&srcfilelen,&func,&funclen)==0 && strcmp(srcfile,"??:0")!=0) {
		fprintf(stderr," ");
		char * wd = getcwd(NULL,0);
		if(wd==NULL) {
			perror("getcwd");
			return;
		}
		fprintf(stderr,"(");
		displayRelPath(stderr,wd,srcfile);
		fprintf(stderr,")");
		free(wd);
	}
	fprintf(stderr,"\n");
	free(srcfile);
	free(func);
}
#else
static void displayStackFrame(unsigned int ST_UNUSED(depth), const struct StackFrame* frame) {
	ST_BODY_UNUSED(depth);
	fprintf(stderr," %p",(void*)frame->ra);
}
#endif

//! releases symbol information used during display
static void completeDisplay(int isend) {
#if defined(STACKTRACE_USE_BACKTRACE)
#endif
	if(!isend)
		fprintf(stderr," ...\n");
}

void displayCurrentStackTrace(unsigned int limit/*=-1U*/, unsigned int skip/*=0*/) {
	struct StackFrame * cur = allocateStackTrace(1);
#ifdef DEBUG_STACKTRACE
	cur->debug=0;
#endif
	unsigned int i;
	int more;
	if(limit==0)
		return;
	getCurrentStackFrame(cur);
	//printf(" initial (%p\t%p\t%p)\n",cur.ra,cur.sp,*(void**)cur.sp);
	beginDisplay();
	for(; skip!=0; skip--) {
		if(!unrollStackFrame(cur,cur)) {
			completeDisplay(1);
			return;
		}
		//printf(" skip (%p\t%p\t%p)\n",cur.ra,cur.sp,*(void**)cur.sp);
	}
	for(i=0; (more=unrollStackFrame(cur,cur)) && i<limit; i++) {
		//printf(" out (%p\t%p\t%p)\n",cur.ra,cur.sp,*(void**)cur.sp);
		displayStackFrame(i,cur);
	}
	completeDisplay(!more);
	freeStackTrace(cur);
}

void displayStackTrace(const struct StackFrame* frame) {
	int i;
	beginDisplay();
	for(i=0; frame!=NULL && frame->caller!=frame; i++) {
		displayStackFrame(i,frame);
		frame=frame->caller;
	}
	if(frame!=NULL)
		displayStackFrame(i+1,frame);
	completeDisplay(frame==NULL);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

/*! @file
 * @brief Implements functionality for performing stack traces
 * @author ejt (Generalized and implementation for non-MIPS platforms)
 * @author Stuart Scandrett (original inspiration, Aperios/MIPS stack operations)
 *
 * $Author: ejtttje $
 * $Name:  $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Date: 2009/11/20 00:50:23 $
 */
#endif
