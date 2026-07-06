# DataVault — Personal Data Store

A record-oriented database storage engine built from scratch in **C**, implementing indexed CRUD operations, multi-table management, foreign-key style relations, and dynamic schema persistence — entirely over raw file I/O, with no external database library.

The project is structured as four progressive stages (`PDS1` → `PDS4`), each adding a layer of database functionality on top of the last.

---

## Stages

### PDS1 — Single-Table Storage Engine
- Implements a single-table key-value store backed by two files: a data file (`.dat`) holding fixed-size records, and an index file (`.ndx`) holding an in-memory-mirrored array of `Rec_ndx` structs (`old_key`, `key`, `loc`, `is_deleted`).
- Core operations: `create_db`, `open_db`, `store_db`, `get_db`, `update_db`, `delete_db`, `undelete_db`, `close_db`.
- Soft deletion via an `is_deleted` flag rather than physically removing records, preserving record slots for `undelete_db`.

### PDS2 — Multi-Table Support
- Extends the engine to manage two tables simultaneously via a `DBInfo` struct wrapping an array of `TableInfo` structs.
- Table-scoped versions of all CRUD operations (`store_table`, `get_table`, `update_table`, `delete_table`, `undelete_table`), each parameterized by table name.

### PDS3 — Relations Between Tables
- Introduces `RelInfo` and `RelPair` (`pkey`, `fkey`, `is_deleted`) to model foreign-key style relationships between two tables (e.g. student ↔ course).
- Adds relation lifecycle operations: `create_relation`, `open_relation`, `store_relation`, `get_relation`, `delete_relation`, `undelete_relation`, `close_relation`.

### PDS4 — Dynamic Schema & Querying
- Generalizes the engine to an arbitrary number of tables and relations, using dynamically allocated arrays (`malloc`-based `TableInfo*` and `RelInfo*`) instead of fixed-size structs.
- Adds **schema persistence** — `save_schema` / `load_schema` — so a database's structure survives across program runs without being hardcoded.
- Adds **field-based lookup** — `get_table_by_field`, allowing a record to be retrieved by matching an arbitrary struct field (via byte offset and size) rather than only by primary key, i.e. a basic non-key query capability.
- Ships with a working example schema (`univ.sch`) and populated data/index files (`Student.dat`, `Course.dat`, etc.) demonstrating a university-style Student/Course database.

---

## Architecture

```
Application (tester code)
        │
        ▼
   DataVault Engine (pds4.c)
        │
   ┌────┴─────┐
   ▼          ▼
Data file   Index file
(.dat)      (.ndx)
  raw         in-memory array of
  fixed-      {old_key, key, loc,
  size        is_deleted} per record,
  records     flushed to disk on close
```

- Each table's index is loaded fully into memory (`ndxarray`) on `open_table` and written back on `close_table`, trading memory for fast in-memory key lookups instead of scanning the data file directly.
- Records are located by key via a linear scan of the in-memory index array, which maps a key to a byte offset (`loc`) in the `.dat` file.
- Relations are stored as flat files of `(pkey, fkey)` pairs, letting one table's records be traversed via another's, independent of physical storage order.

---

## Building

Each stage is self-contained in its own directory (`PDS1`, `PDS2`, `PDS3`, `PDS4`):

```bash
cd PDS4
gcc -o pds4 pds4.c pds4_tester.c
./pds4
```

## Tech Highlights
- Manual binary file I/O (`fopen`, `fread`, `fwrite`, `fseek`) — no ORM, no external DB engine.
- Custom index structure for O(1)-ish in-memory key lookup instead of full-file scans.
- Soft-delete/undelete pattern preserving data integrity.
- Schema serialization for persistent, non-hardcoded database structure.
- Progressive design across four stages, each a strict superset of the previous stage's capability.
