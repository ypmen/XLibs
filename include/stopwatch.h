/*
 * stopwatch.h
 *
 *  Created on: Apr 19, 2020
 *      Author: ypmen
 */

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

using namespace std;

class Stopwatch
{
public:
	Stopwatch(const char * des)
	{
		strcpy(description, des);
		gettimeofday(&start, NULL);
	};
	~Stopwatch()
	{
		gettimeofday(&end, NULL);
		fprintf(stderr, "time elapse of %s = %ld\n", description, (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec));
	};

private:
    struct timeval start;
    struct timeval end;
    char description[64];
};



#endif /* STOPWATCH_H_ */
