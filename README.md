# FNA Chat

A terminal-based messaging system written in C++, made for my Data Structure (CSE 221) course.

## About the Project

FNA Chat is a chat application that runs in the terminal. Users can create an account, log in, send private messages, chat in groups, add friends and search for other users. There is also an admin panel for managing users.

The main goal of this project is to show how five basic data structures work together in one application. Each feature uses a data structure that fits it, for example a queue for the inbox and a stack for undoing a deleted message.

## Purpose

In the Data Structure course we learned queues, stacks, linked lists, trees and hash tables separately. I wanted to build something where all of them are used for a real job, instead of only writing separate practice programs. A chat system was a good fit because it needs fast user lookup, ordered messages, undo and lists of friends.

## Data Structures Used

| Data Structure | Where it is used |
|----------------|------------------|
| **Queue** (linked list) | Each user's inbox. The first message sent is the first one shown (FIFO). |
| **Stack** (linked list) | Undo delete. The last deleted message is the first one recovered (LIFO). |
| **Linked List** | Each user's friend list, and the list of all groups. |
| **Binary Search Tree** | Searching users by username and showing all users in alphabetical order. |
| **Hash Table** | Storing all users for fast login and lookup (size 101, collisions handled with chaining). |

## Features

- Register, login and logout
- Password recovery: recover username by email, reset password with a security question, or reset with a code sent to your email (simulated)
- Send private messages, with a "typing..." effect and online notification
- Inbox with unread messages marked as `[NEW]`, and mark as read / unread
- Delete a message and undo the delete
- Forward a message to another user
- Search messages by keyword (not case-sensitive, the keyword is highlighted)
- Add and remove friends
- Search users and see who is online and when they were last seen
- Group chat: create groups, add members, send and read group messages (members only)
- Change password
- Admin panel: list users, delete users, manage any user's friends, system statistics and data backup
- All data is saved to text files, so nothing is lost when the program closes
- At every input step, type `b` to go back or `m` to go to the menu

## Technologies Used

- C++ (C++11)
- `windows.h` for console colors on Windows, ANSI color codes on Linux/Mac
- File handling (`fstream`) for saving data

## Requirements

- Windows, Linux or Mac
- A C++ compiler that supports C++11 or newer, for example MinGW g++ (comes with Code::Blocks)

On Windows, emojis like 🔔 may show as boxes in the old command prompt. They show correctly in Windows Terminal. The program works the same either way.

## How to Compile and Run

**Using Code::Blocks**

1. Open `fna_chat.cpp` in Code::Blocks.
2. Press **F9** (Build and run).

**Using the command line**

```
g++ -std=c++11 fna_chat.cpp -o FNA_Chat
```

Then run `FNA_Chat.exe` on Windows, or `./FNA_Chat` on Linux/Mac.

## How to Use

1. Start the program and choose **1. Register New Account**.
2. Log in with **2. Login**.
3. Use the Chat Menu to send messages, check your inbox, manage friends and use group chat.
4. Choose **12. Logout** when you are done, then **4. Exit** from the main menu.

An admin account is created automatically the first time the program runs:

- Username: `admin`
- Password: `adminF13`

The admin sees an extra option, **13. Admin Panel**.

The program saves its data in these files, in the same folder:

| File | What it holds |
|------|---------------|
| `users.txt` | Accounts and friend lists |
| `messages.txt` | Inbox messages of every user |
| `groups.txt` | Groups, members and group messages |
| `backup_users.txt` | Created when the admin uses Backup Data |
| `email_outbox.txt` | The "sent" password reset emails (simulated) |

These files are not uploaded to GitHub (see `.gitignore`).

## Project Structure

```
├── fna_chat.cpp   (full source code)
├── README.md
└── .gitignore
```

The whole program is in one file. Inside, it is divided into sections: colors, input helpers, message queue, undo stack, user and friend list, hash table, BST, groups, and the main `ChatSystem` class that has all the menus.

## Limitations

- It is not a network chat. All users use the same program on the same computer, one at a time.
- Passwords are saved as plain text in `users.txt`.
- A console program cannot send real emails, so the reset code is written to `email_outbox.txt` instead.
- Undo delete works only until the program is closed.

## Academic Information

- **Course:** Data Structure (CSE 221)
- **Year:** 2nd Year
- **Semester:** 1st Semester
