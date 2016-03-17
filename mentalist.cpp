/* =================================================================== 
 *  @AUTHOR: MOHAMMAD ALAM
 *  @EMAIL: mohammad.alam@utsa.edu
 * =================================================================== */

/*
 * TODO: ignore LOCK variables
 */

#include "pin.H"
#include "LM.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <utility>
#include <ext/hash_set>
#include "read_symbols.h"
#include <map>
#include <list>
#include <vector>
#include <sys/time.h>
#define MALLOC "malloc"
#define CALLOC "calloc"
#define REALLOC "realloc"
#define FREE "free"

#define WRITE_HISTORY_LEN 2
#define READ_HISTORY_LEN 4

#define HIDDEN_LAYER_SIZE 10
#define MAX_TRAINING_RUNS 10

using namespace std;
//using __gnu_cxx::hash_set;
//using __gnu_cxx::hash;




/* =================================================================== */
// Constants
/* =================================================================== */
static const addr_type INVALID_ADDR = 0;

UINT64 start_time, end_time;

// Output file name buffer size.
const UINT16 BUF_SIZE = 512;
// Maximum number of modules this tool can handle
const UINT16 MAX_MODULES = 32;

const CHAR *excluded_lib_names[] = { "lib64",
                                     "libpthread",
                                     "libstc++",
                                     "libm",
                                     "libgcc_s",
                                     "libc",
                                     "ld_linux",
                                     "ld-linux",
                                     "librt", "libdl", "libz", "libcrypt", "libnsl", "libfreebl3", "libnss"                                                                        };
const UINT16 n_excluded_lib_names = 15;                                   
/* =================================================================== */
// Global Variables 
/* =================================================================== */

// There is one separate output file of each thread. The rest are one for the 
// whole program
// out_file - actual trace file
// conf_file - configuration file
// sync_file - synchronization variables file
// malloc_file - malloced addresses file
// free_file - freed addresses file
// symbol_file - file for global symbols
// others_file - file containing total dynamic instruction count 
//               and possibly other things
// modules_file - file for module id, address range & name
// loop_file - for loop information

fstream out_file,conf_file;
// malloc_file, free_file, symbol_file, 
//        others_file, modules_file;
PIN_LOCK malloc_lock, free_lock;
PIN_LOCK ins_lock;

LearningModule *ann;
int exec_mode;
INT32 total_threads;
INT32 no_of_training_runs;
INT32 training_runs[MAX_TRAINING_RUNS];

typedef UINT32 MEM_ACCESS_STAT;
volatile MEM_ACCESS_STAT mem_access_counter = 1;
//MEM_ACCESS_STAT mem_access_counter;
BOOL *is_write;
BOOL  *is_read;
BOOL *has_read2;
ADDRINT *waddr;
UINT32 *wlen;
ADDRINT *raddr;
ADDRINT *raddr2;
UINT32 *rlen;

// Initial value is 1 for main thread
volatile INT16 active_threads = 1;
volatile INT16 start_profile = 0;
//volatile INT16 lib_module_start = 0;


// This variables are for reading symbols from binary
//static	char *fname = NULL;
//static tp_node *nodes = NULL;
//static unsigned int n_nodes = 0;




typedef struct meta_data
{
        pc_type last_writer_addr; /* last write PC */
        int last_writer_tid;

}MetaData;

/*
 * table to store and access  meta_data
 * mapped with shared accessed mem addresses
 * with at least one read access and write access
 * key = read mem addr, value = Meta Data
 */

map<addr_type, MetaData> info_table; 

/* =================================================================== */
// Commandline Switches 
/* =================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                            "o", "nova.out", 
                            "specify the output file name" );
KNOB<string> KnobConfFile(KNOB_MODE_WRITEONCE, "pintool",
                            "c", "nova.conf", 
                            "specify the file name for configuration ");
KNOB<string> KnobStdinFile(KNOB_MODE_WRITEONCE,  "pintool", "stdin", "", "STDIN file for the application");
KNOB<string> KnobDataDir(KNOB_MODE_WRITEONCE,"pintool","datadir","", "trace data for offline training");
KNOB<string> KnobMlpPath(KNOB_MODE_WRITEONCE,"pintool","mlp","", "path of mlp file");
KNOB<int> KnobMode(KNOB_MODE_WRITEONCE,"pintool","mode", "0","offline only 1,  online only 2, both 0");
/* =================================================================== */

