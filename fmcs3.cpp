#include <iostream>
#include <exception>
#include <memory>
#include <vector>
#include <map>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <sstream>
#include <limits>
#include <algorithm>

using namespace std;

// Singleton Logger for logging exceptions and activities
class Logger {
private:
    static Logger* instance;
    ofstream logFile;

    Logger() {
        logFile.open("food_court_log.txt", ios::app);
        if (!logFile.is_open()) {
            cerr << "Error opening log file!" << endl;
        }
    }

public:
    static Logger* getInstance() {
        if (!instance) {
            instance = new Logger();
        }
        return instance;
    }

    void log(const string& message) {
        time_t now = time(nullptr);
        string timeStr = ctime(&now);
        timeStr.erase(timeStr.length() - 1); // Remove newline
        logFile << "[" << timeStr << "] " << message << endl;
        cout << "[LOG] " << message << endl;
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
};
Logger* Logger::instance = nullptr;

// Custom Exception Hierarchy
class FoodCourtException : public exception {
public:
    virtual const char* what() const throw() = 0;
};

class ChoiceException : public FoodCourtException {
public:
    const char* what() const throw() override {
        return "You have entered wrong choice\nPlease enter choice between 1 to 8";
    }
};

class QuantityException : public FoodCourtException {
public:
    const char* what() const throw() override {
        return "Invalid quantity entered. Please enter a positive number.";
    }
};

class FileException : public FoodCourtException {
public:
    const char* what() const throw() override {
        return "File operation failed!";
    }
};

class AuthException : public FoodCourtException {
public:
    const char* what() const throw() override {
        return "Authentication failed! Invalid credentials.";
    }
};

// Role Enumeration
enum class UserRole {
    ADMIN,
    CUSTOMER,
    UNKNOWN
};

// Authentication Service
class AuthService {
private:
    static const string ADMIN_USERNAME;
    static const string ADMIN_PASSWORD;
    map<string, string> customers; // phone -> name

public:
    AuthService() {
        loadCustomers();
    }

    UserRole authenticate() {
        int roleChoice;
        cout << "\n===== Authentication =====\n";
        cout << "1. Admin Login\n";
        cout << "2. Customer Login\n";
        cout << "Enter your choice: ";
        cin >> roleChoice;
        cin.ignore();

        if (roleChoice == 1) {
            return adminLogin();
        } else if (roleChoice == 2) {
            return customerLogin();
        } else {
            throw ChoiceException();
        }
    }

    UserRole adminLogin() {
        string username, password;
        cout << "Enter admin username: ";
        getline(cin, username);
        cout << "Enter admin password: ";
        getline(cin, password);

        if (username == ADMIN_USERNAME && password == ADMIN_PASSWORD) {
            Logger::getInstance()->log("Admin logged in");
            return UserRole::ADMIN;
        }
        throw AuthException();
    }

    UserRole customerLogin() {
        string name, phone;
        cout << "Enter your name: ";
        getline(cin, name);
        cout << "Enter your phone number: ";
        getline(cin, phone);

        // Check if customer exists
        if (customers.find(phone) == customers.end()) {
            customers[phone] = name;
            saveCustomers();
            Logger::getInstance()->log("New customer registered: " + name + " (" + phone + ")");
        } else {
            Logger::getInstance()->log("Existing customer logged in: " + name + " (" + phone + ")");
        }

        return UserRole::CUSTOMER;
    }

    void loadCustomers() {
        ifstream file("customers.txt");
        if (file.is_open()) {
            string phone, name;
            while (getline(file, phone)) { // Fixed missing closing parenthesis
                if (getline(file, name)) {
                    customers[phone] = name;
                }
            }
            file.close();
        }
    }

    void saveCustomers() {
        ofstream file("customers.txt");
        if (!file.is_open()) {
            throw FileException();
        }
        for (const auto& customer : customers) {
            file << customer.first << endl;
            file << customer.second << endl;
        }
        file.close();
    }
};

const string AuthService::ADMIN_USERNAME = "admin";
const string AuthService::ADMIN_PASSWORD = "admin@12345";

// Person class (Base class)
class Person {
protected:
    string name;
    string contactNumber;

public:
    virtual void getInfo() {
        cout << "Enter your name: ";
        getline(cin, name);
        cout << "Enter your contact number: ";
        getline(cin, contactNumber);
    }

