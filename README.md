# Hybrid Inventory Manager

A console-based inventory manager that combines a **C backend** (binary file I/O with `fread`/`fwrite`/`fseek`) with a **C++ frontend** (STL `std::vector`, `std::sort`, classes, and an interactive menu).

---

## Project Structure

```
hybrid-inventory-manager/
├── include/
│   ├── inventory.h          # Shared C struct + function declarations
│   └── InventoryManager.h   # C++ class declaration
├── src/
│   ├── inventory.c          # C backend  – file storage
│   ├── InventoryManager.cpp # C++ layer  – menu + STL
│   └── main.cpp             # Entry point
├── Makefile
├── CMakeLists.txt
└── README.md
```

---

## Build & Run

### Option A – Make (recommended)

```bash
# Inside the project directory
make          # compiles everything into ./inventory_manager
./inventory_manager
```

To clean build artefacts:
```bash
make clean
```

### Option B – CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
./inventory_manager
```

> **Requirements**: GCC / G++ with C11 and C++17 support (any modern GCC ≥ 7 or Clang ≥ 5).

---

## Data Persistence

All records are stored in **`inventory.dat`** (created automatically in the working directory on first run). The file is a flat binary array of `Item` structs; each record is a fixed `sizeof(Item)` bytes, which allows direct `fseek`-based access by record index.

Deleted items are **soft-deleted** (`is_deleted = 1`); their slot is retained in the file so existing offsets never move.

---

## Menu

```
  ┌─────────────────────────┐
  │  1  Add item             │
  │  2  View item            │
  │  3  Update item          │
  │  4  Delete item          │
  │  5  List all             │
  │  6  Exit                 │
  └─────────────────────────┘
```

---

## C / C++ Responsibilities

| Concern | Layer |
|---|---|
| Binary file read / write / seek | C (`inventory.c`) |
| Duplicate ID detection | C (`add_item`) |
| Soft-delete logic | C (`delete_item`) |
| Interactive menu & input validation | C++ (`InventoryManager.cpp`) |
| Temporary item buffer | C++ `std::vector<Item>` |
| Sorted listing | C++ `std::sort` with lambda |

---

## Test Cases

### Test 1 – Persistence across restart
- **Steps**: Add items with IDs 1, 2, 3. Choose **Exit**. Relaunch the program. Choose **List all**.
- **Expected**: All three items appear with the same names, quantities, and prices entered.
- **Result**: ✅ Pass

### Test 2 – Update persists after restart
- **Steps**: Add item ID 10 (name="Widget", qty=5, price=1.99). Update ID 10 (qty=50, price=2.49). Exit. Relaunch. View item 10.
- **Expected**: qty=50, price=$2.49.
- **Result**: ✅ Pass

### Test 3 – Deleted item hidden from list and view
- **Steps**: Add item ID 7. Delete item 7. Choose **List all**; then choose **View item** → ID 7.
- **Expected**: Item 7 does not appear in the list; View returns "not found or has been deleted."
- **Result**: ✅ Pass

### Test 4 – Duplicate ID rejected
- **Steps**: Add item ID 5. Attempt to add another item with ID 5.
- **Expected**: Second add prints a failure message; only one record exists for ID 5.
- **Result**: ✅ Pass

### Test 5 – Input validation
- **Steps**: At "Add item", enter ID = -3, then ID = 0, then ID = 1 (valid). Enter an empty name, then a valid name. Enter qty = -1, then 0 (valid). Enter price = -5.0, then 0.0 (valid).
- **Expected**: Each invalid entry displays an error and re-prompts; the item is only created after all fields are valid.
- **Result**: ✅ Pass

---

## Acceptance Checklist

- [x] Add 3 items → exit → rerun → **List all** shows all 3
- [x] Update an item → exit → rerun → updated values persist
- [x] Deleted items do **not** appear in List or View
- [x] Duplicate IDs are rejected with an error message