INT32 Usage() {
  cerr <<
      "This tool collects traces to extract atomic sections.\n";

  cerr << KNOB_BASE::StringKnobSummary();

  cerr << endl;

  return -1;
}
/* =================================================================== */
VOID MemInsBefore(BOOL _is_write, BOOL _is_read, BOOL _has_read2,
                   ADDRINT _waddr, UINT32 _wlen,
                   ADDRINT _raddr, ADDRINT _raddr2, UINT32 _rlen,
                   UINT32 thread_id) {

  if (!start_profile) return;
  GetLock(&ins_lock, 1);
  is_write[thread_id] = _is_write;
  is_read[thread_id] = _is_read;
  has_read2[thread_id] = _has_read2;
  waddr[thread_id] = _waddr;
  wlen[thread_id] = _wlen;
  raddr[thread_id] = _raddr;
  raddr2[thread_id] = _raddr2;
  rlen[thread_id] = _rlen;
}

/* =================================================================== */

VOID MemInsAfter(UINT32 thread_id, ADDRINT inst_ptr/*, UINT32 mod, ADDRINT rva*/) {
  
  if (active_threads < 2) return;
  if (!start_profile) return;
  MEM_ACCESS_STAT val = 1;
  __asm__ __volatile__("lock xaddl %3, %0"
                       : "=m"(mem_access_counter), "=r"(val)
                       :  "m"(mem_access_counter), "1"(val)
                      );
  ReleaseLock(&ins_lock);

  if(is_read[thread_id]){
    map<addr_type, MetaData>::iterator it;
    it = info_table.find( raddr[thread_id] );
    if(it != info_table.end()) { /*found in the table: previous write occured */
      if(readInstList[thread_id].size() == READ_HISTORY_LEN)
      {
        assert(writeInstList[thread_id].size() == READ_HISTORY_LEN);
        readInstList[thread_id].pop_front();
        writeInstList[thread_id].pop_front();
      }
      readInstList[thread_id].push_back(inst_ptr);
      writeInstList[thread_id].push_back(it->second.last_writer_addr);
  
      // execute online training or classification of data using ann
      if(readInstList.size() == SLIDING_WIN && writeInstList.size() == SLIDING_WIN)
        ann->execLM(thread_id);
      
    }
  }
  if(has_read2[thread_id]){
    map<addr_type, MetaData>::iterator it;
    it = info_table.find( raddr2[thread_id] );
    if(it != info_table.end()) { /*found in the table: previous write occured */
      if(readInstList[thread_id].size() == READ_HISTORY_LEN)
      {
        assert(writeInstList[thread_id].size() == READ_HISTORY_LEN);
        readInstList[thread_id].pop_front();
        writeInstList[thread_id].pop_front();
      }
      readInstList[thread_id].push_back(inst_ptr);
      writeInstList[thread_id].push_back(it->second.last_writer_addr);

      // execute online training or classification of data using ann
      if(readInstList.size() == SLIDING_WIN && writeInstList.size() == SLIDING_WIN)
        ann->execLM(thread_id);
    }
  }
  if(is_write[thread_id]){
    MetaData temp;
    temp.last_writer_addr = inst_ptr;
    temp.last_writer_tid = thread_id;
    info_table.insert ( pair<addr_type,MetaData>(waddr[thread_id],temp) );

  }
  
 
  
}


/* =================================================================== */

VOID * New_Malloc( CONTEXT *context, AFUNPTR orgFuncptr, size_t size)
{
  VOID *ret;

  PIN_CallApplicationFunction( context, PIN_ThreadId(),
                                CALLINGSTD_DEFAULT, orgFuncptr,
                                PIN_PARG(void *), &ret,
                                PIN_PARG(size_t), size,
                                PIN_PARG_END() );

  //GetLock(&malloc_lock, 1);
  //malloc_file << (ADDRINT)ret << " " << size << endl;
  //ReleaseLock(&malloc_lock);
  return ret;
}

/* =================================================================== */

