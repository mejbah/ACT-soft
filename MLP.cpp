#include"MLP.h"
using namespace std;
//#define ANN_DEBUG

int false_positive_count = 0;
int false_negative_count = 0;




void createANN( CvANN_MLP *machineBrain, int input_size, int hidden_size, int output_size )
{
    CvMat  neuralLayers1;

    //The matrix representation of our ANN. We'll have 3 layers.
    CvMat* neuralLayers = cvCreateMat(3, 1, CV_32SC1);
    cvGetRows(neuralLayers, &neuralLayers1, 0, 3);

    //Setting the number of neurons on each layer of the ANN
    cvSet1D(&neuralLayers1, 0, cvScalar(input_size));
    cvSet1D(&neuralLayers1, 1, cvScalar(hidden_size));
    cvSet1D(&neuralLayers1, 2, cvScalar(output_size));

    //Create our ANN
    machineBrain->create(neuralLayers,CvANN_MLP::SIGMOID_SYM, 1.0, 1.0);

}

void trainMachine( std::vector<TrainingData> &td, CvANN_MLP *machineBrain, int train_sample_count, bool update_weights ){

  //Create the matrices
  //
  //Input data samples. Matrix of order (train_sample_count x input_size
  CvMat* trainData = cvCreateMat(train_sample_count, INPUT_SIZE, CV_32FC1);

  //Output data samples. Matrix of order (train_sample_count x 1)
  CvMat* trainClasses = cvCreateMat(train_sample_count, OUTPUT_SIZE, CV_32FC1);

  //The weight of each training data sample. We'll later set all to equal weights.
  CvMat* sampleWts = cvCreateMat(train_sample_count, 1, CV_32FC1);


  CvMat trainData1, trainClasses1,/* neuralLayers1,*/ sampleWts1;

  cvGetRows(trainData, &trainData1, 0, train_sample_count);

  cvGetRows(trainClasses, &trainClasses1, 0, train_sample_count);
  cvGetRows(sampleWts, &sampleWts1, 0, train_sample_count);

#ifdef ANN_DEBUG
  std::fstream log_file;
  log_file.open("training.log", std::fstream::out);
#endif

  //Assemble the ML training data.
   for (int i=0; i<train_sample_count; i++)
   {
       TrainingData train_sample = td[i];
       //input
       for(int j=0; j<INPUT_SIZE; j++)
       {
#ifdef ANN_DEBUG
         log_file << (float)train_sample.input[j] << ", ";
#endif
         cvSet2D(&trainData1, i, j, cvScalar(float(train_sample.input[j])));
       }
       //Output
       cvSet1D(&trainClasses1, i, cvScalar(float(train_sample.output)));
//        log_file << (float)train_sample.output<< std::endl;

#ifdef ANN_DEBUG
       if(train_sample.output == 0 ) 
           log_file << "nve " << train_sample.output << std::endl;
       else
           log_file << train_sample.output << std::endl;
#endif
       //Weight (setting everything to 1)
       cvSet1D(&sampleWts1, i, cvScalar(1));
   }

  //Train it with our data
   CvTermCriteria term_crit;
   int iter_cnt;
   if( update_weights )
   {
         
         iter_cnt = machineBrain->train(
         trainData,
         trainClasses,
         sampleWts,
         0,
         CvANN_MLP_TrainParams(
           term_crit = cvTermCriteria( CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 0.001 ),
           CvANN_MLP_TrainParams::BACKPROP,
           0.1,
           0.1
           ),

          CvANN_MLP::UPDATE_WEIGHTS//|CvANN_MLP::NO_INPUT_SCALE
         );
   }
   else{
         std::cout << "Start new LM Training with new input vectors..."<< std::endl;
        iter_cnt = machineBrain->train(
         trainData,
         trainClasses,
         sampleWts,
         0,

         CvANN_MLP_TrainParams(
           term_crit = cvTermCriteria( CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 0.001 ),
           CvANN_MLP_TrainParams::BACKPROP,
           0.1,
           0.1
           )
	//,CvANN_MLP::NO_INPUT_SCALE
 
         );

   }

//   std::cout << "Training iterations " << iter_cnt << std::endl;
   cvReleaseMat(&trainData);
   cvReleaseMat(&sampleWts);
   cvReleaseMat(&trainClasses);


}

