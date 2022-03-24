#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define POLICIES 16
#define CUSTOMERS 50000

int feedback[POLICIES][CUSTOMERS];

void init();
void get_binary(int*, int, int);
void update_top10(double*, double*, int*);
double get_average(double*, int);
double get_std_deviation(double*, int, double);


int main(void){
  init();
  
  int i, j, k; //iterators
  double selected_policies; // a counter for selected policies as a double to avoid rounding errors

  int number_of_combinations = pow(2, POLICIES);
  int combination[POLICIES]; // this array represents a combination in form of a binary number
                             // where 1 means the policy is selected

  /*
    arrays for the top 10 averages, standard deviations, and combinations
    The arrays are meant to be in non-increasing order.
    That is, the smallest element is in position 9
   */
  double top10_averages[10];
  double top10_std_dev[10];
  int top10_combinations[10];
  
  double current_avg; // variable storing the average of a given combination
  
  double current[CUSTOMERS]; // current value for customers based on the selected policies
  
double start,stop;
start=omp_get_wtime();

  
#pragma omp parallel default(none) shared(feedback,number_of_combinations,top10_averages,top10_std_dev,top10_combinations) private(combination,current,current_avg,selected_policies) private(i,j,k)
{
/*
    Array initialisations with impossible low values
   */
#pragma omp for 
  for(i = 0; i < 10; i++){
    top10_averages[i]=-101.0;
    top10_std_dev[i]=-101.0;
    top10_combinations[i]=-1;
  }
  
  
#pragma omp for schedule(dynamic)
  for(i = 0; i < number_of_combinations ; i++){
    
    for(j = 0; j < CUSTOMERS; j++){ // reset the current values
      current[j] = 0.0;
    }
    
    selected_policies = 0.0; 

    get_binary(combination, i, POLICIES); // get the combination in form of a binary number

    //First, count the number of policies
    for(j = 0; j < POLICIES; j++){
      if(combination[j] == 1){
	selected_policies++;
      }
    }

    //increase by one to avoid division by zero
    if(selected_policies == 0){
      selected_policies++;
    }
    
    for(j = 0; j < POLICIES; j++){ // main loop to compute the average of combination

      if(combination[j] == 1){
	
	for(k = 0; k < CUSTOMERS; k++){
	  // Sum the values for feedback on the selected policies
	  current[k] += feedback[j][k] / selected_policies; 
	}
	
      }

    }

    //get the current average
    current_avg = get_average(current, CUSTOMERS);

    /*
      if the current average is greater than top10_averages[9]
      1. replace the smallest average in the top 10
      2. compute the standard deviation and add it
      3. record the current combination
      4. resort the top 10s as needed
    */
    #pragma omp critical
    if(current_avg > top10_averages[9]){
	top10_averages[9] = current_avg;
	top10_std_dev[9] = get_std_deviation(current, CUSTOMERS, current_avg);
	top10_combinations[9] = i;
	update_top10(top10_averages, top10_std_dev, top10_combinations);
    }
    

  }
  }
  

  // Printing the results
  stop = omp_get_wtime();
  
  printf("Time taken : %.2f\n",stop-start);
  
  printf("Top 10 Results:\n");
  
  for(i = 0; i < 10; i++){
    printf("Combination: %d -- Average: %.2f -- Standard Deviation: %.2f\n", top10_combinations[i], top10_averages[i], top10_std_dev[i]);
  }
  
  return 0;
}


/*
Function to update all three top10 arrays according to a non-increasing ordering on avgs
 */
void update_top10(double* avgs, double* devs, int* combs){
  int i, j;

  i = 8;
  while(i >= 0 && avgs[i] < avgs[9]){
    i--;
  }

  i++;

  double tmp_avg;
  double tmp_dev;
  int tmp_comb;


  for(j = i; j < 10; j++){
    tmp_avg = avgs[9];
    avgs[9] = avgs[j];
    avgs[j] = tmp_avg;

    tmp_dev = devs[9];
    devs[9] = devs[j];
    devs[j] = tmp_dev;

    tmp_comb = combs[9];
    combs[9] = combs[j];
    combs[j] = tmp_comb;
  }
}

// Simple function to compute the average of an array of doubles
double get_average(double* array, int length){
  double sum = 0.0;
  int i;

  for(i = 0; i < length; i++){
    sum += array[i] / length;
  }
  return sum;
}

// Simple function to compute the standard deviation of an array of doubles given their average
double get_std_deviation(double* array, int length, double avg){
  double sum = 0.0;
  int i;

  for(i = 0; i < length; i++){
    sum += pow(array[i]-avg,2) / length;
  }
  return sqrt(sum);
}

// Updates values in array, so that it contains the binary representation of the integer value
void get_binary(int* array, int value, int length){
  int i;
  for(i = 0; value > 0;i++){
    array[i] = value % 2;
    value = value / 2;
  }

  for(i = i; i < length; i++){
    array[i] = 0;
  }
}

// Initialisation of customers' opinions.
void init(){
  /*
    pos_or_neg is used to decide whether the policy is generally bad received or not.
    pos_or_neg == 3, then negatively received,  positively received otherwise
   */
  int i, j, pos_or_neg;
  for(i = 0; i < POLICIES; i++){
       pos_or_neg = rand() % 4;
       if(i % 2 == 0){ // half of the time we have a uniform distribution between -50 and 50
          for(j = 0; j < CUSTOMERS; j++){
    	feedback[i][j] = (rand() % 101) - 50; // values between -50 and 50
          }
        } else if(pos_or_neg == 3){ // roughly 1/8 of the time we have negatively received values
          for(j = 0; j < CUSTOMERS; j++){
    	feedback[i][j] = (rand() % 121) - 100; // values between -100 and 20
          }
        } else {
          for(j = 0; j < CUSTOMERS; j++){ // roughly 3/8 of the time we have positively received values
    	feedback[i][j] = (rand() % 121) - 20; // values between -20 and 100
          }
        }
      }
}


