#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <utility>
#include <chrono>
#include <algorithm>
#include <climits>

// ---------------------- STRUKTURA ---------------------
struct StockData {
    std::string date;
    std::string ticker;
    double open;
    double high;
    double low;
    double close;
    long long volume;
    double dividend;
};

// ---------------------- KLASA ---------------------
class StockDatabase {
private:
    std::vector<StockData> allData;
    std::unordered_map<std::string, std::vector<StockData>> tickerMap;
    std::unordered_map<std::string, std::vector<StockData>> dateMap;
    std::unordered_set<std::string> tickerSet;
    std::unordered_map<std::string, std::unordered_map<std::string, StockData>> tickerDateMap;
    std::priority_queue<std::pair<double, std::string>> topDividends;

    static auto now() -> std::chrono::high_resolution_clock::time_point {
        return std::chrono::high_resolution_clock::now();
    }

    static void printQueryTime(int query, const std::chrono::high_resolution_clock::time_point& start) {
        auto end = now();
        std::cout << "Query " << query << " took "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << " ms\n\n";
    }

    static double toDouble(const std::string& token) {
        return token.empty() ? 0.0 : std::stod(token);
    }

    static long long toLongLong(const std::string& token) {
        return token.empty() ? 0LL : std::stoll(token);
    }

    bool hasTickerDate(const std::string& ticker, const std::string& date) const {
        auto it = tickerDateMap.find(ticker);
        return it != tickerDateMap.end() && it->second.count(date);
    }

    const std::vector<StockData>* getTickerData(const std::string& ticker) const {
        auto it = tickerMap.find(ticker);
        return it != tickerMap.end() ? &it->second : nullptr;
    }

    const std::vector<StockData>* getDateData(const std::string& date) const {
        auto it = dateMap.find(date);
        return it != dateMap.end() ? &it->second : nullptr;
    }

