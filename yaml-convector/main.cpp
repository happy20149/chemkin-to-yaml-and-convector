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


//测试获取yaml中的化学动力学相关的数据
// 测试获取yaml中的化学动力学相关的数据
void analyzeKinetics(const std::string& yamlFile) {
    try {
        // 加载YAML文件
        std::cout << "加载化学动力学文件: " << yamlFile << std::endl;
        YamlValue doc = YamlParser::loadFile(yamlFile);

        if (!doc.isMap()) {
            std::cerr << "错误: YAML根节点必须是映射表类型" << std::endl;
            return;
        }

        const auto& root = doc.asMap();

        // 检查是否存在反应节点
        if (!root.count("reactions")) {
            std::cout << "未找到反应数据" << std::endl;
            return;
        }

        // 获取反应列表
        const auto& reactions = root.at("reactions").asSequence();
        std::cout << "找到 " << reactions.size() << " 个反应" << std::endl;

        // 遍历所有反应
        for (size_t i = 0; i < reactions.size(); i++) {
            try {  // 为每个反应添加异常处理，防止一个反应的错误影响其他反应
                const auto& reaction = reactions[i];

                if (!reaction.isMap()) continue;
                const auto& rxnData = reaction.asMap();

                std::cout << "\n反应 #" << (i + 1) << ":" << std::endl;

                // 反应方程式 - 添加错误处理
                if (rxnData.count("equation")) {
                    try {
                        std::cout << "  方程式: " << rxnData.at("equation").asString() << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "  方程式错误: " << e.what() << std::endl;
                        
                        // 打印实际类型信息，帮助调试
                        if (rxnData.at("equation").isNumber()) {
                            std::cout << "  (实际是数值类型: " << rxnData.at("equation").asNumber() << ")" << std::endl;
                        } else if (rxnData.at("equation").isBoolean()) {
                            std::cout << "  (实际是布尔类型)" << std::endl;
                        } else if (rxnData.at("equation").isMap()) {
                            std::cout << "  (实际是映射表类型)" << std::endl;
                        } else if (rxnData.at("equation").isSequence()) {
                            std::cout << "  (实际是序列类型)" << std::endl;
                        } else {
                            std::cout << "  (实际是未知类型)" << std::endl;
                        }
                        
                        // 出现错误但继续处理该反应的其他属性
                    }
                }

                // 反应类型
                if (rxnData.count("type")) {
                    try {
                        std::cout << "  类型: " << rxnData.at("type").asString() << std::endl;
                    } catch (const std::exception&) {
                        std::cerr << "  类型字段格式错误" << std::endl;
                    }
                }

                // 阿伦尼乌斯参数 (速率常数)
                if (rxnData.count("rate-constant") && rxnData.at("rate-constant").isMap()) {
                    const auto& rate = rxnData.at("rate-constant").asMap();

                    std::cout << "  速率常数:" << std::endl;

                    if (rate.count("A")) {
                        try {
                            std::cout << "    A = " << rate.at("A").asNumber();
                            if (rate.count("A-units")) {
                                std::cout << " " << rate.at("A-units").asString();
                            }
                            std::cout << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    A参数格式错误" << std::endl;
                        }
                    }

                    if (rate.count("b")) {
                        try {
                            std::cout << "    b = " << rate.at("b").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    b参数格式错误" << std::endl;
                        }
                    }

                    if (rate.count("Ea")) {
                        try {
                            std::cout << "    Ea = " << rate.at("Ea").asNumber();
                            if (rate.count("Ea-units")) {
                                std::cout << " " << rate.at("Ea-units").asString();
                            }
                            std::cout << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    Ea参数格式错误" << std::endl;
                        }
                    }
                }

                // 第三体效应
                if (rxnData.count("efficiencies") && rxnData.at("efficiencies").isMap()) {
                    const auto& effs = rxnData.at("efficiencies").asMap();
                    std::cout << "  第三体效率:" << std::endl;

                    for (const auto& [species, eff] : effs) {
                        try {
                            std::cout << "    " << species << ": " << eff.asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    " << species << ": 格式错误" << std::endl;
                        }
                    }
                }

                // 高低压极限反应速率常数 (压力相关反应)
                if (rxnData.count("low-P-rate-constant") && rxnData.at("low-P-rate-constant").isMap()) {
                    const auto& lowP = rxnData.at("low-P-rate-constant").asMap();
                    std::cout << "  低压极限速率常数:" << std::endl;

                    if (lowP.count("A")) {
                        try {
                            std::cout << "    A = " << lowP.at("A").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    A参数格式错误" << std::endl;
                        }
                    }
                    if (lowP.count("b")) {
                        try {
                            std::cout << "    b = " << lowP.at("b").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    b参数格式错误" << std::endl;
                        }
                    }
                    if (lowP.count("Ea")) {
                        try {
                            std::cout << "    Ea = " << lowP.at("Ea").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    Ea参数格式错误" << std::endl;
                        }
                    }
                }

                // Troe参数
                if (rxnData.count("Troe") && rxnData.at("Troe").isMap()) {
                    const auto& troe = rxnData.at("Troe").asMap();
                    std::cout << "  Troe参数:" << std::endl;

                    if (troe.count("a")) {
                        try {
                            std::cout << "    a = " << troe.at("a").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    a参数格式错误" << std::endl;
                        }
                    }
                    if (troe.count("T***")) {
                        try {
                            std::cout << "    T*** = " << troe.at("T***").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    T***参数格式错误" << std::endl;
                        }
                    }
                    if (troe.count("T*")) {
                        try {
                            std::cout << "    T* = " << troe.at("T*").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    T*参数格式错误" << std::endl;
                        }
                    }
                    if (troe.count("T**")) {
                        try {
                            std::cout << "    T** = " << troe.at("T**").asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    T**参数格式错误" << std::endl;
                        }
                    }
                }

                // 温度依赖性
                if (rxnData.count("duplicate")) {
                    std::cout << "  复制反应: 是" << std::endl;
                }

                // 特殊反应级数
                if (rxnData.count("orders") && rxnData.at("orders").isMap()) {
                    const auto& orders = rxnData.at("orders").asMap();
                    std::cout << "  特殊反应级数:" << std::endl;

                    for (const auto& [species, order] : orders) {
                        try {
                            std::cout << "    " << species << ": " << order.asNumber() << std::endl;
                        } catch (const std::exception&) {
                            std::cerr << "    " << species << ": 格式错误" << std::endl;
                        }
                    }
                }
            } catch (const std::exception& e) {
                // 处理单个反应时发生的异常
                std::cerr << "处理反应 #" << (i + 1) << " 时出错: " << e.what() << std::endl;
                std::cerr << "继续处理下一个反应..." << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }
}





int main() {

    try {
        // 替换为实际的YAML文件路径
        std::string yamlFile = "E:\\mechanism.yaml";

        // 分析化学动力学数据
        analyzeKinetics(yamlFile);
        

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "程序执行错误: " << e.what() << std::endl;
        return 1;
    }
    //try {
    //    // 从字符串加载YAML
    //    std::cout << "Loading YAML from string...\n" << std::endl;
    //    YamlValue doc = YamlParser::loadString(sampleYaml);

    //    // 打印解析结果
    //    std::cout << "Parsed YAML content:\n" << std::endl;
    //    doc.print();

    //    // 访问特定字段
    //    if (doc.isMap()) {
    //        const auto& root = doc.asMap();

    //        if (root.count("description")) {
    //            std::cout << "\nDescription: " << root.at("description").asString() << std::endl;
    //        }

    //        if (root.count("version")) {
    //            std::cout << "Version: " << root.at("version").asNumber() << std::endl;
    //        }

    //        // 访问嵌套数据
    //        if (root.count("phases")) {
    //            const auto& phases = root.at("phases").asSequence();
    //            std::cout << "\nFound " << phases.size() << " phase(s):" << std::endl;

    //            for (const auto& phase : phases) {
    //                const auto& phaseMap = phase.asMap();
    //                std::cout << "  Phase: " << phaseMap.at("name").asString();
    //                if (phaseMap.count("thermo")) {
    //                    std::cout << ", Thermo model: " << phaseMap.at("thermo").asString();
    //                }
    //                std::cout << std::endl;
    //            }
    //        }
    //    }

    //    // 尝试从文件加载YAML
    //    std::cout << "\nTry loading from a file? (y/n): ";
    //    char choice;
    //    std::cin >> choice;

    //    if (choice == 'y' || choice == 'Y') {
    //        std::string filename;
    //        std::cout << "Enter YAML file path: ";
    //        std::cin >> filename;

    //        YamlValue fileDoc = YamlParser::loadFile(filename);
    //        std::cout << "\nParsed YAML content from file:\n" << std::endl;
    //        fileDoc.print();
    //    }

    //    return 0;
    //}
    //catch (const std::exception& e) {
    //    std::cerr << "Error: " << e.what() << std::endl;
    //    return 1;
    //}
}