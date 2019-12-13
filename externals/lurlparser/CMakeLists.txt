add_library(lurlparser
        LUrlParser/LUrlParser.cpp
        LUrlParser/LUrlParser.h
)

create_target_directory_groups(lurlparser)

target_include_directories(lurlparser INTERFACE ./LUrlParser)
