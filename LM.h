#ifndef LM_H
#define LM_H
#include "MLP.h"
#include <fstream>
#include <iostream>
#include <list>
using namespace std;
#define PREDICTION_THRESHOLD 0.50
#define ERROR_THRESHOLD 1
#define COUNT_INTERVAL 1000


typedef unsigned long pc_type;
typedef long line_no_type;
typedef unsigned long addr_type;
typedef unsigned long access_count_type;

enum modeLM {
  OFFLINE_TRAIN=0,
  ONLINE_TRAIN,
  ONLINE_TEST
};

extern vector<list<pc_type>>readInstList; //read instructions sliding window
extern vector<list<pc_type>>writeInstList;// for storing corresponding last write inst

class LearningModule {
protected:
  CvANN_MLP *mlp;
  int id; // same as processor number
  int intervalCount; // interval of instructions to check correctness of mlp
  int intervalErrorCount; // error reported within the interval
  unsigned int bugReportCount;   
  char mlp_file_name[100];
  modeLM lm_mode;   
public:
  LearningModule( int proc_id, int input_size, int hidden_size, int output_size, string mlp_dir );
  ~LearningModule();
  void initialize( int parent_id );
  void setMode( modeLM m);
  modeLM getMode() { return lm_mode; }
  void offlineTrain( int exec_no, int total_threads, string data_dir, bool update_weights ); 
  double validation( int exec_no, int total_threads, string data_dir );
  int getId() const { return id; }
  unsigned int getIntervalErrorCount() { return intervalErrorCount;}
  void save();
  void load();
  bool mlpFileExists(); 
  void execLM(int thread_id);
};
#endif
