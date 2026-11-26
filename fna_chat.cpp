// FNA CHAT - A TERMINAL BASED MESSAGING SYSTEM

// Course: Data Structure (CSE 221)
// Assignment: Build a terminal-based application with 4-5 features

// Features:
//   - User Authentication (Register/Login/Logout)
//   - Private and Group Chat
//   - Message Inbox with Queue
//   - Undo Delete with Stack
//   - Friend Management with Linked List
//   - User Search with Binary Search Tree
//   - Fast Login with Hash Table
//   - Admin Panel
//   - Password Recovery
//   - Message Forwarding and Search
//   - Back ('b') and Menu ('m') option at every step

#define _CRT_SECURE_NO_WARNINGS   // lets Visual Studio use ctime() and localtime()

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cctype>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX    // stops windows.h from breaking std::min / std::max
    #endif
    #include <windows.h>    // For color on Windows
#else
    // For Linux/Mac - ANSI color codes
    #include <unistd.h>     // For usleep()
    #define SetConsoleTextAttribute(h, c)
#endif

using namespace std;

//  COLOR CODES 
// For better UI experience

enum Color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7,
    GRAY = 8,
    BRIGHT_BLUE = 9,
    BRIGHT_GREEN = 10,
    BRIGHT_CYAN = 11,
    BRIGHT_RED = 12,
    BRIGHT_MAGENTA = 13,
    BRIGHT_YELLOW = 14,
    BRIGHT_WHITE = 15
};

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void setColor(int color) {
    WORD attributes = 0;

    switch(color) {
        case BLACK: attributes = 0; break;
        case BLUE: attributes = 1; break;
        case GREEN: attributes = 2; break;
        case CYAN: attributes = 3; break;
        case RED: attributes = 4; break;
        case MAGENTA: attributes = 5; break;
        case YELLOW: attributes = 6; break;
        case WHITE: attributes = 7; break;
        case GRAY: attributes = 8; break;
        case BRIGHT_BLUE: attributes = 9; break;
        case BRIGHT_GREEN: attributes = 10; break;
        case BRIGHT_CYAN: attributes = 11; break;
        case BRIGHT_RED: attributes = 12; break;
        case BRIGHT_MAGENTA: attributes = 13; break;
        case BRIGHT_YELLOW: attributes = 14; break;
        case BRIGHT_WHITE: attributes = 15; break;
        default: attributes = 7; break;
    }

    SetConsoleTextAttribute(hConsole, attributes);
}
#else
void setColor(int color) {
    int ansiCode;
    switch(color) {
        case BLACK: ansiCode = 30; break;
        case BLUE: ansiCode = 34; break;
        case GREEN: ansiCode = 32; break;
        case CYAN: ansiCode = 36; break;
        case RED: ansiCode = 31; break;
        case MAGENTA: ansiCode = 35; break;
        case YELLOW: ansiCode = 33; break;
        case WHITE: ansiCode = 37; break;
        case GRAY: ansiCode = 90; break;
        case BRIGHT_BLUE: ansiCode = 94; break;
        case BRIGHT_GREEN: ansiCode = 92; break;
        case BRIGHT_CYAN: ansiCode = 96; break;
        case BRIGHT_RED: ansiCode = 91; break;
        case BRIGHT_MAGENTA: ansiCode = 95; break;
        case BRIGHT_YELLOW: ansiCode = 93; break;
        case BRIGHT_WHITE: ansiCode = 97; break;
        default: ansiCode = 37; break;
    }
    cout << "\033[1;" << ansiCode << "m";
}
#endif

void resetColor() {
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 7);
#else
    cout << "\033[0m";
#endif
}

//  DATE/TIME UTILITY 
string getCurrentTimestamp() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timestamp(dt);
    timestamp.pop_back(); // Remove newline
    return timestamp;
}

string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << ltm->tm_mday << "/" << (1 + ltm->tm_mon) << "/" << (1900 + ltm->tm_year);
    return ss.str();
}

//  INPUT HELPERS (NEW) 
// Every input is read as a full line. Because of this:
//   - typing letters in a menu can no longer cause an endless loop
//   - names and group names with spaces work correctly
//   - "Press Enter" never needs to be pressed twice
// At every step the user can type  b = go back one step,  m = go to the menu

enum NavAction {
    NAV_OK,    // the user typed a normal value
    NAV_BACK,  // the user typed b
    NAV_MENU   // the user typed m
};

string trimText(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

string toLowerText(string text) {
    for (char& c : text) c = (char)tolower((unsigned char)c);
    return text;
}

void printError(const string& text) {
    setColor(BRIGHT_RED);
    cout << "   [ERROR] " << text << "\n";
    resetColor();
}

void printSuccess(const string& text) {
    setColor(BRIGHT_GREEN);
    cout << "   [SUCCESS] " << text << "\n";
    resetColor();
}

void printInfo(const string& text) {
    setColor(BRIGHT_CYAN);
    cout << "   [INFO] " << text << "\n";
    resetColor();
}

void showNavHint() {
    setColor(GRAY);
    cout << "   (type 'b' to go back, 'm' for the menu)\n";
    resetColor();
}

// Reads one line. Returns NAV_BACK for "b" and NAV_MENU for "m".
NavAction readLine(const string& prompt, string& output) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) exit(0);   // input closed (data is saved after every change)
    line = trimText(line);
    string lower = toLowerText(line);
    if (lower == "b") return NAV_BACK;
    if (lower == "m") return NAV_MENU;
    output = line;
    return NAV_OK;
}

// Reads one word with no spaces (usernames, passwords, emails)
NavAction readWord(const string& prompt, string& output) {
    while (true) {
        string text;
        NavAction action = readLine(prompt, text);
        if (action != NAV_OK) return action;
        if (text.empty()) {
            printError("This cannot be empty.");
            continue;
        }
        if (text.find(' ') != string::npos) {
            printError("Spaces are not allowed here.");
            continue;
        }
        output = text;
        return NAV_OK;
    }
}

// Reads a whole number between minValue and maxValue
NavAction readNumber(const string& prompt, int minValue, int maxValue, int& output) {
    while (true) {
        string text;
        NavAction action = readLine(prompt, text);
        if (action != NAV_OK) return action;

        stringstream ss(text);
        int value;
        char extra;
        if (ss >> value && !(ss >> extra) && value >= minValue && value <= maxValue) {
            output = value;
            return NAV_OK;
        }
        printError("Please enter a number from " + to_string(minValue) + " to " + to_string(maxValue) + ".");
    }
}

// Reads a yes / no answer
NavAction readYesNo(const string& prompt, bool& yes) {
    while (true) {
        string text;
        NavAction action = readLine(prompt, text);
        if (action != NAV_OK) return action;

        string lower = toLowerText(text);
        if (lower == "y" || lower == "yes") { yes = true; return NAV_OK; }
        if (lower == "n" || lower == "no") { yes = false; return NAV_OK; }
        printError("Please type y or n.");
    }
}

void pressEnterToContinue() {
    cout << "\n   Press Enter to continue...";
    string dummy;
    if (!getline(cin, dummy)) exit(0);
}

// Usernames: 5-20 letters, numbers or _ (keeps the save files safe)
bool isValidUsername(const string& username) {
    if (username.length() < 5 || username.length() > 20) return false;
    for (unsigned char c : username) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidEmail(const string& email) {
    size_t at = email.find('@');
    if (at == string::npos || at == 0 || email.find('@', at + 1) != string::npos) return false;
    size_t dot = email.find('.', at + 2);
    return dot != string::npos && dot < email.length() - 1;
}

// The save files use '|' between fields, so '|' and '%' inside a field are
// written as %7C and %25. This way messages can contain any character.
string encodeField(const string& text) {
    string result;
    for (char c : text) {
        if (c == '%') result += "%25";
        else if (c == '|') result += "%7C";
        else result += c;
    }
    return result;
}

string decodeField(const string& text) {
    string result;
    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == '%' && text.compare(i, 3, "%25") == 0) { result += '%'; i += 2; }
        else if (text[i] == '%' && text.compare(i, 3, "%7C") == 0) { result += '|'; i += 2; }
        else result += text[i];
    }
    return result;
}

// Splits a saved line into its fields
vector<string> splitLine(const string& line) {
    vector<string> parts;
    string part;
    stringstream ss(line);
    while (getline(ss, part, '|')) parts.push_back(part);
    if (!line.empty() && line.back() == '|') parts.push_back("");
    return parts;
}

//  MESSAGE STRUCTURE 
struct Message {
    string sender;
    string text;
    string timestamp;
    bool isRead;
    Message* next;

    Message(string s, string t) : sender(s), text(t), next(nullptr) {
        timestamp = getCurrentTimestamp();
        isRead = false;
    }
};

//  QUEUE FOR INBOX (FIFO) 
// Feature: Messages are stored in Queue - first sent, first seen
class MessageQueue {
private:
    Message* front;
    Message* rear;
    int size;

public:
    MessageQueue() : front(nullptr), rear(nullptr), size(0) {}

    // Enqueue - add message to inbox
    void enqueue(string sender, string text) {
        Message* newMsg = new Message(sender, text);
        enqueueMessage(newMsg);
    }

    // NEW: add an existing message node to the end of the queue.
    // Used when loading from file and for undo, so the original time
    // and read status of the message are kept.
    void enqueueMessage(Message* msg) {
        msg->next = nullptr;
        if (rear == nullptr) {
            front = rear = msg;
        } else {
            rear->next = msg;
            rear = msg;
        }
        size++;
    }

    // Dequeue - remove and return oldest message
    Message* dequeue() {
        if (front == nullptr) return nullptr;
        Message* temp = front;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        size--;
        return temp;
    }

    // Peek at oldest message without removing
    Message* peek() {
        return front;
    }

