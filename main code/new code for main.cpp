#include <bits/stdc++.h>
#include "parser.h"

#define all(v) v.begin(), v.end()

using namespace std;

/** Global Constants **/
const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

bool sortByServiceTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b) {
    return (get<2>(a) < get<2>(b));
}

bool sortByArrivalTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b) {
    return (get<1>(a) < get<1>(b));
}

bool sortByResponseRatioDesc(const tuple<string, double, int> &a, const tuple<string, double, int> &b) {
    return get<1>(a) > get<1>(b);
}

bool sortByPriorityLevel(const tuple<int, int, int> &a, const tuple<int, int, int> &b) {
    if (get<0>(a) == get<0>(b))
        return get<2>(a) > get<2>(b);
    return get<0>(a) > get<0>(b);
}

void clearTimeline() {
    for (int i = 0; i < last_instant; i++) {
        fill(timeline[i], timeline[i] + process_count, ' ');
    }
}

string getProcessName(const tuple<string, int, int> &process) {
    return get<0>(process);
}

int getArrivalTime(const tuple<string, int, int> &process) {
    return get<1>(process);
}

int getServiceTime(const tuple<string, int, int> &process) {
    return get<2>(process);
}

int getPriorityLevel(const tuple<string, int, int> &process) {
    return get<2>(process);
}

double calculateResponseRatio(int waitTime, int serviceTime) {
    return (waitTime + serviceTime) * 1.0 / serviceTime;
}

void fillInWaitTime() {
    for (int i = 0; i < process_count; i++) {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i]; k++) {
            if (timeline[k][i] != '*') {
                timeline[k][i] = '.';
            }
        }
    }
}

void firstComeFirstServe() {
    int time = getArrivalTime(processes[0]);
    for (int i = 0; i < process_count; i++) {
        int arrivalTime = getArrivalTime(processes[i]);
        int serviceTime = getServiceTime(processes[i]);

        finishTime[i] = time + serviceTime;
        turnAroundTime[i] = finishTime[i] - arrivalTime;
        normTurn[i] = turnAroundTime[i] * 1.0 / serviceTime;

        fill(timeline[time], timeline[finishTime[i]], '*');
        fill(timeline[arrivalTime], timeline[time], '.');

        time += serviceTime;
    }
}

void roundRobin(int quantum) {
    queue<pair<int, int>> readyQueue;
    int currentIndex = 0;

    if (getArrivalTime(processes[currentIndex]) == 0) {
        readyQueue.push({currentIndex, getServiceTime(processes[currentIndex])});
        currentIndex++;
    }

    int currentQuantum = quantum;

    for (int time = 0; time < last_instant; time++) {
        if (!readyQueue.empty()) {
            auto [processIndex, remainingServiceTime] = readyQueue.front();
            remainingServiceTime--;
            currentQuantum--;

            timeline[time][processIndex] = '*';

            while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time + 1) {
                readyQueue.push({currentIndex, getServiceTime(processes[currentIndex])});
                currentIndex++;
            }

            if (remainingServiceTime == 0) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
                readyQueue.pop();
                currentQuantum = quantum;
            } else if (currentQuantum == 0) {
                readyQueue.pop();
                readyQueue.push({processIndex, remainingServiceTime});
                currentQuantum = quantum;
            }

            fillInWaitTime();
        }

        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time + 1) {
            readyQueue.push({currentIndex, getServiceTime(processes[currentIndex])});
            currentIndex++;
        }
    }
}

void shortestProcessNext() {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> minHeap;
    int currentIndex = 0;

    for (int time = 0; time < last_instant; time++) {
        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) <= time) {
            minHeap.push({getServiceTime(processes[currentIndex]), currentIndex});
            currentIndex++;
        }

        if (!minHeap.empty()) {
            auto [serviceTime, processIndex] = minHeap.top();
            minHeap.pop();

            for (int t = time; t < time + serviceTime; t++) {
                timeline[t][processIndex] = '*';
            }

            finishTime[processIndex] = time + serviceTime;
            turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
            normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;

            time += serviceTime - 1;
        }
    }
}

void shortestRemainingTime() {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> minHeap;
    int currentIndex = 0;

    for (int time = 0; time < last_instant; time++) {
        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time) {
            minHeap.push({getServiceTime(processes[currentIndex]), currentIndex});
            currentIndex++;
        }

        if (!minHeap.empty()) {
            auto [remainingTime, processIndex] = minHeap.top();
            minHeap.pop();

            timeline[time][processIndex] = '*';

            if (remainingTime == 1) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
            } else {
                minHeap.push({remainingTime - 1, processIndex});
            }
        }
    }

    fillInWaitTime();
}

void highestResponseRatioNext() {
    vector<tuple<string, double, int>> readyProcesses;
    int currentIndex = 0;

    for (int time = 0; time < last_instant; time++) {
        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) <= time) {
            readyProcesses.emplace_back(getProcessName(processes[currentIndex]), 1.0, 0);
            currentIndex++;
        }

        for (auto &proc : readyProcesses) {
            int processIndex = processToIndex[get<0>(proc)];
            int waitTime = time - getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            get<1>(proc) = calculateResponseRatio(waitTime, serviceTime);
        }

        sort(all(readyProcesses), sortByResponseRatioDesc);

        if (!readyProcesses.empty()) {
            int processIndex = processToIndex[get<0>(readyProcesses[0])];

            while (time < last_instant && get<2>(readyProcesses[0]) != getServiceTime(processes[processIndex])) {
                timeline[time][processIndex] = '*';
                time++;
                get<2>(readyProcesses[0])++;
            }

            time--;
            readyProcesses.erase(readyProcesses.begin());

            finishTime[processIndex] = time + 1;
            turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
            normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
        }
    }

    fillInWaitTime();
}