VOID * New_Calloc( CONTEXT *context, AFUNPTR orgFuncptr, size_t nelem, size_t elesize)
{
  VOID *ret;

  PIN_CallApplicationFunction( context, PIN_ThreadId(),
                                CALLINGSTD_DEFAULT, orgFuncptr,
                                PIN_PARG(void *), &ret,
                                PIN_PARG(size_t), nelem,
                                PIN_PARG(size_t), elesize,
                                PIN_PARG_END() );

  //GetLock(&malloc_lock, 1);
  //malloc_file << (ADDRINT)ret << " " << (nelem * elesize) << endl;
  //ReleaseLock(&malloc_lock);
  return ret;
}

/* =================================================================== */

VOID * New_Realloc( CONTEXT *context, AFUNPTR orgFuncptr, void *ptr, size_t size)
{
  VOID *ret;

  PIN_CallApplicationFunction( context, PIN_ThreadId(),
                                CALLINGSTD_DEFAULT, orgFuncptr,
                                PIN_PARG(void *), &ret,
                                PIN_PARG(void *), ptr,
                                PIN_PARG(size_t), size,
                                PIN_PARG_END() );

  if (size == 0 && ptr != NULL) {
    MEM_ACCESS_STAT val = 1;
    __asm__ __volatile__("lock xaddl %3, %0"
                        : "=m"(mem_access_counter), "=r"(val)
                        :  "m"(mem_access_counter), "1"(val)
                        );
   // GetLock(&free_lock, 1);
   // free_file << val << " " << (ADDRINT)ptr << endl;
   // ReleaseLock(&free_lock);
  } else {
    //GetLock(&malloc_lock, 1);
    //malloc_file << (ADDRINT)ret << " " << size << endl;
    //ReleaseLock(&malloc_lock);
  }
  return ret;
}

/* ===================================================================== */

VOID New_Free( CONTEXT *context, AFUNPTR orgFuncptr, void *ptr)
{
  if (start_profile) GetLock(&ins_lock, 1);
  PIN_CallApplicationFunction( context, PIN_ThreadId(),
                                CALLINGSTD_DEFAULT, orgFuncptr,
                                PIN_PARG(void),
                                PIN_PARG(void *), ptr,
                                PIN_PARG_END() );

  MEM_ACCESS_STAT val = 1;
  __asm__ __volatile__("lock xaddl %3, %0"
                      : "=m"(mem_access_counter), "=r"(val)
                      :  "m"(mem_access_counter), "1"(val)
                      );
  if (start_profile) ReleaseLock(&ins_lock);
  //GetLock(&free_lock, 1);
  //free_file << val << " " << (ADDRINT)ptr << endl;
  //ReleaseLock(&free_lock);
}


/* =================================================================== */
//added for testing

//added for testing end

