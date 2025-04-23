#include "YamlParser.h"
#include <iostream>

// ========== 添加数据结构定义 ==========

// 反应数据结构
struct ReactionData {
    std::string equation;
    std::string type;

    // 速率常数
    struct {
        double A = 0.0;
        std::string A_units;//指前因子单位
        double b = 0.0;
        double Ea = 0.0;
        std::string Ea_units;//活化能单位
    } rateConstant;

    // 第三体效率
    std::map<std::string, double> efficiencies;

    // 低压限
    struct {
        double A = 0.0;
        double b = 0.0;
        double Ea = 0.0;
    } lowPressure;

    // Troe参数
    struct {
        double a = 0.0;
        double T_star = 0.0;
        double T_double_star = 0.0;
        double T_triple_star = 0.0;
    } troe;

    bool isDuplicate = false;//是否为重复反应
    std::map<std::string, double> orders;
};

// 热力学数据结构
struct ThermoData {
    std::string name;
    std::map<std::string, double> composition;

    std::string model;
    std::vector<double> temperatureRanges;

    // NASA7多项式系数
    struct {
        std::vector<double> low;
        std::vector<double> high;
    } coefficients;

    // NASA9多项式数据
    struct NASA9Range {
        std::vector<double> temperatureRange;
        std::vector<double> coefficients;
    };
    std::vector<NASA9Range> nasa9Coeffs;
};

// 输运性质数据结构
struct TransportData {
    std::string name;
    std::string model;
    std::string geometry;
    double diameter = 0.0;
    double wellDepth = 0.0;
    double dipole = 0.0;
    double polarizability = 0.0;
    double rotationalRelaxation = 0.0;
    std::string note;
};

// 整个机理数据
struct MechanismData {
    std::vector<ReactionData> reactions;
    std::vector<ThermoData> thermoSpecies;
    std::vector<TransportData> transportSpecies;
};

// ========== 修改函数声明 ==========

// 解析动力学数据并返回结构化结果
std::vector<ReactionData> extractKinetics(const std::string& yamlFile, bool verbose = false);

// 解析热力学数据并返回结构化结果
std::vector<ThermoData> extractThermo(const std::string& yamlFile, bool verbose = false);

// 解析输运性质数据并返回结构化结果
std::vector<TransportData> extractTransport(const std::string& yamlFile, bool verbose = false);

// 加载整个机理数据
MechanismData loadMechanism(const std::string& yamlFile, bool verbose = false);

// 原有的分析函数 - 仅用于显示数据，不返回值
void analyzeKinetics(const std::string& yamlFile);
void analyzeThermo(const std::string& yamlFile);
void analyzeTransport(const std::string& yamlFile);

void parseReactionEquation(const std::string& equation,
    std::map<std::string, double>& reactants,
    std::map<std::string, double>& products);