    virtual void displayInfo() const {
        cout << "Name: " << name << endl;
        cout << "Contact Number: " << contactNumber << endl;
    }

    virtual void saveToFile(ofstream& file) const {
        file << name << endl;
        file << contactNumber << endl;
    }

    virtual void loadFromFile(ifstream& file) {
        getline(file, name);
        getline(file, contactNumber);
    }

    virtual ~Person() = default;
};

// Customer class
class Customer : public Person {
private:
    static int customerCounter;
    const int customerId;

public:
    Customer() : customerId(++customerCounter) {}
    public:
    string getName() const {
        return name;
    }

    string getContactNumber() const {
        return contactNumber;
    }
    void getInfo() override {
        Person::getInfo();
        Logger::getInstance()->log("New customer created: " + name + " (" + contactNumber + ")");
        saveCustomerToFile();
    }

    void displayInfo() const override {
        cout << "Customer ID: " << customerId << endl;
        Person::displayInfo();
    }

    void saveToFile(ofstream& file) const override {
        Person::saveToFile(file);
        file << customerId << endl;
    }

    void loadFromFile(ifstream& file) override {
        Person::loadFromFile(file);
        int id;
        file >> id;
        // customerId is const, can't be modified after construction
        file.ignore();
    }

    static int getTotalCustomers() {
        loadCustomerCounter();
        return customerCounter;
    }

    static void saveCustomerCounter() {
        ofstream file("customer_counter.txt");
        if (!file.is_open()) {
            throw FileException();
        }
        file << customerCounter;
        file.close();
    }

    static void loadCustomerCounter() {
        ifstream file("customer_counter.txt");
        if (file.is_open()) {
            file >> customerCounter;
            file.close();
        }
    }

private:
    void saveCustomerToFile() const {
        ofstream file("customer_data.txt", ios::app);
        if (!file.is_open()) {
            throw FileException();
        }
        saveToFile(file);
        file.close();
        saveCustomerCounter();
    }
};
int Customer::customerCounter = 0;

// Item Interface
class IItem {
public:
    virtual string getName() const = 0;
    virtual float getPrice() const = 0;
    virtual void saveToFile(ofstream& file) const = 0;
    virtual ~IItem() = default;
};

// Concrete Items
class Pizza : public IItem {
private:
    string flavor;
    string size;
    float price;

public:
    Pizza(const string& f, const string& s, float p) : flavor(f), size(s), price(p) {}

    string getName() const override {
        return size + " " + flavor + " Pizza";
    }

    float getPrice() const override {
        return price;
    }

    void saveToFile(ofstream& file) const override {
        file << "PIZZA" << endl;
        file << flavor << endl;
        file << size << endl;
        file << price << endl;
    }
};

class Dosa : public IItem {
private:
    string type;
    float price;

public:
    Dosa(const string& t, float p) : type(t), price(p) {}

    string getName() const override {
        return type + " Dosa";
    }

    float getPrice() const override {
        return price;
    }

    void saveToFile(ofstream& file) const override {
        file << "DOSA" << endl;
        file << type << endl;
        file << price << endl;
    }
};

class Sandwich : public IItem {
private:
    string type;
    float price;

public:
    Sandwich(const string& t, float p) : type(t), price(p) {}

    string getName() const override {
        return type + " Sandwich";
    }

    float getPrice() const override {
        return price;
    }

    void saveToFile(ofstream& file) const override {
        file << "SANDWICH" << endl;
        file << type << endl;
        file << price << endl;
    }
};

class Beverage : public IItem {
private:
    string type;
    float price;

public:
    Beverage(const string& t, float p) : type(t), price(p) {}

    string getName() const override {
        return type;
    }

    float getPrice() const override {
        return price;
    }

    void saveToFile(ofstream& file) const override {
        file << "BEVERAGE" << endl;
        file << type << endl;
        file << price << endl;
    }
};

// Order class
class Order {
private:
    vector<pair<shared_ptr<IItem>, int>> items; // Item and quantity
    static int orderCounter;
    const int orderId;
    time_t orderTime;
    string customerName;
    string customerPhone;

public:
    Order(const string& name = "", const string& phone = "") 
        : orderId(++orderCounter), customerName(name), customerPhone(phone) {
        orderTime = time(nullptr);
        saveOrderCounter();
    }

