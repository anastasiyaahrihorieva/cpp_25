#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>

class QuadraticEquation {
private:
    double a, b, c;
public:
    QuadraticEquation(double a, double b, double c) : a(a), b(b), c(c) {}

    std::vector<double> solve() const {
        double D = b * b - 4 * a * c;
        if (D < 0) return {};
        else if (D == 0) return { -b / (2 * a) };
        else {
            return { (-b + sqrt(D)) / (2 * a), (-b - sqrt(D)) / (2 * a) };
        }
    }
};

class Student {
protected:
    std::string name;
public:
    Student(const std::string& name) : name(name) {}
    virtual ~Student() = default;
    virtual std::vector<double> solveQuadraticEquation(const QuadraticEquation& eq) = 0;
    std::string getName() const { return name; }
};

class GoodStudent : public Student {
public:
    GoodStudent(const std::string& name) : Student(name) {}

    std::vector<double> solveQuadraticEquation(const QuadraticEquation& eq) override {
        return eq.solve();
    }
};

class AverageStudent : public Student {
private:
    std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<double> dist{ 0.0, 1.0 };
public:
    AverageStudent(const std::string& name) : Student(name) {}

    std::vector<double> solveQuadraticEquation(const QuadraticEquation& eq) override {
        if (dist(gen) > 0.5) {
            return eq.solve();
        }
        else {
            return { (rand() % 10) - 5.0 };
        }
    }
};

class BadStudent : public Student {
public:
    BadStudent(const std::string& name) : Student(name) {}

    std::vector<double> solveQuadraticEquation(const QuadraticEquation& eq) override {
        return { 0.0 };
    }
};

struct Mail {
    QuadraticEquation equation;
    std::vector<double> studentSolution;
    std::string studentName;
};

class Teacher {
private:
    std::queue<Mail> mailQueue;
    std::map<std::string, int> resultsTable;

public:
    void receiveMail(const Mail& mail) {
        mailQueue.push(mail);
        if (resultsTable.find(mail.studentName) == resultsTable.end()) {
            resultsTable[mail.studentName] = 0;
        }
    }

    void checkAllMail() {
        while (!mailQueue.empty()) {
            Mail mail = mailQueue.front();
            mailQueue.pop();

            auto correctSolution = mail.equation.solve();
            bool isCorrect = (mail.studentSolution == correctSolution);

            if (isCorrect) {
                resultsTable[mail.studentName]++;
            }
        }
    }

    void publishResults() const {
        std::ofstream res_table("results_table.txt");
        std::cout << "\nFinal results:\n";
        for (int i = 1; i <= 20; ++i) {
            std::string name = "name" + std::to_string(i);
            if (resultsTable.count(name)) {
                res_table << name << ": " << resultsTable.at(name) << " correct answers\n";
                std::cout << name << ": " << resultsTable.at(name) << " correct answers\n";
            }
        }
    }
};

int main() {
    Teacher teacher;
    std::vector<std::unique_ptr<Student>> students;

    for (int i = 1; i <= 20; ++i) {
        std::string name = "name" + std::to_string(i);

        if (i <= 9) {
            students.push_back(std::make_unique<GoodStudent>(name));
        }
        else if (i <= 15) {
            students.push_back(std::make_unique<AverageStudent>(name));
        }
        else {
            students.push_back(std::make_unique<BadStudent>(name));
        }
    }

    std::ifstream file("quadratic_equations.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        double a, b, c;
        std::string name;

        if (!(iss >> a >> b >> c >> name)) {
            continue; 
        }

        QuadraticEquation eq(a, b, c);

        auto it = std::find_if(students.begin(), students.end(),
            [&name](const auto& s) { return s->getName() == name; });

        if (it != students.end()) {
            Mail mail{ eq, (*it)->solveQuadraticEquation(eq), name };
            teacher.receiveMail(mail);
        }
    }

    teacher.checkAllMail();
    teacher.publishResults();

    return 0;
}