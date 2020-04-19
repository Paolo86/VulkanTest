#pragma once
#include <fstream>
#include <vector>
#include <stdlib.h>
#include "Logger.h"

class FileUtils
{
public:
	static std::vector<char> ReadFile(const std::string path)
	{
		FILE *fp;
		long lSize;
		char *buffer;

		fp = fopen(path.c_str(), "rb");

		if (fp == NULL)
		{
			Logger::LogError("Could not open", path);
		}

		fseek(fp, 0L, SEEK_END);
		lSize = ftell(fp);
		rewind(fp);

		/* allocate memory for entire content */
		buffer = (char*)calloc(1, lSize + 1);

		/* copy the file into the buffer */
		if (1 != fread(buffer, lSize, 1, fp))
			fclose(fp), free(buffer), Logger::LogError("Failed to read",path), exit(1);

		/* do your work here, buffer is a string contains the whole text */
		std::vector<char> result;
		for (int i = 0; i < lSize; i++)
		{
			result.push_back(buffer[i]);
		}

		fclose(fp);
		free(buffer);

		return result;
	}
};