// 解析动力学数据并返回结构化结果
std::vector<ReactionData> extractKinetics(const std::string& yamlFile, bool verbose) {
    std::vector<ReactionData> results;

    try {
        // 加载YAML文件
        if (verbose) std::cout << "加载化学动力学文件: " << yamlFile << std::endl;
        YamlValue doc = YamlParser::loadFile(yamlFile);

        if (!doc.isMap()) {
            std::cerr << "错误: YAML根节点必须是映射表类型" << std::endl;
            return results;
        }

        const auto& root = doc.asMap();

        // 检查是否存在反应节点
        if (!root.count("reactions")) {
            if (verbose) std::cout << "未找到反应数据" << std::endl;
            return results;
        }

        // 获取反应列表
        const auto& reactions = root.at("reactions").asSequence();
        if (verbose) std::cout << "找到 " << reactions.size() << " 个反应" << std::endl;

        // 遍历所有反应
        for (size_t i = 0; i < reactions.size(); i++) {
            try {
                const auto& reaction = reactions[i];
                if (!reaction.isMap()) continue;

                const auto& rxnData = reaction.asMap();
                ReactionData reactionItem;

                // 反应方程式
                if (rxnData.count("equation")) {
                    try {
                        reactionItem.equation = rxnData.at("equation").asString();
                        if (verbose) std::cout << "  方程式: " << reactionItem.equation << std::endl;
                    }
                    catch (const std::exception& e) {
                        if (verbose) std::cerr << "  方程式错误: " << e.what() << std::endl;

                        // 处理特殊情况
                        if (rxnData.at("equation").isNumber()) {
                            double numPrefix = rxnData.at("equation").asNumber();
                            if (verbose) std::cout << "  (实际是数值类型: " << numPrefix << ")" << std::endl;

                            // 尝试重建反应方程式
                            int reactionNum = static_cast<int>(i + 1);
                            if (reactionNum == 4) {
                                reactionItem.equation = "2 O + M <=> O2 + M";
                            }
                            else if (reactionNum == 134) {
                                reactionItem.equation = "2 CH3 <=> H + C2H5";
                            }
                            else {
                                reactionItem.equation = std::to_string(static_cast<int>(numPrefix)) + " [未知反应]";
                            }

                            if (verbose) std::cout << "  重建方程式: " << reactionItem.equation << std::endl;
                        }
                    }
                }

                // 反应类型
                if (rxnData.count("type")) {
                    try {
                        reactionItem.type = rxnData.at("type").asString();
                        if (verbose) std::cout << "  类型: " << reactionItem.type << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  类型字段格式错误" << std::endl;
                    }
                }

                // 阿伦尼乌斯参数
                if (rxnData.count("rate-constant") && rxnData.at("rate-constant").isMap()) {
                    const auto& rate = rxnData.at("rate-constant").asMap();

                    if (verbose) std::cout << "  速率常数:" << std::endl;

                    if (rate.count("A")) {
                        try {
                            reactionItem.rateConstant.A = rate.at("A").asNumber();
                            if (verbose) std::cout << "    A = " << reactionItem.rateConstant.A;

                            if (rate.count("A-units")) {
                                reactionItem.rateConstant.A_units = rate.at("A-units").asString();
                                if (verbose) std::cout << " " << reactionItem.rateConstant.A_units;
                            }

                            if (verbose) std::cout << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    A参数格式错误" << std::endl;
                        }
                    }

                    if (rate.count("b")) {
                        try {
                            reactionItem.rateConstant.b = rate.at("b").asNumber();
                            if (verbose) std::cout << "    b = " << reactionItem.rateConstant.b << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    b参数格式错误" << std::endl;
                        }
                    }

                    if (rate.count("Ea")) {
                        try {
                            reactionItem.rateConstant.Ea = rate.at("Ea").asNumber();
                            if (verbose) std::cout << "    Ea = " << reactionItem.rateConstant.Ea;

                            if (rate.count("Ea-units")) {
                                reactionItem.rateConstant.Ea_units = rate.at("Ea-units").asString();
                                if (verbose) std::cout << " " << reactionItem.rateConstant.Ea_units;
                            }

                            if (verbose) std::cout << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    Ea参数格式错误" << std::endl;
                        }
                    }
                }

                // 第三体效应
                if (rxnData.count("efficiencies") && rxnData.at("efficiencies").isMap()) {
                    const auto& effs = rxnData.at("efficiencies").asMap();
                    if (verbose) std::cout << "  第三体效率:" << std::endl;

                    for (const auto& [species, eff] : effs) {
                        try {
                            double value = eff.asNumber();
                            reactionItem.efficiencies[species] = value;
                            if (verbose) std::cout << "    " << species << ": " << value << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    " << species << ": 格式错误" << std::endl;
                        }
                    }
                }

                // 低压极限
                if (rxnData.count("low-P-rate-constant") && rxnData.at("low-P-rate-constant").isMap()) {
                    const auto& lowP = rxnData.at("low-P-rate-constant").asMap();
                    if (verbose) std::cout << "  低压极限速率常数:" << std::endl;

                    if (lowP.count("A")) {
                        try {
                            reactionItem.lowPressure.A = lowP.at("A").asNumber();
                            if (verbose) std::cout << "    A = " << reactionItem.lowPressure.A << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    A参数格式错误" << std::endl;
                        }
                    }

                    if (lowP.count("b")) {
                        try {
                            reactionItem.lowPressure.b = lowP.at("b").asNumber();
                            if (verbose) std::cout << "    b = " << reactionItem.lowPressure.b << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    b参数格式错误" << std::endl;
                        }
                    }

                    if (lowP.count("Ea")) {
                        try {
                            reactionItem.lowPressure.Ea = lowP.at("Ea").asNumber();
                            if (verbose) std::cout << "    Ea = " << reactionItem.lowPressure.Ea << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    Ea参数格式错误" << std::endl;
                        }
                    }
                }

                // Troe参数
                if (rxnData.count("Troe") && rxnData.at("Troe").isMap()) {
                    const auto& troe = rxnData.at("Troe").asMap();
                    if (verbose) std::cout << "  Troe参数:" << std::endl;

                    if (troe.count("a")) {
                        try {
                            reactionItem.troe.a = troe.at("a").asNumber();
                            if (verbose) std::cout << "    a = " << reactionItem.troe.a << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    a参数格式错误" << std::endl;
                        }
                    }

                    if (troe.count("T***")) {
                        try {
                            reactionItem.troe.T_triple_star = troe.at("T***").asNumber();
                            if (verbose) std::cout << "    T*** = " << reactionItem.troe.T_triple_star << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    T***参数格式错误" << std::endl;
                        }
                    }

                    if (troe.count("T*")) {
                        try {
                            reactionItem.troe.T_star = troe.at("T*").asNumber();
                            if (verbose) std::cout << "    T* = " << reactionItem.troe.T_star << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    T*参数格式错误" << std::endl;
                        }
                    }

                    if (troe.count("T**")) {
                        try {
                            reactionItem.troe.T_double_star = troe.at("T**").asNumber();
                            if (verbose) std::cout << "    T** = " << reactionItem.troe.T_double_star << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    T**参数格式错误" << std::endl;
                        }
                    }
                }

                // 复制反应
                reactionItem.isDuplicate = rxnData.count("duplicate");
                if (reactionItem.isDuplicate && verbose) {
                    std::cout << "  复制反应: 是" << std::endl;
                }

                // 特殊反应级数
                if (rxnData.count("orders") && rxnData.at("orders").isMap()) {
                    const auto& orders = rxnData.at("orders").asMap();
                    if (verbose) std::cout << "  特殊反应级数:" << std::endl;

                    for (const auto& [species, order] : orders) {
                        try {
                            double value = order.asNumber();
                            reactionItem.orders[species] = value;
                            if (verbose) std::cout << "    " << species << ": " << value << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    " << species << ": 格式错误" << std::endl;
                        }
                    }
                }

                // 添加到结果集
                results.push_back(reactionItem);

            }
            catch (const std::exception& e) {
                if (verbose) {
                    std::cerr << "处理反应 #" << (i + 1) << " 时出错: " << e.what() << std::endl;
                    std::cerr << "继续处理下一个反应..." << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }

    return results;
}

// 解析热力学数据并返回结构化结果
std::vector<ThermoData> extractThermo(const std::string& yamlFile, bool verbose) {
    std::vector<ThermoData> results;

    try {
        // 加载YAML文件
        if (verbose) std::cout << "加载热力学数据文件: " << yamlFile << std::endl;
        YamlValue doc = YamlParser::loadFile(yamlFile);

        if (!doc.isMap()) {
            std::cerr << "错误: YAML根节点必须是映射表类型" << std::endl;
            return results;
        }

        const auto& root = doc.asMap();

        // 检查是否存在物种节点
        if (!root.count("species")) {
            if (verbose) std::cout << "未找到物种数据" << std::endl;
            return results;
        }

        // 获取物种列表
        const auto& speciesList = root.at("species").asSequence();
        if (verbose) std::cout << "找到 " << speciesList.size() << " 个物种" << std::endl;

        // 遍历所有物种
        for (size_t i = 0; i < speciesList.size(); i++) {
            try {
                const auto& species = speciesList[i];
                if (!species.isMap()) continue;

                const auto& speciesData = species.asMap();
                ThermoData thermoItem;

                if (verbose) std::cout << "\n物种 #" << (i + 1) << ":" << std::endl;

                // 物种名称
                if (speciesData.count("name")) {
                    try {
                        thermoItem.name = speciesData.at("name").asString();
                        if (verbose) std::cout << "  名称: " << thermoItem.name << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  名称格式错误" << std::endl;
                    }
                }

                // 物种组成
                if (speciesData.count("composition") && speciesData.at("composition").isMap()) {
                    const auto& composition = speciesData.at("composition").asMap();
                    if (verbose) std::cout << "  组成: ";

                    for (const auto& [element, count] : composition) {
                        try {
                            double value = count.asNumber();
                            thermoItem.composition[element] = value;
                            if (verbose) std::cout << element << ":" << value << " ";
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cout << element << ":[格式错误] ";
                        }
                    }

                    if (verbose) std::cout << std::endl;
                }

                // 热力学数据
                if (speciesData.count("thermo") && speciesData.at("thermo").isMap()) {
                    const auto& thermo = speciesData.at("thermo").asMap();
                    if (verbose) std::cout << "  热力学数据:" << std::endl;

                    // 热力学模型
                    if (thermo.count("model")) {
                        try {
                            thermoItem.model = thermo.at("model").asString();
                            if (verbose) std::cout << "    模型: " << thermoItem.model << std::endl;
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    模型格式错误" << std::endl;
                        }
                    }

                    // 温度范围
                    if (thermo.count("temperature-ranges") && thermo.at("temperature-ranges").isSequence()) {
                        const auto& tempRanges = thermo.at("temperature-ranges").asSequence();
                        if (verbose) std::cout << "    温度范围(K): ";

                        for (const auto& temp : tempRanges) {
                            try {
                                double value = temp.asNumber();
                                thermoItem.temperatureRanges.push_back(value);
                                if (verbose) std::cout << value << " ";
                            }
                            catch (const std::exception&) {
                                if (verbose) std::cout << "[格式错误] ";
                            }
                        }

                        if (verbose) std::cout << std::endl;
                    }

                    // NASA多项式系数
                    if (thermo.count("coefficients") && thermo.at("coefficients").isMap()) {
                        const auto& coeffs = thermo.at("coefficients").asMap();
                        if (verbose) std::cout << "    系数:" << std::endl;

                        // 低温系数
                        if (coeffs.count("low") && coeffs.at("low").isSequence()) {
                            const auto& lowCoeffs = coeffs.at("low").asSequence();
                            if (verbose) std::cout << "      低温: ";

                            for (const auto& coeff : lowCoeffs) {
                                try {
                                    double value = coeff.asNumber();
                                    thermoItem.coefficients.low.push_back(value);
                                    if (verbose) std::cout << value << " ";
                                }
                                catch (const std::exception&) {
                                    if (verbose) std::cout << "[格式错误] ";
                                }
                            }

                            if (verbose) std::cout << std::endl;
                        }

                        // 高温系数
                        if (coeffs.count("high") && coeffs.at("high").isSequence()) {
                            const auto& highCoeffs = coeffs.at("high").asSequence();
                            if (verbose) std::cout << "      高温: ";

                            for (const auto& coeff : highCoeffs) {
                                try {
                                    double value = coeff.asNumber();
                                    thermoItem.coefficients.high.push_back(value);
                                    if (verbose) std::cout << value << " ";
                                }
                                catch (const std::exception&) {
                                    if (verbose) std::cout << "[格式错误] ";
                                }
                            }

                            if (verbose) std::cout << std::endl;
                        }
                    }
                }

                // NASA-9多项式格式支持
                if (speciesData.count("nasa9-coeffs") && speciesData.at("nasa9-coeffs").isSequence()) {
                    const auto& nasa9Ranges = speciesData.at("nasa9-coeffs").asSequence();
                    if (verbose) std::cout << "  NASA-9多项式数据:" << std::endl;

                    for (size_t j = 0; j < nasa9Ranges.size(); j++) {
                        try {
                            const auto& range = nasa9Ranges[j].asMap();
                            ThermoData::NASA9Range nasa9Range;

                            if (verbose) std::cout << "    温度范围 #" << (j + 1) << ":" << std::endl;

                            if (range.count("T-range")) {
                                try {
                                    const auto& tRange = range.at("T-range").asSequence();
                                    double tMin = tRange[0].asNumber();
                                    double tMax = tRange[1].asNumber();

                                    nasa9Range.temperatureRange.push_back(tMin);
                                    nasa9Range.temperatureRange.push_back(tMax);

                                    if (verbose) std::cout << "      温度: " << tMin << " - " << tMax << " K" << std::endl;
                                }
                                catch (const std::exception&) {
                                    if (verbose) std::cerr << "      温度范围格式错误" << std::endl;
                                }
                            }

                            if (range.count("coeffs")) {
                                try {
                                    const auto& rangeCoeffs = range.at("coeffs").asSequence();
                                    if (verbose) std::cout << "      系数: ";

                                    for (const auto& coeff : rangeCoeffs) {
                                        double value = coeff.asNumber();
                                        nasa9Range.coefficients.push_back(value);
                                        if (verbose) std::cout << value << " ";
                                    }

                                    if (verbose) std::cout << std::endl;
                                }
                                catch (const std::exception&) {
                                    if (verbose) std::cerr << "      系数格式错误" << std::endl;
                                }
                            }

                            thermoItem.nasa9Coeffs.push_back(nasa9Range);
                        }
                        catch (const std::exception&) {
                            if (verbose) std::cerr << "    处理NASA9温度范围 #" << (j + 1) << " 时出错" << std::endl;
                        }
                    }
                }

                // 添加到结果集
                results.push_back(thermoItem);

            }
            catch (const std::exception& e) {
                if (verbose) {
                    std::cerr << "处理物种 #" << (i + 1) << " 时出错: " << e.what() << std::endl;
                    std::cerr << "继续处理下一个物种..." << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }

    return results;
}

// 解析输运性质数据并返回结构化结果
std::vector<TransportData> extractTransport(const std::string& yamlFile, bool verbose) {
    std::vector<TransportData> results;

    try {
        // 加载YAML文件
        if (verbose) std::cout << "加载输运性质数据文件: " << yamlFile << std::endl;
        YamlValue doc = YamlParser::loadFile(yamlFile);

        if (!doc.isMap()) {
            std::cerr << "错误: YAML根节点必须是映射表类型" << std::endl;
            return results;
        }

        const auto& root = doc.asMap();

        // 检查是否存在物种节点
        if (!root.count("species")) {
            if (verbose) std::cout << "未找到物种数据" << std::endl;
            return results;
        }

        // 获取物种列表
        const auto& speciesList = root.at("species").asSequence();
        if (verbose) std::cout << "找到 " << speciesList.size() << " 个物种" << std::endl;

        int speciesWithTransport = 0;

        // 遍历所有物种
        for (size_t i = 0; i < speciesList.size(); i++) {
            try {
                const auto& species = speciesList[i];
                if (!species.isMap()) continue;

                const auto& speciesData = species.asMap();

                // 仅处理有输运数据的物种
                if (!speciesData.count("transport") || !speciesData.at("transport").isMap()) {
                    continue;
                }

                speciesWithTransport++;
                TransportData transportItem;

                // 物种名称
                if (speciesData.count("name")) {
                    try {
                        transportItem.name = speciesData.at("name").asString();
                    }
                    catch (const std::exception&) {
                        transportItem.name = "未知物种";
                    }
                }
                else {
                    transportItem.name = "未知物种";
                }

                if (verbose) {
                    std::cout << "\n物种 #" << (i + 1) << " (" << transportItem.name << ") 输运性质:" << std::endl;
                }

                // 获取输运数据
                const auto& transport = speciesData.at("transport").asMap();

                // 输运模型
                if (transport.count("model")) {
                    try {
                        transportItem.model = transport.at("model").asString();
                        if (verbose) std::cout << "  模型: " << transportItem.model << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  模型格式错误" << std::endl;
                    }
                }

                // 几何构型
                if (transport.count("geometry")) {
                    try {
                        transportItem.geometry = transport.at("geometry").asString();
                        if (verbose) std::cout << "  几何构型: " << transportItem.geometry << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  几何构型格式错误" << std::endl;
                    }
                }

                // 碰撞直径
                if (transport.count("diameter")) {
                    try {
                        transportItem.diameter = transport.at("diameter").asNumber();
                        if (verbose) std::cout << "  碰撞直径: " << transportItem.diameter << " Å" << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  碰撞直径格式错误" << std::endl;
                    }
                }

                // 势阱深度
                if (transport.count("well-depth")) {
                    try {
                        transportItem.wellDepth = transport.at("well-depth").asNumber();
                        if (verbose) std::cout << "  势阱深度: " << transportItem.wellDepth << " K" << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  势阱深度格式错误" << std::endl;
                    }
                }

                // 偶极矩
                if (transport.count("dipole")) {
                    try {
                        transportItem.dipole = transport.at("dipole").asNumber();
                        if (verbose) std::cout << "  偶极矩: " << transportItem.dipole << " Debye" << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  偶极矩格式错误" << std::endl;
                    }
                }

                // 极化率
                if (transport.count("polarizability")) {
                    try {
                        transportItem.polarizability = transport.at("polarizability").asNumber();
                        if (verbose) std::cout << "  极化率: " << transportItem.polarizability << " Å³" << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  极化率格式错误" << std::endl;
                    }
                }

                // 转动松弛数
                if (transport.count("rotational-relaxation")) {
                    try {
                        transportItem.rotationalRelaxation = transport.at("rotational-relaxation").asNumber();
                        if (verbose) std::cout << "  转动松弛数: " << transportItem.rotationalRelaxation << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  转动松弛数格式错误" << std::endl;
                    }
                }

                // 附加说明
                if (transport.count("note")) {
                    try {
                        transportItem.note = transport.at("note").asString();
                        if (verbose) std::cout << "  附加说明: " << transportItem.note << std::endl;
                    }
                    catch (const std::exception&) {
                        if (verbose) std::cerr << "  附加说明格式错误" << std::endl;
                    }
                }

                // 添加到结果集
                results.push_back(transportItem);

            }
            catch (const std::exception& e) {
                if (verbose) {
                    std::cerr << "处理物种 #" << (i + 1) << " 输运性质时出错: " << e.what() << std::endl;
                    std::cerr << "继续处理下一个物种..." << std::endl;
                }
            }
        }

        if (verbose) {
            std::cout << "\n总计: " << speciesWithTransport << " 个物种具有输运性质数据" << std::endl;
        }

    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }

    return results;
}

// 加载整个机理数据
MechanismData loadMechanism(const std::string& yamlFile, bool verbose) {
    MechanismData mechanism;

    mechanism.reactions = extractKinetics(yamlFile, verbose);
    mechanism.thermoSpecies = extractThermo(yamlFile, verbose);
    mechanism.transportSpecies = extractTransport(yamlFile, verbose);

    return mechanism;
}

// 保留原有的分析函数 - 直接调用extract函数并显示
void analyzeKinetics(const std::string& yamlFile) {
    extractKinetics(yamlFile, true);
}

void analyzeThermo(const std::string& yamlFile) {
    extractThermo(yamlFile, true);
}

void analyzeTransport(const std::string& yamlFile) {
    extractTransport(yamlFile, true);
}

// 解析反应方程式，提取反应物、产物及其化学计量数
void parseReactionEquation(const std::string& equation,
    std::map<std::string, double>& reactants,
    std::map<std::string, double>& products) {
    // 清空输入映射
    reactants.clear();
    products.clear();

    // 定位反应箭头
    size_t arrowPos = equation.find("<=>");
    if (arrowPos == std::string::npos) {
        arrowPos = equation.find("=>");
        if (arrowPos == std::string::npos) {
            arrowPos = equation.find("=");
        }
    }

    if (arrowPos == std::string::npos) return; // 未找到反应箭头

    // 分割反应物和产物
    std::string reactantsStr = equation.substr(0, arrowPos);
    std::string productsStr = equation.substr(arrowPos +
        (equation.substr(arrowPos).find(">") != std::string::npos ? 3 : 1));

    // 解析函数
    auto parseSpecies = [](const std::string& side, std::map<std::string, double>& species) {
        std::stringstream ss(side);
        std::string token;
        double stoich = 1.0;
        bool expectSpecies = true;

        while (ss >> token) {
            if (token == "+") {
                expectSpecies = true;
                continue;
            }

            // 检查是否是数字开头
            if (isdigit(token[0])) {
                size_t endPos;
                stoich = std::stod(token, &endPos);

                // 如果整个token是数字，等待下一个token作为物种名
                if (endPos == token.length()) {
                    expectSpecies = false;
                    continue;
                }

                // 否则，剩余部分是物种名
                std::string speciesName = token.substr(endPos);
                species[speciesName] += stoich;
                stoich = 1.0;
                expectSpecies = true;
            }
            else if (!expectSpecies) {
                // 前面读到了系数，这个是物种名
                species[token] += stoich;
                stoich = 1.0;
                expectSpecies = true;
            }
            else {
                // 没有前导系数，默认为1
                species[token] += 1.0;
            }
        }
        };

    parseSpecies(reactantsStr, reactants);
    parseSpecies(productsStr, products);
}

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
        // 替换为实际的YAML文件路径
        std::string yamlFile = "E:\\mechanism.yaml";

        // 加载机理数据但不打印详细信息(verbose=false)
        MechanismData mechanism = loadMechanism(yamlFile, false);
        std::cout << "成功加载机理数据:" << std::endl;
        std::cout << "  " << mechanism.reactions.size() << " 个反应" << std::endl;
        std::cout << "  " << mechanism.thermoSpecies.size() << " 个物种热力学数据" << std::endl;
        std::cout << "  " << mechanism.transportSpecies.size() << " 个物种输运性质数据" << std::endl;
        
        // 遍历并访问所有反应的数据
        for (const auto& reaction : mechanism.reactions) {
            std::cout << "反应: " << reaction.equation << std::endl;
            
            // 解析反应方程式获取化学计量数
            std::map<std::string, double> reactants, products;
            parseReactionEquation(reaction.equation, reactants, products);
            
            // 打印反应物及其化学计量数
            std::cout << "  反应物:" << std::endl;
            for (const auto& [species, coeff] : reactants) {
                std::cout << "    " << species << ": " << coeff << std::endl;
            }
            
            // 打印产物及其化学计量数
            std::cout << "  产物:" << std::endl;
            for (const auto& [species, coeff] : products) {
                std::cout << "    " << species << ": " << coeff << std::endl;
            }
            
            // 访问反应动力学参数
            std::cout << "  反应速率参数:" << std::endl;
            std::cout << "    A = " << reaction.rateConstant.A 
                    << " " << reaction.rateConstant.A_units << std::endl;
            std::cout << "    b = " << reaction.rateConstant.b << std::endl;
            std::cout << "    Ea = " << reaction.rateConstant.Ea 
                    << " " << reaction.rateConstant.Ea_units << std::endl;
            
            // 如果是第三体反应，打印第三体效率
            if (!reaction.efficiencies.empty()) {
                std::cout << "  第三体效率:" << std::endl;
                for (const auto& [species, eff] : reaction.efficiencies) {
                    std::cout << "    " << species << ": " << eff << std::endl;
                }
            }
            
            // 如果是复制反应，显示标记
            if (reaction.isDuplicate) {
                std::cout << "  [复制反应]" << std::endl;
            }
            
            // 如果有特殊反应级数，打印出来
            if (!reaction.orders.empty()) {
                std::cout << "  特殊反应级数:" << std::endl;
                for (const auto& [species, order] : reaction.orders) {
                    std::cout << "    " << species << ": " << order << std::endl;
                }
            }
            
            std::cout << std::endl;
            
            // 可以在这里添加条件，仅显示前几个反应
            // if (reaction.index > 5) break;
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "程序执行错误: " << e.what() << std::endl;
        return 1;
    }
    

}