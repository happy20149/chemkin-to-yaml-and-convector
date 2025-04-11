#include "YamlParser.h"
#include <iostream>

// 示例YAML数据
const char* sampleYaml = R"(
description: 这是一个示例YAML文件
version: 2.5

# 相定义
phases:
  - name: gas
    thermo: ideal-gas
    elements: [H, O, Ar]
    species: [H2, O2, H2O, Ar]
    state:
      T: 300.0  # 温度，K
      P: 101325.0  # 压力，Pa
    
# 物种定义
species:
  - name: H2
    composition: {H: 2}
    thermo:
      model: NASA7
      temperature-ranges: [200.0, 1000.0, 3500.0]
)";

int main() {
    try {
        // 从字符串加载YAML
        std::cout << "Loading YAML from string...\n" << std::endl;
        YamlValue doc = YamlParser::loadString(sampleYaml);

        // 打印解析结果
        std::cout << "Parsed YAML content:\n" << std::endl;
        doc.print();

        // 访问特定字段
        if (doc.isMap()) {
            const auto& root = doc.asMap();

            if (root.count("description")) {
                std::cout << "\nDescription: " << root.at("description").asString() << std::endl;
            }

            if (root.count("version")) {
                std::cout << "Version: " << root.at("version").asNumber() << std::endl;
            }

            // 访问嵌套数据
            if (root.count("phases")) {
                const auto& phases = root.at("phases").asSequence();
                std::cout << "\nFound " << phases.size() << " phase(s):" << std::endl;

                for (const auto& phase : phases) {
                    const auto& phaseMap = phase.asMap();
                    std::cout << "  Phase: " << phaseMap.at("name").asString();
                    if (phaseMap.count("thermo")) {
                        std::cout << ", Thermo model: " << phaseMap.at("thermo").asString();
                    }
                    std::cout << std::endl;
                }
            }
        }

        // 尝试从文件加载YAML
        std::cout << "\nTry loading from a file? (y/n): ";
        char choice;
        std::cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            std::string filename;
            std::cout << "Enter YAML file path: ";
            std::cin >> filename;

            YamlValue fileDoc = YamlParser::loadFile(filename);
            std::cout << "\nParsed YAML content from file:\n" << std::endl;
            fileDoc.print();
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}