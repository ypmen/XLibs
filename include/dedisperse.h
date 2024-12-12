/*
 * dedisperse.h
 *
 *  Created on: Apr 29, 2020
 *      Author: ypmen
 */

#ifndef DEDISPERSE_H_
#define DEDISPERSE_H_

#include <assert.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "utils.h"
#include "logging.h"
#include "constants.h"

extern unsigned int num_threads;

#define HAVE_YMW16 1

#endif /* DEDISPERSE_H_ */
