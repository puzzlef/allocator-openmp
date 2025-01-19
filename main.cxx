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
  constexpr size_t ALLOCS   = 1ULL << 28;
  constexpr size_t SIZE     = 1ULL << 6;
  constexpr size_t MIXED    = 64;
  omp_set_num_threads(MAX_THREADS);
  // Manage memory using malloc and free.
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
  // Manage memory using malloc and free (mixed).
  {
    vector<void*> ptrs(ALLOCS/MIXED);
    float ta = measureDuration([&] {
      for (size_t l=0; l<MIXED; ++l) {
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i)
          ptrs[i] = malloc(SIZE);
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i)
          free(ptrs[i]);
      }
    });
    printf("malloc + free: %.3f ms\n", ta);
  }
  // Manage memory using new and delete.
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
  // Manage memory using new and delete (mixed).
  {
    vector<void*> ptrs(ALLOCS/MIXED);
    float ta = measureDuration([&] {
      for (size_t l=0; l<MIXED; ++l) {
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i)
          ptrs[i] = new char[SIZE];
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i)
          delete[] (char*) ptrs[i];
      }
    });
    printf("new + delete: %.3f ms\n", ta);
  }
  // Manage memory using FixedArenaAllocator.
  {
    vector<void*> ptrs(ALLOCS);
    vector<FixedArenaAllocator<SIZE, ALLOCS>*> mems(MAX_THREADS);
    for (int i=0; i<MAX_THREADS; ++i)
      mems[i] = new FixedArenaAllocator<SIZE, ALLOCS>(malloc(ALLOCS * SIZE));
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
  // Manage memory using FixedArenaAllocator (mixed).
  {
    vector<void*> ptrs(ALLOCS/MIXED);
    vector<FixedArenaAllocator<SIZE, ALLOCS/MIXED>*> mems(MAX_THREADS);
    for (int i=0; i<MAX_THREADS; ++i)
      mems[i] = new FixedArenaAllocator<SIZE, ALLOCS/MIXED>(malloc(ALLOCS/MIXED * SIZE));
    float ta = measureDuration([&] {
      for (size_t l=0; l<MIXED; ++l) {
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i) {
          int t = omp_get_thread_num();
          ptrs[i] = mems[t]->allocate();
        }
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i) {
          int t = omp_get_thread_num();
          mems[t]->deallocate(ptrs[i]);
        }
      }
    });
    printf("FixedArenaAllocator.allocate + deallocate: %.3f ms\n", ta);
  }
  // Manage memory using ArenaAllocator (growing).
  {
    constexpr size_t CAPACITY = 4096;
    vector<void*> ptrs(ALLOCS);
    vector<ArenaAllocator<SIZE, CAPACITY>*> mems(MAX_THREADS);
    for (int i=0; i<MAX_THREADS; ++i)
      mems[i] = new ArenaAllocator<SIZE, CAPACITY>();
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
  // Manage memory using ArenaAllocator (growing, mixed).
  {
    constexpr size_t CAPACITY = 4096;
    vector<void*> ptrs(ALLOCS/MIXED);
    vector<ArenaAllocator<SIZE, CAPACITY>*> mems(MAX_THREADS);
    for (int i=0; i<MAX_THREADS; ++i)
      mems[i] = new ArenaAllocator<SIZE, CAPACITY>();
    float ta = measureDuration([&] {
      for (size_t l=0; l<MIXED; ++l) {
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i) {
          int t = omp_get_thread_num();
          ptrs[i] = mems[t]->allocate();
        }
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i) {
          int t = omp_get_thread_num();
          mems[t]->deallocate(ptrs[i]);
        }
      }
    });
    printf("ArenaAllocator.allocate + deallocate: %.3f ms\n", ta);
  }
  // Manage memory using ConcurrentPow2Allocator (growing).
  {
    constexpr size_t CAPACITY = 4096;
    vector<void*> ptrs(ALLOCS);
    ConcurrentPow2Allocator<SIZE*CAPACITY> mems;
    float tm = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i) {
        // int t = omp_get_thread_num();
        ptrs[i] = mems.allocate(SIZE);
      }
    });
    printf("ConcurrentPow2Allocator.allocate: %.3f ms\n", tm);
    float tf = measureDuration([&] {
      #pragma omp parallel for schedule(dynamic, 2048)
      for (size_t i=0; i<ALLOCS; ++i) {
        // int t = omp_get_thread_num();
        mems.deallocate(ptrs[i], SIZE);
      }
    });
    printf("ConcurrentPow2Allocator.deallocate: %.3f ms\n", tf);
  }
  // Manage memory using ConcurrentPow2Allocator (growing, mixed).
  {
    constexpr size_t CAPACITY = 4096;
    vector<void*> ptrs(ALLOCS/MIXED);
    ConcurrentPow2Allocator<SIZE*CAPACITY> mems;
    float ta = measureDuration([&] {
      for (size_t l=0; l<MIXED; ++l) {
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i) {
          // int t = omp_get_thread_num();
          ptrs[i] = mems.allocate(SIZE);
        }
        #pragma omp parallel for schedule(dynamic, 2048)
        for (size_t i=0; i<ALLOCS/MIXED; ++i) {
          // int t = omp_get_thread_num();
          mems.deallocate(ptrs[i], SIZE);
        }
      }
    });
    printf("ConcurrentPow2Allocator.allocate + deallocate: %.3f ms\n", ta);
  }
  printf("Performed %zu allocations of %zu bytes each.\n", ALLOCS, SIZE);
  printf("\n");
}
#pragma endregion
