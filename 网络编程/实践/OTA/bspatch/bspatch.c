/*-
 * Copyright 2003-2005 Colin Percival
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTprintUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "bspatch.h"
#include "../bzip2/bzlib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// #define BSPATCH_EXECUTABLE

static int64_t offtin(uint8_t *buf)
{
	int64_t y;

	y = buf[7] & 0x7F;
	y = y * 256;
	y += buf[6];
	y = y * 256;
	y += buf[5];
	y = y * 256;
	y += buf[4];
	y = y * 256;
	y += buf[3];
	y = y * 256;
	y += buf[2];
	y = y * 256;
	y += buf[1];
	y = y * 256;
	y += buf[0];

	if (buf[7] & 0x80)
		y = -y;

	return y;
}

int bspatch(const uint8_t *old, int64_t oldsize, uint8_t *new, int64_t newsize, struct bspatch_stream *stream)
{
	uint8_t buf[8];
	int64_t oldpos, newpos;
	int64_t ctrl[3];
	int64_t i;

	oldpos = 0;
	newpos = 0;
	while (newpos < newsize)
	{
		/* Read control data */
		for (i = 0; i <= 2; i++)
		{
			if (stream->read(stream, buf, 8))
				return -1;
			ctrl[i] = offtin(buf);
		};

		/* Sanity-check */
		if (newpos + ctrl[0] > newsize)
			return -1;

		/* Read diff string */
		if (stream->read(stream, new + newpos, ctrl[0]))
			return -1;

		/* Add old data to diff string */
		for (i = 0; i < ctrl[0]; i++)
			if ((oldpos + i >= 0) && (oldpos + i < oldsize))
				new[newpos + i] += old[oldpos + i];

		/* Adjust pointers */
		newpos += ctrl[0];
		oldpos += ctrl[0];

		/* Sanity-check */
		if (newpos + ctrl[1] > newsize)
			return -1;

		/* Read extra string */
		if (stream->read(stream, new + newpos, ctrl[1]))
			return -1;

		/* Adjust pointers */
		newpos += ctrl[1];
		oldpos += ctrl[2];
	};

	return 0;
}


// 解压后读取
static int bz2_read(const struct bspatch_stream *stream, void *buffer, int length)
{
	int n;
	int bz2print;
	BZFILE *bz2;

	bz2 = (BZFILE *)stream->opaque;
	n = BZ2_bzRead(&bz2print, bz2, buffer, length);
	if (n != length)
		return -1;

	return 0;
}

// 直接读取
// static int readFile(const struct bspatch_stream *stream, void *buffer, int length)
// {
//     FILE *pf = (FILE *)stream->opaque;
//     if (fread(buffer, 1, length, pf) != (size_t)length)
//         return -1;
//     return 0;
// }

int getFileSize(FILE *f)
{
	fseek(f, 0, SEEK_END);
	int filesize = ftell(f);
	rewind(f);
	return filesize;
}

