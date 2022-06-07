#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <unistd.h>
#include "fat.h"
#include "fat_file.h"


/**
 * Write inside one block in the filesystem.
 * @param  fs           filesystem
 * @param  block_id     index of block in the filesystem
 * @param  block_offset offset inside the block
 * @param  size         size to write, must be less than BLOCK_SIZE
 * @param  buffer       data buffer
 * @return              written byte count
 */
int mini_fat_write_in_block(FAT_FILESYSTEM *fs, const int block_id, const int block_offset, const int size, const void * buffer) {
	assert(block_offset >= 0);
	assert(block_offset < fs->block_size);
	assert(size + block_offset <= fs->block_size);

	int written = 0;

	// TODO: write in the real file.

	FILE *fd = fopen(fs->filename, "rb+");
	if(fd == NULL || size == 0){
		return NULL;
	}
	
	fseek(fd, block_id*fs->block_size+block_offset, SEEK_CUR);	
	
	int s = fwrite(buffer, 1,  size, fd);

	//printf("buffer: %s\n", buffer);
	written += s;

	fclose(fd);
	fs->block_map[block_id] = FILE_DATA_BLOCK;
	return written;
}

/**
 * Read inside one block in the filesystem
 * @param  fs           filesystem
 * @param  block_id     index of block in the filesystem
 * @param  block_offset offset inside the block
 * @param  size         size to read, must fit inside the block
 * @param  buffer       buffer to write the read stuff to
 * @return              read byte count
 */
int mini_fat_read_in_block(FAT_FILESYSTEM *fs, const int block_id, const int block_offset, const int size, void * buffer) {
	assert(block_offset >= 0);
	assert(block_offset < fs->block_size);
	assert(size + block_offset <= fs->block_size);

	int read = 0;

	// TODO: read from the real file.
	
	FILE *fd = fopen(fs->filename, "rb");
	
	if(fd== NULL || size == 0){
		return NULL;
	}

	fseek(fd, block_id*fs->block_size+block_offset, SEEK_SET);

	//fgets((char *)buffer, size, fd);
		int s = fread(buffer,1, size, fd);
	//printf("buffer: %s\n", buffer);
	read += s;
	fclose(fd);

	return read;
}


/**
 * Find the first empty block in filesystem.
 * @return -1 on failure, index of block on success
 */
int mini_fat_find_empty_block(const FAT_FILESYSTEM *fat) {
	// TODO: find an empty block in fat and return its index.
	
	for(int i=0; i<fat->block_map.size(); i++){
		if(fat->block_map[i] == EMPTY_BLOCK){
			return i;
		}
	}
	return -1;
}

/**
 * Find the first empty block in filesystem, and allocate it to a type,
 * i.e., set block_map[new_block_index] to the specified type.
 * @return -1 on failure, new_block_index on success
 */
int mini_fat_allocate_new_block(FAT_FILESYSTEM *fs, const unsigned char block_type) {
	int new_block_index = mini_fat_find_empty_block(fs);
	if (new_block_index == -1)
	{
		fprintf(stderr, "Cannot allocate block: filesystem is full.\n");
		return -1;
	}
	fs->block_map[new_block_index] = block_type;
	return new_block_index;
}

void mini_fat_dump(const FAT_FILESYSTEM *fat) {
	printf("Dumping fat with %d blocks of size %d:\n", fat->block_count, fat->block_size);
	for (int i=0; i<fat->block_count;++i) {
		printf("%d ", (int)fat->block_map[i]);
	}
	printf("\n");

	for (int i=0; i<fat->files.size(); ++i) {
		mini_file_dump(fat, fat->files[i]);
	}
}

static FAT_FILESYSTEM * mini_fat_create_internal(const char * filename, const int block_size, const int block_count) {
	FAT_FILESYSTEM * fat = new FAT_FILESYSTEM;
	fat->filename = filename;
	fat->block_size = block_size;
	fat->block_count = block_count;
	fat->block_map.resize(fat->block_count, EMPTY_BLOCK); // Set all blocks to empty.
	fat->block_map[0] = METADATA_BLOCK;
	return fat;
}

/**
 * Create a new virtual disk file.
 * The file should be of the exact size block_size * block_count bytes.
 * Overwrites existing files. Resizes block_map to block_count size.
 * @param  filename    name of the file on real disk
 * @param  block_size  size of each block
 * @param  block_count number of blocks
 * @return             FAT_FILESYSTEM pointer with parameters set.
 */
