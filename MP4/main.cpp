#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>
#include <random>
#include <map>
#include <algorithm>

using namespace std;

mutex coutMutex;

struct Player {
    string name;
    int score = 0;
    string answer;
    bool usedLifeline[3] = {false, false, false}; // 0: Phone-a-Friend, 1: 50/50, 2: Ask Audience
};

struct Question {
    string questionText;
    vector<string> options;
    char correctAnswer;
};

vector<Question> questions = {
    {"Who is the president of the United States in 2025?", {"A. Joe Biden", "B. Donald Trump", "C. Kamala Harris", "D. Ron DeSantis"}, 'A'},
    {"What is the capital of Japan?", {"A. Seoul", "B. Tokyo", "C. Kyoto", "D. Osaka"}, 'B'},
    {"Which planet is known as the Red Planet?", {"A. Venus", "B. Mars", "C. Jupiter", "D. Mercury"}, 'B'}
};

void lifelineMenu(Player& player, const Question& q) {
    while (true) {
        cout << "\nAvailable Lifelines:\n";
        if (!player.usedLifeline[0]) cout << "1. Phone-a-Friend\n";
        if (!player.usedLifeline[1]) cout << "2. 50/50\n";
        if (!player.usedLifeline[2]) cout << "3. Ask the Audience\n";
        cout << "0. Back\nChoose lifeline: ";
        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                if (!player.usedLifeline[0]) {
                    player.usedLifeline[0] = true;
                    cout << "[Phone-a-Friend]: I think the answer is " << q.correctAnswer << endl;
                    return;
                }
                break;
            case 2:
                if (!player.usedLifeline[1]) {
                    player.usedLifeline[1] = true;
                    cout << "[50/50]: Remaining options: " << q.correctAnswer << " and A" << endl;
                    return;
                }
                break;
            case 3:
                if (!player.usedLifeline[2]) {
                    player.usedLifeline[2] = true;
                    cout << "[Ask the Audience]: Most people chose " << q.correctAnswer << endl;
                    return;
                }
                break;
            case 0:
                return;
            default:
                cout << "Invalid choice.\n";
        }
    }
}

void answerQuestion(Player& player, const Question& q) {
    char choice;
    cout << "\n" << player.name << ", it's your turn!" << endl;
    cout << q.questionText << endl;
    for (const auto& opt : q.options) cout << opt << endl;

    while (true) {
        cout << "L. Lifeline\nEnter your answer (A/B/C/D): ";
        cin >> choice;

        if (toupper(choice) == 'L') {
            lifelineMenu(player, q);
        } else if (toupper(choice) >= 'A' && toupper(choice) <= 'D') {
            player.answer = toupper(choice);
            break;
        } else {
            cout << "Invalid answer. Try again.\n";
        }
    }
}

bool evaluateAnswer(Player& player, const Question& q) {
    this_thread::sleep_for(chrono::milliseconds(500));
    return player.answer[0] == q.correctAnswer;
}

void runGame(vector<Player>& players) {
    for (int i = 0; i < questions.size(); ++i) {
        const Question& q = questions[i];

        for (auto& player : players) {
            answerQuestion(player, q); // now sequential
        }

        vector<future<bool>> results;
        for (auto& player : players) {
            results.emplace_back(async(launch::async, evaluateAnswer, ref(player), cref(q)));
        }

        for (size_t j = 0; j < players.size(); ++j) {
            if (results[j].get()) {
                players[j].score += 10;
                cout << players[j].name << " answered correctly! +10 points.\n";
            } else {
                cout << players[j].name << " answered incorrectly.\n";
            }
        }
    }

    cout << "\nFinal Scores:\n";
    int maxScore = 0;
    for (const auto& p : players) {
        cout << p.name << ": " << p.score << "\n";
        maxScore = max(maxScore, p.score);
    }

    cout << "\nWinner(s):\n";
    for (const auto& p : players) {
        if (p.score == maxScore) cout << p.name << endl;
    }
}

int main() {
    int contestantCount;
    cout << "==== WHO WANTS TO BE A MILLIONAIRE ====" << endl;
    cout << "1. Participate\n2. Exit\nChoice: ";
    int choice;
    cin >> choice;
    if (choice != 1) return 0;

    cout << "Enter number of contestants: ";
    cin >> contestantCount;

    vector<Player> players(contestantCount);
    for (int i = 0; i < contestantCount; ++i) {
        cout << "Enter name for Player " << (i + 1) << ": ";
        cin >> players[i].name;
    }

    runGame(players);
    return 0;
}
