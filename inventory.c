/*
 * inventory.c  –  C backend
 * Handles all binary file I/O for the Hybrid Inventory Manager.
 *
 * File layout: flat sequence of Item records, fixed size.
 * fseek is used for O(1) access by record index.
 */

#include "inventory.h"

#include <stdio.h>
#include <string.h>

#define DATA_FILE "inventory.dat"
#define RECORD_SIZE ((long)sizeof(Item))

/* ---------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------- */

/* Open the data file.  mode follows fopen() conventions.        */
static FILE *open_file(const char *mode)
{
    return fopen(DATA_FILE, mode);
}

/* Count the total number of records (active + deleted) in file. */
static long record_count(FILE *fp)
{
    fseek(fp, 0L, SEEK_END);
    return ftell(fp) / RECORD_SIZE;
}

/* Read a single record by its 0-based index.                    */
static int read_record(FILE *fp, long index, Item *out)
{
    if (fseek(fp, index * RECORD_SIZE, SEEK_SET) != 0) return 0;
    return (fread(out, RECORD_SIZE, 1, fp) == 1);
}

/* Write a single record at its 0-based index.                   */
static int write_record(FILE *fp, long index, const Item *item)
{
    if (fseek(fp, index * RECORD_SIZE, SEEK_SET) != 0) return 0;
    return (fwrite(item, RECORD_SIZE, 1, fp) == 1);
}

/* Return the 0-based file index of the first record whose id
 * matches, or -1 if not found.                                  */
static long find_index_by_id(FILE *fp, int id)
{
    long count = record_count(fp);
    Item tmp;
    for (long i = 0; i < count; ++i) {
        if (!read_record(fp, i, &tmp)) continue;
        if (tmp.id == id) return i;
    }
    return -1L;
}

/* ---------------------------------------------------------------
 * Public C API
 * --------------------------------------------------------------- */

/*
 * add_item – append a new record.
 * Returns 0 if the ID already exists (active or deleted) or if the
 * file cannot be opened.
 */
int add_item(const Item item)
{
    /* Check for duplicate ID (including soft-deleted records so IDs
     * stay truly unique across the lifetime of the file).          */
    FILE *fp = open_file("rb");
    if (fp) {
        long idx = find_index_by_id(fp, item.id);
        fclose(fp);
        if (idx >= 0) return 0;   /* duplicate */
    }

    /* Append the new record.  "ab" always writes at end-of-file.   */
    fp = open_file("ab");
    if (!fp) return 0;
    int ok = (fwrite(&item, RECORD_SIZE, 1, fp) == 1);
    fclose(fp);
    return ok;
}

/*
 * get_item – fetch one active item by id into *out.
 * Returns 0 if not found or is soft-deleted.
 */
int get_item(int id, Item *out)
{
    FILE *fp = open_file("rb");
    if (!fp) return 0;

    long idx = find_index_by_id(fp, id);
    if (idx < 0) { fclose(fp); return 0; }

    Item tmp;
    int ok = read_record(fp, idx, &tmp);
    fclose(fp);

    if (!ok || tmp.is_deleted) return 0;
    *out = tmp;
    return 1;
}

/*
 * update_item – overwrite an existing active record in-place.
 * The id field of *updated must match the id parameter;
 * is_deleted is forced to 0 to prevent accidental un-deletion path.
 */
int update_item(int id, const Item *updated)
{
    FILE *fp = open_file("r+b");
    if (!fp) return 0;

    long idx = find_index_by_id(fp, id);
    if (idx < 0) { fclose(fp); return 0; }

    Item existing;
    if (!read_record(fp, idx, &existing) || existing.is_deleted) {
        fclose(fp);
        return 0;
    }

    /* Build the record to write – preserve the original id.        */
    Item to_write = *updated;
    to_write.id         = id;
    to_write.is_deleted = 0;

    int ok = write_record(fp, idx, &to_write);
    fclose(fp);
    return ok;
}

/*
 * delete_item – soft delete: set is_deleted = 1.
 * Returns 0 if not found or already deleted.
 */
int delete_item(int id)
{
    FILE *fp = open_file("r+b");
    if (!fp) return 0;

    long idx = find_index_by_id(fp, id);
    if (idx < 0) { fclose(fp); return 0; }

    Item tmp;
    if (!read_record(fp, idx, &tmp) || tmp.is_deleted) {
        fclose(fp);
        return 0;
    }

    tmp.is_deleted = 1;
    int ok = write_record(fp, idx, &tmp);
    fclose(fp);
    return ok;
}

/*
 * list_items – fill buffer with up to max_items active records.
 * Returns the number of active records copied.
 */
int list_items(Item *buffer, int max_items)
{
    FILE *fp = open_file("rb");
    if (!fp) return 0;

    long count = record_count(fp);
    int  found = 0;

    for (long i = 0; i < count && found < max_items; ++i) {
        Item tmp;
        if (!read_record(fp, i, &tmp)) continue;
        if (tmp.is_deleted) continue;
        buffer[found++] = tmp;
    }

    fclose(fp);
    return found;
}
