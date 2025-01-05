#include <cstdint>
#include <cstdio>
#include <vector>
#include <omp.h>
#include "inc/main.hxx"

using namespace std;




#pragma region CONFIGURATION
#ifndef MAX_THREADS
/** Maximum number of threads to use. */
#define MAX_THREADS 64
#endif
#ifndef REPEAT_METHOD
/** Number of times to repeat each method. */
#define REPEAT_METHOD 5
#endif
#pragma endregion




#pragma region METHODS
/**
 * Main function.
 * @param argc number of command-line arguments
 * @param argv command-line arguments
 * @returns 0 if successful
 */
int main(int argc, char **argv) {
  constexpr size_t ALLOCS   = 1ULL << 26;
  constexpr size_t SIZE     = 1ULL << 6;
  constexpr size_t CAPACITY = 1ULL << 32;
  omp_set_num_threads(MAX_THREADS);
  // Allocate memory using malloc.
  {
    vector<void*> ptrs(ALLOCS);
    float tm = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i)
        ptrs[i] = malloc(SIZE);
    });
    printf("malloc: %.3f ms\n", tm);
    float tf = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i)
        free(ptrs[i]);
    });
    printf("free: %.3f ms\n", tf);
  }
  // Allocate memory using new.
  {
    vector<void*> ptrs(ALLOCS);
    float tm = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i)
        ptrs[i] = new char[SIZE];
    });
    printf("new: %.3f ms\n", tm);
    float tf = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i)
        delete[] (char*) ptrs[i];
    });
    printf("delete: %.3f ms\n", tf);
  }
  // Allocate memory using FixedArenaAllocator.
  {
    vector<void*> ptrs(ALLOCS);
    vector<FixedArenaAllocator<SIZE, CAPACITY>*> mems(MAX_THREADS);
    for (int i=0; i<MAX_THREADS; ++i)
      mems[i] = new FixedArenaAllocator<SIZE, CAPACITY>(malloc(CAPACITY));
    float tm = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i) {
        int t = omp_get_thread_num();
        ptrs[i] = mems[t]->allocate();
      }
    });
    printf("FixedArenaAllocator.allocate: %.3f ms\n", tm);
    float tf = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i) {
        int t = omp_get_thread_num();
        mems[t]->deallocate(ptrs[i]);
      }
    });
    printf("FixedArenaAllocator.deallocate: %.3f ms\n", tf);
  }
  // Allocate memory using ArenaAllocator (growing).
  {
    vector<void*> ptrs(ALLOCS);
    vector<ArenaAllocator<SIZE, 4096*SIZE>*> mems(MAX_THREADS);
    for (int i=0; i<MAX_THREADS; ++i)
      mems[i] = new ArenaAllocator<SIZE, 4096*SIZE>();
    float tm = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i) {
        int t = omp_get_thread_num();
        ptrs[i] = mems[t]->allocate();
      }
    });
    printf("ArenaAllocator.allocate: %.3f ms\n", tm);
    float tf = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i) {
        int t = omp_get_thread_num();
        mems[t]->deallocate(ptrs[i]);
      }
    });
    printf("ArenaAllocator.deallocate: %.3f ms\n", tf);
  }
  printf("Performed %zu allocations of %zu bytes each.\n", ALLOCS, SIZE);
  printf("\n");
}
#pragma endregion