    void addItem(shared_ptr<IItem> item, int quantity) {
        if (quantity <= 0) {
            throw QuantityException();
        }
        items.emplace_back(item, quantity);
    }

    float calculateTotal() const {
        float total = 0.0f;
        for (const auto& item : items) {
            total += item.first->getPrice() * item.second;
        }
        return total;
    }

    void displayOrder() const {
        cout << "\n========= Order #" << orderId << " =========" << endl;
        cout << "Customer: " << customerName << endl;
        cout << "Phone: " << customerPhone << endl;
        cout << "Order Time: " << ctime(&orderTime);
        cout << "---------------------------------" << endl;
        
        for (const auto& item : items) {
            cout << left << setw(25) << item.first->getName() 
                 << " x " << setw(3) << item.second 
                 << " = Rs. " << item.first->getPrice() * item.second << endl;
        }
        
        cout << "---------------------------------" << endl;
        cout << right << setw(35) << "Total: Rs. " << calculateTotal() << endl;
        cout << "=================================\n" << endl;
    }

    void saveToFile(ofstream& file) const {
        file << orderId << endl;
        file << orderTime << endl;
        file << customerName << endl;
        file << customerPhone << endl;
        file << items.size() << endl;
        
        for (const auto& item : items) {
            item.first->saveToFile(file);
            file << item.second << endl;
        }
    }

    void saveToCentralFile() const {
        ofstream file("all_orders.txt", ios::app);
        if (!file.is_open()) {
            throw FileException();
        }
        
        file << "========= Order #" << orderId << " =========" << endl;
        file << "Customer: " << customerName << endl;
        file << "Phone: " << customerPhone << endl;
        file << "Order Time: " << ctime(&orderTime);
        file << "---------------------------------" << endl;
        
        for (const auto& item : items) {
            file << left << setw(25) << item.first->getName() 
                 << " x " << setw(3) << item.second 
                 << " = Rs. " << item.first->getPrice() * item.second << endl;
        }
        
        file << "---------------------------------" << endl;
        file << right << setw(35) << "Total: Rs. " << calculateTotal() << endl;
        file << "=================================\n" << endl;
        file.close();
    }

    static int getTotalOrders() {
        loadOrderCounter();
        return orderCounter;
    }

    static void saveOrderCounter() {
        ofstream file("order_counter.txt");
        if (!file.is_open()) {
            throw FileException();
        }
        file << orderCounter;
        file.close();
    }

    static void loadOrderCounter() {
        ifstream file("order_counter.txt");
        if (file.is_open()) {
            file >> orderCounter;
            file.close();
        }
    }

    static shared_ptr<IItem> createItemFromFile(ifstream& file) {
        string type;
        getline(file, type);
        
        if (type == "PIZZA") {
            string flavor, size;
            float price;
            getline(file, flavor);
            getline(file, size);
            file >> price;
            file.ignore();
            return make_shared<Pizza>(flavor, size, price);
        }
        else if (type == "DOSA") {
            string dosaType;
            float price;
            getline(file, dosaType);
            file >> price;
            file.ignore();
            return make_shared<Dosa>(dosaType, price);
        }
        else if (type == "SANDWICH") {
            string sandwichType;
            float price;
            getline(file, sandwichType);
            file >> price;
            file.ignore();
            return make_shared<Sandwich>(sandwichType, price);
        }
        else if (type == "BEVERAGE") {
            string beverageType;
            float price;
            getline(file, beverageType);
            file >> price;
            file.ignore();
            return make_shared<Beverage>(beverageType, price);
        }
        
        return nullptr;
    }
};
int Order::orderCounter = 0;

// Abstract Stall class (Template Method Pattern)
class Stall : public Person {
protected:
    string stallName;
    int stallNumber;
    static float totalSales;
    vector<Order> orders;

    virtual void displayMenu() const = 0;
    virtual shared_ptr<IItem> createItem(int choice) = 0;

public:
    Stall(const string& name, int number) : stallName(name), stallNumber(number) {
        loadOrders();
    }

