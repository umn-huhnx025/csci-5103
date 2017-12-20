/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "disk.h"
#include "page_table.h"
#include "program.h"

#include <bsd/stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Memory access variables
int num_page_faults = 0;
int num_disk_reads = 0;
int num_disk_writes = 0;

// Page replacement function wrapper
next_page_t next_page;

// Disk memory
struct disk *disk;

// Page queue
int *queue;
int num_elements = 0;

// Round robin replacement variable
int rr_next_page = 0;

// Returns the next page to be used. next_page_rand() uses the random
// replacement algorithm to randomly choose which page will be evicted and
// replaced.
//
// next_page_rand() takes as an input a pointer to the page table.
int next_page_rand(struct page_table *pt) {
  int *frame = malloc(sizeof(int));
  int *bits = malloc(sizeof(int));
  int page;

  // Get the number of pages in the page table
  int num_pages = page_table_get_npages(pt);

  // Keep choosing a random page until one in use is found
  while (1) {
    // Generate a random number on the range of valid page table indices
    page = arc4random_uniform(num_pages);

    // Get page from page table
    page_table_get_entry(pt, page, frame, bits);

    // Break only if page is currently assigned to a frame
    if (*bits) {
      break;
    }
  }

  // Free memory
  free(frame);
  free(bits);

  return page;
}

// Returns the next page to be used. next_page_fifo() uses a first in first out
// policy to choose which page will be evicted and replaced.
//
// next_page_fifo() takes as an input a pointer to the page table.
int next_page_fifo(struct page_table *pt) {
  int page = queue[0];
  return page;
}

// Returns the next page to be used. next_page_custom() uses a round robin
// policy for page replacement. That is, first page
//
// next_page_custom() takes as an input a pointer to the page table.
int next_page_custom(struct page_table *pt) {
  int *frame = malloc(sizeof(int));
  int *bits = malloc(sizeof(int));
  int page;

  // Get the number of pages in the page table
  int num_pages = page_table_get_npages(pt);

  // Keep choosing the next page until one in use is found
  while (1) {
    // Use round robin to select next page
    page = rr_next_page;
    rr_next_page = (rr_next_page + 1) % num_pages;

    // Get page from page table
    page_table_get_entry(pt, page, frame, bits);

    // Break only if page is currently assigned to a frame
    if (*bits) {
      break;
    }
  }

  // Free memory
  free(frame);
  free(bits);

  return page;
}

