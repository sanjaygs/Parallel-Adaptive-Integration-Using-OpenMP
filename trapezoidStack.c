#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <sys/time.h>

#define INITIAL_SIZE 128 

double f(double a)
{
	return exp(-a*a);
         
}

struct mainStack{               
  int count;              
  int size;             
  int mem_reserve;         
  void* elements;          
};

typedef struct mainStack* myStack;


void create_stack(myStack* stack, int element_size);
int empty_stack(myStack stack);
void push_stack(myStack stack, void* element);
void pop_stack(myStack stack, void* element);


void 
create_stack(
             myStack* stack,     
             int element_size)   
{
  int initial_size = INITIAL_SIZE;

  (*stack) = (myStack) malloc(sizeof(struct mainStack));
  if (!(*stack)){
    fprintf(stderr, "error: could not allocate memory for stack.. Abort.\n"); 
    exit(1);
  }    

  (*stack)->elements = (void*) malloc(element_size * initial_size);
  (*stack)->mem_reserve = initial_size; 
  if (!(*stack)->elements){
    fprintf(stderr, "error: could not allocate memory for stack.. Abort.\n");
    exit(1);
  }

  (*stack)->size = element_size;
  (*stack)->count = 0;

}


int empty_stack
(myStack stack)
{
  return stack->count <= 0;
}


void push_stack(
           myStack stack,   
           void* element)   
{
  int i, interval_reserve;
  int stack_length;
   
  if (stack->count >= stack->mem_reserve){

      for (i = stack->count, stack_length = 0; 
           i > 0; 
           i>>1, stack_length++);
      interval_reserve = 1 << stack_length;
 
      stack->elements = (void *) realloc(stack->elements, 
                      stack->size * interval_reserve);
      if (!stack->elements){
        fprintf(stderr, "error: can't reallocate stack.. Aborting\n");
        exit(1);
      }

      stack->mem_reserve = interval_reserve;
  }
  
  memcpy((char*)stack->elements + stack->count*stack->size, 
            element, stack->size);
  stack->count++;

}


void pop_stack(
          myStack stack,
          void* element)
{
  if (stack->count <= 0){
    fprintf(stderr, "error: trying to pop from empty stack.\n");
    exit(2);
  }

  stack->count--;
  memcpy(element, 
         (char*)stack->elements + stack->count*stack->size, 
         stack->size);
}






typedef struct _work_t{
  double a;
  double b;
  double tol;
} work_t;



double  
integral(
     double (*f)(double), 
     double ah,           
     double bh,           
     double tolh)         

{
  double a, b, tolerance;       
  double integral_result;       
  double h;                     
  double mid;                   
  double one_trapezoid_area;    
  double two_trapezoid_area;    
  double left_area;            
  double right_area;            
  printf("Parallel Execution\n");
  struct timeval  TimeValue_Start;
	struct timezone TimeZone_Start;

	struct timeval  TimeValue_Final;
	struct timezone TimeZone_Final;
	long   time_start, time_end;
     double  time_overhead;

  myStack stack;
  work_t work;

  int ready, idle, busy;

  work.a = ah;
  work.b = bh;
  work.tol = tolh;

  create_stack(&stack, sizeof(work_t));
  push_stack(stack, &work);

  integral_result = 0.0;

  busy = 0;
  
  
  gettimeofday(&TimeValue_Start, &TimeZone_Start);

#pragma omp parallel default(none) \
    shared(stack, integral_result,f,busy) \
    private(a,b,tolerance, work, h, mid, one_trapezoid_area, \
            two_trapezoid_area, idle, ready)
  {

    ready = 0;
    idle = 1;

    while(!ready){
#pragma omp critical (stack)
      {
        if (!empty_stack(stack)){          
          pop_stack(stack, &work);

          if (idle){           
            busy += 1;
            idle = 0;
          }

        }else{
         
          if (!idle){
            busy -= 1;
            idle = 1;
          }

          if (busy == 0)
            ready = 1;

        }
      }

      if (idle)
        continue;

      b = work.b;
      a = work.a;
      tolerance = work.tol;

      h = b - a;
      mid = (a+b)/2;
      one_trapezoid_area = h * (f(a) + f(b)) / 2.0;
      two_trapezoid_area = h/2 * (f(a) + f(mid)) / 2.0 +
        h/2 * (f(mid) + f(b)) / 2.0;

      if (fabs(one_trapezoid_area - two_trapezoid_area) < 3.0 * tolerance){
         
#pragma omp critical (result)
        integral_result += two_trapezoid_area;

      }else{
        

        work.a = a;
        work.b = mid;
        work.tol = tolerance/2;
#pragma omp critical (stack)
        {
          push_stack(stack, &work);  
          work.a = mid;
          work.b = b;
          push_stack(stack, &work);
        }
      }
    } 
  } 
gettimeofday(&TimeValue_Final, &TimeZone_Final);
time_start = TimeValue_Start.tv_sec * 1000000 + TimeValue_Start.tv_usec;
time_end = TimeValue_Final.tv_sec * 1000000 + TimeValue_Final.tv_usec;
	time_overhead = (time_end - time_start)/1000000.0;
printf("Time in Seconds (T) : %lf\n",time_overhead);

  return integral_result;
}
double  
integralSeq(
     double (*f)(double), 
     double ah,           
     double bh,           
     double tolh)         

