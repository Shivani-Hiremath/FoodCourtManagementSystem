# 🍕 Food Court Management System  

## 📌 Overview  
The **Food Court Management System** is a C++ project built using **Object-Oriented Programming (OOP)** principles. It simulates a food court environment with customers, stalls, menu items, orders, logging, and admin functionality. The system demonstrates advanced C++ features such as **inheritance, polymorphism, interfaces, smart pointers, exception handling, file persistence, and the Singleton pattern**.  

---

## 🚀 Features  
- 👤 **Authentication**: Separate flows for **Admin** and **Customer**.  
- 🏪 **Stall Management**: Pizza, Dosa, Sandwich, and Beverage stalls, each with its own menu.  
- 📦 **Order Management**: Customers can place orders with multiple items and quantities.  
- 💾 **File Persistence**: All customers, orders, and sales data are saved and loaded from files.  
- 📝 **Logging**: Centralized logging of system events and exceptions using **Singleton Logger**.  
- ⚡ **Smart Pointers**: Uses `shared_ptr` and `unique_ptr` for safe and efficient memory management.  
- 📊 **Reports**: Admin can view total sales and generate reports.  

---

## 🛠️ Technical Concepts Used  

### 1. **Design Patterns**
- **Singleton Pattern** → `Logger` ensures only one logging instance exists.  
- **Factory Method** → Stalls create items dynamically via `createItem`.  
- **Facade Pattern** → `FoodCourtManager` controls the flow between Admin and Customer.  

### 2. **OOP Features**
- **Inheritance** → `Person → Customer, Stall`; `Stall → PizzaStall, DosaStall…`.  
- **Polymorphism** → `IItem` interface implemented by `Pizza, Dosa, Sandwich, Beverage`.  
- **Encapsulation** → Protected/private members with public methods for access.  
- **Abstract Classes** → `Stall`, `IItem`, and custom exception hierarchy.  

### 3. **Smart Pointers**
- **`shared_ptr<IItem>`** → Used for managing order items (multiple references across system).  
- **`unique_ptr<Stall>`** → Used by `FoodCourtManager` to manage stall objects.  

### 4. **Exception Handling**
- Custom exceptions:  
  - `ChoiceException` → Invalid menu choice  
  - `QuantityException` → Invalid order quantity  
  - `FileException` → File read/write failure  

### 5. **File Handling**
- Orders, customers, and counters saved in text files.  
- Data persistence allows the system to resume state across runs.  

---

## 📂 Class Structure  

1. **Logger (Singleton)** → Centralized event logging  
2. **FoodCourtException (Base Exception)** → ChoiceException, QuantityException, FileException  
3. **AuthService** → Handles login and customer data persistence  
4. **Person (Base Class)** → Common attributes for `Customer` and `Stall`  
5. **Customer (Inherits Person)** → Represents a customer, maintains ID  
6. **IItem (Interface)** → Abstract representation of a menu item  
7. **Pizza, Dosa, Sandwich, Beverage** → Implement `IItem`  
8. **Order** → Represents a customer’s order, calculates totals  
9. **Stall (Abstract, Inherits Person)** → Base for all stalls  
10. **PizzaStall, DosaStall, SandwichStall, BeverageStall** → Concrete stalls  
11. **FoodCourtManager** → Facade controlling entire application  

---

## ⚙️ How It Works (Flow)  

1. **Start Application** → `FoodCourtManager` initialized.  
2. **Authentication** → `AuthService` prompts for role (Admin / Customer).  
   - Admin → Admin menu (reports, sales).  
   - Customer → Registers/logs in, proceeds to stalls.  
3. **Ordering Process**  
   - Customer selects a stall.  
   - Stall shows menu (`displayMenu`).  
   - Customer picks an item and quantity.  
   - `Order::addItem(shared_ptr<IItem>, quantity)` called.  
   - Items saved to file and sales updated.  
4. **Logging** → Every step logged by `Logger` (Singleton).  
5. **Exit** → All data persisted (customers, orders, sales).  

---

## 🖥️ Tech Stack  
- **Language**: C++17  
- **Paradigm**: Object-Oriented Programming  
- **Libraries**: Standard Template Library (STL), `<memory>`, `<fstream>`, `<exception>`  
- **Concepts**: Smart Pointers, Exception Handling, File I/O, Polymorphism, Design Patterns  

---

## 📊 Example Use Case  

1. Customer logs in with name & phone.  
2. Selects **Pizza Stall**.  
3. Orders:  
   - 1 × Margherita Pizza  
   - 1 × Veg Sandwich  
   - 2 × Margherita Pizza  
4. System internally:  
   - Creates **shared_ptr<Pizza> and shared_ptr<Sandwich>** for items.  
   - Stores them in `Order` with quantities.  
   - Logs events via `Logger`.  
   - Saves order to customer file + central file.  

---

## 📜 How to Run  
```bash
g++ -std=c++17 *.cpp -o foodcourt
./foodcourt
