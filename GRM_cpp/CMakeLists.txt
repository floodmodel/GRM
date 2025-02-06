# CMakeList.txt: GRM_cpp에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#
cmake_minimum_required (VERSION 3.10)

project ("GRM_cpp")
set(CMAKE_CXX_STANDARD 17)


add_subdirectory(../../gentle/cpp  # 이건 폴더 이름
                       ./build/gentleLib_out #gentle 폴더의 내용(CMakeList.txt)를 빌드한 결과물을 배치할 위치

) 

find_package(OpenMP REQUIRED)


set(SOURCES  
    stdafx.h         
    grm.h
    grmVersion.h # grm 버전 명시

    main.cpp

    domain.cpp
    flowControl.cpp
    network.cpp
    project.cpp
    setupModel.cpp
    simulator.cpp
    soilLayer.cpp
    solver.cpp
    surfalceLayer.cpp

    output.cpp
    outputRaster.cpp

    weatherData.cpp

       )


# Release 모드 설정
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable(GRM_Linux.exe ${SOURCES})


# my_lib 라이브러리를 링크
target_link_libraries(GRM_Linux.exe 
    PRIVATE gentleLib 
    OpenMP::OpenMP_CXX
    )



# TODO: 필요한 경우 테스트를 추가하고 대상을 설치합니다.