    bool isEmpty() {
        return front == nullptr;
    }

    int getSize() {
        return size;
    }

    // Display all messages (unread ones are marked [NEW], then marked as read)
    void displayMessages() {
        if (isEmpty()) {
            setColor(RED);
            cout << "   [INFO] No messages in inbox.\n";
            resetColor();
            return;
        }

        Message* current = front;
        int msgNum = 1;
        while (current != nullptr) {
            setColor(CYAN);
            cout << "   " << msgNum << ". ";
            setColor(YELLOW);
            cout << "From: " << current->sender;
            setColor(WHITE);
            cout << " | " << current->timestamp;
            if (!current->isRead) {
                setColor(BRIGHT_RED);
                cout << "  [NEW]";
            }
            cout << "\n";
            setColor(GREEN);
            cout << "      Message: " << current->text << "\n";
            resetColor();
            current->isRead = true;
            current = current->next;
            msgNum++;
        }
    }

    // Get message at index (for delete operation)
    Message* getMessageAtIndex(int index) {
        if (index < 0 || index >= size){
            return nullptr;
        }
        Message* current = front;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
        return current;
    }

    // Remove message at index (returns the removed message)
    Message* removeAtIndex(int index) {
        if (index < 0 || index >= size){
            return nullptr;
        }

        if (index == 0) {
            return dequeue();
        }

        Message* prev = front;
        for (int i = 0; i < index - 1; i++) {
            prev = prev->next;
        }

        Message* toRemove = prev->next;
        prev->next = toRemove->next;
        if (toRemove == rear) {
            rear = prev;
        }
        size--;
        toRemove->next = nullptr;
        return toRemove;
    }

    // Mark message as read
    void markAsRead(int index){
        if (index < 0 || index >= size){
            setColor(BRIGHT_RED);
            cout << "   [ERROR] Invalid message number!\n";
            resetColor();
            return;
        }

        Message* current = front;
        for (int i = 0; i < index; i++){
            current = current->next;
        }

        current->isRead = true;
        setColor(BRIGHT_GREEN);
        cout << "   [SUCCESS] Message marked as read!\n";
        resetColor();
    }

    // Mark message as unread
    void markAsUnread(int index){
        if (index < 0 || index >= size){
            setColor(BRIGHT_RED);
            cout << "   [ERROR] Invalid message number!\n";
            resetColor();
            return;
        }

        Message* current = front;
        for (int i = 0; i < index; i++){
            current = current->next;
        }

        current->isRead = false;
        setColor(BRIGHT_GREEN);
        cout << "   [SUCCESS] Message marked as unread!\n";
        resetColor();
    }

    // Save to file (prefix tells whose inbox or which group the message belongs to)
    void saveToFile(ofstream& file, const string& prefix){
        Message* current = front;
        while (current != nullptr){
            file << prefix << "|" << encodeField(current->sender) << "|" << encodeField(current->text) << "|"
                 << current->timestamp << "|" << current->isRead << "\n";
            current = current->next;
        }
    }

    // Search messages by keyword and display results (not case-sensitive)
    void searchAndDisplay(string keyword){
        if (isEmpty()){
            setColor(BRIGHT_RED);
            cout << "   [INFO] No messages to search.\n";
            resetColor();
            return;
        }

        string lowerKeyword = toLowerText(keyword);
        Message* current = front;
        int found = 0;

        while (current != nullptr){
            // Search in sender name and message text
            size_t foundInSender = toLowerText(current->sender).find(lowerKeyword);
            size_t foundInText = toLowerText(current->text).find(lowerKeyword);

            if (foundInSender != string::npos || foundInText != string::npos){
                found++;
                setColor(BRIGHT_GREEN);
                cout << "\n   📧 Message #" << found << ":\n";
                resetColor();
                setColor(BRIGHT_CYAN);
                cout << "   From: ";
                setColor(BRIGHT_YELLOW);
                cout << current->sender << "\n";
                setColor(BRIGHT_CYAN);
                cout << "   Time: ";
                setColor(BRIGHT_WHITE);
                cout << current->timestamp << "\n";
                setColor(BRIGHT_CYAN);
                cout << "   Message: ";
                setColor(BRIGHT_GREEN);

                // Highlight the keyword
                string msgText = current->text;
                string lowerText = toLowerText(msgText);
                size_t start = 0, pos;
                while ((pos = lowerText.find(lowerKeyword, start)) != string::npos){
                    cout << msgText.substr(start, pos - start);
                    setColor(BRIGHT_RED);
                    cout << msgText.substr(pos, keyword.length());
                    setColor(BRIGHT_GREEN);
                    start = pos + keyword.length();
                }
                cout << msgText.substr(start) << "\n";
                resetColor();

                setColor(BRIGHT_CYAN);
                cout << "   Status: ";

                if (current->isRead){
                    setColor(BRIGHT_GREEN);
                    cout << "✓ Read\n";
                } else{
                    setColor(BRIGHT_YELLOW);
                    cout << "○ Unread\n";
                }
                resetColor();
            }
            current = current->next;
        }

        if (found == 0){
            setColor(BRIGHT_RED);
            cout << "   ❌ No messages found containing \"" << keyword << "\"\n";
            resetColor();
        } else{
            setColor(BRIGHT_GREEN);
            cout << "\n   ✅ Found " << found << " message(s) containing \"" << keyword << "\"\n";
            resetColor();
        }
    }

    // Get count of unread messages by traversing the queue
    int getUnreadCount(){
        int unreadCount = 0;
        Message* current = front;  // Start from the front

        // Traverse through all messages in the linked list
        while (current != nullptr){
            if (!current->isRead){  // If message is not read
                unreadCount++;
            }
            current = current->next;  // Move to next message
        }
        return unreadCount;
    }

    ~MessageQueue(){
        while (!isEmpty()){
            delete dequeue();
        }
    }
};

//  STACK FOR UNDO DELETE (LIFO) 
// Feature: Stores deleted messages for recovery
class UndoStack{
private:
    struct StackNode{
        Message* msg;
        StackNode* next;
        StackNode(Message* m) : msg(m), next(nullptr){}
    };
    StackNode* top;

public:
    UndoStack() : top(nullptr){}

    void push(Message* msg){
        StackNode* newNode = new StackNode(msg);
        newNode->next = top;
        top = newNode;
    }

    Message* pop(){
        if (top == nullptr) return nullptr;
        StackNode* temp = top;
        Message* msg = temp->msg;
        top = top->next;
        delete temp;
        return msg;
    }

    bool isEmpty(){
        return top == nullptr;
    }

    ~UndoStack(){
        while (!isEmpty()){
            delete pop();
        }
    }
};

//  FRIEND NODE (LINKED LIST) 
struct FriendNode{
    string username;
    FriendNode* next;
    FriendNode(string name) : username(name), next(nullptr){}
};

//  USER STRUCTURE 
struct User {
    string username;
    string password;
    string email;           // For password recovery
    string securityQuestion; // Security question
    string securityAnswer;   // Security answer
    string lastSeen;
    bool isOnline;
    bool isAdmin;
    FriendNode* friends;
    MessageQueue* inbox;
    UndoStack* undoStack;
    User* next;

    // Members are initialized in the same order as they are declared
    User(string uname, string pwd, bool admin = false)
        : username(uname), password(pwd), email(""),
          securityQuestion(""), securityAnswer(""), lastSeen("Never"),
          isOnline(false), isAdmin(admin),
          friends(nullptr), inbox(new MessageQueue()),
          undoStack(new UndoStack()), next(nullptr){}

    ~User(){
        // Clean up friend list
        while (friends != nullptr){
            FriendNode* temp = friends;
            friends = friends->next;
            delete temp;
        }
        delete inbox;
        delete undoStack;
    }

    void addFriend(string friendName){
        FriendNode* newFriend = new FriendNode(friendName);
        newFriend->next = friends;
        friends = newFriend;
    }

    bool hasFriend(string friendName){
        FriendNode* current = friends;
        while (current != nullptr) {
            if (current->username == friendName){
                return true;
            }
            current = current->next;
        }
        return false;
    }

