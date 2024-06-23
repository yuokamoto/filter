#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <algorithm> 

#include <yaml-cpp/yaml.h>

#include "../filter.hpp"

// Function to generate a Gnuplot script
void generateGnuplotScript(const std::vector<double>& v1, const std::vector<double>& v2,
                          const std::string& title, const std::string& dataFilePath, const std::string& scriptFilePath) {
    std::ofstream scriptFile(scriptFilePath);
    scriptFile << "set terminal png\n";
    scriptFile << "set output 'graph.png'\n";
    scriptFile << "set grid\n";
    scriptFile << "set title '" << title << "' \n";
    scriptFile << "set xlabel 'time[s]'\n";
    scriptFile << "set ylabel 'output'\n";
    scriptFile << "set yrange [" << *std::min_element(v2.begin(), v2.end())*1.1 << ":"  << *std::max_element(v2.begin(), v2.end())*1.1 << "]\n";
    scriptFile << "plot '" << dataFilePath << "' using 1:2 with lines title 'input', ";
    scriptFile << "'" << dataFilePath << "' using 1:3 with lines title 'filtered_out'\n";
    scriptFile.close();
}

// Function to save vector data to a file
void saveVectorDataToFile(const std::vector<double>& v1, const std::vector<double>& v2, const std::vector<double>& v3, 
                          const std::string& filePath) {
    std::ofstream outFile(filePath);
    outFile << std::fixed << std::setprecision(6);

    for (std::size_t i = 0; i < v1.size(); ++i) {
        outFile << v1[i] << " " << v2[i] <<  " " << v3[i] << "\n";
    }

    outFile.close();
}

// Function to load constraints from a YAML file
bool loadConstraintsFromYaml(const std::string& filePath, double& tau,
                            double& step_input, double& dt, double& duration) {
    try {
        YAML::Node config = YAML::LoadFile(filePath);

        step_input = config["step_input"].as<double>();

        dt = config["dt"].as<double>();
        duration = config["duration"].as<double>();

        tau = config["tau"].as<double>();

        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Failed to load constraints from YAML: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./program <filename.yaml>" << std::endl;
        return 1;
    }

    const std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    // Load constraints from YAML
    double tau, step_input, dt, duration;
    if (!loadConstraintsFromYaml(filename, tau, step_input, dt, duration)) {
        return 1;
    }

    // Generate trajectory
    FirstOrderSystem fol(tau);

    // Simulation condition
    std::vector<double> tref;
    std::vector<double> input;
    std::vector<double> filtered_output;

    // Calculate pos, vel, and acc
    for (double t = 0; t < duration; t += dt) {
        const double res = fol.update(t, step_input);
        // const double res = fol.update2(dt, step_input);
        tref.push_back(t);
        input.push_back(step_input);
        filtered_output.push_back(res);
    }

    // Save data to a file
    std::string dataFilePath = "data.txt";
    saveVectorDataToFile(tref, input, filtered_output, dataFilePath);

    // Generate Gnuplot script
    std::string scriptFilePath = "script.gnu";
    generateGnuplotScript(input, filtered_output, "First Order System, Tau=" + std::to_string(tau), dataFilePath, scriptFilePath);

    // Execute Gnuplot script
    std::string command = "gnuplot " + scriptFilePath;
    std::system(command.c_str());

    return 0;
}
