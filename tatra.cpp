
/**
 * @file tatra.cpp
 * @brief Simple command-line Task Tracker application in C++.
 *
 * This application allows users to manage tasks via command-line interface.
 * Tasks are stored in a JSON file ("tasks.json") and support basic CRUD operations.
 *
 * Features:
 * - Add new tasks with descriptions.
 * - Update task descriptions.
 * - Delete tasks (soft delete).
 * - Mark tasks as "in-progress" or "done".
 * - List all tasks or filter by status ("todo", "in-progress", "done").
 *
 * Classes:
 * - TaskTracker: Represents a single task, with fields for description, status, timestamps, and deletion flag.
 * - TaskManager: Manages a collection of TaskTracker objects, handles file I/O, and provides task operations.
 *
 * Usage:
 *   task-cli add "description"        - Add a new task
 *   task-cli update <id> "description" - Update task description
 *   task-cli delete <id>               - Delete a task
 *   task-cli mark-in-progress <id>     - Mark task as in progress
 *   task-cli mark-done <id>            - Mark task as done
 *   task-cli list                      - List all tasks
 *   task-cli list done                 - List completed tasks
 *   task-cli list todo                 - List todo tasks
 *   task-cli list in-progress          - List in-progress tasks
 *
 * @author kumar
 * @date 2024
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <algorithm>
using namespace std;

class TaskTracker {
private:
    int id;
    bool isDeleted = false;
    
public:
    string desc;
    string status;
    string createdAt;
    string updatedAt;
    
    TaskTracker(int i) : id(i) {}
    
    TaskTracker(int i, string d, string s, string c, string u) : 
        id(i), desc(d), status(s), createdAt(c), updatedAt(u) {}
    
    int getId() const { return id; }
    
    void addTask(string d, string s, string c, string u) {
        desc = d;
        status = s;
        createdAt = c;
        updatedAt = u;
        isDeleted = false;
    }
    
    void updateTask(string d, string s, string u) {
        desc = d;
        status = s;
        updatedAt = u;
    }
    
    void updateDescription(string d, string u) {
        desc = d;
        updatedAt = u;
    }
    
    void updateStatus(string s, string u) {
        status = s;
        updatedAt = u;
    }
    
    void deleteTask() {
        isDeleted = true;
    }
    
    bool isTaskDeleted() const {
        return isDeleted;
    }
    
    void display() const {
        if (!isDeleted) {
            cout << "ID: " << id << " | " << desc << " | Status: " << status 
            << " | Created: " << createdAt << " | Updated: " << updatedAt << endl;
        }
    }
    
    // Convert task to JSON string
    string toJson() const {
        if (isDeleted) return "";
        return "  {\n    \"id\": " + to_string(id) + ",\n" +
               "    \"description\": \"" + desc + "\",\n" +
               "    \"status\": \"" + status + "\",\n" +
               "    \"createdAt\": \"" + createdAt + "\",\n" +
               "    \"updatedAt\": \"" + updatedAt + "\"\n  }";
    }
};

class TaskManager {
private:
    vector<TaskTracker> tasks;
    string filename = "tasks.json";
    int nextId = 1;
    
    string getCurrentTime() {
        time_t now = time(0);
        char* dt = ctime(&now);
        string timeStr(dt);
        timeStr.pop_back(); // Remove newline
        return timeStr;
    }
    
    void loadTasks() {
        ifstream file(filename);
        if (!file.is_open()) {
            return; // File doesn't exist yet
        }
        
        string line, content = "";
        while (getline(file, line)) {
            content += line + "\n";
        }
        file.close();
        
        if (content.empty() || content.find("[]") != string::npos) {
            return;
        }
        
        // Simple JSON parsing (basic implementation)
        size_t pos = 0;
        while ((pos = content.find("\"id\":", pos)) != string::npos) {
            size_t idStart = content.find(":", pos) + 1;
            size_t idEnd = content.find(",", idStart);
            int id = stoi(content.substr(idStart, idEnd - idStart));
            
            size_t descStart = content.find("\"description\": \"", pos) + 16;
            size_t descEnd = content.find("\"", descStart);
            string desc = content.substr(descStart, descEnd - descStart);
            
            size_t statusStart = content.find("\"status\": \"", pos) + 11;
            size_t statusEnd = content.find("\"", statusStart);
            string status = content.substr(statusStart, statusEnd - statusStart);
            
            size_t createdStart = content.find("\"createdAt\": \"", pos) + 14;
            size_t createdEnd = content.find("\"", createdStart);
            string createdAt = content.substr(createdStart, createdEnd - createdStart);
            
            size_t updatedStart = content.find("\"updatedAt\": \"", pos) + 14;
            size_t updatedEnd = content.find("\"", updatedStart);
            string updatedAt = content.substr(updatedStart, updatedEnd - updatedStart);
            
            tasks.push_back(TaskTracker(id, desc, status, createdAt, updatedAt));
            nextId = max(nextId, id + 1);
            pos = updatedEnd;
        }
    }
    
    void saveTasks() {
        ofstream file(filename);
        file << "[\n";
        
        bool first = true;
        for (const auto& task : tasks) {
            if (!task.isTaskDeleted()) {
                if (!first) file << ",\n";
                file << task.toJson();
                first = false;
            }
        }
        
        file << "\n]";
        file.close();
    }
    
public:
    TaskManager() {
        loadTasks();
    }
    
    void addTask(string description) {
        string currentTime = getCurrentTime();
        TaskTracker newTask(nextId++);
        newTask.addTask(description, "todo", currentTime, currentTime);
        tasks.push_back(newTask);
        saveTasks();
        cout << "Task added successfully (ID: " << newTask.getId() << ")" << endl;
    }
    
    void updateTask(int id, string description) {
        for (auto& task : tasks) {
            if (task.getId() == id && !task.isTaskDeleted()) {
                task.updateDescription(description, getCurrentTime());
                saveTasks();
                cout << "Task updated successfully" << endl;
                return;
            }
        }
        cout << "Task with ID " << id << " not found" << endl;
    }
    
    void deleteTask(int id) {
        for (auto& task : tasks) {
            if (task.getId() == id && !task.isTaskDeleted()) {
                task.deleteTask();
                saveTasks();
                cout << "Task deleted successfully" << endl;
                return;
            }
        }
        cout << "Task with ID " << id << " not found" << endl;
    }
    
    void markInProgress(int id) {
        for (auto& task : tasks) {
            if (task.getId() == id && !task.isTaskDeleted()) {
                task.updateStatus("in-progress", getCurrentTime());
                saveTasks();
                cout << "Task marked as in progress" << endl;
                return;
            }
        }
        cout << "Task with ID " << id << " not found" << endl;
    }
    
    void markDone(int id) {
        for (auto& task : tasks) {
            if (task.getId() == id && !task.isTaskDeleted()) {
                task.updateStatus("done", getCurrentTime());
                saveTasks();
                cout << "Task marked as done" << endl;
                return;
            }
        }
        cout << "Task with ID " << id << " not found" << endl;
    }
    
    void listAllTasks() {
        bool found = false;
        for (const auto& task : tasks) {
            if (!task.isTaskDeleted()) {
                task.display();
                found = true;
            }
        }
        if (!found) {
            cout << "No tasks found" << endl;
        }
    }
    
    void listTasksByStatus(string status) {
        bool found = false;
        for (const auto& task : tasks) {
            if (!task.isTaskDeleted() && task.status == status) {
                task.display();
                found = true;
            }
        }
        if (!found) {
            cout << "No tasks found with status: " << status << endl;
        }
    }
};

void printUsage() {
    cout << "Usage:" << endl;
    cout << "  task-cli add \"description\"        - Add a new task" << endl;
    cout << "  task-cli update <id> \"description\" - Update task description" << endl;
    cout << "  task-cli delete <id>               - Delete a task" << endl;
    cout << "  task-cli mark-in-progress <id>     - Mark task as in progress" << endl;
    cout << "  task-cli mark-done <id>            - Mark task as done" << endl;
    cout << "  task-cli list                      - List all tasks" << endl;
    cout << "  task-cli list done                 - List completed tasks" << endl;
    cout << "  task-cli list todo                 - List todo tasks" << endl;
    cout << "  task-cli list in-progress          - List in-progress tasks" << endl;
}

int main(int argc, char* argv[]) {
    TaskManager manager;
    
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    string command = argv[1];
    
    if (command == "add") {
        if (argc < 3) {
            cout << "Error: Please provide a task description" << endl;
            return 1;
        }
        manager.addTask(argv[2]);
    }
    else if (command == "update") {
        if (argc < 4) {
            cout << "Error: Please provide task ID and new description" << endl;
            return 1;
        }
        int id = stoi(argv[2]);
        manager.updateTask(id, argv[3]);
    }
    else if (command == "delete") {
        if (argc < 3) {
            cout << "Error: Please provide task ID" << endl;
            return 1;
        }
        int id = stoi(argv[2]);
        manager.deleteTask(id);
    }
    else if (command == "mark-in-progress") {
        if (argc < 3) {
            cout << "Error: Please provide task ID" << endl;
            return 1;
        }
        int id = stoi(argv[2]);
        manager.markInProgress(id);
    }
    else if (command == "mark-done") {
        if (argc < 3) {
            cout << "Error: Please provide task ID" << endl;
            return 1;
        }
        int id = stoi(argv[2]);
        manager.markDone(id);
    }
    else if (command == "list") {
        if (argc == 2) {
            manager.listAllTasks();
        }
        else if (argc == 3) {
            string status = argv[2];
            if (status == "in-progress") {
                manager.listTasksByStatus("in-progress");
            }
            else {
                manager.listTasksByStatus(status);
            }
        }
    }
    else {
        cout << "Error: Unknown command '" << command << "'" << endl;
        printUsage();
        return 1;
    }
    
    return 0;
}