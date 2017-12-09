#include<stdio.h>
#include<omp.h>
#include<math.h>

//Counting number of recursive function calls
long long int count=0;

//Integral function: Parameters: 1. Function to integrate 2. Left Interval Boundary 3. Right Interval aboundary 4. Error tolerance
//Sequential Algorithm
double integral(double (*f)(double),double a,double b,double tolerance);

//Parallel Algorithm
double integral_par(double (*f)(double),double a,double b,double tolerance);

//Function to integrate
double func(double x);

int main()
{
   //printf("%d\n",;
   double x_min,x_max;
   double answer = 0.0;
   double threshold;
   double start_time,end_time;

   // This will be replaced by  menu with 20 to 30 choices of functions to integrate. User can choose.
   //printf("Function chosen : exp(-x*x)\n");
   printf("Function chosen : x\n");

   printf("Enter lower and upper limits of integration\n");
   scanf("%lf%lf",&x_min,&x_max);

   printf("Enter threshold of error allowed\n");
   scanf("%lf",&threshold);

   

   // Sequential Algortihm
   start_time = omp_get_wtime();
   //Calculate integral and store result in answer
   answer = integral(func,x_min,x_max,threshold);
   end_time = omp_get_wtime();

   printf("Sequential Algorithm Results:\n");
   printf("Approximate Integral (found by our algorithm): %.15lf\n",answer);
   printf("Time taken by the Algorithm : %.15lf\n",end_time-start_time);
   printf("Number of recursive function calls: %lld\n",count);


   
   // Divide and Conquer Based Parallel Algortihm
   start_time = omp_get_wtime();
   //Calculate integral and store result in answer
   #pragma omp parallel
   {
	   	#pragma omp single
	   	{
	      answer = integral_par(func,x_min,x_max,threshold);
	   	}
   }   
   end_time = omp_get_wtime();

   printf("Parallel Algorithm Results:\n");
   printf("Approximate Integral (found by our algorithm): %.15lf\n",answer);
   printf("Time taken by the Algorithm : %.15lf\n",end_time-start_time);  

   

   return 0;
}

//Function to integrate
double func(double x)
{
	return exp(-x*x);
	
}

//Sequential Algorithm
double integral(double (*f)(),double a,double b,double tolerance)
{
	count = count+1; // Increment number of recursive calls by 1
        double result; // Value to return
	double height; // Height of the interval
	double mid; // Center of the interval
	double full_trapezoid_area; // Area of a trapezium formed with parallel edges of length f(a) and f(b)
	double sum_of_halves_trapezoid_area; // Sum of the areas of the two trapeziums: one with parallel edges of length f(a) and f(mid) and the other with parallel edges f(mid) and f(b)
    double left_area; // Area of left half interval
    double right_area; // Area of right half interval

    height = b-a;
    mid = (a+b)/2;
    full_trapezoid_area = height * (f(a) + f(b)) / 2.0;
    sum_of_halves_trapezoid_area = height/2 * (f(a) + f(mid)) / 2.0 + height/2 * (f(mid) + f(b)) / 2.0;

    if(fabs(full_trapezoid_area - sum_of_halves_trapezoid_area) < 3.0 * tolerance) // error is acceptable
    	result = sum_of_halves_trapezoid_area;
    else // error is not acceptable
    {
    	// divide the total area into 2 parts and call integral on each part with tolerance equal to half of initial tolerance
    	left_area = integral(f,a,mid,tolerance/2);
    	right_area = integral(f,mid,b,tolerance/2);
    	result = left_area + right_area;
    }

    // Return the result
    return result;

}

//Parallel Algorithm 
double integral_par(double (*f)(),double a,double b,double tolerance)
{
	double result; // Value to return
	double height; // Height of the interval
	double mid; // Center of the interval
	double full_trapezoid_area; // Area of a trapezium formed with parallel edges of length f(a) and f(b)
	double sum_of_halves_trapezoid_area; // Sum of the areas of the two trapeziums: one with parallel edges of length f(a) and f(mid) and the other with parallel edges f(mid) and f(b)
    double left_area; // Area of left half interval
    double right_area; // Area of right half interval

    height = b-a;
    mid = (a+b)/2;
    full_trapezoid_area = height * (f(a) + f(b)) / 2.0;
    sum_of_halves_trapezoid_area = height/2 * (f(a) + f(mid)) / 2.0 + height/2 * (f(mid) + f(b)) / 2.0;

    if(fabs(full_trapezoid_area - sum_of_halves_trapezoid_area) < 3.0 * tolerance) // error is acceptable
    	result = sum_of_halves_trapezoid_area;
    else // error is not acceptable
    {
    	// divide the total area into 2 parts and call integral on each part with tolerance equal to half of initial tolerance
    	// Task directive is used to spawn the integrations of left portion and right portion to different threads
        #pragma omp task shared(left_area)
        {
      	   left_area = integral_par(f,a,mid,tolerance/2);
        }

        #pragma omp task shared(right_area)
        {
    	   right_area = integral_par(f,mid,b,tolerance/2);
        }
    	
    	// Result can be calculated only after the left and right areas are calculated
    	#pragma omp taskwait
    	   result = left_area + right_area;
    }

    // Return the result
    return result;

}