    void addStockData(const StockData& sd) {
        allData.push_back(sd);
        tickerMap[sd.ticker].push_back(sd);
        dateMap[sd.date].push_back(sd);
        tickerSet.insert(sd.ticker);
        tickerDateMap[sd.ticker][sd.date] = sd;
        if (sd.dividend > 0) {
            topDividends.push({sd.dividend, sd.ticker});
        }
    }

public:
    void loadCSV(const std::string& filename, int maxLines = 1000000) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filename << std::endl;
            return;
        }

        std::string line;
        std::getline(file, line); // header
        int lineCount = 0;

        while (std::getline(file, line) && lineCount < maxLines) {
            std::stringstream ss(line);
            std::string token;
            StockData sd;

            try {
                std::getline(ss, sd.date, ',');
                std::getline(ss, sd.ticker, ',');
                std::getline(ss, token, ','); sd.open = toDouble(token);
                std::getline(ss, token, ','); sd.high = toDouble(token);
                std::getline(ss, token, ','); sd.low = toDouble(token);
                std::getline(ss, token, ','); sd.close = toDouble(token);
                std::getline(ss, token, ','); sd.volume = toLongLong(token);
                std::getline(ss, token, ','); sd.dividend = toDouble(token);
            } catch (...) {
                continue;
            }

            addStockData(sd);
            lineCount++;
        }
    }

    // 1
    void getDataByDate(const std::string& date) {
        auto start = now();
        if (auto entries = getDateData(date)) {
            for (const auto& data : *entries) {
                std::cout << data.ticker << " " << data.date
                          << " Open: " << data.open
                          << " Close: " << data.close << std::endl;
            }
        } else {
            std::cout << "No data for date " << date << std::endl;
        }

        printQueryTime(1, start);
    }

    // 2
    void averageClosePrice(const std::string& ticker) {
        auto start = now();
        double sum = 0;
        int count = 0;
        if (auto entries = getTickerData(ticker)) {
            for (const auto& d : *entries) { sum += d.close; count++; }
            std::cout << "Average close for " << ticker << ": " << (sum / count) << "\n";
        } else {
            std::cout << "Ticker not found.\n";
        }
        printQueryTime(2, start);
    }

    // 3
    void highestPriceInRange(const std::string& ticker, const std::string& startDate, const std::string& endDate) {
        auto start = now();
        double maxHigh = -1;
        if (auto entries = getTickerData(ticker)) {
            for (const auto& d : *entries)
                if (d.date >= startDate && d.date <= endDate)
                    maxHigh = std::max(maxHigh, d.high);
            std::cout << "Max high: " << maxHigh << "\n";
        } else {
            std::cout << "Ticker not found.\n";
        }
        printQueryTime(3, start);
    }

    // 4
    void listAllTickers() {
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& t : tickerSet) std::cout << t << "\n";
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Query 4 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n\n";
    }

    // 5
    void tickerExists(const std::string& ticker) {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Ticker " << ticker << (tickerSet.count(ticker) ? " exists.\n" : " does NOT exist.\n");
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Query 5 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n\n";
    }

    // 6
    void countDatesWithCloseAbove(double threshold) {
        auto start = std::chrono::high_resolution_clock::now();
        int count = 0;
        for (const auto& kv : dateMap) {
            const auto& dataList = kv.second;
            for (const auto& d : dataList) {
                if (d.close > threshold) {
                    count++;
                    break;
                }
            }
        }
        std::cout << "Number of dates with close > " << threshold << ": " << count << "\n";
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Query 6 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n\n";
    }

    // 7
    void getClosePriceForDate(const std::string& ticker, const std::string& date) {
        auto start = now();
        if (hasTickerDate(ticker, date))
            std::cout << "Close for " << ticker << " on " << date << ": " << tickerDateMap[ticker][date].close << "\n";
        else
            std::cout << "No data found.\n";
        printQueryTime(7, start);
    }

    // 8
    void getAllClosePrices(const std::string& ticker) {
        auto start = now();
        if (auto entries = getTickerData(ticker)) {
            for (const auto& d : *entries)
                std::cout << d.date << ": " << d.close << "\n";
        } else {
            std::cout << "Ticker not found.\n";
        }
        printQueryTime(8, start);
    }

    // 9
    void totalVolume(const std::string& ticker) {
        auto start = now();
        long long sum = 0;
        if (auto entries = getTickerData(ticker)) {
            for (const auto& d : *entries) sum += d.volume;
        }
        std::cout << "Total volume: " << sum << "\n";
        printQueryTime(9, start);
    }

    // 10
    void dataExists(const std::string& ticker, const std::string& date) {
        auto start = now();
        std::cout << (hasTickerDate(ticker, date) ? "Exists\n" : "Not found\n");
        printQueryTime(10, start);
    }

    // 11
    void getOpenClosePrice(const std::string& ticker, const std::string& date) {
        auto start = now();
        if (hasTickerDate(ticker, date)) {
            auto d = tickerDateMap[ticker][date];
            std::cout << "Open: " << d.open << ", Close: " << d.close << "\n";
        } else {
            std::cout << "Data not found.\n";
        }
        printQueryTime(11, start);
    }

    // 12
    void getDividendOnDate(const std::string& ticker, const std::string& date) {
        auto start = now();
        if (hasTickerDate(ticker, date)) {
            auto d = tickerDateMap[ticker][date];
            std::cout << "Dividend: " << d.dividend << "\n";
        } else {
            std::cout << "Data not found.\n";
        }
        printQueryTime(12, start);
    }

    // 13
    void topVolumeStocksOnDate(const std::string& date) {
        auto start = now();
        std::priority_queue<std::pair<long long, std::string>> q;
        if (auto entries = getDateData(date)) {
            for (const auto& d : *entries)
                q.push({d.volume, d.ticker});
            for (int i = 0; i < 10 && !q.empty(); ++i) {
                std::cout << q.top().second << " | " << q.top().first << "\n";
                q.pop();
            }
        } else {
            std::cout << "Date not found.\n";
        }
        printQueryTime(13, start);
    }

    // 14
    void lowestClosingStocks() {
        auto start = std::chrono::high_resolution_clock::now();
        std::priority_queue<std::pair<double, std::string>> maxHeap;
        for (auto& d : allData) {
            maxHeap.push({-d.close, d.ticker});
        }

        std::set<std::string> printed;
        int count = 0;
        while (count < 5 && !maxHeap.empty()) {
            const auto& top = maxHeap.top();
            double negClose = top.first;
            const std::string& ticker = top.second;
            maxHeap.pop();
            if (printed.count(ticker)) continue;
            printed.insert(ticker);
            std::cout << ticker << " | Close: " << -negClose << "\n";
            count++;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Query 14 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n\n";
    }

    // 15
    void topDividendStocks() {
        auto start = std::chrono::high_resolution_clock::now();
        std::set<std::string> printed;
        std::priority_queue<std::pair<double, std::string>> temp = topDividends;
        int count = 0;
        while (count < 5 && !temp.empty()) {
            const auto& top = temp.top();
            double div = top.first;
            const std::string& tick = top.second;
            temp.pop();
            if (printed.count(tick)) continue;
            printed.insert(tick);
            std::cout << tick << " | Dividend: " << div << "\n";
            count++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Query 15 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n\n";
    }

    void insertManualEntry(const std::string& date, const std::string& ticker,
                       double open, double high, double low, double close,
                       long long volume, double dividend) {
        StockData sd = {date, ticker, open, high, low, close, volume, dividend};
        addStockData(sd);
        std::cout << "Manual entry inserted successfully.\n";
    }

};

// ---------------------- MAIN ---------------------
int main() {
    StockDatabase db;
    db.loadCSV("D:\\archive\\all_stock_data.csv", 1000000); // koristi 1M redaka


    db.getDataByDate("1962-01-02");
    db.averageClosePrice("ED");
    db.highestPriceInRange("CVX", "1962-01-01", "1962-12-31");
    db.listAllTickers();
    db.tickerExists("CVX");
    db.tickerExists("XYZ");

    db.countDatesWithCloseAbove(0.25);
    db.getClosePriceForDate("CVX", "1962-01-02");
    db.getAllClosePrices("ED");
    db.totalVolume("CVX");
    db.dataExists("CVX", "1962-01-02");

    db.getOpenClosePrice("CVX", "1962-01-02");
    db.getDividendOnDate("CVX", "1962-01-02");
    db.topVolumeStocksOnDate("1962-01-02");
    db.lowestClosingStocks(); 
    db.topDividendStocks();

    db.insertManualEntry("2025-01-01", "NEW", 100.0, 110.0, 90.0, 105.0, 500000, 2.5);
    db.getDataByDate("2025-01-01");

    return 0;
}