int predictMachine(std::vector<TrainingData> &testd, CvANN_MLP *machineBrain){


    int p_error = 0;
    float _predout[1];
    CvMat predout = cvMat(1, 1, CV_32FC1, _predout);
    float _sample[INPUT_SIZE];
    CvMat sample = cvMat(1, INPUT_SIZE, CV_32FC1, _sample);


    for(int t=0; t<testd.size();t++){
        TrainingData testsample = testd[t];
        for(int i=0; i<INPUT_SIZE;i++){
            sample.data.fl[i]=(float)testsample.input[i];
        }

        machineBrain->predict(&sample, &predout);
        //printf("%f \n",predout.data.fl[0]);

       if( (float)predout.data.fl[0] <= 0.75 ) {
           p_error++;
#if 0
          int curr_readPC=testsample.input[INPUT_SIZE-2];
          int curr_writePC=testsample.input[INPUT_SIZE-1]; 

          for(int i=0; i<INPUT_SIZE-2;i+=2){

            bug_file << hex << (curr_readPC - testsample.input[i])  << " " << (curr_writePC - testsample.input[i+1]) << " ";

          }       
	  bug_file << hex << curr_readPC << " " << curr_writePC;
          bug_file << " | " <<dec<< predout.data.fl[0] << std::endl;
#endif

       }
    }


   return p_error; 
}



void  assembleTestData(std::vector<TrainingData> &td, int exec_no, int thread_no, string program_name )
{
    for(int t=0; t<thread_no; t++)
    {
	  int k=0;
	  long long int slide_win[SLIDING_WIN*2];
	  //long long int no_of_training_data = 0;
	  //std::cout<< dir << std::endl;

	  std::ostringstream ss,fs;
	  ss << t;
	  fs << exec_no;
	  std::ifstream infile;
          string dir = program_name + "/run" + fs.str() + FILE_NAME  + "."+ ss.str();
	  //std :: cout << dir << std::endl;
	  infile.open( dir.c_str());

	//      outfile.open(("./data/"+ fs.str()+"-"+ ss.str()+".out").c_str());
	  if(infile){
	    std::string line="";
	    while(getline(infile,line)) 
	    {
	      bool remoteRAW=false;
	      long long int read_addr, write_addr, negative_write;
	      if(line=="")break;
#if 1
	      if(line.find("R") != string::npos ) // remote RAW : construct input
	      {
		remoteRAW=true;
		line=line.substr(1);
	      }
	      remoteRAW=true; // construct inputs with all RAW

#else
	      remoteRAW=true;
#endif
	      //std::cout << line << std::endl;
	      std::string::size_type pos_comma = line.find(",");
	      std::string::size_type pos_bar = line.find("|");

	      std::string instaddr1 = line.substr(0, pos_comma);
	      stringstream read_addr_ss(instaddr1);
	      read_addr_ss >> hex >> read_addr;
	      std::string instaddr2, instaddr3;
	      if( pos_bar == std::string::npos )
	      {
		instaddr2 = line.substr(pos_comma+1);
		stringstream write_addr_ss(instaddr2);
		write_addr_ss >> hex >> write_addr;
	      }
	      else
	      {
		instaddr2 = line.substr(pos_comma+1, pos_bar);
		stringstream write_addr_ss(instaddr2);
		write_addr_ss >> hex >> write_addr;
		instaddr3 = line.substr(pos_bar + 1);
		stringstream negative_ss(instaddr3);
		negative_ss >> hex >> negative_write;
	      }


	      k++;

	      if (k<=SLIDING_WIN){
		slide_win[2*k-2] = read_addr;
		slide_win[2*k-1] = read_addr;
	      }
	      else{
		for(int x=0; x< SLIDING_WIN-1; x++){

		  slide_win[2*x]=slide_win[2*x+2];
		  slide_win[2*x+1]=slide_win[2*x+3];
		}
		slide_win[SLIDING_WIN*2-2]=read_addr;
		slide_win[SLIDING_WIN*2-1]=write_addr;
	      }

	      if( remoteRAW==true && k>=SLIDING_WIN )
	      {
		TrainingData tdsample;
	#if 0 //edited for adding PC distance instead of raw PC
			  for(int index=0;index<INPUT_SIZE;index++) {
			    tdsample.input[index]=slide_win[index];
			    outfile << tdsample.input[index] << " ";
			  }
			  outfile << std::endl;
	#else
		for(int index = 0; index < SLIDING_WIN-1; index++ ) 
		{
		  tdsample.input[2*index] = slide_win[INPUT_SIZE-2] - slide_win[2*index];

		  tdsample.input[2*index + 1] = slide_win[INPUT_SIZE-1] - slide_win[2*index + 1];

		}
		tdsample.input[INPUT_SIZE-2] = slide_win[INPUT_SIZE - 2];
		tdsample.input[INPUT_SIZE-1] = slide_win[INPUT_SIZE - 1];
			  
	//                  outfile << tdsample.input[INPUT_SIZE-2] << " " << tdsample.input[INPUT_SIZE-1]<< std::endl;
	#endif

		 tdsample.output = 1;
		 td.push_back(tdsample);
	      
		}
	      }

	      infile.close();
	    }
     }
}


