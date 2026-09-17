# C Billing & Inventory System

A modular command-line billing and inventory system written in C. The project
manages products, stock, shopping carts, VAT rates, customers, and invoices
using dynamic memory and linked data structures.

## Features

- Product registration and stock management for up to 10,000 products.
- EAN-8 and EAN-13 checksum validation.
- Shopping cart management with stock tracking.
- Invoice generation with customer NIFs and names.
- VAT rates loaded from a configuration file.
- Product filtering with `*` and `?` wildcards.
- Invoice listing, filtering, deletion, and client NIF updates.
- Explicit memory management with cleanup on normal termination.

## Project Structure

```text
.
├── include/              # Public headers and shared data structures
├── src/                  # Application source code
├── tests/                # Regression inputs and expected outputs
├── .github/workflows/    # Continuous integration
├── .gitignore
├── Makefile
└── README.md
```

## Build

Requirements:

- GCC
- Make

Build the project from the repository root:

```bash
make
```

This creates the `c-billing-system` executable.

Run it directly or pipe commands from a file:

```bash
./c-billing-system
./c-billing-system < input.txt
```

An optional VAT configuration file can be passed as the first argument:

```bash
./c-billing-system path/to/vat.txt
```

Clean generated build artifacts with:

```bash
make clean
```

## Tests

The repository contains a regression suite covering product validation,
inventory and cart operations, invoicing, client filtering, and NIF updates.

Run the complete suite with:

```bash
make test
```

The tests are also executed automatically by GitHub Actions on pushes and
pull requests.

## Command Overview

| Command | Description |
| :---: | :--- |
| `p` | Register a product or update an existing product. |
| `l` | List products, optionally filtered by EAN wildcards. |
| `a` | Add or remove quantities from the active cart; without arguments, list the cart. |
| `r` | Show global billing information or product stock information. |
| `f` | Create an invoice for the current cart. |
| `c` | List invoices, optionally filtered by amount and/or client. |
| `v` | Update the NIF associated with a client. |
| `d` | Reduce product stock or delete an invoice. |
| `q` | Exit and release allocated memory. |

## Technical Focus

The implementation uses modular C source files, header interfaces, linked
lists for carts and invoices, fixed-size product storage, dynamic allocation
for variable-length data, input parsing, barcode validation, and explicit
resource cleanup.
