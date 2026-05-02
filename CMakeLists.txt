cmake_minimum_required(VERSION 3.10)
project(HybridInventoryManager LANGUAGES C CXX)

# ── Standards ─────────────────────────────────────────────────
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

# ── Include path ──────────────────────────────────────────────
include_directories(include)

# ── Sources ───────────────────────────────────────────────────
set(C_SOURCES
    src/inventory.c
)

set(CXX_SOURCES
    src/InventoryManager.cpp
    src/main.cpp
)

# ── Target ────────────────────────────────────────────────────
add_executable(inventory_manager
    ${C_SOURCES}
    ${CXX_SOURCES}
)

# Compile warnings
target_compile_options(inventory_manager PRIVATE
    $<$<COMPILE_LANGUAGE:C>:  -Wall -Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra>
)
