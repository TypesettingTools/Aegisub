#include <iostream>
#include <omp.h>

double wasteTime(double in)
{
	double accum = in;
	for (size_t i=0;i<5000000;i++) {
		accum = accum + accum/2.0 - accum/3.0;
	}
	return accum;
}

int main()
{
	using namespace std;
	cout << "OpenMP test:\n";
	bool run = true;
	int n = 0;
	omp_set_num_threads(8);
	#pragma omp parallel shared(run,n)
	{
		while (run) {
			int myTask;
			double lol = 0.0;
			
			#pragma omp critical (secA)
			{
				myTask = n;
				n++;
				if (myTask >= 50) run = false;
				if (run) {
					cout << "Thread #" <<  omp_get_thread_num() << " on task " << myTask << "/1.\n";
					lol -= wasteTime(myTask * 1.0);
					cout << "Thread #" <<  omp_get_thread_num() << " got " << lol << " on task " << myTask << "/1. Task done.\n";
				}
			}

			if (run) {
				#pragma omp critical (secB)
				{
					cout << "Thread #" <<  omp_get_thread_num() << " on task " << myTask << "/2.\n";
					lol += wasteTime(myTask * 2.0);
					cout << "Thread #" <<  omp_get_thread_num() << " got " << lol << " on task " << myTask << "/2. Task done.\n";
				}

				#pragma omp critical (secC)
				{
					cout << "Thread #" <<  omp_get_thread_num() << " on task " << myTask << "/3.\n";
					lol -= wasteTime(myTask * 3.0);
					cout << "Thread #" <<  omp_get_thread_num() << " finished task " << myTask << " with the useless result of " << lol << "\n";
				}
			}
		}
	}
}