FAT_FILESYSTEM * mini_fat_create(const char * filename, const int block_size, const int block_count) {

	FAT_FILESYSTEM * fat = mini_fat_create_internal(filename, block_size, block_count);

	// TODO: create the corresponding virtual disk file with appropriate size.

	
	FILE *fd = fopen(filename, "wb");

	if(fd == NULL){
		return NULL;
	}

	ftruncate(fileno(fd), block_size*block_count);
	fwrite(&fat->block_size, sizeof(fat->block_size), 1, fd);
	fwrite(&fat->block_count, sizeof(fat->block_count), 1, fd);
	fwrite(&fat->block_map[0], sizeof(fat->block_map[0]), fat->block_count, fd);



	//fseek(fd, block_size*block_count, SEEK_SET);
	//fputc('\0', fd);
	fclose(fd);
	
	//FAT_FILE *fs = mini_file_create_file(fat, filename);
	//fs->size = block_count*block_size;

	//fat->files.push_back(fd);
	return fat;
}

/**
 * Save a virtual disk (filesystem) to file on real disk.
 * Stores filesystem metadata (e.g., block_size, block_count, block_map, etc.)
 * in block 0.
 * Stores file metadata (name, size, block map) in their corresponding blocks.
 * Does not store file data (they are written directly via write API).
 * @param  fat virtual disk filesystem
 * @return     true on success
 */

bool mini_fat_save(const FAT_FILESYSTEM *fat) {
        FILE * fat_fd = fopen(fat->filename, "r+");
        if (fat_fd == NULL) {
                perror("Cannot save fat to file");
                return false;
        }
        // TODO: save all metadata (filesystem metadata, file metadata).
        char bcount[10], bsize[10], buffer[4096];
        sprintf(bcount, "%d", fat->block_count);
        sprintf(bsize, "%d", fat->block_size);
        strcpy(buffer, bcount);
        strcat(buffer, " ");
        strcat(buffer,bsize);


        for(int i=0; i<fat->block_map.size(); i++){
                char num[3]; 
                sprintf(num, "%d ", fat->block_map[i]);
                strcat(buffer," ");
                strcat(buffer, num);
        }
	
        //strcat(buffer, "\n");
	
	fwrite(buffer, 1, strlen(buffer), fat_fd);

        for(int k=0; k<fat->files.size(); k++){
                char fsize[10], namelen[10];
		char buff[4096];
                sprintf(fsize,"%d", fat->files[k]->size);
                sprintf(namelen,"%d", strlen(fat->files[k]->name));
                strcat(buff,fsize);
                strcat(buff, " ");
                strcat(buff,namelen);
                strcat(buff, " ");
                strcat(buff, fat->files[k]->name);

                for(int l=0; l<fat->files[k]->block_ids.size(); l++){
                        char num[10];
                        sprintf(num, "%d ", fat->files[k]->block_ids[l]);
                        strcat(buff," ");
                        strcat(buff, num);
                }
		fseek(fat_fd, fat->files[k]->metadata_block_id*fat->block_size, SEEK_CUR);
		fwrite(buff, 1, strlen(buff), fat_fd);
		memset(buff, 0, 4096);
        }

        fclose(fat_fd);
        return true;
}

FAT_FILESYSTEM * mini_fat_load(const char *filename) {
        FILE * fat_fd = fopen(filename, "r");
        if (fat_fd == NULL) {
                perror("Cannot load fat from file");
                exit(-1);
        }
        // TODO: load all metadata (filesystem metadata, file metadata) and create filesystem.

        int block_size = 1024, block_count = 10;
        FAT_FILESYSTEM * fat = mini_fat_create_internal(filename, block_size, block_count);

        //fseek(fat_fd, 0, SEEK_SET);
        char buffer[1024];
        char *token;
        fread(buffer,1,30, fat_fd);

	//fgets(buffer,1024, fat_fd);

        token = strtok(buffer, " ");
        token = strtok(NULL," ");
        int i = 0;
        int s = 0;
	while(token != NULL){
                token= strtok(NULL," ");
		char buff[1024];
                if(token != NULL){
                        fat->block_map[i] = atoi(token);
			if(atoi(token) == 1){
				s++;
				fseek(fat_fd, i*block_size, SEEK_CUR);
				fread(buff, 1,30, fat_fd);
				printf("b: %s\n",buff);
			}
		}else{
			break;
		}
                i++;
        }
		
	char *t;
	
	for(int k=0; k<s-1; k++){
		fread(buffer, 1, 30,fat_fd);
		printf("Buffer: %s\n", buffer);
		t = strtok(buffer, " ");
		int fsize = atoi(t);
		t = strtok(NULL, " ");
		t = strtok(NULL, " ");
		printf("fname: %s\n", t);
		FAT_FILE *fd = mini_file_create_file(fat,t);
		fd->size = fsize;
		fat->files.push_back(fd);
		while(t != NULL){
			t = strtok(NULL, " ");
			fd->block_ids.push_back(atoi(t));
			fat->files[k]->block_ids.push_back(atoi(t));
		}

	}
	


        fclose(fat_fd);
        return fat;
}
