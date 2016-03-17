#include "LM.h"
vector<list<pc_type>>readInstList; //read instructions sliding window
vector<list<pc_type>>writeInstList;// for storing corresponding last write inst

LearningModule::LearningModule( int proc_id, int input_size, int hidden_size, int output_size, string mlp_dir ){
  id = proc_id;
  mlp = new CvANN_MLP();
  createANN(mlp, input_size, hidden_size, output_size);
  intervalCount= 0;
  intervalErrorCount = 0; 
  bugReportCount = 0;   
  std::stringstream ss;
  ss<<proc_id;
  string lm_files = mlp_dir + "/mlp" + ss.str() + ".xml"; 
  strcpy(mlp_file_name,lm_files.c_str());
//  sprintf(mlp_file_name,"mlp%d.xml", proc_id);
}

LearningModule::~LearningModule()
{
  save();
  delete mlp;
}

/*
 * create mlp by copying other mlp
 */
void LearningModule::initialize( int parent_tid ){

  if(mlpFileExists()) {
    mlp->load(mlp_file_name, "mlp");     
  }
  else{
    std::cout<< "ERROR:: no saved mlp file found!! " << std::endl;
  }
}


void LearningModule::offlineTrain( int exec_no, int total_threads, string data_dir, bool update_weights ){

  // 1.assemble data
  std::vector<TrainingData> td;
  assembleData(td, exec_no, total_threads, data_dir); //new assemble data  for Remote write data
  
  std:: cout << "Training run" << exec_no << " data size :" << td.size() << std::endl;
  // 2.train mlp
  if( td.size()> 0 ){
     trainMachine( td, mlp, td.size(), update_weights );  
     save();
  }
   
}

double LearningModule:: validation( int exec_no, int total_threads, string data_dir ){
  std::vector<TrainingData> td;
  assembleTestData( td, exec_no, total_threads, data_dir );

  int total_data = td.size();
  int p_error = 0;
  if(td.size() > 0 ){
    p_error = predictMachine( td, mlp); 
    
  }
  return double(p_error)/total_data;
}


#if 0
bool LearningModule::predictValid( VAddr readPC, VAddr writePC ){
   VAddr processed_in[INPUT_SIZE];
   if(insCount >= SLIDING_WIN) {
     for(int i=0; i< INPUT_SIZE-2; i+=2)  {
       processed_in[i] = inputs[INPUT_SIZE-2] - inputs[i];
       processed_in[i+1] = inputs[INPUT_SIZE-1] - inputs[i+1];

     } 

     processed_in[INPUT_SIZE-2] = inputs[INPUT_SIZE-2];
     processed_in[INPUT_SIZE-1] = inputs[INPUT_SIZE-1];
  
 
     float _predout[1];
     CvMat predout = cvMat(1, 1, CV_32FC1, _predout);
     float _sample[INPUT_SIZE];
     CvMat sample = cvMat(1, INPUT_SIZE, CV_32FC1, _sample);
     for( int i=0; i<INPUT_SIZE; i++) {
      sample.data.fl[i]= processed_in[i];
     }
     mlp->predict(&sample, &predout);
     if( predout.data.fl[0] <= PREDICTION_THRESHOLD ) {
       return  false;
     }
     return true;
   }
   return true;
  
}
#endif




void  LearningModule::save(){
  cv::FileStorage fs(mlp_file_name, cv::FileStorage::WRITE);
  mlp->write(*fs, "mlp"); //model name = "mlp" 
}

void LearningModule::load(){
  std::cout << "Loading MLP .." << id << endl;
  mlp->load(mlp_file_name, "mlp");
}

bool LearningModule::mlpFileExists() {
  ifstream f(mlp_file_name);
  if (f.good()) {
    f.close();
    return true;
  } else {
    f.close();
    return false;
  }   
}
/*
 * online training or testing
 */
void LearningModule::execLM (int thread_id) {
  
  intervalCount++; 
  if( lm_mode == ONLINE_TEST && intervalCount > COUNT_INTERVAL && intervalErrorCount > ERROR_THRESHOLD )
  {
     lm_mode = ONLINE_TRAIN;
     intervalCount = 0;
     intervalErrorCount = 0;
  }
  else {
    if( lm_mode == ONLINE_TRAIN && intervalCount > COUNT_INTERVAL ){
      lm_mode=ONLINE_TRAIN;
      intervalCount = 0;
      assert(intervalErrorCount == 0);
    } 
  }
  pc_type readInsPtrs[SLIDING_WIN];
  pc_type writeInsPtrs[SLIDING_WIN];
  pc_type processed_in[INPUT_SIZE];
  //prepare input vector
 
  list<pc_type>::iterator it;
  int i=0;
  for(it=readInstList[thread_id].begin(); it !=readInstList[thread_id].end(); ++it )
  {
    readInsPtrs[i++] = *it;
  }
  for(it=writeInstList[thread_id].begin(); it !=writeInstList[thread_id].end(); ++it )
  {
    writeInsPtrs[i++] = *it;
  }
  for(int index = 0; index < SLIDING_WIN - 1; index++ ) 
  {
    processed_in[2*index] = readInsPtrs[SLIDING_WIN-1] - readInsPtrs[index];

    processed_in[2*index + 1] = writeInsPtrs[SLIDING_WIN-1] - writeInsPtrs[index];
  }
  processed_in[INPUT_SIZE-2] = readInsPtrs[SLIDING_WIN-1];
  processed_in[INPUT_SIZE-1] = writeInsPtrs[SLIDING_WIN-1];
  
  if(lm_mode == ONLINE_TEST ){			  
	  float _predout[1];
	  CvMat predout = cvMat(1, 1, CV_32FC1, _predout);
	  float _sample[INPUT_SIZE];
	  CvMat sample = cvMat(1, INPUT_SIZE, CV_32FC1, _sample);
	  for( int i=0; i<INPUT_SIZE; i++) {
	      sample.data.fl[i]= processed_in[i];
	  }

	  mlp->predict(&sample, &predout);
	  if( predout.data.fl[0] <= PREDICTION_THRESHOLD ) {
	       bugReportCount++;
	       intervalErrorCount++;
	  }
	  return;
 }
 else {
   //TODO:training online with positive examples only??
   //TODO: get rid of this unnecessary vector!
   TrainingData tdsample;
   vector<TrainingData>td;
   for(int i=0; i<INPUT_SIZE; i++)
     tdsample.input[i]=processed_in[i];
   tdsample.output = 1;
   td.push_back(tdsample);
  
   trainMachine( td, mlp, td.size(), true ); 

 }
  
}