    void takeOrder(const string& customerName, const string& customerPhone) {
        char moreItems;
        Order currentOrder(customerName, customerPhone);
        
        do {
            system("cls"); // Clear screen
            displayMenu();
            cout << "Enter your choice: ";
            int choice;
            cin >> choice;
            cin.ignore(); // Clear input buffer
            
            try {
                shared_ptr<IItem> item = createItem(choice);
                
                cout << "Enter quantity: ";
                int quantity;
                cin >> quantity;
                cin.ignore();
                
                if (quantity <= 0) {
                    throw QuantityException();
                }
                
                currentOrder.addItem(item, quantity);
                
                cout << "Add more items? (Y/N): ";
                cin >> moreItems;
                cin.ignore();
            } catch (const FoodCourtException& e) {
                Logger::getInstance()->log(e.what());
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                moreItems = 'Y';
            }
        } while (toupper(moreItems) == 'Y');
        
        orders.push_back(currentOrder);
        totalSales += currentOrder.calculateTotal();
        currentOrder.displayOrder();
        currentOrder.saveToCentralFile(); // Save to central orders file
        saveOrders(); // Save to stall-specific file
    }

    void displayStallInfo() const {
        cout << "\nStall Name: " << stallName << endl;
        cout << "Stall Number: " << stallNumber << endl;
        cout << "Total Orders: " << orders.size() << endl;
        cout << "Total Sales: Rs. " << getTotalSales() << endl;
    }

    float getTotalSales() const {
        float sales = 0.0f;
        for (const auto& order : orders) {
            sales += order.calculateTotal();
        }
        return sales;
    }

    string getStallName() const { return stallName; }

    static float getOverallSales() {
        loadTotalSales();
        return totalSales;
    }

    void saveToFile(ofstream& file) const {
        file << stallName << endl;
        file << stallNumber << endl;
        file << orders.size() << endl;
        
        for (const auto& order : orders) {
            order.saveToFile(file);
        }
    }

    void loadFromFile(ifstream& file) {
        getline(file, stallName);
        file >> stallNumber;
        file.ignore();
        
        int orderCount;
        file >> orderCount;
        file.ignore();
        
        orders.clear();
        for (int i = 0; i < orderCount; i++) {
            int orderId;
            time_t orderTime;
            string customerName, customerPhone;
            file >> orderId;
            file >> orderTime;
            file.ignore();
            getline(file, customerName);
            getline(file, customerPhone);
            
            Order order(customerName, customerPhone);
            
            int itemCount;
            file >> itemCount;
            file.ignore();
            
            for (int j = 0; j < itemCount; j++) {
                auto item = Order::createItemFromFile(file);
                int quantity;
                file >> quantity;
                file.ignore();
                if (item) {
                    order.addItem(item, quantity);
                }
            }
            
            orders.push_back(order);
        }
    }

protected:
    void saveOrders() const {
        ofstream file(getStallFileName());
        if (!file.is_open()) {
            throw FileException();
        }
        saveToFile(file);
        file.close();
        saveTotalSales();
    }

    void loadOrders() {
        ifstream file(getStallFileName());
        if (file.is_open()) {
            loadFromFile(file);
            file.close();
        }
    }

    string getStallFileName() const {
        return "stall_" + to_string(stallNumber) + ".txt";
    }

    static void saveTotalSales() {
        ofstream file("total_sales.txt");
        if (!file.is_open()) {
            throw FileException();
        }
        file << totalSales;
        file.close();
    }

    static void loadTotalSales() {
        ifstream file("total_sales.txt");
        if (file.is_open()) {
            file >> totalSales;
            file.close();
        }
    }

public:
    virtual ~Stall() {
        saveOrders();
    }
};
float Stall::totalSales = 0.0f;

// Concrete Stall classes
class PizzaStall : public Stall {
private:
    map<int, pair<string, float>> pizzaFlavors = {
        {1, {"Margherita", 0}},
        {2, {"Pepperoni", 0}},
        {3, {"Vegetarian", 0}},
        {4, {"Hawaiian", 0}}
    };
    
    map<int, pair<string, float>> sizes = {
        {1, {"Small", 200}},
        {2, {"Medium", 350}},
        {3, {"Large", 500}}
    };

    void displayMenu() const override {
        cout << "\n===== " << stallName << " Menu =====" << endl;
        cout << "Pizza Flavors:" << endl;
        for (const auto& flavor : pizzaFlavors) {
            cout << flavor.first << ". " << flavor.second.first << endl;
        }
        
        cout << "\nSizes:" << endl;
        for (const auto& size : sizes) {
            cout << size.first << ". " << size.second.first << " - Rs. " << size.second.second << endl;
        }
    }