VOID Image(IMG img, VOID *v) {  
  
  for (UINT16 i = 0; i < n_excluded_lib_names; i++)
    // Is this image suppossed to be excluded?
    if (IMG_Name(img).find(excluded_lib_names[i]) != string::npos)
      return;

  
  // This section iterates over all instructions of the image
  for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
      RTN_Open(rtn);
      for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
        // Avoid instrumenting the instrumentation
        if (!INS_IsOriginal(ins))
          continue;
	if( INS_IsStackRead(ins) || INS_IsStackWrite(ins)) {
          continue;
        }
        if ((INS_IsMemoryWrite(ins) || INS_IsMemoryRead(ins)) && INS_HasFallThrough(ins)) {
          
          if (INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) && 
              INS_HasMemoryRead2(ins)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemInsBefore,
                          IARG_BOOL, true, 
                          IARG_BOOL, true,
                          IARG_BOOL, true,
                          IARG_MEMORYWRITE_EA,
                          IARG_MEMORYWRITE_SIZE,
                          IARG_MEMORYREAD_EA,
                          IARG_MEMORYREAD2_EA,
                          IARG_MEMORYREAD_SIZE,
                          IARG_THREAD_ID,
                          IARG_END);
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)MemInsAfter,
                           IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
          }
          else if (INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) && 
                  !INS_HasMemoryRead2(ins)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemInsBefore,
                          IARG_BOOL, true, 
                          IARG_BOOL, true, 
                          IARG_BOOL, false,
                          IARG_MEMORYWRITE_EA,
                          IARG_MEMORYWRITE_SIZE,
                          IARG_MEMORYREAD_EA,
                          IARG_ADDRINT, 0,
                          IARG_MEMORYREAD_SIZE,
                          IARG_THREAD_ID,
                          IARG_END);
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)MemInsAfter,
                           IARG_THREAD_ID, IARG_INST_PTR,IARG_END);
          }
          else if (INS_IsMemoryWrite(ins) && !INS_IsMemoryRead(ins)) {

            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemInsBefore,
                          IARG_BOOL, true, 
                          IARG_BOOL, false,
                          IARG_BOOL, false,
                          IARG_MEMORYWRITE_EA,
                          IARG_MEMORYWRITE_SIZE,
                          IARG_ADDRINT, 0,
                          IARG_ADDRINT, 0,
                          IARG_UINT32, 0,
                          IARG_THREAD_ID,
                          IARG_END);
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)MemInsAfter,
                           IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
          }
          else if (!INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) && 
                  INS_HasMemoryRead2(ins)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemInsBefore,
                          IARG_BOOL, false, 
                          IARG_BOOL, true,
                          IARG_BOOL, true,
                          IARG_ADDRINT, 0,
                          IARG_UINT32, 0,
                          IARG_MEMORYREAD_EA,
                          IARG_MEMORYREAD2_EA,
                          IARG_MEMORYREAD_SIZE,
                          IARG_THREAD_ID,
                          IARG_END);
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)MemInsAfter,
                           IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
          }
          else if (!INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins) && 
                  !INS_HasMemoryRead2(ins)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)MemInsBefore,
                          IARG_BOOL, false, 
                          IARG_BOOL, true,
                          IARG_BOOL, false,
                          IARG_ADDRINT, 0,
                          IARG_UINT32, 0,
                          IARG_MEMORYREAD_EA,
                          IARG_ADDRINT, 0,
                          IARG_MEMORYREAD_SIZE,
                          IARG_THREAD_ID,
                          IARG_END);
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)MemInsAfter,
                           IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
          }
          else {
            ASSERTX(0);
          }
        } 
      }
      RTN_Close(rtn);

    }
}

/* =================================================================== */


/*
 * get time val in milisecond
 */
static UINT64 get_timestamp(){
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  UINT64 ret = tv.tv_usec;
 
  /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
  ret /= 1000;

  /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
  ret += (tv.tv_sec * 1000);

  return ret;
}

VOID Fini(INT32 code, VOID *v) {
  if(exec_mode!=1){
    end_time =  get_timestamp();
 
    cout << "MENTALIST ELAPSED TIME:: " << (end_time - start_time) << " milliseconds" << endl;
    out_file << "MENTALIST ELAPSED TIME:: " << (end_time - start_time) << " milliseconds" << endl;
  }
  out_file.close();  
  conf_file.close();
//  sync_file.close();
//  malloc_file.close();
//  free_file.close();
//  others_file.close();
//  modules_file.close();

  ann->save();
  delete ann;
  delete[] is_write;
  delete[] is_read;
  delete[] has_read2;
  delete[] waddr;
  delete[] raddr;
  delete[] raddr2;
  delete[] wlen;
  delete[] rlen;
}

/* =================================================================== */

// Format of conf file
//  1. Number of total threads
VOID ReadConfFile() {
  string str;
  getline(conf_file, str);
  stringstream ss(str);
  ss >> total_threads;
  cout << "total threads " << total_threads << endl;
  getline(conf_file, str);
  stringstream ss1(str);
  ss1 >> no_of_training_runs;
  for( INT32 i=0; i<no_of_training_runs; i++)
  {
    getline(conf_file,str);
    stringstream ss_runs(str);
    ss_runs >> training_runs[i];
  }

  
}

/* =================================================================== */

VOID Initialize() {
  
  conf_file.open(KnobConfFile.Value().c_str(), fstream::in);
  ReadConfFile();

  out_file.open( KnobOutputFile.Value().c_str(), fstream::out); 
  

  InitLock(&ins_lock);
  
  is_write = new BOOL[total_threads];
  is_read = new BOOL[total_threads];
  has_read2 = new BOOL[total_threads];
  waddr = new ADDRINT[total_threads];
  wlen = new UINT32[total_threads];
  raddr = new ADDRINT[total_threads];
  raddr2 = new ADDRINT[total_threads];
  rlen = new UINT32[total_threads];
  for (INT32 i = 0; i < total_threads; i++) {
    is_write[i] = is_read[i] = has_read2[i] = false;
    waddr[i] = raddr[i] = raddr2[i] = 0;
    wlen[i] = rlen[i] = 0;
  }
}