void feedbackQueue1() {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> minHeap;
    unordered_map<int, int> remainingServiceTime;
    int currentIndex = 0;

    if (getArrivalTime(processes[0]) == 0) {
        minHeap.push({0, currentIndex});
        remainingServiceTime[currentIndex] = getServiceTime(processes[currentIndex]);
        currentIndex++;
    }

    for (int time = 0; time < last_instant; time++) {
        if (!minHeap.empty()) {
            auto [priorityLevel, processIndex] = minHeap.top();
            minHeap.pop();

            remainingServiceTime[processIndex]--;

            timeline[time][processIndex] = '*';

            if (remainingServiceTime[processIndex] > 0) {
                minHeap.push({priorityLevel + 1, processIndex});
            } else {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
            }

            while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time + 1) {
                minHeap.push({0, currentIndex});
                remainingServiceTime[currentIndex] = getServiceTime(processes[currentIndex]);
                currentIndex++;
            }
        }

        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time + 1) {
            minHeap.push({0, currentIndex});
            remainingServiceTime[currentIndex] = getServiceTime(processes[currentIndex]);
            currentIndex++;
        }
    }

    fillInWaitTime();
}

void feedbackQueue2i(int quantum) {
    queue<pair<int, int>> readyQueue;
    int currentIndex = 0;

    if (getArrivalTime(processes[currentIndex]) == 0) {
        readyQueue.push({currentIndex, getServiceTime(processes[currentIndex])});
        currentIndex++;
    }

    int currentQuantum = quantum;

    for (int time = 0; time < last_instant; time++) {
        if (!readyQueue.empty()) {
            auto [processIndex, remainingServiceTime] = readyQueue.front();
            remainingServiceTime--;
            currentQuantum--;

            timeline[time][processIndex] = '*';

            while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time + 1) {
                readyQueue.push({currentIndex, getServiceTime(processes[currentIndex])});
                currentIndex++;
            }

            if (remainingServiceTime == 0) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
                readyQueue.pop();
                currentQuantum = quantum;
            } else if (currentQuantum == 0) {
                readyQueue.pop();
                readyQueue.push({processIndex, remainingServiceTime});
                currentQuantum = quantum;
            }
        }

        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) == time + 1) {
            readyQueue.push({currentIndex, getServiceTime(processes[currentIndex])});
            currentIndex++;
        }
    }

    fillInWaitTime();
}

void agingAlgorithm() {
    priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<>> minHeap;
    int currentIndex = 0;

    for (int time = 0; time < last_instant; time++) {
        while (currentIndex < process_count && getArrivalTime(processes[currentIndex]) <= time) {
            minHeap.push({0, time - getArrivalTime(processes[currentIndex]), currentIndex});
            currentIndex++;
        }

        if (!minHeap.empty()) {
            auto [priorityLevel, waitingTime, processIndex] = minHeap.top();
            minHeap.pop();

            timeline[time][processIndex] = '*';

            finishTime[processIndex] = time + 1;
            turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
            normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);

            // Reinsert process with increased priority level
            priorityLevel++;
            minHeap.push({priorityLevel, time - getArrivalTime(processes[processIndex]), processIndex});
        }
    }

    fillInWaitTime();
}

void printAlgorithm(int algorithmIndex) {
    cout << "Algorithm: " << ALGORITHMS[algorithmIndex] << endl;
}

void printProcesses() {
    for (const auto &process : processes) {
        cout << "Process: " << getProcessName(process)
             << ", Arrival Time: " << getArrivalTime(process)
             << ", Service Time: " << getServiceTime(process) << endl;
    }
}

void printArrivalTime() {
    cout << "Arrival Times: ";
    for (int i = 0; i < process_count; i++) {
        cout << getArrivalTime(processes[i]) << " ";
    }
    cout << endl;
}

void printServiceTime() {
    cout << "Service Times: ";
    for (int i = 0; i < process_count; i++) {
        cout << getServiceTime(processes[i]) << " ";
    }
    cout << endl;
}

void printFinishTime() {
    cout << "Finish Times: ";
    for (int i = 0; i < process_count; i++) {
        cout << finishTime[i] << " ";
    }
    cout << endl;
}

void printTurnAroundTime() {
    cout << "Turnaround Times: ";
    for (int i = 0; i < process_count; i++) {
        cout << turnAroundTime[i] << " ";
    }
    cout << endl;
}

void printNormTurnAroundTime() {
    cout << "Normalized Turnaround Times: ";
    for (int i = 0; i < process_count; i++) {
        cout << fixed << setprecision(2) << normTurn[i] << " ";
    }
    cout << endl;
}

void printStatistics() {
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurnAroundTime();
}

int main() {
    // Example usage:
    parseProcesses();  // Assumed to populate the `processes` vector and set the `last_instant` variable

    // Select algorithm index based on user input or testing:
    int algorithmIndex = 1;  // Example for FCFS
    switch (algorithmIndex) {
        case 1:
            firstComeFirstServe();
            break;
        case 2:
            roundRobin(3);  // Example quantum of 3
            break;
        case 3:
            shortestProcessNext();
            break;
        case 4:
            shortestRemainingTime();
            break;
        case 5:
            highestResponseRatioNext();
            break;
        case 6:
            feedbackQueue1();
            break;
        case 7:
            feedbackQueue2i(3);  // Example quantum of 3
            break;
        case 8:
            agingAlgorithm();
            break;
    }

    if (SHOW_STATISTICS == "stats") {
        printAlgorithm(algorithmIndex);
        printStatistics();
    }

    return 0;
}
