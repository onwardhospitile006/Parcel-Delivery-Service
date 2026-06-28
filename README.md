# Parcel Delivery Tracking System

A **multi-process Parcel Delivery Tracking System** developed in **C** to demonstrate core **Operating System concepts** including role-based access control, file locking, shared memory, semaphores, inter-process communication (IPC) and signal handling.

---

## Features

### Admin

* Login as administrator
* Register and remove delivery agents
* Add, assign, view, and delete parcels
* View parcel count per agent

### Agent

* Login using agent credentials
* View assigned parcels
* Update parcel status
* Update parcel location
* View pending deliveries

### Customer

* Self-register
* Login using customer credentials
* View and track own parcels
* Mark parcel as delivered
* Delete own account

---

## Operating System Concepts Used

* **Role-Based Access Control**

  * Separate privileges for Admin, Agent, and Customer.

* **POSIX File Locking**

  * Uses `fcntl()` advisory locks to prevent concurrent file corruption.

* **System V Shared Memory**

  * Stores parcel data in shared memory for efficient concurrent access.

* **System V Semaphores**

  * Synchronizes access to shared parcel data and prevents race conditions.

* **Inter-Process Communication (IPC)**

  * Multiple processes communicate through shared memory and semaphores.

* **Signal Handling**

  * Handles `SIGINT` for graceful shutdown and automatic data persistence.

* **Persistent Storage**

  * User and parcel information are stored in text files.

---

## Project Structure

```text
parcelproject/
│
├── src/
│   ├── main.c
│   ├── auth.c
│   ├── dispatcher.c
│   ├── agent.c
│   ├── customer.c
│   ├── file_db.c
│   ├── ipc.c
│   └── sync.c
│
├── include/
│   ├── auth.h
│   ├── customer.h
│   ├── dispatcher.h
│   ├── file_db.h
│   ├── ipc.h
│   ├── structures.h
│   └── sync.h
│
├── data/
│   ├── users.txt
│   └── parcels.txt
│
├── Makefile
└── README.md
```

---

## Build

Compile the project using:

```bash
make
```

Or manually:

```bash
gcc src/*.c -Iinclude -o parcel_tracker
```

---

## Run

```bash
./parcel_tracker
```

---

## Default Admin Credentials

```text
Username : admin
Password : admin
```

---

## User Roles

| Role         | Description                        |
| ------------ | ---------------------------------- |
| **Admin**    | Manage agents and parcels          |
| **Agent**    | Update parcel status and location  |
| **Customer** | Track parcels and confirm delivery |

---

## Parcel Workflow

```text
Unassigned
      │
      ▼
 Assigned
      │
      ▼
 InTransit
      │
      ▼
 Delivered
```

### Delivery Locations

```text
ParcelWarehouse
        ↓
Shipping
        ↓
DestinationWarehouse
        ↓
OutForDelivery
        ↓
DestinationReached
```

---

## Synchronization

### User Data

* Stored in `data/users.txt`
* Protected using POSIX `fcntl()` file locks.

### Parcel Data

* Stored in System V shared memory.
* Protected using System V semaphores.

---

## Data Persistence

* Parcel data is automatically saved to `data/parcels.txt` during program termination.
* A `SIGINT` handler ensures graceful shutdown and cleanup.

---

## Requirements

* GCC Compiler
* GNU Make
* Linux / Unix Operating System
