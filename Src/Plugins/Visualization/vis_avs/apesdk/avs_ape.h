// AVS APE (Plug-in Effect) header

// base class to derive from
class C_RBASE {
	public:
		C_RBASE() { }
		virtual ~C_RBASE() { };
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)=0; // returns 1 if fbout has dest, 0 if framebuffer has dest
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent){return 0;};
		virtual char *get_desc()=0;
		virtual void load_config(unsigned char *data, int len) { }
		virtual int  save_config(unsigned char *data) { return 0; }
};

// if you want to support SMP rendering, you can derive from this class instead, 
// and expose the creator via: _AVS_APE_RetrFuncEXT2
// (in general, exposing both for compatibility is a good idea)
class C_RBASE2 : public C_RBASE {
	public:
		C_RBASE2() { }
		virtual ~C_RBASE2() { };

    int getRenderVer2() { return 2; }


    virtual int smp_getflags() { return 0; } // return 1 to enable smp support

    // returns # of threads you desire, <= max_threads, or 0 to not do anything
    // default should return max_threads if you are flexible
    virtual int smp_begin(int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) { return 0; }  
    virtual void smp_render(int this_thread, int max_threads, char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) { }; 
    virtual int smp_finish(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) { return 0; }; // return value is that of render() for fbstuff etc

};

// lovely helper functions for blending
static unsigned int __inline BLEND(unsigned int a, unsigned int b)
{
	register unsigned int r,t;
	r=(a&255)+((b)&255);
	t=min(r,255);
	r=((a>>8)&255)+((b>>8)&255);
	t|=min(r,255)<<8;
	r=((a>>16)&255)+((b>>16)&255);
	return t|min(r,255)<<16;
}

static unsigned int __inline BLEND_AVG(unsigned int a, unsigned int b)
{
	return ((a>>1)&~((1<<7)|(1<<15)|(1<<23)))+((b>>1)&~((1<<7)|(1<<15)|(1<<23)));
}

//extended APE stuff

// to use this, you should have:
// APEinfo *g_extinfo;
// void __declspec(dllexport) AVS_APE_SetExtInfo(HINSTANCE hDllInstance, APEinfo *ptr)
// {
//    g_extinfo = ptr;
// }

typedef void *VM_CONTEXT;
typedef void *VM_CODEHANDLE;
typedef struct
{
  int ver; // ver=1 to start
  double *global_registers; // 100 of these

  // lineblendmode: 0xbbccdd
  // bb is line width (minimum 1)
  // dd is blend mode:
     // 0=replace
     // 1=add
     // 2=max
     // 3=avg
     // 4=subtractive (1-2)
     // 5=subtractive (2-1)
     // 6=multiplicative
     // 7=adjustable (cc=blend ratio)
     // 8=xor
     // 9=minimum
  int *lineblendmode; 

  //evallib interface
  VM_CONTEXT (*allocVM)(); // return a handle
  void (*freeVM)(VM_CONTEXT); // free when done with a VM and ALL of its code have been freed, as well

  // you should only use these when no code handles are around (i.e. it's okay to use these before
  // compiling code, or right before you are going to recompile your code. 
  void (*resetVM)(VM_CONTEXT);
  double * (*regVMvariable)(VM_CONTEXT, char *name);

  // compile code to a handle
  VM_CODEHANDLE (*compileVMcode)(VM_CONTEXT, char *code);

  // execute code from a handle
  void (*executeCode)(VM_CODEHANDLE, char visdata[2][2][576]);

  // free a code block
  void (*freeCode)(VM_CODEHANDLE);

  // requires ver >= 2
  void (*doscripthelp)(HWND hwndDlg,char *mytext); // mytext can be NULL for no custom page

  /// requires ver >= 3
  void *(*getNbuffer)(int w, int h, int n, int do_alloc); // do_alloc should be 0 if you dont want it to allocate if empty
                                                          // w and h should be the current width and height
                                                          // n should be 0-7

} APEinfo;