/* ================================================================== */
#if 0
VOID WriteSymbols() {
  for (UINT32 i = 0; i < n_nodes; i++) {
    symbol_file << nodes[i].addr << " " << nodes[i].size << " "
                << string(nodes[i].var_name) << endl;
  }
  symbol_file.close();
}
#endif
/* =================================================================== */


VOID ThreadBegin(THREADID thread_id, CONTEXT *ctxt, INT32 flags, VOID *v)
{
  INT16 val = 1;
  __asm__ __volatile__("lock xaddw %3, %0"
                       : "=m"(active_threads), "=r"(val)
                       :  "m"(active_threads), "1"(val)
                      );
	ASSERTX(thread_id < (UINT32)total_threads);
  if (start_profile == 0 && active_threads > 1) start_profile = 1;
  cout << "Thread start " << thread_id << endl;
  list<pc_type> empty_list;
  readInstList.push_back(empty_list);
  writeInstList.push_back(empty_list);
}

/* =================================================================== */

VOID ThreadEnd(THREADID thread_id, const CONTEXT *ctxt, INT32 code, VOID *v)
{
  INT16 val = -1;
  __asm__ __volatile__("lock xaddw %3, %0"
                       : "=m"(active_threads), "=r"(val)
                       :  "m"(active_threads), "1"(val)
                      );
  cout << "Thread end " << thread_id << endl;
}

/* =================================================================== */


/*
 *MAIN FUNC
 */
int main(int argc, char *argv[]) {

  PIN_InitSymbols();
  
  if( PIN_Init(argc,argv) ) {
      return Usage();
  }
  
  Initialize();  
 	
  //fname=rs_get_executable(argc, argv);
  //rs_read_symbol_table(fname, &nodes, &n_nodes);
  //WriteSymbols(); 
  
  string stdinFile = KnobStdinFile.Value();
  if(stdinFile.size() > 0) {
    assert(freopen(stdinFile.c_str(), "rt", stdin));
  }
  //stringstream mode_ss(KnobMode.Value()); 
  //mode_ss >> exec_mode;
  exec_mode = KnobMode.Value();
  cout << "lm mode : " << exec_mode << endl;
  cout << "total threads" << total_threads << endl;
  string mlp_path = KnobMlpPath.Value();
  ann = new LearningModule ( 0 , INPUT_SIZE , HIDDEN_LAYER_SIZE , OUTPUT_SIZE, mlp_path );
  if( exec_mode == 0 || exec_mode == 1){
    if(exec_mode == 1 ) { // only offline training
      start_time = get_timestamp();
    }
    string data_dir = KnobDataDir.Value();
    bool update_weights = false; 
    for( INT32 i=0; i < no_of_training_runs; i++ ) {
      std::cout << "Training with run" << training_runs[i] << std::endl;
      ann->offlineTrain( training_runs[i], total_threads, data_dir, update_weights );
      update_weights = true;
    }
    double validation_error = ann->validation(training_runs[0], total_threads, data_dir); //TODO: separate validation set
    std::cout << "Training : " << no_of_training_runs << " executions" << std::endl;
    std::cout << "Validation Error : " << validation_error * 100  << "%" << std::endl; 

  }
  if( exec_mode != 1 ){ // not only offline training
    ann->load();


  	
  
    PIN_AddThreadStartFunction(ThreadBegin,0);
    PIN_AddThreadFiniFunction(ThreadEnd,0);
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);
   
    start_time = get_timestamp(); 
    // Never returns
    PIN_StartProgram();
  }  
  else {
    end_time =  get_timestamp();
 
    cout << "::ELAPSED TIME:: " << (end_time - start_time) << " milliseconds" << endl;
    out_file << "::ELAPSED TIME:: " << (end_time - start_time) << " milliseconds" << endl;

  }
  return 0;
}

/* ================================================================== */
// eof 
/* ================================================================== */
