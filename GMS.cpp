#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
using namespace std;

// ---------------- RISK CLASSIFICATION ----------------
string riskLevel(double g) {
    if (g < 10) return "Normal";
    else if (g < 12) return "Pre-Diabetic";
    else return "DANGER";
}

// ---------------- TIME FORMAT (BD) ----------------
string formatTime(double t) {
    int hour = (int)t;
    int minute = (int)((t - hour) * 60);

    string period = "AM";
    if (hour >= 12) {
        period = "PM";
        if (hour > 12) hour -= 12;
    }
    if (hour == 0) hour = 12;

    char buf[15];
    sprintf(buf, "%02d:%02d %s", hour, minute, period.c_str());
    return string(buf);
}

// ---------------- MEAL INFO BASED ON TIME ----------------
string mealInfo(double t) {
    int hour = (int)t;
    if (hour >= 5 && hour < 8) return "Before Meal";
    else if (hour >= 8 && hour <= 11) return "After Meal";
    else if (hour >= 12 && hour <= 16) return "Before Meal";
    else return "After Meal";
}

int main() {

    int n;
    cout << "Enter number of data points: ";
    cin >> n;

    double x[20], y[20];
    int missingPos[20], missingCount = 0;

    cout << "\nEnter Time (hr) and Glucose (real-time, e.g. 5,6,7)\n";
    cout << "If glucose is missing, enter -1\n\n";

    for (int i = 0; i < n; i++) {
        cout << "Time[" << i << "]: ";
        cin >> x[i];

        if (i > 0 && x[i] <= x[i - 1])
            x[i] += 12;   // BD time normalization

        cout << "Glucose[" << i << "]: ";
        cin >> y[i];

        if (y[i] == -1)
            missingPos[missingCount++] = i;

        cout << endl;
    }

    ofstream report("report.txt");

    // ---------------- REPORT HEADER ----------------
    report << setw(35) << "GLUCOSE MONITORING REPORT\n";
    report << setw(35) << "=========================\n\n";

    report << left << setw(15) << "Time"
           << setw(15) << "Glucose"
           << setw(15) << "Risk"
           << "Meal Info / Algorithm\n";
    report << string(70, '-') << endl;

    // ---------------- MISSING VALUE HANDLING ----------------
    if (missingCount == 0) {
        report << "No Missing Value Detected\n";
        report << string(70, '-') << endl;
    }
    else if (missingCount == 1) { // Single missing -> Forward/Backward
        int k = missingPos[0];
        double h = x[1] - x[0];
        double missing;

        string algo;
        if (k <= n / 2) {
            missing = y[0] + ((x[k] - x[0]) / h) * (y[1] - y[0]); // Newton Forward
            algo = "Newton Forward Interpolation";
        } else {
            int last = n - 1;
            missing = y[last] + ((x[k] - x[last]) / h) * (y[last] - y[last - 1]); // Newton Backward
            algo = "Newton Backward Interpolation";
        }
        y[k] = missing;
    }
    else if (missingCount > 1) { // Multiple missing -> Linear
        report << "Method Used: Segment-wise Linear Interpolation\n";
        report << "Multiple Missing Values Detected\n";
        report << string(70, '-') << endl;
        for (int m = 0; m < missingCount; m++) {
            int i = missingPos[m];
            int L = i - 1, R = i + 1;
            while (L >= 0 && y[L] == -1) L--;
            while (R < n && y[R] == -1) R++;
            if (L >= 0 && R < n) {
                y[i] = y[L] + (y[R] - y[L]) * (x[i] - x[L]) / (x[R] - x[L]);
            }
        }
    }

    // ---------------- FINAL DATA TABLE ----------------
    for (int i = 0; i < n; i++) {
        string algo = "";
        for (int m = 0; m < missingCount; m++)
            if (i == missingPos[m])
                algo = missingCount == 1 ? (i <= n/2 ? "Newton Forward Interpolation" : "Newton Backward Interpolation")
                                         : "Segment-wise Linear Interpolation";

        report << left << setw(15) << formatTime(x[i])
               << setw(15) << fixed << setprecision(2) << y[i]
               << setw(15) << riskLevel(y[i])
               << mealInfo(x[i]) + (algo != "" ? " / " + algo : "") << endl;
    }

    // ---------------- INTERPOLATION ----------------
    double t;
    cout << "Enter time for interpolation (e.g. 10.5): ";
    cin >> t;
    if (t < x[0]) t += 12;

    int k = missingCount > 0 ? missingPos[0] : 1;
    double interp =
        y[k - 1] +
        (t - x[k - 1]) *
        (y[k] - y[k - 1]) / (x[k] - x[k - 1]);

    report << "\n" << string(70, '-') << endl;
    report << "Interpolated Glucose at "
           << formatTime(t) << " : "
           << fixed << setprecision(2)
           << interp << " (" << riskLevel(interp) << ")\n";

    // ---------------- DANGER TIME ----------------
    double danger;
    cout << "Enter danger glucose level (e.g. 12): ";
    cin >> danger;

    double rate = (y[k + 1] - y[k - 1]) / (x[k + 1] - x[k - 1]);
    double guess = x[k], timeD;
    for (int i = 0; i < 5; i++) {
        double f = y[k] + rate * (guess - x[k]) - danger;
        timeD = guess - f / rate;
        guess = timeD;
    }

    report << "Danger level crossed at          : "
           << formatTime(timeD) << endl;
    report << string(70, '-') << endl;

    // ---------------- MEDICAL SUGGESTIONS ----------------
    report << "\nMEDICAL SUGGESTIONS\n";
    report << "-------------------\n";

    if (interp < 10) {
        report << "- Glucose level is normal\n";
        report << "- Maintain healthy lifestyle\n";
    } else if (interp < 12) {
        report << "- Patient is pre-diabetic\n";
        report << "- Reduce sugar intake\n";
        report << "- Regular monitoring required\n";
    } else {
        report << "- CRITICAL CONDITION DETECTED\n";
        report << "- Immediate medical attention required\n";
        report << "- Strict dietary control necessary\n";
        report << "- Consult a specialist immediately\n";
    }

    report << "\n(Automatically generated using Numerical Methods)\n";
    report.close();

    cout << "\nReport generated successfully: report.txt\n";
    return 0;
}
