#ifndef MINNOW_TMP_H_
#define MINNOW_TMP_H_

#include <stdlib.h>

/* `mnw_QuantizeFloat` stores the integral part of an array of floats, `in`,
 * within an array of integers, `out`. */
void mnw_QuantizeFloat(float *in, int64_t *out, int64_t len);

#endif /* MINNOW_TMP_H_ */
