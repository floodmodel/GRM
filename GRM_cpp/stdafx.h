#pragma once

#include <filesystem>
#include <map>
#include <omp.h>
#include <string>
#include <thread>
#include <vector>
#include <float.h> // MP 추가
#include <sys/stat.h>  // MP 추가
#include <ctime> // MP 추가


//#include <minwindef.h>

#include "bitmap_image.hpp"


#ifdef _WIN32

#include <io.h>

#else

//#include <string>
#include "grmVersion.h"
//#include <string_view>
//#include <cstring>
//#include <locale>

#endif 
