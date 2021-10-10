//test
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <bits/stdc++.h>

using namespace std;

const int READ = 0, WRITE = 1, MAX_ARGS = 20;

/* parses and process the individual lines from the text file
 *
 * only recieves one or no pipe arguement, for example it will
 * accept "ls -l" or "ls | more" but not "ls | more | sort -r"
 */
int parseLine(string line);
// opens the text file and sends lines to parseLine()
int openFile(string name);

int main(int argc, char **argv) {
    char *argvs[MAX_ARGS];
    for (int i = 1; i < argc; i++) {
        argvs[i - 1] = argv[i];
    }
    argvs[argc - 1] = NULL;

    openFile("text");

    int pipefd[2];
    pipe(pipefd);
    int cpid = fork();

    if (cpid == 0) { // child process executes ls
        close(pipefd[READ]);
        dup2(pipefd[WRITE], WRITE);
        execlp(argvs[0], argvs[0], argvs[1], NULL);
    } else { // parent process executes more
        waitpid(cpid, 0, 0);
        close(pipefd[WRITE]);
        dup2(pipefd[READ], READ);
        execlp(argvs[2], argvs[2], NULL);
        return 0;
    }
}

int parseLine(string line) {
    size_t found = line.find("|");
    char* str = strcpy(new char[line.length() + 1], line.c_str());
    char* args[MAX_ARGS];
    
    int i = 0;
    if (found != string::npos) {
        char* token = strtok(str, "|");
        char* args2[MAX_ARGS];
        char** allArgs[] = {args, args2};
        while (token != NULL) {
            stringstream s(token);
            string word;
            int j = 0;
            while (s >> word) {
                allArgs[i][j] = strcpy(new char[word.length()], word.c_str());
                j++;
            }
            allArgs[i][j] = NULL;
            j = 0;
            i++;
            token = strtok(NULL, "|");
        }
        int pipefd[2];
        pipe(pipefd);
        int cpid, cpid2;
        cpid = fork();

        if (cpid == 0) { // child process executes first command
            close(pipefd[READ]);
            dup2(pipefd[WRITE], WRITE);
            execvp(args[0], args);
        } else { // parent process executes the piped command
            cpid2 = fork();
            if (cpid2 == 0) {
                close(pipefd[WRITE]);
                dup2(pipefd[READ], READ);
                execvp(args2[0], args2);                
            } else {
                waitpid(cpid, 0, 0);
                waitpid(cpid, 0, 0);
            }
        }
    } else {
        stringstream s(line);
        string word;
        while (s >> word) {
            args[i] = strcpy(new char[word.length()], word.c_str());
            i++;
        }
        args[i] = NULL;

        int cpid = fork();

        if (cpid == 0) {
            execvp(args[0], args);
        } else {
            waitpid(cpid, 0, 0);
        }
    }
    return 0;
}

int openFile(string name) {
    string line;
    ifstream myfile(name);
    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            parseLine(line);
        }
        myfile.close();
    } else {
        cout << "unable to get file\n";
    }
    return 0;
}