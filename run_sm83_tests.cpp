// Single-step SM83 test runner
//
// Usage: ./run_sm83_tests path/to/sm83/v1
//
// Requires nlohmann/json.hpp in the same directory (or adjust the include path).
//
// You WILL need to adapt the "load_state" / "check_state" / "flat_read" /
// "flat_write" sections below to match your actual CPU/Bus class APIs -
// the field names here (regs.A, regs.F, bus.write_memory, etc.) are
// guesses based on earlier code you've shown me. Just swap them for
// whatever your real classes expose.

#include "json.hpp" 
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "types.h"
#include "gameboy.h"  


using json = nlohmann::json;
namespace fs = std::filesystem;

struct TestResult {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures; // human readable descriptions, capped
};

// A flat 64KB RAM model, exactly what these tests assume - no MBC,
// no memory-mapped I/O, just plain memory. Point your Bus's read/write
// at this instead of your normal cartridge/IO dispatch WHILE RUNNING
// THESE TESTS ONLY. Easiest way: add a "test_mode" flag to your Bus
// that, when set, reads/writes this flat array directly instead of
// going through get_correct_memory()/IO registers.
static byte flat_ram[65536];

byte flat_read(word addr) {
    return flat_ram[addr];
}

void flat_write(word addr, byte value) {
    flat_ram[addr] = value;
}

// Loads a test's "initial" block into your CPU registers and the flat RAM.
void load_state(CPU& cpu, const json& state) {
     cpu.regs.PC = state["pc"];
     cpu.regs.SP = state["sp"];
     cpu.regs.regs_8b[A]  = state["a"];
     cpu.regs.regs_8b[F]  = state["f"];
     cpu.regs.regs_8b[B]  = state["b"];
     cpu.regs.regs_8b[C]  = state["c"];
     cpu.regs.regs_8b[D]  = state["d"];
     cpu.regs.regs_8b[E]  = state["e"];
     cpu.regs.regs_8b[H]  = state["h"];
     cpu.regs.regs_8b[L]  = state["l"];

    for (auto& mem : state["ram"]) {
        word addr = mem[0].get<int>();
        byte val = mem[1].get<int>();
        flat_write(addr, val);
    }
}

// Compares your CPU's post-instruction state against the test's "final" block.
// Returns an empty string if everything matches, otherwise a description
// of the first mismatch found.
std::string check_state(CPU& cpu, const json& final_state) {
    
    if (cpu.regs.PC != final_state["pc"].get<int>())
        return "PC mismatch: got " + std::to_string(cpu.regs.PC) +
               " expected " + std::to_string(final_state["pc"].get<int>());
    if (cpu.regs.SP != final_state["sp"].get<int>())
        return "SP mismatch: got " + std::to_string(cpu.regs.SP) +
               " expected " + std::to_string(final_state["sp"].get<int>());
    if (cpu.regs.regs_8b[A] != final_state["a"].get<int>())
        return "A mismatch";
    if (cpu.regs.regs_8b[F] != final_state["f"].get<int>())
        return "F mismatch: got " + std::to_string(cpu.regs.regs_8b[F]) +
               " expected " + std::to_string(final_state["f"].get<int>());
    if (cpu.regs.regs_8b[B] != final_state["b"].get<int>()) return "B mismatch";
    if (cpu.regs.regs_8b[C] != final_state["c"].get<int>()) return "C mismatch";
    if (cpu.regs.regs_8b[D] != final_state["d"].get<int>()) return "D mismatch";
    if (cpu.regs.regs_8b[E] != final_state["e"].get<int>()) return "E mismatch";
    if (cpu.regs.regs_8b[H] != final_state["h"].get<int>()) return "H mismatch";
    if (cpu.regs.regs_8b[L] != final_state["l"].get<int>()) return "L mismatch";

    for (auto& mem : final_state["ram"]) {
        word addr = mem[0].get<int>();
        byte expected = mem[1].get<int>();
        byte actual = flat_read(addr);
        if (actual != expected) {
            return "RAM mismatch at " + std::to_string(addr) +
                ": got " + std::to_string((int)actual) +
                " expected " + std::to_string((int)expected);
        }
    }

    return "";
}

TestResult run_one_file(const fs::path& path , CPU& cpu ) {
    TestResult result;

    std::ifstream f(path);
    json tests;
    f >> tests;

    for (auto& test : tests) {
        // reset flat RAM between tests so stale bytes from a previous
        // test don't leak in and mask/create false results
        std::fill(std::begin(flat_ram), std::end(flat_ram), 0);

        load_state(cpu,  test["initial"]);

        byte instruction = cpu.fetch();
        cpu.decode(instruction);

        std::string mismatch = check_state(cpu,  test["final"]);

        if (mismatch.empty()) {
            result.passed++;
        }
        else {
            result.failed++;
            if (result.failures.size() < 3) { // cap how many we print per file
                std::string name = test.value("name", "?");
                result.failures.push_back(name + ": " + mismatch);
            }
        }
    }

    return result;
}

int main1() {

    fs::path test_dir = "C:/Users/paulh/Desktop/emulators/gameboy/roms/sm83/v1";

    Gameboy gb;
    gb.bus.test_ram_override = flat_ram;

    int total_passed = 0, total_failed = 0;
    int files_with_failures = 0;
    int files_processed = 0;


    for (auto& entry : fs::directory_iterator(test_dir)) {
        if (entry.path().extension() != ".json") continue;

        TestResult r = run_one_file(entry.path(), gb.cpu);
        total_passed += r.passed;
        total_failed += r.failed;

        files_processed++;
        std::cout << "[" << files_processed << "] "
            << entry.path().filename().string() << "...\n";
        std::cout.flush();


        if (r.failed > 0) {
            files_with_failures++;
            std::cout << entry.path().filename().string()
                << ": " << r.passed << " passed, " << r.failed << " failed\n";
            for (auto& msg : r.failures) {
                std::cout << "    " << msg << "\n";
            }
        }
    }

    std::cout << "\n=== TOTAL: " << total_passed << " passed, "
        << total_failed << " failed across "
        << files_with_failures << " opcode file(s) with failures ===\n";

    return total_failed > 0 ? 1 : 0;
}