void  assembleData(std::vector<TrainingData> &td, int exec_no, int thread_no, string data_dir )
{

  
  for(int t=0; t<thread_no; t++)
  {
	 

          int k=0;
          long long int slide_win[SLIDING_WIN*2];
      
	  std::ostringstream ss,fs;
	  ss << t;
	  fs << exec_no;
	  std::ifstream infile;
	  string dir = data_dir + "/run" + fs.str() + FILE_NAME  + "."+ ss.str();
	  std :: cout << dir << std::endl;
	  infile.open( dir.c_str());

	
	  if(infile){
	    std::string line="";
	    while(getline(infile,line)) 
	    {
	      bool remoteRAW=false;
	      long long int read_addr, write_addr, negative_write;
	      if(line=="")break;
#if 1
	      if(line[0]=='R') // remote RAW : construct input
	      {
		remoteRAW=true;
		line=line.substr(1);
	      }
	      remoteRAW=true; // construct inputs with all RAW

#else
              remoteRAW=true; // construct inputs with all RAW
#endif
	      //std::cout << line << std::endl;
	      std::string::size_type pos_comma = line.find(",");
	      std::string::size_type pos_bar = line.find("|");
              assert(pos_comma != string::npos);
	      std::string instaddr1 = line.substr(0, pos_comma);
	      stringstream read_addr_ss(instaddr1);
	      read_addr_ss >> hex >> read_addr;
	      std::string instaddr2, instaddr3;
	      if( pos_bar == std::string::npos )
	      {
		instaddr2 = line.substr(pos_comma+1);
		stringstream write_addr_ss(instaddr2);
		write_addr_ss >> hex >> write_addr;
	      }
	      else
	      {
		instaddr2 = line.substr(pos_comma+1, pos_bar);
		stringstream write_addr_ss(instaddr2);
		write_addr_ss >> hex >> write_addr;
		instaddr3 = line.substr(pos_bar + 1);
		stringstream negative_ss(instaddr3);
		negative_ss >> hex >> negative_write;
	      }


	      k++;

	      if (k<=SLIDING_WIN){
		slide_win[2*k-2] = read_addr;
		slide_win[2*k-1] = read_addr;
	      }
	      else{
		for(int x=0; x< SLIDING_WIN-1; x++){

		  slide_win[2*x]=slide_win[2*x+2];
		  slide_win[2*x+1]=slide_win[2*x+3];
		}
		slide_win[SLIDING_WIN*2-2]=read_addr;
		slide_win[SLIDING_WIN*2-1]=write_addr;
	      }

	      if( remoteRAW && k>=SLIDING_WIN )
	      {
		TrainingData tdsample;
	#if 0 //edited for adding PC distance instead of raw PC
			  for(int index=0;index<INPUT_SIZE;index++) {
			    tdsample.input[index]=slide_win[index];
			    outfile << tdsample.input[index] << " ";
			  }
			  outfile << std::endl;
	#else
		for(int index = 0; index < SLIDING_WIN-1; index++ ) 
		{
		  tdsample.input[2*index] = slide_win[INPUT_SIZE-2] - slide_win[2*index];

		  tdsample.input[2*index + 1] = slide_win[INPUT_SIZE-1] - slide_win[2*index + 1];

		}
		tdsample.input[INPUT_SIZE-2] = slide_win[INPUT_SIZE - 2];
		tdsample.input[INPUT_SIZE-1] = slide_win[INPUT_SIZE - 1];
			  
	//                  outfile << tdsample.input[INPUT_SIZE-2] << " " << tdsample.input[INPUT_SIZE-1]<< std::endl;
	#endif

		 tdsample.output = 1;
		 td.push_back(tdsample);

		 if( pos_bar != std::string::npos ) //construct nagetive example
		 {
		    long long int slide_winr[SLIDING_WIN*2];
		    for(int index=0;index<INPUT_SIZE-1;index++) slide_winr[index]=slide_win[index];

		    slide_winr[INPUT_SIZE-1]=negative_write;

		    TrainingData tdNsample;
	#if 0 //edited for adding PC distance instead of raw PC

			    for(int index=0;index<INPUT_SIZE;index++) 
			    {  
			      tdNsample.input[index]=slide_winr[index];
			      outfile << tdNsample.input[index] << " ";
			    }
			    outfile << std::endl;
	#else
		    for(int index = 0; index < SLIDING_WIN-1; index++ ) 
		    {
		      tdNsample.input[2*index] = slide_winr[INPUT_SIZE-2] - slide_winr[2*index];
		      tdNsample.input[2*index + 1] = slide_winr[INPUT_SIZE-1] - slide_winr[2*index + 1];

		    } 
		    tdNsample.input[INPUT_SIZE-2] = slide_winr[INPUT_SIZE - 2];
		    tdNsample.input[INPUT_SIZE-1] = slide_winr[INPUT_SIZE - 1]; 		

	#endif                    
		    tdNsample.output = -1;
		    td.push_back(tdNsample);

		  }
	      
		}
	      }

	      infile.close();
	    }
  }

}