// Handles all page faults that occur while the program is running. The handler
// is responsible for mapping page to frames, replacing pages when there are no
// free frames, and managing page bits, such as PROT_READ,PROT_WRITE, and
// PROT_EXEC.
//
// page_fault_hander() takes as input a pointer to the page table and the page
// at which the page fault occured on.
void page_fault_handler(struct page_table *pt, int page) {
  // Memory variables
  int num_frames;
  int num_pages;
  char *physmem = NULL;

  // Page variables
  int *cur_frame;
  int *cur_bits;

  // Frame variables
  int frame_i;
  int page_i;
  int frame_taken = 0;
  int free_frame_found = 0;
  int old_page;
  int *old_frame = NULL;
  int *old_bits = NULL;

  // Update number of page faults
  num_page_faults++;

  // Get information about memory
  num_frames = page_table_get_nframes(pt);
  num_pages = page_table_get_npages(pt);
  physmem = page_table_get_physmem(pt);

  // Allocate memory for page pointers
  cur_frame = malloc(sizeof(int));
  cur_bits = malloc(sizeof(int));

  // Get current page table entry
  page_table_get_entry(pt, page, cur_frame, cur_bits);

  // If the page already has protection bits set, adjust the bits accordingly.
  // Otherwise, the page needs to have a frame assigned.
  if (*cur_bits == (PROT_READ | PROT_WRITE)) {
    // If page has read and write protections, add the execute protection
    page_table_set_entry(pt, page, *cur_frame,
                         (PROT_READ | PROT_WRITE | PROT_EXEC));
    return;
  } else if (*cur_bits == PROT_READ) {
    // If page only has read protection, add the write protection
    page_table_set_entry(pt, page, *cur_frame, (PROT_READ | PROT_WRITE));
    return;
  }

  // Check if there are any free frames
  old_frame = malloc(sizeof(int));
  old_bits = malloc(sizeof(int));

  frame_taken = 0;
  free_frame_found = 0;

  for (frame_i = 0; frame_i < num_frames; frame_i++) {
    frame_taken = 0;

    for (page_i = 0; page_i < num_pages; page_i++) {
      // Get page i from page table
      page_table_get_entry(pt, page_i, old_frame, old_bits);

      if ((*old_bits) && *old_frame == frame_i) {
        // If bits are set and old_frame == frame_i, then frame is taken
        frame_taken = 1;
        break;
      }
    }

    // If frame is not taken, use frame_i for new page
    if (frame_taken == 0) {
      *old_frame = frame_i;
      free_frame_found = 1;
      break;
    }
  }

  // Check if a free frame was found
  if (!free_frame_found) {
    // If no free frame was found, replace an old page
    old_page = next_page(pt);

    // Get page from page table
    page_table_get_entry(pt, old_page, old_frame, old_bits);

    // Check if PROT_WRITE bit is set for page about to be replaced
    if (*old_bits == (PROT_READ | PROT_WRITE) ||
        *old_bits == (PROT_READ | PROT_WRITE | PROT_EXEC)) {
      // Update frame back to disk
      disk_write(disk, old_page, &physmem[(*old_frame) * PAGE_SIZE]);
      num_disk_writes++;

      // Update page table to remove protection bits
      page_table_set_entry(pt, old_page, 0, 0);
    } else {
      // Update page table to remove protection bits
      page_table_set_entry(pt, old_page, 0, 0);
    }
  }

  // Read memory from disk into the physical memory
  disk_read(disk, page, &physmem[(*old_frame) * PAGE_SIZE]);
  num_disk_reads++;

  // Update page table to have page point to respective frame
  page_table_set_entry(pt, page, *old_frame, PROT_READ);

  // Add page to the queue
  if (num_elements == num_frames) {
    // If queue is full, remove first element, move everything over, and add
    // new page
    for (frame_i = 1; frame_i < num_frames; frame_i++) {
      queue[frame_i - 1] = queue[frame_i];
    }
    queue[num_elements - 1] = page;
  } else {
    // If queue is not full, add page to the end
    queue[num_elements] = page;
    num_elements++;
  }

  // Free allocated memory
  free(cur_frame);
  free(cur_bits);
  free(old_frame);
  free(old_bits);

  return;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf(
        "use: virtmem <npages> <nframes> <rand|fifo|custom> "
        "<sort|scan|focus>\n");
    return 1;
  }

  int npages = atoi(argv[1]);
  int nframes = atoi(argv[2]);
  const char *method = argv[3];
  const char *program = argv[4];

  queue = malloc(nframes * sizeof(int));

  disk = disk_open("myvirtualdisk", npages);
  if (!disk) {
    fprintf(stderr, "couldn't create virtual disk: %s\n", strerror(errno));
    return 1;
  }

  if (!strcmp(method, "rand")) {
    next_page = next_page_rand;

  } else if (!strcmp(method, "fifo")) {
    next_page = next_page_fifo;

  } else if (!strcmp(method, "custom")) {
    next_page = next_page_custom;

  } else {
    fprintf(stderr, "unknown replacement method: %s\n", method);
    exit(1);
  }

  struct page_table *pt =
      page_table_create(npages, nframes, page_fault_handler);
  if (!pt) {
    fprintf(stderr, "couldn't create page table: %s\n", strerror(errno));
    return 1;
  }

  char *virtmem = page_table_get_virtmem(pt);

  // char *physmem = page_table_get_physmem(pt);

  if (!strcmp(program, "sort")) {
    sort_program(virtmem, npages * PAGE_SIZE);

  } else if (!strcmp(program, "scan")) {
    scan_program(virtmem, npages * PAGE_SIZE);

  } else if (!strcmp(program, "focus")) {
    focus_program(virtmem, npages * PAGE_SIZE);

  } else {
    fprintf(stderr, "unknown program: %s\n", argv[4]);
    exit(1);
  }

#ifdef BENCHMARK
  printf("%d,%d,%s,%s,%d,%d,%d\n", npages, nframes, method, program,
         num_page_faults, num_disk_reads, num_disk_writes);
#else
  printf("Number of page faults: %d.\n", num_page_faults);
  printf("Number of disk read: %d.\n", num_disk_reads);
  printf("Number of disk writes: %d.\n", num_disk_writes);
#endif

  page_table_delete(pt);
  disk_close(disk);

  // Free memory allocated for page queue
  free(queue);

  return 0;
}