    shared_ptr<IItem> createItem(int choice) override {
        int flavorChoice, sizeChoice;
        
        cout << "Select flavor (1-4): ";
        cin >> flavorChoice;
        cin.ignore();
        
        if (pizzaFlavors.find(flavorChoice) == pizzaFlavors.end()) {
            throw ChoiceException();
        }
        
        cout << "Select size (1-3): ";
        cin >> sizeChoice;
        cin.ignore();
        
        if (sizes.find(sizeChoice) == sizes.end()) {
            throw ChoiceException();
        }
        
        string name = pizzaFlavors.at(flavorChoice).first;
        string size = sizes.at(sizeChoice).first;
        float price = sizes.at(sizeChoice).second;
        
        return make_shared<Pizza>(name, size, price);
    }

public:
    PizzaStall() : Stall("Pizza Palace", 1) {}
};

class DosaStall : public Stall {
private:
    map<int, pair<string, float>> dosaTypes = {
        {1, {"Plain Dosa", 50}},
        {2, {"Masala Dosa", 70}},
        {3, {"Cheese Dosa", 90}},
        {4, {"Onion Dosa", 60}}
    };

    void displayMenu() const override {
        cout << "\n===== " << stallName << " Menu =====" << endl;
        for (const auto& dosa : dosaTypes) {
            cout << dosa.first << ". " << dosa.second.first << " - Rs. " << dosa.second.second << endl;
        }
    }

    shared_ptr<IItem> createItem(int choice) override {
        if (dosaTypes.find(choice) == dosaTypes.end()) {
            throw ChoiceException();
        }
        return make_shared<Dosa>(dosaTypes.at(choice).first, dosaTypes.at(choice).second);
    }

public:
    DosaStall() : Stall("Dosa Corner", 2) {}
};

class SandwichStall : public Stall {
private:
    map<int, pair<string, float>> sandwichTypes = {
        {1, {"Veg Sandwich", 60}},
        {2, {"Cheese Sandwich", 80}},
        {3, {"Grilled Sandwich", 100}},
        {4, {"Club Sandwich", 120}}
    };

    void displayMenu() const override {
        cout << "\n===== " << stallName << " Menu =====" << endl;
        for (const auto& sandwich : sandwichTypes) {
            cout << sandwich.first << ". " << sandwich.second.first << " - Rs. " << sandwich.second.second << endl;
        }
    }

    shared_ptr<IItem> createItem(int choice) override {
        if (sandwichTypes.find(choice) == sandwichTypes.end()) {
            throw ChoiceException();
        }
        return make_shared<Sandwich>(sandwichTypes.at(choice).first, sandwichTypes.at(choice).second);
    }

public:
    SandwichStall() : Stall("Sandwich Delight", 3) {}
};

class BeverageStall : public Stall {
private:
    map<int, pair<string, float>> beverageTypes = {
        {1, {"Tea", 15}},
        {2, {"Coffee", 20}},
        {3, {"Cold Coffee", 40}},
        {4, {"Green Tea", 25}}
    };

    void displayMenu() const override {
        cout << "\n===== " << stallName << " Menu =====" << endl;
        for (const auto& beverage : beverageTypes) {
            cout << beverage.first << ". " << beverage.second.first << " - Rs. " << beverage.second.second << endl;
        }
    }

    shared_ptr<IItem> createItem(int choice) override {
        if (beverageTypes.find(choice) == beverageTypes.end()) {
            throw ChoiceException();
        }
        return make_shared<Beverage>(beverageTypes.at(choice).first, beverageTypes.at(choice).second);
    }

public:
    BeverageStall() : Stall("Chai Point", 4) {}
};

// Food Court Manager (Facade Pattern)
class FoodCourtManager {
private:
    vector<unique_ptr<Stall>> stalls;
    unique_ptr<Customer> currentCustomer;
    AuthService authService;
    UserRole currentRole;
    string currentCustomerName;
    string currentCustomerPhone;

    void initializeStalls() {
        stalls.push_back(make_unique<PizzaStall>());
        stalls.push_back(make_unique<DosaStall>());
        stalls.push_back(make_unique<SandwichStall>());
        stalls.push_back(make_unique<BeverageStall>());
    }

