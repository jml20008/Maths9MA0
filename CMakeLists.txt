# Compile using `fxsdk build-cg` (normal cmake alone does not configure toolchain).
cmake_minimum_required(VERSION 3.15)
project(Maths9MA0 LANGUAGES C)
include(GenerateG3A)
find_package(Gint 2.1 REQUIRED)
add_executable(maths9ma0 src/main.c src/app_data.c src/engine.c)
target_compile_features(maths9ma0 PRIVATE c_std_11)
target_compile_options(maths9ma0 PRIVATE -Os -Wall -Wextra)
target_link_libraries(maths9ma0 Gint::Gint)
generate_g3a(TARGET maths9ma0 OUTPUT "MATHS9MA0.g3a"
    NAME "Maths9MA0" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
