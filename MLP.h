#ifndef MLP_H
#define MLP_H

#include <opencv/cv.h>
#include <opencv/ml.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cmath>


#define PARSEC
#define NUM_EXEC 20
#define NUM_TrainEXEC 1//range 1-9
#define FILE_NAME "/raw"

#define SLIDING_WIN 4
#define INPUT_SIZE SLIDING_WIN*2
//#define MAX_DATA_SIZE 30000000
#define OUTPUT_SIZE 1
#define POSITIVE 1
#define NEGATIVE 0
#define THRESHOLD 0.49

using namespace cv;

//extern std::fstream bug_file;

extern int false_positive_count; // reported as bug but not a bug
extern int false_negative_count; // reported as correct when it is wrong

//extern  long long int no_of_testing_data;
extern  int p_error;

typedef struct{
  unsigned long int input[INPUT_SIZE];
  float output;
}TrainingData;

void trainMachine( std::vector<TrainingData> &td, CvANN_MLP *machineBrain, int train_sample_count, bool update_weights);
int predictMachine(std::vector<TrainingData> &testD,CvANN_MLP *machineBrain);


void prepareTestData(std::vector<TrainingData> &testD, int exec_no );
void createANN( CvANN_MLP *machineBrain, int input_size, int hidden_size, int output_size );
long long int testData1(std::vector<TrainingData> &td, int exec_no);


long long int  assembleData1(std::vector<TrainingData> &td, int exec_no, int thread_no, string program_name);
void  assembleData(std::vector<TrainingData> &td, int exec_no, int thread_no, string program_name );
int predictMachine1(std::vector<TrainingData> &testD,CvANN_MLP *machineBrain);

void assembleTestData(std::vector<TrainingData> &td, int exec_no, int thread_no, string program_name );
long long int  assembleFalseData(std::vector<TrainingData> &td, int exec_no, int thread_no, string program_name );



#endif // MLP_H
