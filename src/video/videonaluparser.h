#pragma once

#include "videoh264.h"
#include <stdint.h>
#include <vector>

class VideoNaluParser {
public:
	VideoNaluParser();

public:
	static void 	parse(const char* data, int size, std::vector<NaluItem>& nalus);

private:
	static int 	find_nal_unit(char* buf, int size, int* nal_start, int* nal_end, int* type);
	static int	get_nal_type(char* buf);
	static void	dump_8bytes(char* buf);

};


