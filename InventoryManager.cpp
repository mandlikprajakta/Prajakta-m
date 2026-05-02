/*
 * InventoryManager.cpp  –  C++ layer
 *
 * Wraps the C backend, adds STL (std::vector + std::sort),
 * and provides all user-facing menu operations.
 */

#include "InventoryManager.h"
#include "inventory.h"

#include <algorithm>   // std::sort
#include <cstring>     // std::strncpy
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

/* ---------------------------------------------------------------
 * Anonymous-namespace helpers (input validation / formatting)
 * --------------------------------------------------------------- */
namespace {

/* Clear a bad std::cin state and discard the rest of the line.   */
void clear_cin()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/* Prompt until the user enters a positive integer.               */
int prompt_positive_id(const std::string &prompt)
{
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v > 0) { clear_cin(); return v; }
        std::cout << "  [!] ID must be a positive integer. Try again.\n";
        clear_cin();
    }
}

/* Prompt until the user enters an integer >= 0.                  */
int prompt_nonneg_int(const std::string &prompt)
{
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0) { clear_cin(); return v; }
        std::cout << "  [!] Value must be 0 or greater. Try again.\n";
        clear_cin();
    }
}

/* Prompt until the user enters a float >= 0.                     */
float prompt_nonneg_float(const std::string &prompt)
{
    float v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0.0f) { clear_cin(); return v; }
        std::cout << "  [!] Price must be 0.00 or greater. Try again.\n";
        clear_cin();
    }
}

/* Prompt until the user enters a non-empty string (max 39 chars).*/
std::string prompt_name(const std::string &prompt)
{
    std::string s;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, s);
        if (!s.empty() && s.size() <= 39) return s;
        if (s.empty())
            std::cout << "  [!] Name must not be empty. Try again.\n";
        else
            std::cout << "  [!] Name too long (max 39 chars). Try again.\n";
    }
}

/* Pretty-print a single Item row.                                */
void print_item_row(const Item &it)
{
    std::cout
        << "  " << std::left
        << std::setw(6)  << it.id
        << std::setw(42) << it.name
        << std::setw(10) << it.quantity
        << "$" << std::fixed << std::setprecision(2) << it.price
        << "\n";
}

void print_table_header()
{
    std::cout
        << "\n  " << std::left
        << std::setw(6)  << "ID"
        << std::setw(42) << "Name"
        << std::setw(10) << "Qty"
        << "Price\n"
        << "  " << std::string(65, '-') << "\n";
}

} // anonymous namespace

/* ---------------------------------------------------------------
 * InventoryManager public interface
 * --------------------------------------------------------------- */

void InventoryManager::run()
{
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout <<   "║   Hybrid Inventory Manager  v1.0     ║\n";
    std::cout <<   "╚══════════════════════════════════════╝\n";

    int choice = 0;
    while (true) {
        print_menu();
        std::cout << "  Choice: ";
        if (!(std::cin >> choice)) { clear_cin(); continue; }
        clear_cin();

        switch (choice) {
            case 1: do_add();    break;
            case 2: do_view();   break;
            case 3: do_update(); break;
            case 4: do_delete(); break;
            case 5: do_list();   break;
            case 6:
                std::cout << "\n  Goodbye!\n\n";
                return;
            default:
                std::cout << "  [!] Please enter a number between 1 and 6.\n";
        }
    }
}

/* ---------------------------------------------------------------
 * Private helpers
 * --------------------------------------------------------------- */

void InventoryManager::print_menu() const
{
    std::cout << "\n"
              << "  ┌─────────────────────────┐\n"
              << "  │  1  Add item             │\n"
              << "  │  2  View item            │\n"
              << "  │  3  Update item          │\n"
              << "  │  4  Delete item          │\n"
              << "  │  5  List all             │\n"
              << "  │  6  Exit                 │\n"
              << "  └─────────────────────────┘\n";
}

void InventoryManager::do_add()
{
    std::cout << "\n--- Add Item ---\n";
    Item it{};
    it.id         = prompt_positive_id ("  ID       : ");
    std::string n = prompt_name        ("  Name     : ");
    std::strncpy(it.name, n.c_str(), 39);
    it.name[39]   = '\0';
    it.quantity   = prompt_nonneg_int  ("  Quantity : ");
    it.price      = prompt_nonneg_float("  Price    : ");
    it.is_deleted = 0;

    if (add_item(it))
        std::cout << "  [OK] Item " << it.id << " added.\n";
    else
        std::cout << "  [!] Failed – ID " << it.id
                  << " may already exist.\n";
}

void InventoryManager::do_view()
{
    std::cout << "\n--- View Item ---\n";
    int id = prompt_positive_id("  ID: ");

    Item it{};
    if (get_item(id, &it)) {
        print_table_header();
        print_item_row(it);
        std::cout << "\n";
    } else {
        std::cout << "  [!] Item " << id
                  << " not found or has been deleted.\n";
    }
}

void InventoryManager::do_update()
{
    std::cout << "\n--- Update Item ---\n";
    int id = prompt_positive_id("  ID to update: ");

    Item existing{};
    if (!get_item(id, &existing)) {
        std::cout << "  [!] Item " << id
                  << " not found or has been deleted.\n";
        return;
    }

    std::cout << "  Current values  →  name='" << existing.name
              << "'  qty=" << existing.quantity
              << "  price=$" << std::fixed << std::setprecision(2)
              << existing.price << "\n"
              << "  (Enter new values)\n";

    Item updated{};
    updated.id          = id;
    std::string n       = prompt_name        ("  New Name     : ");
    std::strncpy(updated.name, n.c_str(), 39);
    updated.name[39]    = '\0';
    updated.quantity    = prompt_nonneg_int  ("  New Quantity : ");
    updated.price       = prompt_nonneg_float("  New Price    : ");
    updated.is_deleted  = 0;

    if (update_item(id, &updated))
        std::cout << "  [OK] Item " << id << " updated.\n";
    else
        std::cout << "  [!] Update failed.\n";
}

void InventoryManager::do_delete()
{
    std::cout << "\n--- Delete Item ---\n";
    int id = prompt_positive_id("  ID to delete: ");

    if (delete_item(id))
        std::cout << "  [OK] Item " << id << " deleted.\n";
    else
        std::cout << "  [!] Item " << id
                  << " not found or already deleted.\n";
}

void InventoryManager::do_list()
{
    std::cout << "\n--- List All Items ---\n";

    const int MAX = 1024;
    std::vector<Item> buf(MAX);

    int count = list_items(buf.data(), MAX);
    if (count == 0) {
        std::cout << "  (no active items)\n";
        return;
    }

    buf.resize(static_cast<std::size_t>(count));

    /* Sort by id (ascending) using std::sort + lambda.            */
    std::sort(buf.begin(), buf.end(),
              [](const Item &a, const Item &b){ return a.id < b.id; });

    print_table_header();
    for (const Item &it : buf) print_item_row(it);
    std::cout << "\n  " << count << " item(s) listed.\n";
}