int bsPatchFile(const char *oldfile, const char *newfile, const char *patchfile)
{
	printf("%s %s %s\n", oldfile, newfile, patchfile);
	FILE *f;
	FILE *fd;
	int bz2print;
	uint8_t header[24];
	uint8_t *old, *new;
	int64_t oldsize, newsize;
	BZFILE *bz2;
	struct bspatch_stream stream;
	struct stat sb;

	/* Open patch file */
	// 必须加b
	if ((f = fopen(patchfile, "rb+")) == NULL)
		printf("fopen(%s) fail", patchfile);
	if (f == NULL)
	{
		printf("fopen(%s) failed\n", patchfile);
		return -1;
	}


	// printf("filesize = %ld\n", filesize);
	// if (filesize < 24) {
	// 	printf("patch file too small\n");
	// 	fclose(f);
	// 	return -1;
	// }
	/* Read header */
	if (fread(header, sizeof(char), 24, f) != 24)
	{
		if (feof(f))
			printf("Corrupt patch1\n");
		printf("fread(%s)", patchfile);
	}

	/* Check for appropriate magic */
	if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
		printf("Corrupt patch2\n");

	/* Read lengths from header */
	newsize = offtin(header + 16);
	if (newsize < 0)
		printf("Corrupt patch2\n");

	/* Close patch file and re-open it via libbzip2 at the right places */
	if (((fd = fopen(oldfile, "rb+")) < 0) ||
		((oldsize = getFileSize(fd)) == -1) ||
		((old = malloc(oldsize + 1)) == NULL) ||
		(fseek(fd, 0, SEEK_SET) != 0) ||
		(fread(old, sizeof(char), oldsize, fd) != oldsize) ||
		// (fstat(fd, &sb)) ||
		(fclose(fd) == -1))
		printf("%s", oldfile);
	if ((new = malloc(newsize + 1)) == NULL)
		printf("fail malloc\n");
	fflush(fd);
	if (NULL == (bz2 = BZ2_bzReadOpen(&bz2print, f, 0, 0, NULL, 0)))
		printf("BZ2_bzReadOpen, bz2print=%d", bz2print);

	stream.read = bz2_read;
	stream.opaque = bz2;
	if (bspatch(old, oldsize, new, newsize, &stream))
		printf("bspatch");

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&bz2print, bz2);
	fclose(f);

	/* Write the new file */
	if (((fd = fopen(newfile, "wb+")) < 0) ||
		(fwrite(new, sizeof(char), newsize, fd) != newsize) || (fclose(fd) == -1))
		printf("%s", newfile);
	fflush(fd);

	free(new);
	free(old);

	return 0;
};

#if defined(BSPATCH_EXECUTABLE)

// 根据补丁文件和旧版本文件生成新版本文件
int main(int argc, char *argv[])
{
	FILE *f;
	int fd;
	int bz2print;
	uint8_t header[24];  // 补丁文件的头部
	uint8_t *old, *new;
	int64_t oldsize, newsize;
	BZFILE *bz2;  // bzip2文件指针
	struct bspatch_stream stream;  // bspatch用于提取补丁的流结构体
	struct stat sb;  // 旧文件的状态结构体，生成新文件时让新文件继承旧文件的权限

	if (argc != 4)
		printf("usage: %s oldfile newfile patchfile\n", argv[0]);

	/* Open patch file */
	if ((f = fopen(argv[3], "r")) == NULL)
		printf("fopen(%s)", argv[3]);

	/* Read header */
	if (fread(header, 1, 24, f) != 24)
	{
		if (feof(f))
			printf("Corrupt patch\n");
		printf("fread(%s)", argv[3]);
	}

	/* Check for appropriate magic */
	if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
		printf("Corrupt patch\n");

	/* Read lengths from header */
	newsize = offtin(header + 16);
	if (newsize < 0)
		printf("Corrupt patch\n");

	/* Close patch file and re-open it via libbzip2 at the right places */
	if (((fd = open(argv[1], O_RDONLY, 0)) < 0) ||
		((oldsize = lseek(fd, 0, SEEK_END)) == -1) ||
		((old = malloc(oldsize + 1)) == NULL) ||
		(lseek(fd, 0, SEEK_SET) != 0) ||
		(read(fd, old, oldsize) != oldsize) ||
		(fstat(fd, &sb)) ||
		(close(fd) == -1))
		printf("%s", argv[1]);
	if ((new = malloc(newsize + 1)) == NULL)
		printf("fail malloc");

	// 初始化读bzip2
	if (NULL == (bz2 = BZ2_bzReadOpen(&bz2print, f, 0, 0, NULL, 0)))
		printf("BZ2_bzReadOpen, bz2print=%d", bz2print);

	// 应用补丁
	stream.read = bz2_read;
	// stream.read = readFile;
	stream.opaque = bz2;
	if (bspatch(old, oldsize, new, newsize, &stream))
		printf("bspatch");

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&bz2print, bz2);
	fclose(f);

	// 以原文件权限创建新文件（如已存在则覆盖）
	if (
		((fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, sb.st_mode)) < 0) ||
		(write(fd, new, newsize) != newsize) ||
		(close(fd) == -1))
		printf("%s", argv[2]);

	free(new);
	free(old);

	return 0;
}

#endif
