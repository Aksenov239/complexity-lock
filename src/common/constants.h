/**
 * @author Aleksandar Dragojevic aleksandar.dragojevic@epfl.ch
 */

#ifndef SMABS_CONSTANTS_H_
#define SMABS_CONSTANTS_H_

#define CACHE_LINE_SIZE_BYTES 64
#define LOG_CACHE_LINE_SIZE_BYTES 6

#define CACHE_LINE_ALIGNED __attribute__((aligned(CACHE_LINE_SIZE_BYTES)))

#endif /* SMABS_CONSTANTS_H_ */