{
  double a, b, tolerance;       
  double integral_result;       
  double h;                     
  double mid;                   
  double one_trapezoid_area;    
  double two_trapezoid_area;    
  double left_area;            
  double right_area; 
	printf("Sequential Execution\n");
	           
  
  struct timeval  TimeValue_Start;
	struct timezone TimeZone_Start;

	struct timeval  TimeValue_Final;
	struct timezone TimeZone_Final;
	long   time_start, time_end;
     double  time_overhead;

  myStack stack;
  work_t work;

  int ready, idle, busy;

  work.a = ah;
  work.b = bh;
  work.tol = tolh;

  create_stack(&stack, sizeof(work_t));
  push_stack(stack, &work);

  integral_result = 0.0;

  busy = 0;
  
  
  gettimeofday(&TimeValue_Start, &TimeZone_Start);


  {

    ready = 0;
    idle = 1;

    while(!ready){

      {
        if (!empty_stack(stack)){          
          pop_stack(stack, &work);

          if (idle){           
            busy += 1;
            idle = 0;
          }

        }else{
         
          if (!idle){
            busy -= 1;
            idle = 1;
          }

          if (busy == 0)
            ready = 1;

        }
      }

      if (idle)
        continue;

      b = work.b;
      a = work.a;
      tolerance = work.tol;

      h = b - a;
      mid = (a+b)/2;
      one_trapezoid_area = h * (f(a) + f(b)) / 2.0;
      two_trapezoid_area = h/2 * (f(a) + f(mid)) / 2.0 +
        h/2 * (f(mid) + f(b)) / 2.0;

      if (fabs(one_trapezoid_area - two_trapezoid_area) < 3.0 * tolerance){
         

        integral_result += two_trapezoid_area;

      }else{
        

        work.a = a;
        work.b = mid;
        work.tol = tolerance/2;

        {
          push_stack(stack, &work);  
          work.a = mid;
          work.b = b;
          push_stack(stack, &work);
        }
      }
    } 
  } 
gettimeofday(&TimeValue_Final, &TimeZone_Final);
time_start = TimeValue_Start.tv_sec * 1000000 + TimeValue_Start.tv_usec;
time_end = TimeValue_Final.tv_sec * 1000000 + TimeValue_Final.tv_usec;
	time_overhead = (time_end - time_start)/1000000.0;
printf("Time in Seconds (T) : %lf\n",time_overhead);

  return integral_result;
}



main()
{
	double x,y,z;
	printf("Enter the upper and lower bound values : ");
	scanf("%lf",&y);
	scanf("%lf",&x); 
	printf("Definite integral is %lf\n",integralSeq(f,x,y,1));
	printf("Definite integral is %lf\n",integral(f,x,y,1));
}
