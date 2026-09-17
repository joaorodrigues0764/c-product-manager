# C Billing & Inventory System

A high-performance command-line billing and inventory management system written entirely in C. This project handles product registration, dynamic shopping cart management, VAT calculations, and invoice generation, designed to simulate a real-world Point of Sale (POS) backend.

## 🚀 Features

- **Inventory Management:** Register up to 10,000 unique products with details including description, EAN-8/EAN-13 barcode, price, stock quantity, and VAT class.
- **Barcode Validation:** Built-in EAN-8 and EAN-13 checksum validation to ensure data integrity during product entry.
- **Dynamic Shopping Cart:** Add or remove items from an active shopping cart with automatic stock tracking.
- **Invoicing System:** Generate chronological invoices tied to specific customer NIFs (Tax Identification Numbers) and names.
- **Custom VAT Rates:** Support for external configuration files mapping VAT classes (e.g., A, B, C, D) to specific percentage rates.
- **Memory Efficient:** Dynamically allocates memory only when needed and ensures clean memory release upon exit.

## 🛠️ Architecture

The system is modularized into a standard C project structure for maintainability and scalability:

- `src/` - Contains the implementation files (`.c`), handling logic for products, invoices, cart operations, and the main entry point.
- `include/` - Contains the header files (`.h`) defining the data structures and function prototypes.

## ⚙️ Build and Run

The project includes a `Makefile` for easy compilation.

### Prerequisites
- GCC Compiler (version 7 or higher recommended)
- Make

### Compilation
To build the project, simply run the following command in the root directory:

```bash
make
```
This will compile the source files and generate an executable named `c-billing-system`.

### Execution
Run the system via the terminal:

```bash
./c-billing-system
```
You can also run it by piping an input file containing a sequence of commands:
```bash
./c-billing-system < inputs.txt
```

### Cleanup
To remove the compiled executable and object files:
```bash
make clean
```

## 📋 Commands Overview

The system interacts via single-letter commands followed by necessary arguments. Here are the core commands supported:

| Command | Action |
| :---: | :--- |
| **`p`** | Register or update a product (`p <ean> <vat_class> <price> <qty> <description>`) |
| **`l`** | List available products in the system. Supports wildcard (`*`, `?`) filtering. |
| **`a`** | Add or remove items from the current shopping cart (`a [qty] <ean>`) |
| **`r`** | Display global billing summary or check stock for a specific product. |
| **`f`** | Checkout the cart and generate an invoice (`f [nif] <customer_name>`) |
| **`c`** | List all generated invoices, optionally filtered by a specific customer. |
| **`d`** | Delete a specific invoice or reduce stock of a product. |
| **`q`** | Quit the program and free all allocated memory. |

## 🛡️ Quality Assurance

The code is strictly validated using:
- **Valgrind & AddressSanitizer:** To guarantee zero memory leaks and safe memory access patterns.
- **Compiler Flags:** Compiled with `-O3 -Wall -Wextra -Werror` to ensure optimized, warning-free code.
