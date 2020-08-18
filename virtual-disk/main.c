/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

//Replacement Policy
int policy = 0;

//General Globals
struct disk *disk;
struct page_table *pt;
unsigned char *virtmem;
unsigned char *physmem;

//Policy Counters
int fifoCounter = 0;
int lruCounter = 0;

//Summary Statistics
int pageFaults, diskReads, diskWrites = 0;

//Frame Variables for Frame Table
struct frame{
    int page;
    int fifo;
    int bits;
    int lru;
};
struct frame *frameTable = 0;


/***************************************
 * Initialize the Frame Table
 **************************************/
void setup_frame_table(int nframes){
    //Loop through frame table and initialize all to 0
    for(int i = 0; i < nframes; i++){
        frameTable[i].page = -1;
        frameTable[i].bits = 0;
        frameTable[i].fifo = 0;
        frameTable[i].lru = 0;
    }

}

/***************************************
 * Check for open spots in  Frame Table
 **************************************/
int check_frame_table(int nframes){
    //Return an open spot in table
    for(int i = 0; i < nframes; i++){
        if(frameTable[i].page == -1)
            return i;
    }
    //Signal the table is full
    return -1;
}

/***************************************
 * RANDOM Replace Policy
 **************************************/
int random_policy(int nframes){
    //Use rand() to pick randomly
    return rand() % nframes;
}

/***************************************
 * FIFO Replace Policy
 **************************************/
int fifo_policy(int nframes){
    //Relevant variables
    int fifoFrame = 0;
    int fifoTime = frameTable[0].fifo;

    //Find least recently used
    for(int i = 0; i < nframes; i++)
        if(frameTable[i].fifo < fifoTime){
            fifoFrame = i;
            fifoTime = frameTable[i].fifo;
        }
    //Return fifo frame
    return fifoFrame;
}

/***************************************
 * CUSTOM Replace Policy - based off LRU
 **************************************/
int custom_policy(int nframes){
    //Loop through the frames
    while(lruCounter < nframes){
        //if we find a lru, return it
        if(frameTable[lruCounter].lru == 0)
            return lruCounter;
        
        //Otherwise, reset all and increase counter
        frameTable[lruCounter].lru = 0;
        lruCounter++;
        
        //Reset counter if we are greater than frames
        if(lruCounter >= nframes)
            lruCounter = 0;
    }
    return 0;
}


/***************************************
 * Choose Frame To Be Replaced
 **************************************/
int choose_frame(struct page_table *pt){
    //Relevant variables
    int nframes = page_table_get_nframes(pt); 
    int replacementFrame;

    //Check frame table for empty frame
    if( (replacementFrame = check_frame_table(nframes)) != -1)
        return replacementFrame;
    
    //Use algorithm to determine frame to replace
    switch (policy){
        case 1:
            replacementFrame = random_policy(nframes);
            break;
        case 2:
            replacementFrame = fifo_policy(nframes);
            break;
        case 3:
            replacementFrame = custom_policy(nframes);     
            break;   
    }
    
    //Return the fromt
    return replacementFrame; 
}


/***************************************
 * Page Fault Handler Function
 **************************************/
void page_fault_handler( struct page_table *pt, int page)
{
    //Increase number of page faults
    pageFaults++;

    //Get Page Table Entry
    int frame;
    int bits;
    page_table_get_entry(pt, page, &frame, &bits);

    //Only READ bit - no need to replace, just update
    if(bits&PROT_READ){
        page_table_set_entry(pt, page, frame, (PROT_READ|PROT_WRITE));
        frameTable[frame].bits = (PROT_READ|PROT_WRITE);
        frameTable[frame].lru = 1;
        return;
    }

    //Determine frame to replace
    int replace = choose_frame(pt);
    //Check to see if just write - dirty bit
    if(frameTable[replace].bits&PROT_WRITE){
        disk_write(disk, frameTable[replace].page, &physmem[replace*PAGE_SIZE]);
        diskWrites++;
    }

    //Make sure we are still valid
    if(frameTable[replace].bits > 0)
        page_table_set_entry(pt, frameTable[replace].page, 0, 0);

    //Update the page table entry
    page_table_set_entry(pt, page, replace, PROT_READ);
    fifoCounter++;
    //Read the disk
    disk_read(disk, page, &physmem[replace*PAGE_SIZE]);
    diskReads++;
   
    //Update the frame table information
    frameTable[replace].page = page;
    frameTable[replace].fifo = fifoCounter;
    frameTable[replace].bits = (PROT_READ);
}



/***************************************
 * Print the usage for the program
 **************************************/
void usage(){
    printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <alpha|beta|gamma|delta>\n");
    return;
}


/***************************************
 * Main Driver
 **************************************/
int main( int argc, char *argv[] )
{
    //Check for correct numberof arguments
	if(argc!=5) {
        usage();
		return 1;
	}
    //Store arguments for later use
	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
    const char *replacement = argv[3];
	const char *program = argv[4];

    //Check and set the appropriate replacement policy
    if(strcmp(replacement, "rand") == 0)
        policy = 1;
    else if(strcmp(replacement, "fifo") == 0)
        policy = 2;
    else if(strcmp(replacement, "custom") == 0)
        policy = 3;
    else{
        fprintf(stderr, "Invalid replacement policy\n");
        usage();
        return 1;
    }


    //Create the virtual disk
	disk = disk_open("myvirtualdisk", npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

    //Create the page table
	pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

    //Get and store the virtual memory
	virtmem = page_table_get_virtmem(pt);
    //Get and store the physical memory
	physmem = page_table_get_physmem(pt);

    //Make a frame table
    frameTable = malloc(sizeof(*frameTable)*nframes);
    setup_frame_table(nframes);

    //Check and call the program entered
	if(!strcmp(program,"alpha"))
		alpha_program(virtmem,npages*PAGE_SIZE);
    else if(!strcmp(program,"beta")) 
		beta_program(virtmem,npages*PAGE_SIZE);
     else if(!strcmp(program,"gamma")) 
		gamma_program(virtmem,npages*PAGE_SIZE);
    else if(!strcmp(program,"delta"))
		delta_program(virtmem,npages*PAGE_SIZE);
    else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);
		return 1;
	}

    //Print the final stats
    printf("\n-Program Execution Summary-\n");
    printf("Page Faults:\t%i\n", pageFaults);
    printf("Disk Reads:\t%i\n", diskReads);
    printf("Disk Writes:\t%i\n", diskWrites);

    //CLose and delete PT and Disk
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