    void displayCustomerMenu() const {
        cout << "\n======== Customer Menu ========" << endl;
        cout << "1. Pizza Stall" << endl;
        cout << "2. Dosa Stall" << endl;
        cout << "3. Sandwich Stall" << endl;
        cout << "4. Beverage Stall" << endl;
        cout << "8. Exit" << endl;
        cout << "==============================" << endl;
    }

    void displayAdminMenu() const {
        cout << "\n======== Admin Menu ========" << endl;
        cout << "5. View Stall Information" << endl;
        cout << "6. View Total Sales" << endl;
        cout << "7. Generate Sales Report" << endl;
        cout << "8. Exit" << endl;
        cout << "===========================" << endl;
    }

    void generateSalesReport() const {
        ofstream report("sales_report.txt");
        if (!report.is_open()) {
            throw FileException();
        }

        time_t now = time(nullptr);
        report << "Sales Report - " << ctime(&now) << endl;
        report << "============================================\n";
        
        for (const auto& stall : stalls) {
            report << "Stall: " << stall->getStallName() << endl;
            report << "Total Sales: Rs. " << stall->getTotalSales() << endl;
            report << "--------------------------------------------\n";
        }
        
        report << "\nGRAND TOTAL: Rs. " << Stall::getOverallSales() << endl;
        report << "Total Customers: " << Customer::getTotalCustomers() << endl;
        report << "Total Orders: " << Order::getTotalOrders() << endl;
        report << "============================================" << endl;
        
        report.close();
        cout << "Sales report generated successfully!\n";
    }

public:
    FoodCourtManager() {
        Customer::loadCustomerCounter();
        Order::loadOrderCounter();
        Stall::getOverallSales(); // This loads the total sales
        
        initializeStalls();
        currentRole = authService.authenticate();
        
        if (currentRole == UserRole::CUSTOMER) {
            currentCustomer = make_unique<Customer>();
            currentCustomerName = currentCustomer->getName();
            currentCustomerPhone = currentCustomer->getContactNumber();
        }
    }

    void runCustomer() {
        int choice;
        while (true) {
            system("cls");
            displayCustomerMenu();
            cout << "Enter your choice: ";
            cin >> choice;
            cin.ignore();

            try {
                switch (choice) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        stalls[choice - 1]->takeOrder(currentCustomerName, currentCustomerPhone);
                        cout << "Press Enter to continue...";
                        cin.ignore();
                        break;
                    case 8:
                        cout << "Thank you for visiting!" << endl;
                        Customer::saveCustomerCounter();
                        return;
                    default:
                        throw ChoiceException();
                }
            } catch (const FoodCourtException& e) {
                Logger::getInstance()->log(e.what());
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Press Enter to continue...";
                cin.ignore();
            }
        }
    }

    void runAdmin() {
        int choice;
        while (true) {
            system("cls");
            displayAdminMenu();
            cout << "Enter your choice: ";
            cin >> choice;
            cin.ignore();

            try {
                switch (choice) {
                    case 5:
                        for (const auto& stall : stalls) {
                            stall->displayStallInfo();
                        }
                        cout << "Press Enter to continue...";
                        cin.ignore();
                        break;
                    case 6:
                        cout << "\nTotal Sales across all stalls: Rs. " << Stall::getOverallSales() << endl;
                        cout << "Total Customers: " << Customer::getTotalCustomers() << endl;
                        cout << "Total Orders: " << Order::getTotalOrders() << endl;
                        cout << "Press Enter to continue...";
                        cin.ignore();
                        break;
                    case 7:
                        generateSalesReport();
                        cout << "Press Enter to continue...";
                        cin.ignore();
                        break;
                    case 8:
                        cout << "Logging out..." << endl;
                        return;
                    default:
                        throw ChoiceException();
                }
            } catch (const FoodCourtException& e) {
                Logger::getInstance()->log(e.what());
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Press Enter to continue...";
                cin.ignore();
            }
        }
    }

    void run() {
        if (currentRole == UserRole::ADMIN) {
            runAdmin();
        } else if (currentRole == UserRole::CUSTOMER) {
            runCustomer();
        }
    }
};

int main() {
    try {
        FoodCourtManager manager;
        manager.run();
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    return 0;
}