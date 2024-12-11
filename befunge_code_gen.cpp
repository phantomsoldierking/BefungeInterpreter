#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cctype>

using namespace std;

// GLOBAL VARIABLES
vector<vector<char>> grid; // BEFUNGE GRID
int x = 0;                // X COORDINATE OF POINTER
int y = 0;                // Y COORDINATE OF POINTER
string direction = "right"; // CURRENT DIRECTION OF MOTION
stack<int> st;         // LIFO NUMBER STORAGE
bool inQuotes = false;    // WHETHER WE'RE IN STRING MODE

ofstream outputFile("output.txt"); // OUTPUT FILE STREAM

void readFile(const string &filename);
void step();
void processInstruction(char instruction);
void move();
void printGrid();
void writeOutput(const string &output);
void logState() {
    cerr << "Pointer at (" << x << ", " << y << ") facing " << direction << endl;
    cerr << "Stack: ";
    stack<int> temp = st;
    while (!temp.empty()) {
        cerr << temp.top() << " ";
        temp.pop();
    }
    cerr << endl;
}

void safePopTwo(int &a, int &b) {
    if (st.size() < 2) {
        cerr << "ERROR: Stack underflow during operation!" << endl;
        exit(EXIT_FAILURE);
    }
    a = st.top(); st.pop();
    b = st.top(); st.pop();
}

int main(int argc, char *argv[]) {
    // CONFIRM A FILENAME WAS GIVEN
    if (argc < 2) {
        cerr << "ERROR: No input file specified!" << endl;
        return EXIT_FAILURE;
    }

    // OPEN OUTPUT FILE
    if (!outputFile.is_open()) {
        cerr << "ERROR: Could not open output file!" << endl;
        return EXIT_FAILURE;
    }

    // READ THE GRID FROM THE FILE
    readFile(argv[1]);

    // SEED RANDOM NUMBER GENERATOR
    srand(time(0));

    // REPEATEDLY TRAVERSE THE GRID
    while (grid[y][x] != '@') {
        step();
    }

    outputFile.close();
    return EXIT_SUCCESS;
}

void readFile(const string &filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "ERROR: Specified input file does not exist!" << endl;
        exit(EXIT_FAILURE);
    }

    // READ LINES INTO THE GRID
    string line;
    size_t maxLineLength = 0;
    vector<string> lines;

    while (getline(file, line)) {
        lines.push_back(line);
        maxLineLength = max(maxLineLength, line.length());
    }

    // CREATE AND POPULATE THE GRID
    for (const string &line : lines) {
        vector<char> row(maxLineLength, ' ');
        for (size_t i = 0; i < line.length(); ++i) {
            row[i] = line[i];
        }
        grid.push_back(row);
    }
}

void printGrid() {
    for (const auto &row : grid) {
        for (char col : row) {
            cout << col;
        }
        cout << endl;
    }
}

void step() {
    processInstruction(grid[y][x]);
    move();
}

