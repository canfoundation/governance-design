cmake_minimum_required(VERSION 3.5)
project(community VERSION 1.0.0)

find_package(eosio.cdt)

add_contract( community community code.cpp position.cpp badge.cpp community.cpp )