    // NEW: removes a friend from the linked list (used by the user, the admin
    // and when an account is deleted). Returns false if not found.
    bool removeFriend(string friendName){
        FriendNode* current = friends;
        FriendNode* prev = nullptr;
        while (current != nullptr){
            if (current->username == friendName){
                if (prev == nullptr){
                    friends = current->next;
                } else{
                    prev->next = current->next;
                }
                delete current;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    void displayFriends(){
        if (friends == nullptr) {
            setColor(RED);
            cout << "   [INFO] No friends added yet.\n";
            resetColor();
            return;
        }

        setColor(CYAN);
        cout << "   Friends List: ";
        FriendNode* current = friends;
        while (current != nullptr) {
            setColor(GREEN);
            cout << current->username;
            if (current->next != nullptr) cout << ", ";
            current = current->next;
        }
        cout << "\n";
        resetColor();
    }

    int getUnreadCount(){
        // Call MessageQueue's efficient unread counter
        return inbox->getUnreadCount();
    }
};

//  HASH TABLE FOR USER MANAGEMENT 
// Feature: O(1) average case login/search
class HashTable{
private:
    static const int TABLE_SIZE = 101;  // Prime number for better distribution
    User** table;

    int hashFunction(string key){
        // unsigned values, so special characters can never give a negative index
        unsigned int hash = 0;
        for (unsigned char c : key) {
            hash = (hash * 31 + c) % TABLE_SIZE;
        }
        return (int)hash;
    }

public:
    HashTable(){
        table = new User*[TABLE_SIZE]();
    }

    ~HashTable(){
        for (int i = 0; i < TABLE_SIZE; i++){
            User* current = table[i];
            while (current != nullptr){
                User* temp = current;
                current = current->next;
                delete temp;
            }
        }
        delete[] table;
    }

    void insert(User* user){
        int index = hashFunction(user->username);
        user->next = table[index];
        table[index] = user;
    }

    User* search(string username){
        int index = hashFunction(username);
        User* current = table[index];
        while (current != nullptr){
            if (current->username == username){
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    bool remove(string username){
        int index = hashFunction(username);
        User* current = table[index];
        User* prev = nullptr;

        while (current != nullptr){
            if (current->username == username){
                if (prev == nullptr){
                    table[index] = current->next;
                } else{
                    prev->next = current->next;
                }
                delete current;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    // Get all users (for BST insertion)
    vector<User*> getAllUsers(){
        vector<User*> users;
        for (int i = 0; i < TABLE_SIZE; i++){
            User* current = table[i];
            while (current != nullptr){
                users.push_back(current);
                current = current->next;
            }
        }
        return users;
    }
};

//  BST NODE FOR USER SEARCH 
// Feature: Fast user search with sorted output
struct BSTNode{
    User* user;
    BSTNode* left;
    BSTNode* right;

    BSTNode(User* u) : user(u), left(nullptr), right(nullptr) {}
};

class UserBST{
private:
    BSTNode* root;

    BSTNode* insertHelper(BSTNode* node, User* user){
        if (node == nullptr){
            return new BSTNode(user);
        }
        if (user->username < node->user->username){
            node->left = insertHelper(node->left, user);
        } else if (user->username > node->user->username){
            node->right = insertHelper(node->right, user);
        }
        return node;
    }

    BSTNode* searchHelper(BSTNode* node, string username){
        if (node == nullptr || node->user->username == username){
            return node;
        }
        if (username < node->user->username){
            return searchHelper(node->left, username);
        }
        return searchHelper(node->right, username);
    }

    void inorderHelper(BSTNode* node){
        if (node != nullptr) {
            inorderHelper(node->left);
            setColor(GREEN);
            cout << "   • " << node->user->username;
            if (node->user->isOnline){
                setColor(GREEN);
                cout << " 🟢 Online";
            } else{
                setColor(RED);
                cout << " ⚫ Offline";
            }
            cout << "\n";
            resetColor();
            inorderHelper(node->right);
        }
    }

    void destroyTree(BSTNode* node){
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    UserBST() : root(nullptr){}

    ~UserBST(){
        destroyTree(root);
    }

    void insert(User* user){
        root = insertHelper(root, user);
    }

    User* search(string username){
        BSTNode* result = searchHelper(root, username);
        return result ? result->user : nullptr;
    }

    void displayAllUsers(){
        if (root == nullptr){
            setColor(RED);
            cout << "   [INFO] No users found.\n";
            resetColor();
            return;
        }
        setColor(CYAN);
        cout << "\n   === ALL USERS (Sorted Alphabetically) ===\n";
        resetColor();
        inorderHelper(root);
    }

    void rebuildFromHashTable(HashTable& hashTable){
        destroyTree(root);
        root = nullptr;
        vector<User*> users = hashTable.getAllUsers();
        for (User* user : users) {
            insert(user);
        }
    }
};

//  GROUP CHAT STRUCTURE 
struct Group{
    string groupName;
    string creator;
    vector<string> members;
    MessageQueue* messages;
    Group* next;

    Group(string name, string creatorName) : groupName(name), creator(creatorName),
                                              messages(new MessageQueue()), next(nullptr) {
        members.push_back(creatorName);
    }

    ~Group(){
        delete messages;
    }

    void addMember(string username){
        if (find(members.begin(), members.end(), username) == members.end()) {
            members.push_back(username);
        }
    }

    // NEW: only members can send and read group messages
    bool isMember(string username){
        return find(members.begin(), members.end(), username) != members.end();
    }

    // NEW: used when an account is deleted
    void removeMember(string username){
        members.erase(remove(members.begin(), members.end(), username), members.end());
    }

    void sendMessage(string sender, string text){
        messages->enqueue(sender, text);
    }

    void displayMessages(){
        setColor(MAGENTA);
        cout << "\n   === Group: " << groupName << " ===\n";
        resetColor();
        messages->displayMessages();
    }

    void displayMembers(){
    setColor(BRIGHT_CYAN);
    cout << "\n   === Group Members: " << groupName << " ===\n";
    resetColor();

    if (members.empty()){
        setColor(BRIGHT_RED);
        cout << "   No members in this group.\n";
        resetColor();
        return;
    }

    setColor(BRIGHT_GREEN);
    cout << "   Creator: " << creator << " (Admin)\n";
    resetColor();

    setColor(BRIGHT_YELLOW);
    cout << "   Members:\n";
    resetColor();

    for (size_t i = 0; i < members.size(); i++){
        setColor(BRIGHT_WHITE);
        cout << "   " << (i + 1) << ". " << members[i];
        if (members[i] == creator){
            setColor(BRIGHT_GREEN);
            cout << " [Creator]";
        }
        resetColor();
        cout << "\n";
    }

    setColor(BRIGHT_CYAN);
    cout << "   Total Members: " << members.size() << "\n";
    resetColor();
}
};

//  GROUP MANAGER (LINKED LIST) 
class GroupManager {
private:
    Group* head;

public:
    GroupManager() : head(nullptr){}

    ~GroupManager(){
        while (head != nullptr){
            Group* temp = head;
            head = head->next;
            delete temp;
        }
    }

    void createGroup(string name, string creator){
        Group* newGroup = new Group(name, creator);
        newGroup->next = head;
        head = newGroup;
        setColor(GREEN);
        cout << "   [SUCCESS] Group '" << name << "' created!\n";
        resetColor();
    }

    // NEW: adds a group while loading from file (keeps the saved order, no message)
    Group* addLoadedGroup(string name, string creator){
        Group* newGroup = new Group(name, creator);
        if (head == nullptr){
            head = newGroup;
        } else{
            Group* current = head;
            while (current->next != nullptr) current = current->next;
            current->next = newGroup;
        }
        return newGroup;
    }

    Group* findGroup(string name){
        Group* current = head;
        while (current != nullptr){
            if (current->groupName == name){
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    void addToGroup(string groupName, string username, string requester){
        Group* group = findGroup(groupName);
        if (group == nullptr){
            setColor(RED);
            cout << "   [ERROR] Group not found!\n";
            resetColor();
            return;
        }

        if (group->creator != requester){
            setColor(RED);
            cout << "   [ERROR] Only group creator can add members!\n";
            resetColor();
            return;
        }

        if (group->isMember(username)){
            setColor(RED);
            cout << "   [ERROR] " << username << " is already in this group!\n";
            resetColor();
            return;
        }

        group->addMember(username);
        setColor(GREEN);
        cout << "   [SUCCESS] " << username << " added to group '" << groupName << "'!\n";
        resetColor();
    }

    void listGroups(){
        if (head == nullptr){
            setColor(RED);
            cout << "   [INFO] No groups available.\n";
            resetColor();
            return;
        }

        setColor(CYAN);
        cout << "\n   === Available Groups ===\n";
        Group* current = head;
        while (current != nullptr){
            setColor(YELLOW);
            cout << "   • " << current->groupName;
            setColor(WHITE);
            cout << " (Created by: " << current->creator << ", Members: " << current->members.size() << ")\n";
            current = current->next;
        }
        resetColor();
    }

    // NEW: number of groups (for the admin statistics)
    int count(){
        int total = 0;
        for (Group* current = head; current != nullptr; current = current->next) total++;
        return total;
    }

    // NEW: removes a deleted user from every group. If the user created a group,
    // the next member becomes the creator. An empty group is deleted.
    void removeUserEverywhere(string username){
        Group* current = head;
        Group* prev = nullptr;
        while (current != nullptr){
            current->removeMember(username);
            if (current->creator == username && !current->members.empty()){
                current->creator = current->members[0];
            }
            if (current->members.empty()){
                Group* toDelete = current;
                if (prev == nullptr) head = current->next;
                else prev->next = current->next;
                current = current->next;
                delete toDelete;
                continue;
            }
            prev = current;
            current = current->next;
        }
    }

    // NEW: saves all groups, their members and their messages
    void saveToFile(ofstream& file){
        for (Group* current = head; current != nullptr; current = current->next){
            string name = encodeField(current->groupName);
            file << "GROUP|" << name << "|" << encodeField(current->creator) << "\n";
            for (const string& member : current->members){
                file << "MEMBER|" << name << "|" << encodeField(member) << "\n";
            }
            current->messages->saveToFile(file, "GMSG|" + name);
        }
    }
};

//  CHAT SYSTEM MAIN CLASS 
class ChatSystem {
private:
    HashTable users;
    UserBST userBST;
    GroupManager groupManager;
    User* currentUser;
    bool running;

    // Saves everything. Called after every change, so nothing is lost
    // even if the window is closed without using Exit.
    void saveData(bool showMessage = true) {
    ofstream userFile("users.txt");

    vector<User*> allUsers = users.getAllUsers();
    for (User* user : allUsers) {
        // Save user info with recovery data
        userFile << encodeField(user->username) << "|"
                 << encodeField(user->password) << "|"
                 << encodeField(user->email) << "|"
                 << encodeField(user->securityQuestion) << "|"
                 << encodeField(user->securityAnswer) << "|"
                 << user->isAdmin << "|"
                 << encodeField(user->lastSeen) << "\n";

        // Save friends
        FriendNode* friendPtr = user->friends;
        while (friendPtr != nullptr) {
            userFile << "FRIEND|" << encodeField(user->username) << "|" << encodeField(friendPtr->username) << "\n";
            friendPtr = friendPtr->next;
        }
    }
    userFile.close();

    // NEW: save every inbox (before, all messages were lost when the program closed)
    ofstream messageFile("messages.txt");
    for (User* user : allUsers) {
        user->inbox->saveToFile(messageFile, "INBOX|" + encodeField(user->username));
    }
    messageFile.close();

    // NEW: save groups, members and group messages
    ofstream groupFile("groups.txt");
    groupManager.saveToFile(groupFile);
    groupFile.close();

    if (showMessage) {
        setColor(BRIGHT_GREEN);
        cout << "   [INFO] Data saved successfully!\n";
        resetColor();
    }
}

    void backupData(){
        ofstream backup("backup_users.txt");
        if (!backup.is_open()){
            setColor(BRIGHT_RED);
            cout << "   [ERROR] Cannot create backup file!\n";
            resetColor();
            return;
        }

        vector<User*> allUsers = users.getAllUsers();

        for (User* user : allUsers) {
            backup << encodeField(user->username) << "|" << encodeField(user->password) << "|"
                   << encodeField(user->email) << "|" << encodeField(user->securityQuestion) << "|"
                   << encodeField(user->securityAnswer) << "|" << user->isAdmin << "|"
                   << encodeField(user->lastSeen) << "\n";

            // Backup friends
            FriendNode* friendPtr = user->friends;
            while (friendPtr != nullptr) {
                backup << "FRIEND|" << encodeField(user->username) << "|" << encodeField(friendPtr->username) << "\n";
                friendPtr = friendPtr->next;
            }
        }
        backup.close();

        setColor(BRIGHT_GREEN);
        cout << "   [SUCCESS] Data backed up successfully to 'backup_users.txt'!\n";
        resetColor();
    }

void loadData(){
    string line;

    ifstream userFile("users.txt");
    if (userFile.is_open()) {
        // FIX: friends were saved in users.txt but loading skipped them
        // (it looked for a friends.txt file that was never created).
        // Now they are collected here and added after all users are loaded.
        vector<pair<string, string>> friendPairs;

        while (getline(userFile, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            vector<string> parts = splitLine(line);
            if (parts[0] == "FRIEND") {
                if (parts.size() >= 3) friendPairs.push_back({decodeField(parts[1]), decodeField(parts[2])});
                continue;
            }
            if (parts.size() < 2) continue;

            string username = decodeField(parts[0]);
            if (username.empty() || users.search(username) != nullptr) continue;

            bool isAdmin = (parts.size() > 5 && parts[5] == "1");
            User* user = new User(username, decodeField(parts[1]), isAdmin);
            if (parts.size() > 2) user->email = decodeField(parts[2]);
            if (parts.size() > 3) user->securityQuestion = decodeField(parts[3]);
            if (parts.size() > 4) user->securityAnswer = decodeField(parts[4]);
            if (parts.size() > 6 && !parts[6].empty()) user->lastSeen = decodeField(parts[6]);

            users.insert(user);
            userBST.insert(user);
        }
        userFile.close();

        // addFriend() adds to the front, so adding in reverse keeps the saved order
        for (auto it = friendPairs.rbegin(); it != friendPairs.rend(); ++it) {
            User* u = users.search(it->first);
            if (u != nullptr && !u->hasFriend(it->second)) {
                u->addFriend(it->second);
            }
        }
    }

    // NEW: load inbox messages
    ifstream messageFile("messages.txt");
    while (messageFile.is_open() && getline(messageFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        vector<string> parts = splitLine(line);
        if (parts.size() < 6 || parts[0] != "INBOX") continue;

        User* owner = users.search(decodeField(parts[1]));
        if (owner == nullptr) continue;
        Message* msg = new Message(decodeField(parts[2]), decodeField(parts[3]));
        msg->timestamp = parts[4];
        msg->isRead = (parts[5] == "1");
        owner->inbox->enqueueMessage(msg);
    }

    // NEW: load groups, members and group messages
    ifstream groupFile("groups.txt");
    while (groupFile.is_open() && getline(groupFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        vector<string> parts = splitLine(line);
        if (parts.size() < 3) continue;

        string groupName = decodeField(parts[1]);
        if (parts[0] == "GROUP") {
            if (groupManager.findGroup(groupName) == nullptr) {
                groupManager.addLoadedGroup(groupName, decodeField(parts[2]));
            }
            continue;
        }

        Group* group = groupManager.findGroup(groupName);
        if (group == nullptr) continue;

        if (parts[0] == "MEMBER") {
            group->addMember(decodeField(parts[2]));
        } else if (parts[0] == "GMSG" && parts.size() >= 6) {
            Message* msg = new Message(decodeField(parts[2]), decodeField(parts[3]));
            msg->timestamp = parts[4];
            msg->isRead = (parts[5] == "1");
            group->messages->enqueueMessage(msg);
        }
    }
}

    void showHeader(){
       #ifdef _WIN32
            system("cls");
       #else
            system("clear");
       #endif

       setColor(BRIGHT_YELLOW);
       cout << "╔═══════════════════════════════════════════════════════════╗\n";
       setColor(BRIGHT_MAGENTA);
       cout << "║                                                           ║\n";
       setColor(BRIGHT_YELLOW);
       cout << "║                    Welcome to FNA Chat                    ║\n";
       setColor(BRIGHT_GREEN);
       cout << "║              Terminal-Based Messaging System              ║\n";
       setColor(BRIGHT_MAGENTA);
       cout << "║             Data Structure : CSE 221 Project              ║\n";
       setColor(BRIGHT_GREEN);
       cout << "║                                                           ║\n";
       setColor(BRIGHT_YELLOW);
       cout << "╚═══════════════════════════════════════════════════════════╝\n";
       resetColor();

       setColor(BRIGHT_WHITE);
       cout << "                     Date: " << getCurrentDate() << "\n";
       setColor(GRAY);
       cout << "                     Time: " << getCurrentTimestamp() << "\n";
       resetColor();
       cout << "\n";
   }

    void showMainMenu() {
        showHeader();
        setColor(BRIGHT_YELLOW);
        cout << "   ┌─────────────────────────────────────────┐\n";
        cout << "   │                MAIN MENU                │\n";
        cout << "   ├─────────────────────────────────────────┤\n";
        setColor(BRIGHT_GREEN);
        cout << "   │  1. Register New Account                │\n";
        cout << "   │  2. Login                               │\n";
        setColor(BRIGHT_CYAN);
        cout << "   │  3. Forgot Password / Username          │\n";
        setColor(BRIGHT_RED);
        cout << "   │  4. Exit                                │\n";
        setColor(BRIGHT_YELLOW);
        cout << "   └─────────────────────────────────────────┘\n";
        resetColor();
    }

    void showUserMenu(){
        showHeader();
        setColor(GREEN);
        cout << "   Welcome, " << currentUser->username;
        if (currentUser->isOnline){
            setColor(GREEN);
            cout << " is now Online!";
        }
        resetColor();
        cout << "\n";
        setColor(CYAN);
        cout << "   ┌─────────────────────────────────────────┐\n";
        cout << "   │                CHAT MENU                │\n";
        cout << "   ├─────────────────────────────────────────┤\n";
        cout << "   │  1. Send Private Message                │\n";
        cout << "   │  2. View Inbox                          │\n";
        cout << "   │  3. Delete Message (with Undo)          │\n";
        cout << "   │  4. Undo Last Deletion                  │\n";
        cout << "   │  5. Manage Friends                      │\n";
        cout << "   │  6. Search Users                        │\n";
        cout << "   │  7. Group Chat                          │\n";
        cout << "   │  8. View All Users                      │\n";
        cout << "   │  9. Change Password                     │\n";
        cout << "   │ 10. Search Messages                     │\n";
        cout << "   │ 11. Forward Message                     │\n";
        cout << "   │ 12. Logout                              │\n";
        if (currentUser->isAdmin){
            setColor(RED);
        cout << "   │ 13. Admin Panel (Delete Users)          │\n";
            setColor(CYAN);
        }
        cout << "   └─────────────────────────────────────────┘\n";
        resetColor();
    }

    // NEW: finds a user by email (not case-sensitive)
    User* findUserByEmail(const string& email){
        if (email.empty()) return nullptr;
        vector<User*> allUsers = users.getAllUsers();
        for (User* user : allUsers){
            if (!user->email.empty() && toLowerText(user->email) == toLowerText(email)){
                return user;
            }
        }
        return nullptr;
    }

    // NEW: asks for a new password and asks again to confirm it.
    // Used by registration, password recovery and change password.
    NavAction askNewPassword(const string& label, string& result){
        string first;
        int step = 0;   // 0 = new password, 1 = confirm
        while (step < 2) {
            string input;
            NavAction action = readWord(step == 0 ? "   " + label + " (min 4 characters): "
                                                  : "   Confirm password: ", input);
            if (action == NAV_MENU) return NAV_MENU;
            if (action == NAV_BACK) {
                if (step == 0) return NAV_BACK;
                step = 0;
                continue;
            }

            if (step == 0) {
                if (input.length() < 4) {
                    printError("Password must be at least 4 characters!");
                    continue;
                }
                first = input;
                step = 1;
            } else {
                if (input != first) {
                    printError("Passwords do not match! Please enter them again.");
                    step = 0;
                    continue;
                }
                step = 2;
            }
        }
        result = first;
        return NAV_OK;
    }

    //  Password recovery options 
    // Each one returns NAV_OK when finished, NAV_BACK to show the recovery
    // menu again, or NAV_MENU to go to the main menu.

    NavAction recoverUsername(){
        while (true) {
            string email;
            NavAction action = readWord("\n   Enter your registered email: ", email);
            if (action != NAV_OK) return action;

            User* user = findUserByEmail(email);
            if (user != nullptr) {
                setColor(BRIGHT_GREEN);
                cout << "\n   Your username is: " << user->username << "\n";
                resetColor();
                pressEnterToContinue();
                return NAV_OK;
            }
            printError("No account found with this email!");
        }
    }

    NavAction resetBySecurityQuestion(){
        User* user = nullptr;
        int wrongAnswers = 0;
        int step = 0;   // 0 = username, 1 = answer, 2 = new password
        while (true) {
            NavAction action = NAV_OK;

            if (step == 0) {
                string username;
                action = readWord("\n   Enter your username: ", username);
                if (action == NAV_OK) {
                    user = users.search(username);
                    if (user == nullptr) {
                        printError("Username not found!");
                        continue;
                    }
                    if (user->securityQuestion.empty()) {
                        printError("This account has no security question. Try email reset or contact the admin.");
                        continue;
                    }
                }
            } else if (step == 1) {
                cout << "\n   Security Question: " << user->securityQuestion << "\n";
                string answer;
                action = readLine("   Your answer: ", answer);
                // Answers are not case-sensitive ("Dhaka" = "dhaka")
                if (action == NAV_OK && toLowerText(answer) != toLowerText(trimText(user->securityAnswer))) {
                    wrongAnswers++;
                    printError("Incorrect security answer!");
                    if (wrongAnswers >= 3) {
                        printError("Too many wrong answers. Please try again later.");
                        pressEnterToContinue();
                        return NAV_OK;
                    }
                    continue;
                }
            } else {
                string newPassword;
                action = askNewPassword("Enter new password", newPassword);
                if (action == NAV_OK) {
                    user->password = newPassword;
                    saveData(false);
                    setColor(BRIGHT_GREEN);
                    cout << "   [SUCCESS] Password reset successfully!\n";
                    cout << "   Please login with your new password.\n";
                    resetColor();
                    pressEnterToContinue();
                    return NAV_OK;
                }
            }

            if (action == NAV_MENU) return NAV_MENU;
            if (action == NAV_BACK) {
                if (step == 0) return NAV_BACK;
                step--;
                continue;
            }
            step++;
        }
    }

    // FIX: before, anyone who knew an email could reset that account's password.
    // Now a 6-digit code is "emailed" first. This is a simulation: the email is
    // written to email_outbox.txt, because a console program cannot send real emails.
    NavAction resetByEmail(){
        User* user = nullptr;
        string code;
        int wrongCodes = 0;
        int step = 0;   // 0 = email, 1 = code, 2 = new password
        while (true) {
            NavAction action = NAV_OK;

            if (step == 0) {
                string email;
                action = readWord("\n   Enter your registered email: ", email);
                if (action == NAV_OK) {
                    user = findUserByEmail(email);
                    if (user == nullptr) {
                        printError("No account found with this email!");
                        continue;
                    }
                    int number = 100000 + (rand() % 900) * 1000 + rand() % 1000;
                    code = to_string(number);
                    ofstream outbox("email_outbox.txt", ios::app);
                    outbox << "To: " << user->email << " | " << getCurrentTimestamp()
                           << " | Your FNA Chat password reset code is: " << code << "\n";
                    outbox.close();
                    wrongCodes = 0;

                    setColor(BRIGHT_GREEN);
                    cout << "\n   [INFO] A 6-digit reset code was sent to " << user->email << "\n";
                    setColor(GRAY);
                    cout << "   (Demo: the email is saved in 'email_outbox.txt' next to the program.)\n";
                    resetColor();
                }
            } else if (step == 1) {
                string input;
                action = readWord("   Enter the 6-digit code: ", input);
                if (action == NAV_OK && input != code) {
                    wrongCodes++;
                    printError("Wrong code!");
                    if (wrongCodes >= 3) {
                        printError("Too many wrong codes. Please request a new code.");
                        step = 0;
                    }
                    continue;
                }
            } else {
                string newPassword;
                action = askNewPassword("Enter new password", newPassword);
                if (action == NAV_OK) {
                    user->password = newPassword;
                    saveData(false);
                    printSuccess("Password reset successfully!");
                    pressEnterToContinue();
                    return NAV_OK;
                }
            }

            if (action == NAV_MENU) return NAV_MENU;
            if (action == NAV_BACK) {
                if (step == 0) return NAV_BACK;
                step--;
                continue;
            }
            step++;
        }
    }

void forgotPassword(){
    while (true) {
        showHeader();
        setColor(BRIGHT_CYAN);
        cout << "   === PASSWORD RECOVERY ===\n";
        resetColor();

        setColor(BRIGHT_YELLOW);
        cout << "\n   1. Recover Username\n";
        cout << "   2. Reset Password (via Security Question)\n";
        cout << "   3. Reset Password (via Email)\n";
        cout << "   4. Contact Admin\n";
        cout << "   5. Back to Main Menu\n";
        resetColor();
        showNavHint();

        int choice;
        NavAction action = readNumber("\n   Choose option: ", 1, 5, choice);
        if (action != NAV_OK || choice == 5) return;

        if (choice == 1) action = recoverUsername();
        else if (choice == 2) action = resetBySecurityQuestion();
        else if (choice == 3) action = resetByEmail();
        else {
            // Contact Admin
            setColor(BRIGHT_CYAN);
            cout << "\n   === CONTACT ADMIN ===\n";
            cout << "   Admin username: admin\n";
            cout << "   Please contact the system administrator for assistance.\n";
            cout << "   Provide your username and email for verification.\n";
            resetColor();
            pressEnterToContinue();
            action = NAV_BACK;
        }

        if (action != NAV_BACK) return;   // finished or 'm': back to the main menu
    }
}

void registerUser(){
    showHeader();
    setColor(BRIGHT_CYAN);
    cout << "   === REGISTRATION ===\n";
    resetColor();
    showNavHint();

    string username, password, email, securityQuestion, securityAnswer;

    // 0 = username, 1 = password, 2 = email, 3 = security question, 4 = security answer
    int step = 0;
    while (step < 5) {
        NavAction action = NAV_OK;

        if (step == 0) {
            action = readWord("\n   Enter username (5-20 letters, numbers or _): ", username);
            if (action == NAV_OK) {
                if (!isValidUsername(username) || toLowerText(username) == "friend") {
                    printError("Username must be 5-20 characters: letters, numbers or _ only!");
                    continue;
                }
                if (users.search(username) != nullptr) {
                    printError("Username '" + username + "' already exists!");
                    suggestUsernames(username);
                    continue;
                }
            }
        } else if (step == 1) {
            action = askNewPassword("Enter password", password);
        } else if (step == 2) {
            action = readWord("   Enter email (for password recovery) or 0 to skip: ", email);
            if (action == NAV_OK) {
                if (email == "0") {
                    email = "";
                    setColor(BRIGHT_YELLOW);
                    cout << "   [WARNING] Email skipped. Email recovery will not be available!\n";
                    resetColor();
                } else if (!isValidEmail(email)) {
                    printError("Invalid email format! Example: name@gmail.com");
                    continue;
                } else if (findUserByEmail(email) != nullptr) {
                    printError("This email is already used by another account!");
                    continue;
                }
            }
        } else if (step == 3) {
            action = readLine("   Enter security question (e.g., What is your pet's name?) or 0 to skip: ", securityQuestion);
            if (action == NAV_OK) {
                if (securityQuestion.empty()) {
                    printError("Please type a question, or 0 to skip.");
                    continue;
                }
                if (securityQuestion == "0") {
                    securityQuestion = "";
                    securityAnswer = "";
                    setColor(BRIGHT_YELLOW);
                    cout << "   [WARNING] Security question skipped. Question recovery will not be available!\n";
                    resetColor();
                    step = 5;   // nothing more to ask
                    continue;
                }
            }
        } else {
            action = readLine("   Enter security answer: ", securityAnswer);
            if (action == NAV_OK && securityAnswer.empty()) {
                printError("Security answer cannot be empty!");
                continue;
            }
        }

        if (action == NAV_MENU) return;
        if (action == NAV_BACK) {
            if (step == 0) return;   // back to the main menu
            step--;
            continue;
        }
        step++;
    }

    // Create new user
    User* newUser = new User(username, password, false);
    newUser->email = email;
    newUser->securityQuestion = securityQuestion;
    newUser->securityAnswer = securityAnswer;

    users.insert(newUser);
    userBST.insert(newUser);
    saveData(false);

    setColor(BRIGHT_GREEN);
    cout << "\n   ✓ [SUCCESS] Registration successful!\n";
    cout << "   ✓ Username: " << username << "\n";
    if (!email.empty()) cout << "   ✓ Email: " << email << "\n";
    else cout << "   ⚠  Email not provided (email recovery disabled)\n";
    if (!securityQuestion.empty()) cout << "   ✓ Security question saved for password recovery.\n";
    else cout << "   ⚠  Security question not set (question recovery disabled)\n";
    cout << "\n   Please login with your credentials.\n";
    resetColor();

    pressEnterToContinue();
}

void suggestUsernames(string attemptedUsername){
    setColor(BRIGHT_CYAN);
    cout << "   Suggested usernames:\n";
    resetColor();

    vector<string> suggestions;
    suggestions.push_back(attemptedUsername + "123");
    suggestions.push_back(attemptedUsername + "_" + to_string(rand() % 1000));
    suggestions.push_back("the_" + attemptedUsername);
    suggestions.push_back(attemptedUsername + "_user");

    for (string sugg : suggestions){
        if (sugg.length() <= 20 && users.search(sugg) == nullptr){
            setColor(BRIGHT_GREEN);
            cout << "   • " << sugg << " (available)\n";
            resetColor();
        }
    }
}

void login(){
    showHeader();
    setColor(CYAN);
    cout << "   === LOGIN ===\n";
    resetColor();
    showNavHint();

    User* user = nullptr;
    int wrongPasswords = 0;
    int step = 0;   // 0 = username, 1 = password
    while (step < 2) {
        NavAction action = NAV_OK;

        if (step == 0) {
            string username;
            action = readWord("\n   Username: ", username);
            if (action == NAV_OK) {
                user = users.search(username);
                if (user == nullptr) {
                    printError("User not found! Check the spelling or register first.");
                    continue;
                }
            }
        } else {
            string password;
            action = readWord("   Password: ", password);
            if (action == NAV_OK && user->password != password) {
                wrongPasswords++;
                printError("Wrong password!");
                if (wrongPasswords >= 3) {
                    printError("Too many wrong attempts. Please try again later.");
                    pressEnterToContinue();
                    return;
                }
                continue;
            }
        }

        if (action == NAV_MENU) return;
        if (action == NAV_BACK) {
            if (step == 0) return;   // back to the main menu
            step--;
            continue;
        }
        step++;
    }

    currentUser = user;
    currentUser->isOnline = true;
    currentUser->lastSeen = getCurrentTimestamp();
    setColor(GREEN);
    cout << "   [SUCCESS] Login successful! Welcome " << currentUser->username << "!\n";
    resetColor();

    // Show notification for unread messages
    int unread = currentUser->getUnreadCount();
    if(unread > 0){
        setColor(YELLOW);
        cout << "   🔔 You have " << unread << " unread message(s)!\n";
        resetColor();
    }
    pressEnterToContinue();
}

void sendPrivateMessage(){
    showHeader();
    setColor(CYAN);
    cout << "   === SEND PRIVATE MESSAGE ===\n";
    resetColor();
    currentUser->displayFriends();
    showNavHint();

    User* receiver = nullptr;
    string message;
    int step = 0;   // 0 = recipient, 1 = message
    while (step < 2) {
        NavAction action = NAV_OK;

        if (step == 0) {
            string recipient;
            action = readWord("\n   Recipient username: ", recipient);
            if (action == NAV_OK) {
                receiver = users.search(recipient);
                if (receiver == nullptr) {
                    printError("User not found!");
                    continue;
                }
                if (receiver == currentUser) {
                    printError("Cannot send message to yourself!");
                    continue;
                }
            }
        } else {
            showTypingIndicator(currentUser->username);
            action = readLine("   Message: ", message);
            if (action == NAV_OK && message.empty()) {
                printError("Message cannot be empty!");
                continue;
            }
        }

        if (action == NAV_MENU) return;
        if (action == NAV_BACK) {
            if (step == 0) return;
            step--;
            continue;
        }
        step++;
    }

    receiver->inbox->enqueue(currentUser->username, message);
    saveData(false);
    setColor(GREEN);
    cout << "   [SUCCESS] Message sent to " << receiver->username << "!\n";

    if(receiver->isOnline){
        setColor(YELLOW);
        cout << "   🔔 " << receiver->username << " is online. Notification sent!\n";
    }
    resetColor();

    pressEnterToContinue();
}

void showTypingIndicator(string username){
    setColor(BRIGHT_CYAN);
    cout << "   ✎ " << username << " is typing";

    for(int i = 0; i < 3; i++){
        cout << ".";
        cout.flush();

        #ifdef _WIN32
            Sleep(500);  // Windows: 500 milliseconds
        #else
            usleep(500000);  // Linux/Mac: 500,000 microseconds = 0.5 seconds
        #endif
    }

    cout << "\n";
    resetColor();
}

void forwardMessage(){
    showHeader();
    setColor(BRIGHT_CYAN);
    cout << "   === FORWARD MESSAGE ===\n";
    resetColor();
    if(currentUser->inbox->isEmpty()){
        setColor(BRIGHT_RED);
        cout << "   [INFO] No messages to forward.\n";
        resetColor();
        pressEnterToContinue();
        return;
    }

    currentUser->inbox->displayMessages();
    showNavHint();

    Message* msg = nullptr;
    User* receiver = nullptr;
    int step = 0;   // 0 = message number, 1 = recipient
    while (step < 2) {
        NavAction action = NAV_OK;

        if (step == 0) {
            int msgNum;
            action = readNumber("\n   Enter message number to forward (0 to cancel): ", 0,
                                currentUser->inbox->getSize(), msgNum);
            if (action == NAV_OK) {
                if (msgNum == 0) return;
                msg = currentUser->inbox->getMessageAtIndex(msgNum - 1);
            }
        } else {
            string recipient;
            action = readWord("   Enter recipient username: ", recipient);
            if (action == NAV_OK) {
                receiver = users.search(recipient);
                if (receiver == nullptr) {
                    printError("User not found!");
                    continue;
                }
                if (receiver == currentUser) {
                    printError("Cannot forward a message to yourself!");
                    continue;
                }
            }
        }

        if (action == NAV_MENU) return;
        if (action == NAV_BACK) {
            if (step == 0) return;
            step--;
            continue;
        }
        step++;
    }

    string forwardedMsg = "[Forwarded from " + msg->sender + "]: " + msg->text;
    receiver->inbox->enqueue(currentUser->username, forwardedMsg);
    saveData(false);

    setColor(BRIGHT_GREEN);
    cout << "   [SUCCESS] Message forwarded to " << receiver->username << "!\n";
    resetColor();

    pressEnterToContinue();
}

void viewInbox(){
    showHeader();
    setColor(BRIGHT_CYAN);
    cout << "   === INBOX ===\n";
    resetColor();

    if(currentUser->inbox->isEmpty()){
        setColor(BRIGHT_RED);
        cout << "   [INFO] Your inbox is empty.\n";
        resetColor();
        pressEnterToContinue();
        return;
    }

    currentUser->inbox->displayMessages();
    saveData(false);   // messages are now read

    // Add options for message management
    setColor(BRIGHT_YELLOW);
    cout << "\n ┌─────────────────────────────────────┐\n";
    cout << "   │  Options:                           │\n";
    cout << "   │  1. Mark message as read            │\n";
    cout << "   │  2. Mark message as unread          │\n";
    cout << "   │  3. Back to Main Menu               │\n";
    cout << "   └─────────────────────────────────────┘\n";
    resetColor();
    showNavHint();

    int step = 0;   // 0 = option, 1 = message number
    int opt = 3;
    while (step < 2) {
        NavAction action = NAV_OK;

        if (step == 0) {
            action = readNumber("\n   Choose option: ", 1, 3, opt);
            if (action == NAV_OK && opt == 3) return;
        } else {
            int msgNum;
            action = readNumber("   Enter message number: ", 1, currentUser->inbox->getSize(), msgNum);
            if (action == NAV_OK) {
                if (opt == 1) currentUser->inbox->markAsRead(msgNum - 1);
                else currentUser->inbox->markAsUnread(msgNum - 1);
                saveData(false);
            }
        }

        if (action == NAV_MENU) return;
        if (action == NAV_BACK) {
            if (step == 0) return;
            step--;
            continue;
        }
        step++;
    }

    pressEnterToContinue();
}

void deleteMessage(){
    showHeader();
    setColor(CYAN);
    cout << "   === DELETE MESSAGE ===\n";
    resetColor();

    if(currentUser->inbox->isEmpty()){
        setColor(RED);
        cout << "   [INFO] No messages to delete.\n";
        resetColor();
        pressEnterToContinue();
        return;
    }

    currentUser->inbox->displayMessages();
    showNavHint();

    int choice;
    NavAction action = readNumber("\n   Enter message number to delete (0 to cancel): ", 0,
                                  currentUser->inbox->getSize(), choice);
    if (action != NAV_OK || choice == 0) return;

    Message* deleted = currentUser->inbox->removeAtIndex(choice - 1);
    if (deleted != nullptr) {
        currentUser->undoStack->push(deleted);
        saveData(false);
        setColor(GREEN);
        cout << "   [SUCCESS] Message deleted! Use Undo to recover.\n";
        resetColor();
    } else{
        setColor(RED);
        cout << "   [ERROR] Invalid message number!\n";
        resetColor();
    }

    pressEnterToContinue();
}

void undoDelete(){
    showHeader();
    setColor(CYAN);
    cout << "   === UNDO DELETE ===\n";
    resetColor();

    Message* recovered = currentUser->undoStack->pop();
    if (recovered != nullptr){
        // FIX: the same message is put back, so its original time is kept
        currentUser->inbox->enqueueMessage(recovered);
        saveData(false);
        setColor(GREEN);
        cout << "   [SUCCESS] Message from " << recovered->sender << " recovered!\n";
        resetColor();
    } else{
        setColor(RED);
        cout << "   [INFO] Nothing to undo.\n";
        resetColor();
    }

    pressEnterToContinue();
}

void manageFriends(){
    while (true) {
        showHeader();
        setColor(CYAN);
        cout << "   === FRIEND MANAGEMENT ===\n";
        resetColor();
        currentUser->displayFriends();
        cout << "\n";
        setColor(YELLOW);
        cout << "   1. Add Friend\n";
        cout << "   2. Remove Friend\n";
        cout << "   3. Back to Main Menu\n";
        resetColor();
        showNavHint();

        int choice;
        NavAction action = readNumber("\n   Choose option: ", 1, 3, choice);
        if (action != NAV_OK || choice == 3) return;

        if(choice == 1){
            while (true) {
                string friendName;
                action = readWord("   Enter friend username: ", friendName);
                if (action != NAV_OK) break;

                User* friendUser = users.search(friendName);
                if(friendUser == nullptr){
                    printError("User not found!");
                } else if(friendName == currentUser->username){
                    printError("Cannot add yourself as friend!");
                } else if(currentUser->hasFriend(friendName)){
                    printError("Already friends!");
                } else{
                    currentUser->addFriend(friendName);
                    saveData(false);
                    setColor(GREEN);
                    cout << "   [SUCCESS] " << friendName << " added to friends!\n";
                    resetColor();
                    pressEnterToContinue();
                    break;
                }
            }
        }
        else if(choice == 2){
            // Remove friend
            if(currentUser->friends == nullptr){
                setColor(BRIGHT_RED);
                cout << "   [INFO] You have no friends to remove.\n";
                resetColor();
                pressEnterToContinue();
                continue;
            }

            string friendName;
            int step = 0;   // 0 = friend name, 1 = confirm
            while (step < 2) {
                if (step == 0) {
                    action = readWord("   Enter friend username to remove: ", friendName);
                    if (action == NAV_OK && !currentUser->hasFriend(friendName)) {
                        printError("Not friends with this user!");
                        continue;
                    }
                } else {
                    bool yes;
                    setColor(BRIGHT_YELLOW);
                    action = readYesNo("   Confirm removal? (y/n): ", yes);
                    resetColor();
                    if (action == NAV_OK) {
                        if (yes) {
                            currentUser->removeFriend(friendName);
                            saveData(false);
                            printSuccess("Friend removed!");
                        } else {
                            printInfo("Cancelled.");
                        }
                        pressEnterToContinue();
                    }
                }

                if (action == NAV_MENU) return;
                if (action == NAV_BACK) {
                    if (step == 0) break;
                    step--;
                    continue;
                }
                step++;
            }
        }

        if (action == NAV_MENU) return;
    }
}

void searchUsers(){
    showHeader();
    setColor(CYAN);
    cout << "   === SEARCH USER ===\n";
    resetColor();
    showNavHint();

    string username;
    NavAction action = readWord("\n   Enter username to search: ", username);
    if (action != NAV_OK) return;

    User* found = userBST.search(username);
    if (found != nullptr){
        setColor(GREEN);
        cout << "   ✅ User Found!\n";
        cout << "   Username: " << found->username << "\n";
        cout << "   Status: " << (found->isOnline ? "🟢 Online" : "⚫ Offline") << "\n";
        if(!found->isOnline){
            cout << "   Last seen: " << found->lastSeen << "\n";
        }
        resetColor();
    } else{
        setColor(RED);
        cout << "   ❌ User not found!\n";
        resetColor();
    }

    pressEnterToContinue();
}

void searchMessages() {
    showHeader();
    setColor(BRIGHT_CYAN);
    cout << "   === SEARCH MESSAGES ===\n";
    resetColor();
    showNavHint();

    string keyword;
    while (true) {
        NavAction action = readLine("\n   Enter keyword to search: ", keyword);
        if (action != NAV_OK) return;
        if (!keyword.empty()) break;
        printError("Keyword cannot be empty!");
    }

    setColor(BRIGHT_YELLOW);
    cout << "\n   🔍 Searching for: \"" << keyword << "\"\n";
    cout << "   ═══════════════════════════════════════════\n";
    resetColor();

    // Use the MessageQueue's search method
    currentUser->inbox->searchAndDisplay(keyword);

    pressEnterToContinue();
}

// NEW: asks for a group name until an existing group is typed.
// Returns NAV_BACK / NAV_MENU if the user leaves.
NavAction askExistingGroup(const string& prompt, Group*& group){
    while (true) {
        string groupName;
        NavAction action = readLine(prompt, groupName);
        if (action != NAV_OK) return action;

        group = groupManager.findGroup(groupName);
        if (group != nullptr) return NAV_OK;
        printError("Group not found!");
    }
}

void groupChatMenu() {
    while (true) {
        showHeader();
        setColor(BRIGHT_CYAN);
        cout << "   === GROUP CHAT ===\n";
        resetColor();
        groupManager.listGroups();
        cout << "\n";
        setColor(BRIGHT_YELLOW);
        cout << "   1. Create Group\n";
        cout << "   2. Send Message to Group\n";
        cout << "   3. View Group Messages\n";
        cout << "   4. Add Member to Group\n";
        cout << "   5. View Group Members\n";
        cout << "   6. Back to Main Menu\n";
        resetColor();
        showNavHint();

        int choice;
        NavAction action = readNumber("\n   Choose option: ", 1, 6, choice);
        if (action != NAV_OK || choice == 6) return;

        if (choice == 1) {
            while (true) {
                string groupName;
                action = readLine("   Enter group name: ", groupName);
                if (action != NAV_OK) break;

                if (groupName.empty() || groupName.length() > 30) {
                    printError("Group name must be 1-30 characters!");
                } else if (groupManager.findGroup(groupName) != nullptr) {
                    printError("A group with this name already exists!");
                } else {
                    groupManager.createGroup(groupName, currentUser->username);
                    saveData(false);
                    pressEnterToContinue();
                    break;
                }
            }
        }
        else if (choice == 2) {
            Group* group = nullptr;
            int step = 0;   // 0 = group, 1 = message
            while (step < 2) {
                if (step == 0) {
                    action = askExistingGroup("   Enter group name: ", group);
                    // FIX: only members can send messages to a group
                    if (action == NAV_OK && !group->isMember(currentUser->username)) {
                        printError("You are not a member of this group!");
                        continue;
                    }
                } else {
                    string message;
                    action = readLine("   Message: ", message);
                    if (action == NAV_OK) {
                        if (message.empty()) {
                            printError("Message cannot be empty!");
                            continue;
                        }
                        group->sendMessage(currentUser->username, message);
                        saveData(false);
                        setColor(BRIGHT_GREEN);
                        cout << "   [SUCCESS] Message sent to group!\n";
                        resetColor();
                        pressEnterToContinue();
                    }
                }

                if (action == NAV_MENU) return;
                if (action == NAV_BACK) {
                    if (step == 0) break;
                    step--;
                    continue;
                }
                step++;
            }
        }
        else if (choice == 3) {
            Group* group = nullptr;
            while (true) {
                action = askExistingGroup("   Enter group name: ", group);
                if (action != NAV_OK) break;

                // FIX: only members (or the admin) can read a group's messages
                if (!group->isMember(currentUser->username) && !currentUser->isAdmin) {
                    printError("You are not a member of this group!");
                    continue;
                }
                group->displayMessages();
                saveData(false);
                pressEnterToContinue();
                break;
            }
        }
        else if (choice == 4) {
            Group* group = nullptr;
            int step = 0;   // 0 = group, 1 = username
            while (step < 2) {
                if (step == 0) {
                    action = askExistingGroup("   Enter group name: ", group);
                    if (action == NAV_OK && group->creator != currentUser->username) {
                        printError("Only group creator can add members!");
                        continue;
                    }
                } else {
                    string username;
                    action = readWord("   Enter username to add: ", username);
                    if (action == NAV_OK) {
                        // FIX: before, any name could be added, even users that do not exist
                        if (users.search(username) == nullptr) {
                            printError("User not found!");
                            continue;
                        }
                        if (group->isMember(username)) {
                            printError(username + " is already in this group!");
                            continue;
                        }
                        groupManager.addToGroup(group->groupName, username, currentUser->username);
                        saveData(false);
                        pressEnterToContinue();
                    }
                }

                if (action == NAV_MENU) return;
                if (action == NAV_BACK) {
                    if (step == 0) break;
                    step--;
                    continue;
                }
                step++;
            }
        }
        else if (choice == 5) {
            Group* group = nullptr;
            action = askExistingGroup("   Enter group name: ", group);
            if (action == NAV_OK) {
                group->displayMembers();
                pressEnterToContinue();
            }
        }

        if (action == NAV_MENU) return;
    }
}

void viewAllUsers(){
    showHeader();
    userBST.displayAllUsers();
    pressEnterToContinue();
}

void changePassword() {
    showHeader();
    setColor(CYAN);
    cout << "   === CHANGE PASSWORD ===\n";
    resetColor();
    showNavHint();

    int step = 0;   // 0 = current password, 1 = new password
    while (step < 2) {
        NavAction action = NAV_OK;

        if (step == 0) {
            string oldPassword;
            action = readWord("\n   Enter current password: ", oldPassword);
            if (action == NAV_OK && currentUser->password != oldPassword) {
                printError("Incorrect password!");
                continue;
            }
        } else {
            // FIX: the new password is now checked (min 4 characters) and confirmed
            string newPassword;
            action = askNewPassword("Enter new password", newPassword);
            if (action == NAV_OK) {
                currentUser->password = newPassword;
                saveData(false);
                setColor(GREEN);
                cout << "   [SUCCESS] Password changed successfully!\n";
                resetColor();
            }
        }

        if (action == NAV_MENU) return;
        if (action == NAV_BACK) {
            if (step == 0) return;
            step--;
            continue;
        }
        step++;
    }

    pressEnterToContinue();
}

//  ADVANCED ADMIN: REMOVE FRIEND FUNCTION 
// Returns NAV_MENU if the admin typed 'm' (go straight to the chat menu)
NavAction adminRemoveFriend(){
    if(!currentUser->isAdmin){
        setColor(BRIGHT_RED);
        cout << "   [ERROR] Admin access required!\n";
        resetColor();
        return NAV_BACK;
    }

    while (true) {
        setColor(BRIGHT_CYAN);
        cout << "\n ╔════════════════════════════════════════════════╗\n";
        cout << "   ║     ADMIN: FRIEND MANAGEMENT (Super Mode)      ║\n";
        cout << "   ╚════════════════════════════════════════════════╝\n";
        resetColor();

        setColor(BRIGHT_YELLOW);
        cout << "\n ┌─────────────────────────────────────┐\n";
        cout << "   │  1. Remove your OWN friend          │\n";
        cout << "   │  2. Remove friend from OTHER user   │\n";
        cout << "   │  3. Back to Admin Panel             │\n";
        cout << "   └─────────────────────────────────────┘\n";
        resetColor();
        showNavHint();

        int removeChoice;
        NavAction action = readNumber("\n   Choose option: ", 1, 3, removeChoice);
        if (action == NAV_MENU) return NAV_MENU;
        if (action == NAV_BACK || removeChoice == 3) return NAV_BACK;

        // 1 = the admin's own list, 2 = another user's list
        User* targetUser = (removeChoice == 1) ? currentUser : nullptr;
        string friendName;

        // 0 = whose list (only for option 2), 1 = friend name, 2 = confirm
        int step = (removeChoice == 1) ? 1 : 0;
        int firstStep = step;
        bool finished = false;

        if (removeChoice == 1) {
            setColor(BRIGHT_GREEN);
            cout << "\n   === Removing from YOUR friends list ===\n";
            resetColor();
            if (currentUser->friends == nullptr) {
                setColor(BRIGHT_RED);
                cout << "   [INFO] You have no friends to remove.\n";
                resetColor();
                continue;
            }
            setColor(BRIGHT_YELLOW);
            cout << "\n   Your current friends:\n";
            resetColor();
            currentUser->displayFriends();
        }

        while (!finished) {
            if (step == 0) {
                string username;
                action = readWord("\n   Enter username whose friend you want to remove: ", username);
                if (action == NAV_OK) {
                    targetUser = users.search(username);
                    if (targetUser == nullptr) {
                        printError("User not found!");
                        continue;
                    }
                    // Prevent removing friends from another admin
                    if (targetUser->isAdmin && targetUser != currentUser) {
                        printError("Cannot remove friends from another admin!");
                        continue;
                    }
                    if (targetUser->friends == nullptr) {
                        printInfo("This user has no friends to remove.");
                        continue;
                    }
                    setColor(BRIGHT_YELLOW);
                    cout << "\n   Current friends of '" << targetUser->username << "':\n";
                    resetColor();
                    targetUser->displayFriends();
                }
            } else if (step == 1) {
                action = readWord("\n   Enter friend username to remove: ", friendName);
                if (action == NAV_OK && !targetUser->hasFriend(friendName)) {
                    printError("'" + friendName + "' is not in " + targetUser->username + "'s friends list!");
                    continue;
                }
            } else {
                bool yes;
                setColor(BRIGHT_YELLOW);
                action = readYesNo("   Confirm removal? (y/n): ", yes);
                resetColor();
                if (action == NAV_OK) {
                    if (yes) {
                        targetUser->removeFriend(friendName);
                        saveData(false);
                        setColor(BRIGHT_GREEN);
                        cout << "   [SUCCESS] Friend '" << friendName << "' removed from '"
                             << targetUser->username << "'s friends!\n";
                        resetColor();
                    } else {
                        printInfo("Operation cancelled.");
                    }
                    finished = true;
                    continue;
                }
            }

            if (action == NAV_MENU) return NAV_MENU;
            if (action == NAV_BACK) {
                if (step == firstStep) break;   // back to the friend-management options
                step--;
                continue;
            }
            step++;
        }
    }
}

//  END OF ADMIN REMOVE FRIEND FUNCTION 
void adminPanel(){
    if(!currentUser->isAdmin){
        setColor(BRIGHT_RED);
        cout << "   [ERROR] Admin access required!\n";
        resetColor();
        pressEnterToContinue();
        return;
    }

    while (true) {
        showHeader();
        setColor(BRIGHT_RED);
        cout << "   ╔══════════════════════════════════════╗\n";
        cout << "   ║           ADMIN PANEL                ║\n";
        cout << "   ╚══════════════════════════════════════╝\n";
        resetColor();
        cout << "\n";
        setColor(BRIGHT_YELLOW);
        cout << "   ┌─────────────────────────────────────┐\n";
        cout << "   │  1. List All Users                  │\n";
        cout << "   │  2. Delete User                     │\n";
        cout << "   │  3. Manage Friends (Admin Mode)     │\n";
        cout << "   │  4. View System Stats               │\n";
        cout << "   │  5. Backup Data                     │\n";
        cout << "   │  6. Back to Main Menu               │\n";
        cout << "   └─────────────────────────────────────┘\n";
        resetColor();
        showNavHint();

        int choice;
        NavAction action = readNumber("\n   Choose option: ", 1, 6, choice);
        if (action != NAV_OK || choice == 6) return;

        if(choice == 1){
            userBST.displayAllUsers();
            pressEnterToContinue();
        }
        else if(choice == 2){
            User* userToDelete = nullptr;
            int step = 0;   // 0 = username, 1 = confirm
            while (step < 2) {
                if (step == 0) {
                    string username;
                    action = readWord("   Enter username to delete: ", username);
                    if (action == NAV_OK) {
                        userToDelete = users.search(username);
                        if (username == currentUser->username) {
                            printError("Cannot delete yourself!");
                            continue;
                        }
                        if (userToDelete == nullptr) {
                            printError("User not found!");
                            continue;
                        }
                        if (userToDelete->isAdmin) {
                            printError("Cannot delete another admin!");
                            continue;
                        }
                    }
                } else {
                    bool yes;
                    setColor(BRIGHT_YELLOW);
                    action = readYesNo("   Delete '" + userToDelete->username + "' permanently? (y/n): ", yes);
                    resetColor();
                    if (action == NAV_OK) {
                        if (yes) {
                            string username = userToDelete->username;

                            // FIX: also remove the deleted user from everyone's
                            // friend list and from all groups
                            vector<User*> allUsers = users.getAllUsers();
                            for (User* u : allUsers) {
                                if (u != userToDelete) u->removeFriend(username);
                            }
                            groupManager.removeUserEverywhere(username);

                            if (users.remove(username)) {
                                userBST.rebuildFromHashTable(users);
                                saveData(false);
                                setColor(BRIGHT_GREEN);
                                cout << "   [SUCCESS] User '" << username << "' deleted!\n";
                                resetColor();
                            } else {
                                printError("Failed to delete user!");
                            }
                        } else {
                            printInfo("Cancelled.");
                        }
                        pressEnterToContinue();
                    }
                }

                if (action == NAV_MENU) return;
                if (action == NAV_BACK) {
                    if (step == 0) break;
                    step--;
                    continue;
                }
                step++;
            }
        } else if(choice == 3){
            if (adminRemoveFriend() == NAV_MENU) return;
            continue;
        } else if(choice == 4){
            setColor(BRIGHT_CYAN);
            cout << "\n ╔══════════════════════════════════════╗\n";
            cout << "   ║         SYSTEM STATISTICS            ║\n";
            cout << "   ╚══════════════════════════════════════╝\n";
            resetColor();

            vector<User*> allUsers = users.getAllUsers();
            int onlineCount = 0;
            int totalMessages = 0;
            int totalFriends = 0;
            int totalGroups = groupManager.count();

            for(User* u : allUsers){
                if (u->isOnline) onlineCount++;
                totalMessages += u->inbox->getSize();

                // Count friends
                FriendNode* f = u->friends;
                while (f != nullptr){
                    totalFriends++;
                    f = f->next;
                }
            }

            setColor(BRIGHT_GREEN);
            cout << "\n   • Total Users: " << allUsers.size() << "\n";
            cout << "   • Online Users: " << onlineCount << "\n";
            cout << "   • Offline Users: " << (allUsers.size() - onlineCount) << "\n";
            cout << "   • Total Messages: " << totalMessages << "\n";
            cout << "   • Total Friend Connections: " << totalFriends << "\n";
            cout << "   • Total Groups: " << totalGroups << "\n";
            resetColor();
            pressEnterToContinue();
        } else if(choice == 5){
            backupData();
            pressEnterToContinue();
        }

        if (action == NAV_MENU) return;
    }
}

void logout(){
    if (currentUser != nullptr) {
        currentUser->isOnline = false;
        currentUser->lastSeen = getCurrentTimestamp();
        currentUser = nullptr;
        saveData(false);
    }
    setColor(GREEN);
    cout << "   [INFO] Logged out successfully!\n";
    resetColor();
    pressEnterToContinue();
}

public:
    ChatSystem() : currentUser(nullptr), running(true){
    loadData();
    // Create admin user if none exists
    if (users.search("admin") == nullptr){
        User* admin = new User("admin", "adminF13", true);
        users.insert(admin);
        userBST.insert(admin);
        saveData(false);
    }
    }

    ~ChatSystem(){
        saveData(false);
    }

    void run(){
        while(running){
            if(currentUser == nullptr){
                //  MAIN MENU (Not Logged In) 
                showMainMenu();
                int choice;
                // 'b' or 'm' here simply shows the main menu again
                if (readNumber("\n   Choose option: ", 1, 4, choice) != NAV_OK) continue;

                switch(choice){
                    case 1: registerUser(); break;
                    case 2: login(); break;
                    case 3: forgotPassword(); break;
                    case 4: running = false; break;
                }
        } else{
            // USER MENU (Logged In)
            showUserMenu();
            int choice;
            int lastOption = currentUser->isAdmin ? 13 : 12;
            // 'b' or 'm' here simply shows the chat menu again
            if (readNumber("\n   Choose option: ", 1, lastOption, choice) != NAV_OK) continue;

            switch(choice){
                case 1: sendPrivateMessage(); break;
                case 2: viewInbox(); break;
                case 3: deleteMessage(); break;
                case 4: undoDelete(); break;
                case 5: manageFriends(); break;
                case 6: searchUsers(); break;
                case 7: groupChatMenu(); break;
                case 8: viewAllUsers(); break;
                case 9: changePassword(); break;
                case 10: searchMessages(); break;
                case 11: forwardMessage(); break;
                case 12: logout(); break;
                case 13: adminPanel(); break;
            }
        }
        }

        saveData(true);
        setColor(BRIGHT_CYAN);
        cout << "\n   Thank you for using FNA Chat!\n";
        resetColor();
    }
};

//  MAIN FUNCTION 
int main(){
    // Set console for UTF-8 support (for emojis and box characters)
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    srand((unsigned int)time(0));   // different username suggestions and reset codes each run

    ChatSystem chatSystem;
    chatSystem.run();

    return 0;
}