void processInstruction(char instruction) {
    if (inQuotes && instruction != '"') {
        st.push(static_cast<int>(instruction));
        return;
    }
    logState();
    int a, b;
    switch (instruction) {
        case ' ': // NO-OP
            break;
        case '>':
            direction = "right";
            break;
        case '<':
            direction = "left";
            break;
        case '^':
            direction = "up";
            break;
        case 'v':
            direction = "down";
            break;
        case '+':
            safePopTwo(a, b);
            st.push(a + b);
            break;
        case '-':
            safePopTwo(a, b);
            st.push(b - a);
            break;
        case '*':
            safePopTwo(a, b);
            st.push(a * b);
            break;
        case '/':
            safePopTwo(a, b);
            if (a == 0) {
                cerr << "ERROR: Division by zero!" << endl;
                exit(EXIT_FAILURE);
            }
            st.push(b / a);
            break;
        case '%':
            safePopTwo(a, b);
            if (a == 0) {
                cerr << "ERROR: Modulo by zero!" << endl;
                exit(EXIT_FAILURE);
            }
            st.push(b % a);
            break;
        case '!': {
            int value = st.top(); st.pop();
            st.push(value == 0 ? 1 : 0);
            break;
        }
        case '`': {
            int a = st.top(); st.pop();
            int b = st.top(); st.pop();
            st.push(b > a ? 1 : 0);
            break;
        }
        case '?': {
            int randDirection = rand() % 4;
            direction = (randDirection == 0) ? "right" : (randDirection == 1) ? "left" : (randDirection == 2) ? "up" : "down";
            break;
        }
        case '_': {
            int value = st.top(); st.pop();
            direction = (value == 0) ? "right" : "left";
            break;
        }
        case '|': {
            int value = st.top(); st.pop();
            direction = (value == 0) ? "down" : "up";
            break;
        }
        case '"':
            inQuotes = !inQuotes;
            break;
        case ':': {
            int value = st.empty() ? 0 : st.top();
            st.push(value);
            break;
        }
        case '\\': {
            int a = st.top(); st.pop();
            int b = st.empty() ? 0 : st.top();
            if (!st.empty()) st.pop();
            st.push(a);
            st.push(b);
            break;
        }
        case '$':
            st.pop();
            break;
        case '.': {
            int value = st.top(); st.pop();
            string output = to_string(value) + " ";
            cout << output;
            writeOutput(output);
            break;
        }
        case ',': {
            char value = static_cast<char>(st.top()); st.pop();
            string output(1, value);
            cout << value;
            writeOutput(output);
            break;
        }
        case '#':
            move();
            break;
        case 'p': {
            int py, px, v;
            if (st.size() < 3) {
                cerr << "ERROR: Insufficient st for 'p' instruction!" << endl;
                exit(EXIT_FAILURE);
            }
            v = st.top(); st.pop();
            px = st.top(); st.pop();
            py = st.top(); st.pop();
            if (py < 0 || py >= grid.size() || px < 0 || px >= grid[0].size()) {
                cerr << "ERROR: 'p' instruction out of bounds (" << px << ", " << py << ")" << endl;
                exit(EXIT_FAILURE);
            }
            grid[py][px] = static_cast<char>(v);
            break;
        }
        case 'g': {
            int gy, gx;
            if (st.size() < 2) {
                cerr << "ERROR: Insufficient st for 'g' instruction!" << endl;
                exit(EXIT_FAILURE);
            }
            gx = st.top(); st.pop();
            gy = st.top(); st.pop();
            if (gy < 0 || gy >= grid.size() || gx < 0 || gx >= grid[0].size()) {
                cerr << "ERROR: 'g' instruction out of bounds (" << gx << ", " << gy << ")" << endl;
                exit(EXIT_FAILURE);
            }
            st.push(static_cast<int>(grid[gy][gx]));
            break;
        }

        case '&': {
            int value;
            cin >> value;
            st.push(value);
            break;
        }
        case '~': {
            char value;
            cin >> value;
            st.push(static_cast<int>(value));
            break;
        }
        default:
            if (isdigit(instruction)) {
                st.push(instruction - '0');
            }
            break;
    }
}

void move() {
    if (direction == "right") {
        x = (x + 1) % grid[0].size();
    } else if (direction == "left") {
        x = (x - 1 + grid[0].size()) % grid[0].size();
    } else if (direction == "up") {
        y = (y - 1 + grid.size()) % grid.size();
    } else if (direction == "down") {
        y = (y + 1) % grid.size();
    }

    // Ensure bounds check for safety
    if (y < 0 || y >= grid.size() || x < 0 || x >= grid[0].size()) {
        cerr << "ERROR: Pointer out of bounds at (" << x << ", " << y << ")" << endl;
        exit(EXIT_FAILURE);
    }
}


void writeOutput(const string &output) {
    if (outputFile.is_open()) {
        outputFile << output;
    }
}

